// @HEADER
// ************************************************************************
//
//                           Intrepid2 Package
//                 Copyright (2007) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
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
// Questions? Contact Kyungjoo Kim  (kyukim@sandia.gov), or
//                    Mauro Perego  (mperego@sandia.gov)
//
// ************************************************************************
// @HEADER

/** \file   Intrepid_HGRAD_LINE_C1_FEMDef.hpp
    \brief  Definition file for FEM basis functions of degree 1 for H(grad) functions on a Line.
    \author Created by P. Bochev and D. Ridzal.
            Kokkorized by Kyungjoo Kim
 */

#ifndef __INTREPID2_HGRAD_LINE_C1_FEM_DEF_HPP__
#define __INTREPID2_HGRAD_LINE_C1_FEM_DEF_HPP__

namespace Intrepid2 {

  // -------------------------------------------------------------------------------------

  template<typename SpT, typename OT, typename PT>
  template<EOperator opType>
  template<typename outputValueViewType,
           typename inputPointViewType>
  KOKKOS_INLINE_FUNCTION
  void
  Basis_HGRAD_LINE_C1_FEM<SpT,OT,PT>::Serial<opType>::
  getValues( /**/  outputValueViewType output,
             const inputPointViewType  input ) {
    switch (opType) {
    case OPERATOR_VALUE : {
      const auto x = input(0);

      output(0) = (1.0 - x)/2.0;
      output(1) = (1.0 + x)/2.0;
      break;
    }
    case OPERATOR_GRAD : {
      output(0, 0) = -0.5;
      output(1, 0) =  0.5;
      break;
    }
    case OPERATOR_MAX : {
      const auto jend = output.dimension(1);
      const auto iend = output.dimension(0);

      for (size_type j=0;j<jend;++j)
        for (size_type i=0;i<iend;++i)
          output(i, j) = 0.0;
      break;
    }
    default: {
      INTREPID2_TEST_FOR_ABORT( opType != OPERATOR_VALUE &&
                                opType != OPERATOR_GRAD &&
                                opType != OPERATOR_MAX,
                                ">>> ERROR: (Intrepid2::Basis_HGRAD_LINE_C1_FEM::Serial::getValues) operator is not supported");

    }
    }
  }

  // -------------------------------------------------------------------------------------

  template<typename SpT, typename OT, typename PT>
  Basis_HGRAD_LINE_C1_FEM<SpT,OT,PT>::
  Basis_HGRAD_LINE_C1_FEM()
    : impl_(this) {
    this->basisCardinality_  = 2;
    this->basisDegree_       = 1;
    this->basisCellTopology_ = shards::CellTopology(shards::getCellTopologyData<shards::Line<2> >() );
    this->basisType_         = BASIS_FEM_DEFAULT;
    this->basisCoordinates_  = COORDINATES_CARTESIAN;

    // initialize tags
    {
      // Basis-dependent intializations
      const ordinal_type tagSize  = 4;        // size of DoF tag, i.e., number of fields in the tag
      const ordinal_type posScDim = 0;        // position in the tag, counting from 0, of the subcell dim
      const ordinal_type posScOrd = 1;        // position in the tag, counting from 0, of the subcell ordinal
      const ordinal_type posDfOrd = 2;        // position in the tag, counting from 0, of DoF ordinal relative to the subcell

      // An array with local DoF tags assigned to basis functions, in the order of their local enumeration
      ordinal_type tags[8]  = { 0, 0, 0, 1,
                                0, 1, 0, 1 };

      // host tags
      ordinal_type_array_1d_host tagView(&tags[0], 8);

      // Basis-independent function sets tag and enum data in tagToOrdinal_ and ordinalToTag_ arrays:
      // tags are constructed on host and sent to devices
      //ordinal_type_array_2d_host ordinalToTag;
      //ordinal_type_array_3d_host tagToOrdinal;
      this->setOrdinalTagData(this->tagToOrdinal_,
                              this->ordinalToTag_,
                              tagView,
                              this->basisCardinality_,
                              tagSize,
                              posScDim,
                              posScOrd,
                              posDfOrd);
      
      //this->tagToOrdinal_ = Kokkos::create_mirror_view(typename SpT::memory_space(), tagToOrdinal);
      //Kokkos::deep_copy(this->tagToOrdinal_, tagToOrdinal);

      //this->ordinalToTag_ = Kokkos::create_mirror_view(typename SpT::memory_space(), ordinalToTag);
      //Kokkos::deep_copy(this->ordinalToTag_, ordinalToTag);
    }

    // dofCoords on host and create its mirror view to device
    Kokkos::DynRankView<PT,typename SpT::array_layout,Kokkos::HostSpace> 
      dofCoords("dofCoordsHost", this->basisCardinality_,this->basisCellTopology_.getDimension()); 
    
    dofCoords(0,0) = -1.0;
    dofCoords(1,0) =  1.0;
    
    this->dofCoords_ = Kokkos::create_mirror_view(typename SpT::memory_space(), dofCoords);
    Kokkos::deep_copy(this->dofCoords_, dofCoords);
  }


