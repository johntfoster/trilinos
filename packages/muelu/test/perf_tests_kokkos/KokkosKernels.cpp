#include <Kokkos_Core.hpp>
#include <Kokkos_ArithTraits.hpp>
#include <Kokkos_CrsMatrix.hpp>
#include <Kokkos_StaticCrsGraph.hpp>
#include <impl/Kokkos_Timer.hpp>

template<class scalar_type, class local_ordinal_type, class device_type>
class LWGraph_kokkos {
private:
  typedef Kokkos::StaticCrsGraph<local_ordinal_type, Kokkos::LayoutLeft, device_type>   local_graph_type;
  typedef Kokkos::View<const local_ordinal_type*, device_type>                          row_type;
  typedef typename local_graph_type::size_type                                          size_type;
  typedef Kokkos::View<bool*, device_type>                                              boundary_nodes_type;

public:
  LWGraph_kokkos(const local_graph_type& graph) : graph_(graph) {
    minLocalIndex_ = 0;
    maxLocalIndex_ = graph_.numRows();
  }
  ~LWGraph_kokkos() { }

  //! Return number of graph vertices
  KOKKOS_INLINE_FUNCTION size_type GetNodeNumVertices() const {
    return graph_.numRows();
  }
  //! Return number of graph edges
  KOKKOS_INLINE_FUNCTION size_type GetNodeNumEdges() const {
    return graph_.row_map(GetNodeNumVertices());
  }

  //! Return the list of vertices adjacent to the vertex 'v'.
  KOKKOS_INLINE_FUNCTION row_type getNeighborVertices(local_ordinal_type i) const;

  //! Return true if vertex with local id 'v' is on current process.
  KOKKOS_INLINE_FUNCTION bool isLocalNeighborVertex(local_ordinal_type i) const {
    return i >= minLocalIndex_ && i <= maxLocalIndex_;
  }

  //! Set boolean array indicating which rows correspond to Dirichlet boundaries.
  KOKKOS_INLINE_FUNCTION void SetBoundaryNodeMap(const boundary_nodes_type bndry) {
    dirichletBoundaries_ = bndry;
  }

  //! Returns the maximum number of entries across all rows/columns on this node
  KOKKOS_INLINE_FUNCTION size_type getNodeMaxNumRowEntries () const {
    return maxNumRowEntries_;
  }

  //! Returns map with global ids of boundary nodes.
  KOKKOS_INLINE_FUNCTION const boundary_nodes_type GetBoundaryNodeMap() const {
    return dirichletBoundaries_;
  }

private:

  //! Underlying graph (with label)
  const local_graph_type      graph_;

  //! Boolean array marking Dirichlet rows.
  boundary_nodes_type         dirichletBoundaries_;

  //! Local index boundaries (cached from domain map)
  local_ordinal_type    minLocalIndex_, maxLocalIndex_;
  size_type             maxNumRowEntries_;
};

template<class scalar_type, class local_ordinal_type, class device_type>
Kokkos::CrsMatrix<scalar_type, local_ordinal_type, device_type>
kernel_construct(local_ordinal_type numRows) {
  local_ordinal_type numCols         = numRows;
  local_ordinal_type nnz             = 10*numRows;

  local_ordinal_type varianz_nel_row = 0.2*nnz/numRows;
  local_ordinal_type width_row       = 0.01*numRows;

  local_ordinal_type elements_per_row = nnz/numRows;

  local_ordinal_type *rowPtr = new local_ordinal_type[numRows+1];
  rowPtr[0] = 0;
  for (int row = 0; row < numRows; row++) {
    int varianz = (1.0*rand()/INT_MAX-0.5)*varianz_nel_row;

    rowPtr[row+1] = rowPtr[row] + elements_per_row + varianz;
  }
  nnz = rowPtr[numRows];

  local_ordinal_type *colInd = new local_ordinal_type[nnz];
  scalar_type        *values = new scalar_type       [nnz];
  for (int row = 0; row < numRows; row++) {
    for (int k = rowPtr[row]; k < rowPtr[row+1]; k++) {
      int pos = row + (1.0*rand()/INT_MAX-0.5)*width_row;

      if (pos <  0)       pos += numCols;
      if (pos >= numCols) pos -= numCols;
      colInd[k] = pos;
      values[k] = 100.0*rand()/INT_MAX - 50.0;
    }
  }

  typedef Kokkos::CrsMatrix<scalar_type, local_ordinal_type, device_type> local_matrix_type;

  return local_matrix_type("A", numRows, numCols, nnz, values, rowPtr, colInd, false/*pad*/);
}

