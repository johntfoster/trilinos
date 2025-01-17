// @HEADER
// ***********************************************************************
//
//           Panzer: A partial differential equation assembly
//       engine for strongly coupled complex multiphysics systems
//                 Copyright (2011) Sandia Corporation
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
// Questions? Contact Roger P. Pawlowski (rppawlo@sandia.gov) and
// Eric C. Cyr (eccyr@sandia.gov)
// ***********************************************************************
// @HEADER

#ifndef PANZER_PROJECT_TO_FACES_IMPL_HPP
#define PANZER_PROJECT_TO_FACES_IMPL_HPP

#include "Teuchos_Assert.hpp"
#include "Phalanx_DataLayout.hpp"

#include "Panzer_PureBasis.hpp"
#include "Panzer_CommonArrayFactories.hpp"

#include "Teuchos_FancyOStream.hpp"

template<typename EvalT,typename Traits>
panzer::ProjectToFaces<EvalT, Traits>::
ProjectToFaces(
  const Teuchos::ParameterList& p)
{ 
  dof_name = (p.get< std::string >("DOF Name"));

  if(p.isType< Teuchos::RCP<PureBasis> >("Basis"))
    basis = p.get< Teuchos::RCP<PureBasis> >("Basis");
  else
    basis = p.get< Teuchos::RCP<const PureBasis> >("Basis");

  Teuchos::RCP<PHX::DataLayout> basis_layout  = basis->functional;
  Teuchos::RCP<PHX::DataLayout> vector_layout = basis->functional_grad;

  // some sanity checks
  TEUCHOS_ASSERT(basis->isVectorBasis());

  result = PHX::MDField<ScalarT,Cell,BASIS>(dof_name,basis_layout);
  this->addEvaluatedField(result);

  vector_values = PHX::MDField<ScalarT,Cell,BASIS,Dim>(dof_name+"_Vector",vector_layout);
  normals      = PHX::MDField<ScalarT,Cell,BASIS,Dim>(dof_name+"_Normals",vector_layout);
  this->addDependentField(vector_values);
  this->addDependentField(normals);

  this->setName("Project To Faces");
}

// **********************************************************************
template<typename EvalT,typename Traits>
void panzer::ProjectToFaces<EvalT, Traits>::
postRegistrationSetup(typename Traits::SetupData d, 
		      PHX::FieldManager<Traits>& fm)
{
  // setup the field data object
  this->utils.setFieldData(result,fm);
  this->utils.setFieldData(vector_values,fm);
  this->utils.setFieldData(normals,fm);

  num_pts = vector_values.dimension(1);
  num_dim = vector_values.dimension(2);

  TEUCHOS_ASSERT(vector_values.dimension(1) == normals.dimension(1));
  TEUCHOS_ASSERT(vector_values.dimension(2) == normals.dimension(2));
}

// **********************************************************************
template<typename EvalT,typename Traits>
void panzer::ProjectToFaces<EvalT, Traits>::
evaluateFields(typename Traits::EvalData workset)
{ 

  // The coefficients being calculated here in the projection to the face basis
  // are defined as the integral over the face of the field dotted with the face
  // normal vector. For a first-order face basis, single point integration is
  // adequate, so the cubature here just provides the proper weighting.
  // For higher order, a distinction between "cell" and Gauss points will need
  // to be made so the field is appropriately projected.
  const shards::CellTopology & parentCell = *basis->getCellTopology();
  const int intDegree = basis->order();
  TEUCHOS_ASSERT(intDegree == 1);
  Intrepid2::DefaultCubatureFactory<double,Kokkos::DynRankView<double,PHX::Device>,Kokkos::DynRankView<double,PHX::Device>> quadFactory;
  Teuchos::RCP< Intrepid2::Cubature<double,Kokkos::DynRankView<double,PHX::Device>,Kokkos::DynRankView<double,PHX::Device>> > faceQuad;

  // Collect the reference face information. For now, do nothing with the quadPts.
  const unsigned num_faces = parentCell.getFaceCount();
  std::vector<double> refFaceWt(num_faces, 0.0);
  for (unsigned f=0; f<num_faces; f++) {
    faceQuad = quadFactory.create(parentCell.getCellTopologyData(2,f), intDegree);
    const int numQPoints = faceQuad->getNumPoints();
    Kokkos::DynRankView<double,PHX::Device> quadWts("quadWts",numQPoints);
    Kokkos::DynRankView<double,PHX::Device> quadPts("quadPts",numQPoints,num_dim);
    faceQuad->getCubature(quadPts,quadWts);
    for (int q=0; q<numQPoints; q++)
      refFaceWt[f] += quadWts(q);
  }

  // Loop over the faces of the workset cells. An inner quadrature loop should exist too.
  for (index_t cell = 0; cell < workset.num_cells; ++cell) {
    for (int p = 0; p < num_pts; ++p) {
      result(cell,p) = ScalarT(0.0);
      for (int dim = 0; dim < num_dim; ++dim)
        result(cell,p) += vector_values(cell,p,dim) * normals(cell,p,dim);
      result(cell,p) *= refFaceWt[p];
    }
  }

}

#endif
