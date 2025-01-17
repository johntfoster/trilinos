// @HEADER
//
// ***********************************************************************
//
//             Xpetra: A linear algebra interface package
//                  Copyright 2012 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact
//                    Jonathan Hu       (jhu@sandia.gov)
//                    Andrey Prokopenko (aprokop@sandia.gov)
//                    Tobias Wiesner    (tawiesn@sandia.gov)
//                    Ray Tuminaro      (rstumin@sandia.gov)
//
// ***********************************************************************
//
// @HEADER
#ifndef XPETRA_BLOCKEDMULTIVECTOR_HPP
#define XPETRA_BLOCKEDMULTIVECTOR_HPP

/* this file is automatically generated - do not edit (see script/interfaces.py) */

#include "Xpetra_ConfigDefs.hpp"
#include "Xpetra_Map.hpp"
#include "Xpetra_MultiVector.hpp"
#include "Xpetra_MapExtractor.hpp"


namespace Xpetra {

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  // forward declaration of Vector, needed to prevent circular inclusions
  template<class S, class LO, class GO, class N> class Vector;
#endif

  template <class Scalar = double,
            class LocalOrdinal = Map<>::local_ordinal_type,
            class GlobalOrdinal = typename Map<LocalOrdinal>::global_ordinal_type,
            class Node = typename Map<LocalOrdinal, GlobalOrdinal>::node_type>
  class BlockedMultiVector
    : public MultiVector< Scalar, LocalOrdinal, GlobalOrdinal, Node >
  {
  public:
    typedef Scalar scalar_type;
    typedef LocalOrdinal local_ordinal_type;
    typedef GlobalOrdinal global_ordinal_type;
    typedef Node node_type;

  private:
#undef XPETRA_BLOCKEDMULTIVECTOR_SHORT
#include "Xpetra_UseShortNames.hpp"

  public:
    //! @name Constructor/Destructor Methods
    //@{

    //! Constructor
    /*!
     * Const version of constructor which accepts a const version
     * of the multi-vector
     *
     * \note If you change the information in input vector v the data in the
     *       blocked multi-vector are not affected (and vice versa). Consider
     *       the blocked multivector to be a copy of the input multivector (not a view)
     *
     * \param mapExtractor MapExtractor object containing information about the block splitting
     * \param v MultiVector that is to be splitted into a blocked multi vector
     */
    BlockedMultiVector(Teuchos::RCP<const MapExtractor> mapExtractor,
                     Teuchos::RCP<const MultiVector> v)
    : mapextractor_(mapExtractor)
    {
      bThyraMode_  = mapExtractor->getThyraMode();

      vv_.reserve(mapExtractor->NumMaps());

      // add CrsMatrix objects in row,column order
      for (size_t r = 0; r < mapExtractor->NumMaps(); ++r)
        vv_.push_back(mapExtractor->ExtractVector(v, r, bThyraMode_));

      numVectors_ = v->getNumVectors();
    }

    /*!
     * Non-const version of constructor which accepts a non-const version
     * of the multi-vector
     *
     * \note If you change the information in input vector v the data in the
     *       blocked multi-vector are not affected (and vice versa). Consider
     *       the blocked multivector to be a copy of the input multivector (not a view)
     *
     * \param mapExtractor MapExtractor object containing information about the block splitting
     * \param v MultiVector that is to be splitted into a blocked multi vector
     */
    BlockedMultiVector(Teuchos::RCP<const MapExtractor> mapExtractor,
                     Teuchos::RCP<MultiVector> v)
    : mapextractor_(mapExtractor)
    {
      bThyraMode_  = mapExtractor->getThyraMode();

      vv_.reserve(mapExtractor->NumMaps());

      // add CrsMatrix objects in row,column order
      for (size_t r = 0; r < mapExtractor->NumMaps(); ++r)
        vv_.push_back(mapExtractor->ExtractVector(v, r, bThyraMode_));

      numVectors_ = v->getNumVectors();
    }

    //! Destructor.
    virtual ~BlockedMultiVector() { }

    /// \brief Assignment operator: Does a deep copy.
    ///
    /// The assignment operator does a deep copy, just like
    /// subclasses' copy constructors.
    ///
    /// \note This currently only works if both <tt>*this</tt> and the
    ///   input argument are instances of the same subclass.
    BlockedMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>&
    operator= (const MultiVector& rhs) {
      assign (rhs); // dispatch to protected virtual method
      return *this;
    }

    //@}
    //! @name Post-construction modification routines
    //@{

    //! Replace value, using global (row) index.
    virtual void replaceGlobalValue(GlobalOrdinal globalRow, size_t vectorIndex, const Scalar &value) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::replaceGlobalValue: Not (yet) supported by BlockedMultiVector.");
    }