  template<typename SpT, typename OT, typename PT>
  template<typename outputValueValueType, class ...outputValueProperties,
           typename inputPointValueType,  class ...inputPointProperties>
  void
  Basis_HGRAD_LINE_C1_FEM<SpT,OT,PT>::Internal::
  getValues( /**/  Kokkos::DynRankView<outputValueValueType,outputValueProperties...> outputValues,
             const Kokkos::DynRankView<inputPointValueType, inputPointProperties...>  inputPoints,
             const EOperator operatorType ) const {
#ifdef HAVE_INTREPID2_DEBUG
    Intrepid2::getValues_HGRAD_Args(outputValues,
                                    inputPoints,
                                    operatorType,
                                    obj_->getBaseCellTopology(),
                                    obj_->getCardinality() );
#endif

    typedef          Kokkos::DynRankView<outputValueValueType,outputValueProperties...>         outputValueViewType;
    typedef          Kokkos::DynRankView<inputPointValueType, inputPointProperties...>          inputPointViewType;
    typedef typename ExecSpace<typename inputPointViewType::execution_space,SpT>::ExecSpaceType ExecSpaceType;

    // Number of evaluation points = dim 0 of inputPoints
    const auto loopSize = inputPoints.dimension(0);
    Kokkos::RangePolicy<ExecSpaceType,Kokkos::Schedule<Kokkos::Static> > policy(0, loopSize);

    switch (operatorType) {

    case OPERATOR_VALUE: {
      typedef Functor<outputValueViewType,inputPointViewType,OPERATOR_VALUE> FunctorType;
      Kokkos::parallel_for( policy, FunctorType(outputValues, inputPoints) );
      break;
    }
    case OPERATOR_GRAD:
    case OPERATOR_DIV:
    case OPERATOR_CURL:
    case OPERATOR_D1: {
      typedef Functor<outputValueViewType,inputPointViewType,OPERATOR_GRAD> FunctorType;
      Kokkos::parallel_for( policy, FunctorType(outputValues, inputPoints) );
      break;
    }
    case OPERATOR_D2:
    case OPERATOR_D3:
    case OPERATOR_D4:
    case OPERATOR_D5:
    case OPERATOR_D6:
    case OPERATOR_D7:
    case OPERATOR_D8:
    case OPERATOR_D9:
    case OPERATOR_D10: {
      typedef Functor<outputValueViewType,inputPointViewType,OPERATOR_MAX> FunctorType;
      Kokkos::parallel_for( policy, FunctorType(outputValues, inputPoints) );
      break;
    }
    default: {
      INTREPID2_TEST_FOR_EXCEPTION( !Intrepid2::isValidOperator(operatorType), std::invalid_argument,
                                    ">>> ERROR (Basis_HGRAD_LINE_C1_FEM): Invalid operator type");
    }
    }
  }


  template<typename SpT, typename OT, typename PT>
  template<typename dofCoordValueType, class ...dofCoordProperties>
  void
  Basis_HGRAD_LINE_C1_FEM<SpT,OT,PT>::Internal::
  getDofCoords( Kokkos::DynRankView<dofCoordValueType,dofCoordProperties...> dofCoords ) const {

#ifdef HAVE_INTREPID2_DEBUG
    // Verify rank of output array.
    INTREPID2_TEST_FOR_EXCEPTION( dofCoords.rank() != 2, std::invalid_argument,
                                  ">>> ERROR: (Intrepid2::Basis_HGRAD_LINE_C1_FEM::getDofCoords) rank = 2 required for dofCoords array");
    // Verify 0th dimension of output array.
    INTREPID2_TEST_FOR_EXCEPTION( dofCoords.dimension(0) != obj_->basisCardinality_, std::invalid_argument,
                                  ">>> ERROR: (Intrepid2::Basis_HGRAD_LINE_C1_FEM::getDofCoords) mismatch in number of dof and 0th dimension of dofCoords array");
    // Verify 1st dimension of output array.
    INTREPID2_TEST_FOR_EXCEPTION( dofCoords.dimension(1) != obj_->basisCellTopology_.getDimension(), std::invalid_argument,
                                  ">>> ERROR: (Intrepid2::Basis_HGRAD_LINE_C1_FEM::getDofCoords) incorrect reference cell (1st) dimension in dofCoords array");
#endif
    Kokkos::deep_copy(dofCoords, obj_->dofCoords_);
  }

}

#endif
