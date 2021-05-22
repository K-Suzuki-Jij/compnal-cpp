//
//  lapack.hpp
//  compnal
//
//  Created by Kohei Suzuki on 2021/05/22.
//

#ifndef lapack_hpp
#define lapack_hpp

#include "compressed_row_storage.hpp"
#include "braket_vector.hpp"
#include <sstream>

namespace compnal {
namespace sparse_matrix {

extern "C" {
void dsyev_(const char &JOBZ, const char &UPLO, const int &N, double **A,
            const int &LDA, double *W, double *work, const int &Lwork,
            int &INFO);
};

extern "C" {
void dstev_(const char &JOBZ, const int &N, double *D, double *E, double **Z,  const int &LDZ, double* WORK, int& INFO);
};


template <typename RealType>
void LapackDsyev(RealType *gs_value, BraketVector<RealType> *gs_vector,
                 const CRS<RealType> &matrix_in) {
   
   if (matrix_in.GetRowDim() != matrix_in.GetColDim() || matrix_in.GetRowDim() < 1 || matrix_in.GetColDim() < 1) {
      std::stringstream ss;
      ss << "Error in " << __func__ << std::endl;
      ss << "The input matrix is not a square one" << std::endl;
      ss << "row=" << matrix_in.GetRowDim() << ", col=" << matrix_in.GetColDim() << std::endl;
      throw std::runtime_error(ss.str());
   }
   
   double matrix_array[matrix_in.GetRowDim()][matrix_in.GetColDim()];
   
   for (int64_t i = 0; i < matrix_in.GetRowDim(); ++i) {
      for (int64_t j = 0; j < matrix_in.GetColDim(); ++j) {
         matrix_array[i][j] = 0.0;
      }
   }
   
   for (int64_t i = 0; i < matrix_in.GetRowDim(); ++i) {
      for (std::size_t j = matrix_in.Row(i); j < matrix_in.Row(i + 1); ++j) {
         matrix_array[i][matrix_in.Col(j)] = static_cast<double>(matrix_in.Val(j));
         matrix_array[matrix_in.Col(j)][i] = static_cast<double>(matrix_in.Val(j));
      }
   }
   
   int info;
   double val_array[matrix_in.GetRowDim()];
   double work[3 * matrix_in.GetRowDim()];
   
   dsyev_('V', 'L', static_cast<int>(matrix_in.GetRowDim()),
          (double**)matrix_array,
          static_cast<int>(matrix_in.GetRowDim()), val_array, work,
          static_cast<int>(3 * matrix_in.GetRowDim()), info);
   
   gs_vector->Resize(matrix_in.GetRowDim());
   
   for (int64_t i = 0; i < matrix_in.GetRowDim(); ++i) {
      gs_vector->Val(i) = static_cast<RealType>(matrix_array[0][i]);
   }
   
   *gs_value = static_cast<RealType>(val_array[0]);
}

template <typename RealType>
void LapackDstev(RealType *gs_value, BraketVector<RealType> *gs_vector, const std::vector<RealType> &diag, const std::vector<RealType> &off_diag) {
   
   if (off_diag.size() != diag.size()) {
      std::stringstream ss;
      ss << "Error in " << __func__ << std::endl;
      ss << "diag size=" << diag.size() << ", off_diag size=" << off_diag.size() << std::endl;
      throw std::runtime_error(ss.str());
   }
   
   int dim = static_cast<int>(diag.size());

   int info;
   double Lap_D[dim];
   double Lap_E[dim - 1];
   double Lap_Vec[dim][dim];
   double Lap_Work[2*dim];
   
   for (int i = 0; i < dim; i++) {
      Lap_D[i] = static_cast<double>(diag[i]);
   }
   
   for (int i = 0; i < dim - 1; i++) {
      Lap_E[i] = static_cast<double>(off_diag[i]);
   }
   
   dstev_('V', dim, Lap_D, Lap_E, (double**)Lap_Vec, dim, Lap_Work, info);
   
   gs_vector->Resize(dim);
   
   for (int64_t i = 0; i < dim; ++i) {
      gs_vector->Val(i) = static_cast<RealType>(Lap_Vec[0][i]);
   }
   
   *gs_value = static_cast<RealType>(Lap_D[0]);
   
}

}  // namespace sparse_matrix
}  // namespace compnal


#endif /* lapack_hpp */