template<class scalar_type, class local_ordinal_type, class device_type>
void kernel_coalesce_drop_device(Kokkos::CrsMatrix<scalar_type, local_ordinal_type, device_type> A) {
  typedef Kokkos::CrsMatrix<scalar_type, local_ordinal_type, device_type>   local_matrix_type;
  typedef Kokkos::ArithTraits<scalar_type>                                  ATS;
  typedef typename ATS::mag_type                                            magnitude_type;
  typedef Kokkos::View<bool*, device_type>                                  boundary_nodes_type;

  auto numRows = A.numRows();

  scalar_type eps = 0.05;

  typedef Kokkos::RangePolicy<typename local_matrix_type::execution_space> RangePolicy;

  // Stage 0: detect Dirichlet rows
  boundary_nodes_type boundaryNodes("boundaryNodes", numRows);
  Kokkos::parallel_for("MueLu:Utils::DetectDirichletRows", RangePolicy(0, numRows),
    KOKKOS_LAMBDA(const local_ordinal_type row) {
      auto rowView = A.row (row);
      auto length  = rowView.length;

      boundaryNodes(row) = true;
      for (decltype(length) colID = 0; colID < length; colID++)
        if ((rowView.colidx(colID) != row) && (ATS::magnitude(rowView.value(colID)) > 1e-13)) {
          boundaryNodes(row) = false;
          break;
        }
    });

  // Stage 1: calculate the number of remaining entries per row
  typedef Kokkos::StaticCrsGraph<local_ordinal_type, Kokkos::LayoutLeft, device_type> local_graph_type;
  typedef typename local_graph_type::row_map_type row_map_type;
  typedef typename local_graph_type::entries_type entries_type;

  typename entries_type::non_const_type diag("ghosted", numRows);
  typename row_map_type::non_const_type rows("row_map", numRows+1);       // rows(0) = 0 automatically

  local_ordinal_type realnnz = 0;
  Kokkos::parallel_reduce("kernel_cd:stage1_reduce", RangePolicy(0, numRows),
    KOKKOS_LAMBDA(const local_ordinal_type row, local_ordinal_type& nnz) {
      auto rowView = A.row (row);
      auto length  = rowView.length;

      local_ordinal_type rownnz = 0;
      for (decltype(length) colID = 0; colID < length; colID++) {
        local_ordinal_type col = rowView.colidx(colID);

        // Avoid square root by using squared values
        magnitude_type aiiajj = eps*eps * ATS::magnitude(diag(row))*ATS::magnitude(diag(col));                  // eps^2*|a_ii|*|a_jj|
        magnitude_type aij2   = ATS::magnitude(rowView.value(colID)) * ATS::magnitude(rowView.value(colID));    // |a_ij|^2

        if (aij2 > aiiajj || row == col)
          rownnz++;
      }
      rows(row+1) = rownnz;
      nnz += rownnz;
    }, realnnz);

  // parallel_scan (exclusive)
  Kokkos::parallel_scan("kernel_cd:stage1_scan", RangePolicy(0, numRows+1),
    KOKKOS_LAMBDA(const local_ordinal_type i, local_ordinal_type& upd, const bool& final_result) {
      upd += rows(i);
      if (final_result)
        rows(i) = upd;
    });

  // Stage 2: fill in the column indices
  typename boundary_nodes_type::non_const_type bndNodes("boundaryNodes", numRows);
  typename entries_type::non_const_type        cols    ("entries",       realnnz);
  local_ordinal_type numDropped = 0;
  Kokkos::parallel_reduce("kernel_cd:stage2_reduce", RangePolicy(0, numRows),
    KOKKOS_LAMBDA(const local_ordinal_type row, local_ordinal_type& dropped) {
      auto rowView = A.row (row);
      auto length = rowView.length;

      local_ordinal_type rownnz = 0;
      for (decltype(length) colID = 0; colID < length; colID++) {
        local_ordinal_type col = rowView.colidx(colID);

        // Avoid square root by using squared values
        magnitude_type aiiajj = eps*eps * ATS::magnitude(diag(row))*ATS::magnitude(diag(col));                  // eps^2*|a_ii|*|a_jj|
        magnitude_type aij2   = ATS::magnitude(rowView.value(colID)) * ATS::magnitude(rowView.value(colID));    // |a_ij|^2

        if (aij2 > aiiajj || row == col) {
          cols(rows(row) + rownnz) = col;
          rownnz++;
        } else {
          dropped++;
        }
        if (rownnz == 1) {
          // If the only element remaining after filtering is diagonal, mark node as boundary
          // FIXME: this should really be replaced by the following
          //    if (indices.size() == 1 && indices[0] == row)
          //        boundaryNodes[row] = true;
          // We do not do it this way now because there is no framework for distinguishing isolated
          // and boundary nodes in the aggregation algorithms
          bndNodes(row) = true;
        }
      }
    }, numDropped);
  boundaryNodes = bndNodes;

  local_graph_type kokkosGraph(cols, rows);

  LWGraph_kokkos<scalar_type, local_ordinal_type, device_type> graph(kokkosGraph);

  graph.SetBoundaryNodeMap(boundaryNodes);
}