    //! Add value to existing value, using global (row) index.
    virtual void sumIntoGlobalValue(GlobalOrdinal globalRow, size_t vectorIndex, const Scalar &value) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::sumIntoGlobalValue: Not (yet) supported by BlockedMultiVector.");
    }

    //! Replace value, using local (row) index.
    virtual void replaceLocalValue(LocalOrdinal myRow, size_t vectorIndex, const Scalar &value) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::replaceLocalValue: Not supported by BlockedMultiVector.");
    }

    //! Add value to existing value, using local (row) index.
    virtual void sumIntoLocalValue(LocalOrdinal myRow, size_t vectorIndex, const Scalar &value) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::sumIntoLocalValue:Not (yet) supported by BlockedMultiVector.");
    }

    //! Set all values in the multivector with the given value.
    virtual void putScalar(const Scalar &value) {
      for(size_t r = 0; r < getMapExtractor()->NumMaps(); r++) {
        getMultiVector(r)->putScalar(value);
      }
    }

    //@}

    //! @name Data Copy and View get methods
    //@{

    //! Return a Vector which is a const view of column j.
    virtual Teuchos::RCP< const Vector > getVector(size_t j) const {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::getVector: Not (yet) supported by BlockedMultiVector.");
      return Teuchos::null;
    }

    //! Return a Vector which is a nonconst view of column j.
    virtual Teuchos::RCP< Vector> getVectorNonConst(size_t j) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::getVectorNonConst: Not (yet) supported by BlockedMultiVector.");
      return Teuchos::null;
    }

    //! Const view of the local values in a particular vector of this multivector.
    virtual Teuchos::ArrayRCP< const Scalar > getData(size_t j) const {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::getData: Not (yet) supported by BlockedMultiVector.");
      return Teuchos::null;
    }

    //! View of the local values in a particular vector of this multivector.
    virtual Teuchos::ArrayRCP< Scalar > getDataNonConst(size_t j) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::getDataNonConst: Not (yet) supported by BlockedMultiVector.");
      return Teuchos::null;
    }

    //@}

    //! @name Mathematical methods
    //@{

    //! Compute dot product of each corresponding pair of vectors, dots[i] = this[i].dot(A[i]).
    virtual void dot(const MultiVector&A, const Teuchos::ArrayView< Scalar > &dots) const {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::dot: Not (yet) supported by BlockedMultiVector.");
    }

    //! Put element-wise absolute values of input Multi-vector in target: A = abs(this).
    virtual void abs(const MultiVector&A) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::abs: Not (yet) supported by BlockedMultiVector.");
    }

    //! Put element-wise reciprocal values of input Multi-vector in target, this(i,j) = 1/A(i,j).
    virtual void reciprocal(const MultiVector&A) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::reciprocal: Not (yet) supported by BlockedMultiVector.");
    }

    //! Scale the current values of a multi-vector, this = alpha*this.
    virtual void scale(const Scalar &alpha) {
      for (size_t r = 0; r < getMapExtractor()->NumMaps(); ++r) {
        if(getMultiVector(r)!=Teuchos::null) {
          getMultiVector(r)->scale(alpha);
        }
      }
    }

    //! Scale the current values of a multi-vector, this[j] = alpha[j]*this[j].
    virtual void scale (Teuchos::ArrayView< const Scalar > alpha) {
      for (size_t r = 0; r < getMapExtractor()->NumMaps(); ++r) {
        if(getMultiVector(r)!=Teuchos::null) {
          getMultiVector(r)->scale(alpha);
        }
      }
    }

    //! Update multi-vector values with scaled values of A, this = beta*this + alpha*A.
    virtual void update(const Scalar &alpha, const MultiVector&A, const Scalar &beta) {
      Teuchos::RCP<const MultiVector> rcpA = Teuchos::rcpFromRef(A);
      Teuchos::RCP<const BlockedMultiVector> bA = Teuchos::rcp_dynamic_cast<const BlockedMultiVector>(rcpA);
      TEUCHOS_TEST_FOR_EXCEPTION(numVectors_ != rcpA->getNumVectors(),Xpetra::Exceptions::RuntimeError,"BlockedMultiVector::update: update with incompatible vector (different number of vectors in multivector).");
      if(bA != Teuchos::null) {
        TEUCHOS_TEST_FOR_EXCEPTION(bThyraMode_ != bA->getMapExtractor()->getThyraMode(), Xpetra::Exceptions::RuntimeError, "BlockedMultiVector::update: update with incompatible vector (different thyra mode).");
        TEUCHOS_TEST_FOR_EXCEPTION(getMapExtractor()->NumMaps() != bA->getMapExtractor()->NumMaps(), Xpetra::Exceptions::RuntimeError, "BlockedMultiVector::update: update with incompatible vector (different number of partial vectors).");
        for(size_t r = 0; r < getMapExtractor()->NumMaps(); r++) {
          XPETRA_TEST_FOR_EXCEPTION(getMultiVector(r)->getMap()->isSameAs(*(bA->getMultiVector(r)->getMap()))==false, Xpetra::Exceptions::RuntimeError, "BlockedMultiVector::update: update with incompatible vector (different maps in partial vector " << r << ").");
          getMultiVector(r)->update(alpha, *(bA->getMultiVector(r)), beta);
        }
      } else {
        XPETRA_TEST_FOR_EXCEPTION(getMapExtractor()->getFullMap()->isSameAs(*(rcpA->getMap()))==false, Xpetra::Exceptions::RuntimeError, "BlockedMultiVector::update: update with incompatible vector (maps of full vector do not match with map in MapExtractor).");
        for(size_t r = 0; r < getMapExtractor()->NumMaps(); r++) {
          Teuchos::RCP<const MultiVector> part = getMapExtractor()->ExtractVector(rcpA, r, bThyraMode_);
          getMultiVector(r)->update(alpha, *part, beta);
        }
      }
    }

    //! Update multi-vector with scaled values of A and B, this = gamma*this + alpha*A + beta*B.
    virtual void update(const Scalar &alpha, const MultiVector&A, const Scalar &beta, const MultiVector&B, const Scalar &gamma) {
      Teuchos::RCP<const MultiVector> rcpA = Teuchos::rcpFromRef(A);
      Teuchos::RCP<const MultiVector> rcpB = Teuchos::rcpFromRef(B);
      Teuchos::RCP<const BlockedMultiVector> bA = Teuchos::rcp_dynamic_cast<const BlockedMultiVector>(rcpA);
      Teuchos::RCP<const BlockedMultiVector> bB = Teuchos::rcp_dynamic_cast<const BlockedMultiVector>(rcpB);
      if(bA != Teuchos::null && bB != Teuchos::null) {
        TEUCHOS_TEST_FOR_EXCEPTION(bThyraMode_ != bA->getMapExtractor()->getThyraMode(), Xpetra::Exceptions::RuntimeError, "BlockedMultiVector::update: update with incompatible vector (different thyra mode in vector A).");
        TEUCHOS_TEST_FOR_EXCEPTION(getMapExtractor()->NumMaps() != bA->getMapExtractor()->NumMaps(), Xpetra::Exceptions::RuntimeError, "BlockedMultiVector::update: update with incompatible vector (different number of partial vectors in vector A).");
        TEUCHOS_TEST_FOR_EXCEPTION(numVectors_ != bA->getNumVectors(),Xpetra::Exceptions::RuntimeError,"BlockedMultiVector::update: update with incompatible vector (different number of vectors in multivector in vector A).");
        TEUCHOS_TEST_FOR_EXCEPTION(bThyraMode_ != bB->getMapExtractor()->getThyraMode(), Xpetra::Exceptions::RuntimeError, "BlockedMultiVector::update: update with incompatible vector (different thyra mode in vector B).");
        TEUCHOS_TEST_FOR_EXCEPTION(getMapExtractor()->NumMaps() != bB->getMapExtractor()->NumMaps(), Xpetra::Exceptions::RuntimeError, "BlockedMultiVector::update: update with incompatible vector (different number of partial vectors in vector B).");
        TEUCHOS_TEST_FOR_EXCEPTION(numVectors_ != bB->getNumVectors(),Xpetra::Exceptions::RuntimeError,"BlockedMultiVector::update: update with incompatible vector (different number of vectors in multivector in vector B).");

        for(size_t r = 0; r < getMapExtractor()->NumMaps(); r++) {
          XPETRA_TEST_FOR_EXCEPTION(getMultiVector(r)->getMap()->isSameAs(*(bA->getMultiVector(r)->getMap()))==false, Xpetra::Exceptions::RuntimeError, "BlockedMultiVector::update: update with incompatible vector (different maps in partial vector " << r << ").");
          getMultiVector(r)->update(alpha, *(bA->getMultiVector(r)), beta, *(bB->getMultiVector(r)), gamma);
        }
        return;
      }
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::update: only supports update with other BlockedMultiVector.");
    }

    //! Compute 1-norm of each vector in multi-vector.
    virtual void norm1(const Teuchos::ArrayView< typename Teuchos::ScalarTraits< Scalar >::magnitudeType > &norms) const {
      typedef typename ScalarTraits<Scalar>::magnitudeType Magnitude;
      Array<Magnitude> temp_norms(getNumVectors());
      std::fill(temp_norms.begin(),temp_norms.end(),ScalarTraits<Magnitude>::zero());
      for (size_t r = 0; r < getMapExtractor()->NumMaps(); ++r) {
        if(getMultiVector(r)!=Teuchos::null) {
          getMultiVector(r)->norm1(temp_norms);
          for (size_t c = 0; c < getNumVectors(); ++c)
            norms[c] += temp_norms[c];
        }
      }
    }

    //!
    virtual void norm2(const Teuchos::ArrayView< typename Teuchos::ScalarTraits< Scalar >::magnitudeType > &norms) const {
      typedef typename ScalarTraits<Scalar>::magnitudeType Magnitude;
      Array<Magnitude> results(getNumVectors());
      Array<Magnitude> temp_norms(getNumVectors());
      std::fill(results.begin(),results.end(),ScalarTraits<Magnitude>::zero());
      std::fill(temp_norms.begin(),temp_norms.end(),ScalarTraits<Magnitude>::zero());
      for (size_t r = 0; r < getMapExtractor()->NumMaps(); ++r) {
        if(getMultiVector(r)!=Teuchos::null) {
          getMultiVector(r)->norm2(temp_norms);
          for (size_t c = 0; c < getNumVectors(); ++c)
            results[c] += temp_norms[c] * temp_norms[c];
        }
      }
      for (size_t c = 0; c < getNumVectors(); ++c)
        norms[c] = Teuchos::ScalarTraits<Magnitude >::squareroot(results[c]);
    }

    //! Compute Inf-norm of each vector in multi-vector.
    virtual void normInf(const Teuchos::ArrayView< typename Teuchos::ScalarTraits< Scalar >::magnitudeType > &norms) const {
      typedef typename ScalarTraits<Scalar>::magnitudeType Magnitude;
      Array<Magnitude> temp_norms(getNumVectors());
      std::fill(temp_norms.begin(),temp_norms.end(),ScalarTraits<Magnitude>::zero());
      for (size_t r = 0; r < getMapExtractor()->NumMaps(); ++r) {
        if(getMultiVector(r)!=Teuchos::null) {
          getMultiVector(r)->normInf(temp_norms);
          for (size_t c = 0; c < getNumVectors(); ++c)
            norms[c] = std::max(norms[c],temp_norms[c]);
        }
      }
    }

    //! Compute mean (average) value of each vector in multi-vector. The outcome of this routine is undefined for non-floating point scalar types (e.g., int).
    virtual void meanValue(const Teuchos::ArrayView< Scalar > &means) const {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::meanValue: Not (yet) supported by BlockedMultiVector.");
    }

    //! Matrix-matrix multiplication: this = beta*this + alpha*op(A)*op(B).
    virtual void multiply(Teuchos::ETransp transA, Teuchos::ETransp transB, const Scalar &alpha, const MultiVector&A, const MultiVector&B, const Scalar &beta) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::multiply: Not (yet) supported by BlockedMultiVector.");
    }

    //! Element-wise multiply of a Vector A with a MultiVector B.
    virtual void elementWiseMultiply(Scalar scalarAB, const Vector&A, const MultiVector&B, Scalar scalarThis) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::elementWiseMultiply: Not (yet) supported by BlockedMultiVector.");
    }

    //@}

    //! @name Attribute access functions
    //@{

    //! Number of columns in the multivector.
    virtual size_t getNumVectors() const {
      return numVectors_;
    }

    //! Local number of rows on the calling process.
    virtual size_t getLocalLength() const {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::getLocalLength: routine not implemented. It has no value as one must iterate on the partial vectors.");
      return 0;
    }

    //! Global number of rows in the multivector.
    virtual global_size_t getGlobalLength() const {
      return getMapExtractor()->getFullMap()->getGlobalNumElements();
    }

    //@}

    //! @name Overridden from Teuchos::Describable
    //@{

    //! A simple one-line description of this object.
    virtual std::string description() const {
      return std::string("BlockedMultiVector");
    }

    //! Print the object with the given verbosity level to a FancyOStream.
    virtual void describe(Teuchos::FancyOStream &out, const Teuchos::EVerbosityLevel verbLevel=Teuchos::Describable::verbLevel_default) const {
      out << "BlockedMultiVector: " << std::endl;
      for(size_t r = 0; r < getMapExtractor()->NumMaps(); r++)
        getMultiVector(r)->describe(out, verbLevel);
    }

    virtual void replaceMap(const RCP<const Map>& map) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::replaceMap: Not supported by BlockedMultiVector.");
    }

    //! Import.
    virtual void doImport(const DistObject<Scalar, LocalOrdinal, GlobalOrdinal, Node> &source, const Import&importer, CombineMode CM) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::doImport: Not supported by BlockedMultiVector.");
    }

    //! Export.
    virtual void doExport(const DistObject<Scalar, LocalOrdinal, GlobalOrdinal, Node> &dest, const Import& importer, CombineMode CM) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::doExport: Not supported by BlockedMultiVector.");
    }

    //! Import (using an Exporter).
    virtual void doImport(const DistObject<Scalar, LocalOrdinal, GlobalOrdinal, Node> &source, const Export& exporter, CombineMode CM) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::doImport: Not supported by BlockedMultiVector.");
    }

    //! Export (using an Importer).
    virtual void doExport(const DistObject<Scalar, LocalOrdinal, GlobalOrdinal, Node> &dest, const Export& exporter, CombineMode CM) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::doExport: Not supported by BlockedMultiVector.");
    }

    //@}

    //! @name Xpetra specific
    //@{

    //! Set seed for Random function.
    virtual void setSeed(unsigned int seed) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::seed: Not (yet) supported by BlockedMultiVector.");
    }


    virtual void randomize(bool bUseXpetraImplementation = false) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::randomize: Not (yet) supported by BlockedMultiVector.");
    }

    //! Set multi-vector values to random numbers. XPetra implementation
    virtual void Xpetra_randomize()
    {
      Xpetra::MultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>::Xpetra_randomize();
    }

