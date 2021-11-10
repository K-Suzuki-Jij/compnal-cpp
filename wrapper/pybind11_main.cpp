//
//  pybind11_main.cpp
//  compnal
//
//  Created by Kohei Suzuki on 2021/06/29.
//

#include "./src/pybind11_model.hpp"
#include "./src/pybind11_sparse_matrix.hpp"
#include "./src/pybind11_solver_exact_diag.hpp"

PYBIND11_MODULE(compnal, m) {
   
   py::module_ m_model = m.def_submodule("model");
   pybind11ModelXXZ1D<double>(m_model);
   pybind11ModelBoundaryCondition(m_model);
   
   py::module_ m_sp_mat = m.def_submodule("sparse_matrix");
   pybind11SparseMatrixCRS<double>(m_sp_mat);
   pybind11SparseMatrixBraketVector<double>(m_sp_mat);
   pybind11SparseMatrixParameters(m_sp_mat);
   
   py::module_ m_solver = m.def_submodule("solver");
   pybind11SolverExactDiag<compnal::model::XXZ_1D<double>>(m_solver);
   
};