template<class scalar_type, class local_ordinal_type, class device_type>
void kernel_coalesce_drop_serial(Kokkos::CrsMatrix<scalar_type, local_ordinal_type, device_type> A) {
  typedef Kokkos::CrsMatrix<scalar_type, local_ordinal_type, device_type>   local_matrix_type;
  typedef Kokkos::ArithTraits<scalar_type>                                  ATS;
  typedef typename ATS::mag_type                                            magnitude_type;
  typedef Kokkos::View<bool*, device_type>                                  boundary_nodes_type;

  auto numRows = A.numRows();

  scalar_type eps = 0.05;

  typedef Kokkos::RangePolicy<typename local_matrix_type::execution_space> RangePolicy;

  // Stage 0: detect Dirichlet rows
  boundary_nodes_type boundaryNodes("boundaryNodes", numRows);
  for (int row = 0; row < numRows; row++) {
    auto rowView = A.row (row);
    auto length  = rowView.length;

    boundaryNodes(row) = true;
    for (decltype(length) colID = 0; colID < length; colID++)
      if ((rowView.colidx(colID) != row) && (ATS::magnitude(rowView.value(colID)) > 1e-13)) {
        boundaryNodes(row) = false;
        break;
      }
  }

  // Stage 1: calculate the number of remaining entries per row
  typedef Kokkos::StaticCrsGraph<local_ordinal_type, Kokkos::LayoutLeft, device_type> local_graph_type;
  typedef typename local_graph_type::row_map_type row_map_type;
  typedef typename local_graph_type::entries_type entries_type;

  typename entries_type::non_const_type diag("ghosted", numRows);
  typename row_map_type::non_const_type rows("row_map", numRows+1);       // rows(0) = 0 automatically
  typename entries_type::non_const_type cols("entries", 20*numRows);      // estimate

  typename boundary_nodes_type::non_const_type bndNodes("boundaryNodes", numRows);
  local_ordinal_type numDropped = 0;

  local_ordinal_type realnnz = 0;
  for (int row = 0; row < numRows; row++) {
    auto rowView = A.row (row);
    auto length  = rowView.length;

    local_ordinal_type rownnz = 0;
    for (decltype(length) colID = 0; colID < length; colID++) {
      local_ordinal_type col = rowView.colidx(colID);

      // Avoid square root by using squared values
      magnitude_type aiiajj = eps*eps * ATS::magnitude(diag(row))*ATS::magnitude(diag(col));                  // eps^2*|a_ii|*|a_jj|
      magnitude_type aij2   = ATS::magnitude(rowView.value(colID)) * ATS::magnitude(rowView.value(colID));    // |a_ij|^2

      if (aij2 > aiiajj || row == col) {
        cols(rownnz++) = col;

      } else {
        numDropped++;
      }

      if (rownnz == 1) {
        // If the only element remaining after filtering is diagonal, mark node as boundary
        // FIXME: this should really be replaced by the following
        //    if (indices.size() == 1 && indices[0] == row)
        //        boundaryNodes[row] = true;
        // We do not do it this way now because there is no framework for distinguishing isolated
        // and boundary nodes in the aggregation algorithms
        bndNodes(row) = true;
      }
    }
    rows(row+1) = rows(row) + rownnz;
    realnnz += rownnz;
  }
  boundaryNodes = bndNodes;

  local_graph_type kokkosGraph(cols, rows);

  LWGraph_kokkos<scalar_type, local_ordinal_type, device_type> graph(kokkosGraph);

  graph.SetBoundaryNodeMap(boundaryNodes);
}