#ifdef HAVE_XPETRA_KOKKOS_REFACTOR
    typedef typename Kokkos::Details::ArithTraits<Scalar>::val_type impl_scalar_type;
    typedef Kokkos::DualView<impl_scalar_type**, Kokkos::LayoutStride,
        typename node_type::execution_space,
        Kokkos::MemoryUnmanaged> dual_view_type;
    typedef typename dual_view_type::host_mirror_space host_execution_space;
    typedef typename dual_view_type::t_dev::execution_space dev_execution_space;

    /// \brief Return an unmanaged non-const view of the local data on a specific device.
    /// \tparam TargetDeviceType The Kokkos Device type whose data to return.
    ///
    /// \warning Be aware that the view on the multivector data is non-persisting, i.e.
    ///          only valid as long as the multivector does not run of scope!
    template<class TargetDeviceType>
    typename Kokkos::Impl::if_c<
      Kokkos::Impl::is_same<
        typename dev_execution_space::memory_space,
        typename TargetDeviceType::memory_space>::value,
        typename dual_view_type::t_dev_um,
        typename dual_view_type::t_host_um>::type
    getLocalView () const {
      if(Kokkos::Impl::is_same<
                   typename host_execution_space::memory_space,
                   typename TargetDeviceType::memory_space
         >::value) {
        return getHostLocalView();
      } else {
        return getDeviceLocalView();
      }
    }

    virtual typename dual_view_type::t_host_um getHostLocalView ()  const {
      typename dual_view_type::t_host_um test;
      return test;
    }
    virtual typename dual_view_type::t_dev_um  getDeviceLocalView() const {
      typename dual_view_type::t_dev_um test;
      return test;
    }