template<class scalar_type, class local_ordinal_type, class device_type>
int main_(int argc, char **argv) {
  int n    = 100000;
  int loop = 10;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-n") == 0) { n    = atoi(argv[++i]); continue; }
    if (strcmp(argv[i], "-l") == 0) { loop = atoi(argv[++i]); continue; }
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      std::cout << "./MueLu_KokkosKernels.exe [-n <matrix_size>] [-l <number_of_loops>]" << std::endl;
      return 0;
    }
  }

  typedef Kokkos::CrsMatrix<scalar_type, local_ordinal_type, device_type> local_matrix_type;
  typedef typename device_type::execution_space execution_space;

  local_matrix_type A = kernel_construct<scalar_type, local_ordinal_type, device_type>(n);

  execution_space::fence();
  Kokkos::Impl::Timer timer;

  if (typeid(device_type) == typeid(Kokkos::Serial)) {
    for (int i = 0; i < loop; i++)
      kernel_coalesce_drop_serial(A);

  } else {
    for (int i = 0; i < loop; i++)
      kernel_coalesce_drop_device(A);
  }

  double kernel_time = timer.seconds();

  execution_space::fence();

  printf("kernel_coalesce_drop: %.2e (s)\n", kernel_time / loop);

  execution_space::finalize();

  return 0;
}

int main(int argc, char* argv[]) {

  Kokkos::initialize(argc, argv);

  srand(13721);

  std::string node;
  for (int i = 1; i < argc; i++) {
    if (strncmp(argv[i], "--node=", 7) == 0) {
      std::cout << "Use --node <node> instead of --node=<node>" << std::endl;
      Kokkos::finalize();
      return 0;

    } else if (strncmp(argv[i], "--node", 6) == 0)
      node = argv[++i];
  }

  std::cout << "node = " << node << std::endl;

  if (node == "") {
    return main_<double,int,Kokkos::DefaultExecutionSpace>(argc, argv);

  } else if (node == "serial") {
#ifdef KOKKOS_HAVE_SERIAL
    return main_<double,int,Kokkos::Serial>(argc, argv);
#else
    throw std::runtime_error("Serial node type is disabled");
#endif

  } else if (node == "openmp") {
#ifdef KOKKOS_HAVE_OPENMP
    return main_<double,int,Kokkos::OpenMP>(argc, argv);
#else
    throw std::runtime_error("OpenMP node type is disabled");
#endif

  } else if (node == "cuda") {
#ifdef KOKKOS_HAVE_CUDA
    return main_<double,int,Kokkos::CUDA>(argc, argv);
#else
    throw std::runtime_error("CUDA node type is disabled");
#endif
  }

  Kokkos::finalize();

  return 0;
}