#endif

    //@}

    //! Access function for the underlying Map this DistObject was constructed with.
    Teuchos::RCP< const Map> getMap() const { XPETRA_MONITOR("BlockedMultiVector::getMap"); return getMapExtractor()->getFullMap(); }

    /// return partial multivector associated with block row r
    Teuchos::RCP<MultiVector> getMultiVector(size_t r) const {
      XPETRA_MONITOR("BlockedMultiVector::getMultiVector(r)");
      TEUCHOS_TEST_FOR_EXCEPTION(r > getMapExtractor()->NumMaps(), std::out_of_range, "Error, r = " << r << " is too big. The BlockedMultiVector only contains " << getMapExtractor()->NumMaps() << " partial blocks.");
      return vv_[r];
    }

    /// return partial multivector associated with block row r
    Teuchos::RCP<MultiVector> getMultiVector(size_t r, bool bThyraMode) const {
      XPETRA_MONITOR("BlockedMultiVector::getMultiVector(r,bThyraMode)");
      TEUCHOS_TEST_FOR_EXCEPTION(r > getMapExtractor()->NumMaps(), std::out_of_range, "Error, r = " << r << " is too big. The BlockedMultiVector only contains " << getMapExtractor()->NumMaps() << " partial blocks.");
      Teuchos::RCP<MultiVector> ret = vv_[r];
      if(bThyraMode_ != bThyraMode) {
        size_t localLength = ret->getLocalLength();
        size_t numVecs     = vv_[r]->getNumVectors();
        // standard case: bThyraMode_ == true but bThyraMode == false
        ret = getMapExtractor()->getVector(r,numVecs,bThyraMode);
        for(size_t k=0; k < numVecs; k++) {
          Teuchos::ArrayRCP<const Scalar> srcVecData  = vv_[r]->getData(k);
          Teuchos::ArrayRCP<Scalar> targetVecData = ret->getDataNonConst(k);
          for(size_t i=0; i < localLength; i++) {
             targetVecData[i] = srcVecData[i];
          }
        }
      }
      return ret;
    }

    /// set partial multivector associated with block row r
    void setMultiVector(size_t r, Teuchos::RCP<const MultiVector> v, bool bThyraMode) {
      XPETRA_MONITOR("BlockedMultiVector::setMultiVector");
      Teuchos::RCP<const MapExtractor> me = getMapExtractor();
      TEUCHOS_TEST_FOR_EXCEPTION(r >= me->NumMaps(), std::out_of_range, "Error, r = " << r << " is too big. The BlockedMultiVector only contains " << getMapExtractor()->NumMaps() << " partial blocks.");
      //TEUCHOS_TEST_FOR_EXCEPTION(getMapExtractor()->getMap(r,bThyraMode_)->isSameAs(*(v->getMap()))==false, Xpetra::Exceptions::RuntimeError, "Map of provided partial map and map extractor are not compatible. The size of the provided map is " << v->getMap()->getGlobalNumElements() << " and the expected size is " << getMapExtractor()->getMap(r,bThyraMode_)->getGlobalNumElements() << " or the GIDs are not correct (Thyra versus non-Thyra?)");
      TEUCHOS_TEST_FOR_EXCEPTION(numVectors_ != v->getNumVectors(),Xpetra::Exceptions::RuntimeError,"The BlockedMultiVectors expects " << getNumVectors() << " vectors. The provided partial multivector has " << v->getNumVectors() << " vectors.");
      Teuchos::RCP<MultiVector> vv = Teuchos::rcp_const_cast<MultiVector>(v);
      TEUCHOS_TEST_FOR_EXCEPTION(vv==Teuchos::null, Xpetra::Exceptions::RuntimeError, "Partial vector must not be Teuchos::null");
      if(bThyraMode_ == bThyraMode) {
        TEUCHOS_TEST_FOR_EXCEPTION(bThyraMode_ == true && v->getMap()->getMinAllGlobalIndex() > 0, Xpetra::Exceptions::RuntimeError, "BlockedMultiVector is in Thyra mode but partial map starts with GIDs " << v->getMap()->getMinAllGlobalIndex() << " > 0!");
        XPETRA_TEST_FOR_EXCEPTION(me->getMap(r,bThyraMode_)->isSameAs(*(v->getMap()))==false, Xpetra::Exceptions::RuntimeError, "Map of provided partial map and map extractor are not compatible. The size of the provided map is " << v->getMap()->getGlobalNumElements() << " and the expected size is " << getMapExtractor()->getMap(r,bThyraMode_)->getGlobalNumElements() << " or the GIDs are not correct (Thyra versus non-Thyra?)");
        vv_[r] = vv;
      }
      else {
        // standard case: bThyraMode_ == true but bThyraMode == false
        XPETRA_TEST_FOR_EXCEPTION(me->getMap(r,bThyraMode)->isSameAs(*(v->getMap()))==false, Xpetra::Exceptions::RuntimeError, "Map of provided partial map and map extractor are not compatible. The size of the provided map is " << v->getMap()->getGlobalNumElements() << " and the expected size is " << getMapExtractor()->getMap(r,bThyraMode_)->getGlobalNumElements() << " or the GIDs are not correct (Thyra versus non-Thyra?)");
        size_t numVecs     = v->getNumVectors();
        Teuchos::RCP<MultiVector> target = me->getVector(r,numVecs,bThyraMode_);
        size_t localLength = target->getLocalLength();
        for(size_t k=0; k < numVecs; k++) {
          Teuchos::ArrayRCP<const Scalar> srcVecData  = v->getData(k);
          Teuchos::ArrayRCP<Scalar> targetVecData = target->getDataNonConst(k);
          for(size_t i=0; i < localLength; i++) {
             targetVecData[i] = srcVecData[i];
          }
        }
        vv_[r] = target;
      }
    }

    /// access to underlying MapExtractor object
    RCP<const MapExtractor> getMapExtractor() const { return mapextractor_; }

    /// merge BlockedMultiVector blocks to a single MultiVector
    Teuchos::RCP<MultiVector> Merge() const {
      XPETRA_MONITOR("BlockedMultiVector::Merge");
      using Teuchos::RCP;
      using Teuchos::rcp_dynamic_cast;
      //Scalar one = ScalarTraits<SC>::one();

      RCP<MultiVector> v = MultiVectorFactory::Build(getMapExtractor()->getFullMap(), getNumVectors());
      for(size_t r = 0; r < getMapExtractor()->NumMaps(); ++r) {
        getMapExtractor()->InsertVector(getMultiVector(r),r,v,bThyraMode_);
      }

      // TODO plausibility checks

      return v;
    }


  protected:
    /// \brief Implementation of the assignment operator (operator=);
    ///   does a deep copy.
    ///
    /// Each subclass must implement this.  This includes
    /// Xpetra::EpetraMultiVector and Xpetra::TpetraMultiVector.
    virtual void assign (const MultiVector& rhs) {
      throw Xpetra::Exceptions::RuntimeError("BlockedMultiVector::assign: Not (yet) supported by BlockedMultiVector.");
    }

  private:
    Teuchos::RCP<const MapExtractor>        mapextractor_; // map extractor
    std::vector<Teuchos::RCP<MultiVector> > vv_;          ///< array containing RCPs of the partial vectors
    bool                                    bThyraMode_;  ///< boolean flag, which is true, if BlockedCrsMatrix has been created using Thyra-style numbering for sub blocks, i.e. all GIDs of submaps are contiguous and start from 0.
    size_t                                  numVectors_;  ///< number of vectors (columns in multi vector)
  }; // BlockedMultiVector class

} // Xpetra namespace

#define XPETRA_BLOCKEDMULTIVECTOR_SHORT
#endif // XPETRA_BLOCKEDMULTIVECTOR_HPP
