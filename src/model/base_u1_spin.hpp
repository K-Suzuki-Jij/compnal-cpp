//  Copyright 2022 Kohei Suzuki
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
//  Created by Kohei Suzuki on 2021/11/18.
//

#ifndef COMPNAL_MODEL_BASE_U1SPIN_HPP_
#define COMPNAL_MODEL_BASE_U1SPIN_HPP_

#include "../sparse_matrix/all.hpp"
#include "../utility/all.hpp"
#include "../type/all.hpp"

#ifdef _OPENMP
#include <omp.h>
#endif

namespace compnal {
namespace model {

//! @brief The base class for spin systems with the U(1) symmetry.
//! Conserved quantity is the total sz
//! \f$ \langle\hat{S}^{z}_{\rm tot}\rangle=\sum^{N}_{i=1}\langle\hat{S}^{z}_{i}\rangle \f$
//! @tparam RealType Type of real values.
template<typename RealType>
class BaseU1Spin {
   
public:
   //------------------------------------------------------------------
   //----------------------------Type Alias----------------------------
   //------------------------------------------------------------------
   //! @brief Alias of HalfInt type.
   using HalfInt = type::HalfInt;
   
   //! @brief Alias of compressed row strage (CRS) with RealType.
   using CRS = sparse_matrix::CRS<RealType>;
   
   //! @brief Alias of quantum number (total sz) type.
   using QType = HalfInt;
   
   //! @brief Alias of quantum number hash.
   using QHash = utility::HalfIntHash;
   
   //! @brief Alias of RealType.
   using ValueType = RealType;
   
   //------------------------------------------------------------------
   //---------------------------Constructors---------------------------
   //------------------------------------------------------------------
   //! @brief Constructor of BaseU1Spin class.
   BaseU1Spin() {
      SetOnsiteOperator();
   }
   
   //! @brief Constructor of BaseU1Spin class.
   //! @param magnitude_spin The magnitude of the spin \f$ S \f$.
   explicit BaseU1Spin(const HalfInt magnitude_spin) {
      SetMagnitudeSpin(magnitude_spin);
   }
   
   //! @brief Constructor of BaseU1Spin class.
   //! @param magnitude_spin The magnitude of the spin \f$ S \f$.
   //! @param total_sz The total sz \f$ \langle\hat{S}^{z}_{\rm tot}\rangle=\sum^{N}_{i=1}\langle\hat{S}^{z}_{i}\rangle \f$.
   BaseU1Spin(const HalfInt magnitude_spin, const HalfInt total_sz) {
      SetMagnitudeSpin(magnitude_spin);
      SetTotalSz(total_sz);
   }
   
   //------------------------------------------------------------------
   //----------------------Public Member Functions---------------------
   //------------------------------------------------------------------
   //! @brief Set the magnitude of the spin \f$ S \f$.
   //! @param magnitude_spin The magnitude of the spin \f$ S \f$.
   void SetMagnitudeSpin(const HalfInt magnitude_spin) {
      if (magnitude_spin <= 0) {
         std::stringstream ss;
         ss << "Error in " << __FUNCTION__ << " at " << __LINE__ << std::endl;
         ss << "Please set magnitude_spin > 0" << std::endl;
         throw std::runtime_error(ss.str());
      }
      magnitude_spin_ = magnitude_spin;
      dim_onsite_     = 2*magnitude_spin + 1;
      SetOnsiteOperator();
   }
   
   //! @brief Set the target Hilbert space specified by the total sz.
   //! @param total_sz The total sz \f$ \langle\hat{S}^{z}_{\rm tot}\rangle=\sum^{N}_{i=1}\langle\hat{S}^{z}_{i}\rangle \f$.
   void SetTotalSz(const HalfInt total_sz) {
      total_sz_ = total_sz;
   }
      
   //! @brief Calculate the number of electrons from the input onsite basis.
   //! @param basis_onsite The onsite basis.
   //! @return The number of electrons.
   int CalculateNumElectron(const int basis_onsite) const {
      if (0 <= basis_onsite && basis_onsite < dim_onsite_) {
         return 0;
      }
      else {
         std::stringstream ss;
         ss << "Error in " << __FUNCTION__ << " at " << __LINE__  << std::endl;
         ss << "Invalid onsite basis" << std::endl;
         throw std::runtime_error(ss.str());
      }
   }
   
   //! @brief Print the onsite bases.
   void PrintBasisOnsite() const {
      for (int row = 0; row < dim_onsite_; ++row) {
         std::cout << "row " << row << ": |Sz=" << magnitude_spin_ - row << ">" << std::endl;
      }
   }
   
   //! @brief Calculate sectors generated by an onsite operator.
   //! @param row The row in the matrix representation of an onsite operator.
   //! @param col The column in the matrix representation of an onsite operator.
   //! @return Sectors generated by an onsite operator.
   template<typename IntegerType>
   QType CalculateQNumber(const IntegerType row, const IntegerType col) {
      return static_cast<QType>(col - row + total_sz_);
   }
   
   //------------------------------------------------------------------
   //----------------------Static Member Functions---------------------
   //------------------------------------------------------------------
   //! @brief Generate basis of the target Hilbert space specified by
   //! the system size \f$ N\f$, the magnitude of the spin \f$ S\f$,
   //! and the total sz \f$ \langle\hat{S}^{z}_{\rm tot}\rangle \f$.
   //! @param system_size The system size.
   //! @param magnitude_spin The magnitude of the spin \f$ S \f$.
   //! @param total_sz The total sz \f$ \langle\hat{S}^{z}_{\rm tot}\rangle \f$.
   //! @param flag_display_info If true, display the progress status. Set ture by default.
   //! @return Corresponding basis.
   static std::vector<std::int64_t> GenerateBasis(const int system_size,
                                                  const HalfInt magnitude_spin,
                                                  const HalfInt total_sz,
                                                  const bool flag_display_info = true) {
      if (!isValidQNumber(system_size, magnitude_spin, total_sz) || system_size <= 0) {
         std::stringstream ss;
         ss << "Error in " << __FUNCTION__ << " at " << __LINE__ << std::endl;
         ss << "Invalid parameters (system_size or magnitude_spin or total_sz)" << std::endl;
         throw std::runtime_error(ss.str());
      }
      
      const auto start = std::chrono::system_clock::now();
      
      const int dim_onsite = 2*magnitude_spin + 1;
      std::vector<std::int64_t> basis;
      
      if (flag_display_info) {
         std::cout << "Generating Basis..." << std::flush;
      }
      
      const int shifted_2sz = static_cast<int>(2*(system_size*magnitude_spin - total_sz));
      const std::int64_t dim_target = CalculateTargetDim(system_size, magnitude_spin, total_sz);
      std::vector<std::vector<int>> partition_integers;
      utility::GenerateIntegerPartition(&partition_integers, shifted_2sz, magnitude_spin);
      
      std::vector<std::int64_t> site_constant(system_size);
      for (int site = 0; site < system_size; ++site) {
         site_constant[site] = static_cast<std::int64_t>(std::pow(dim_onsite, site));
      }
      
#ifdef _OPENMP
      const int num_threads = omp_get_max_threads();
      std::vector<std::vector<std::int64_t>> temp_basis(num_threads);
      for (auto &&integer_list: partition_integers) {
         const bool condition1 = (0 < integer_list.size()) && (static_cast<int>(integer_list.size()) <= system_size);
         const bool condition2 = (integer_list.size() == 0) && (shifted_2sz  == 0);
         if (condition1 || condition2) {
            for (int j = static_cast<int>(integer_list.size()); j < system_size; ++j) {
               integer_list.push_back(0);
            }
            
            const std::int64_t size = utility::CalculateNumCombination(integer_list);
            std::vector<std::vector<int>> temp_partition_integer(num_threads);
            
#pragma omp parallel num_threads (num_threads)
            {
               const int thread_num = omp_get_thread_num();
               const std::int64_t loop_begin = thread_num*size/num_threads;
               const std::int64_t loop_end   = (thread_num + 1)*size/num_threads;
               temp_partition_integer[thread_num] = integer_list;
               utility::CalculateNthPermutation(&temp_partition_integer[thread_num], loop_begin);
               
               for (std::int64_t j = loop_begin; j < loop_end; ++j) {
                  std::int64_t basis_global = 0;
                  const auto iter_begin = temp_partition_integer[thread_num].begin();
                  const auto iter_end   = temp_partition_integer[thread_num].end();
                  for (auto itr = iter_begin; itr != iter_end; ++itr) {
                     basis_global += *itr*site_constant[std::distance(iter_begin, itr)];
                  }
                  temp_basis[thread_num].push_back(basis_global);
                  std::next_permutation(temp_partition_integer[thread_num].begin(), temp_partition_integer[thread_num].end());
               }
            }
         }
      }
      
      for (auto &&it: temp_basis) {
         basis.insert(basis.end(), it.begin(), it.end());
         std::vector<std::int64_t>().swap(it);
      }
      
#else
      basis.reserve(dim_target);
      
      for (auto &&integer_list: partition_integers) {
         const bool condition1 = (0 < integer_list.size()) && (static_cast<int>(integer_list.size()) <= system_size_);
         const bool condition2 = (integer_list.size() == 0) && (shifted_2sz  == 0);
         if (condition1 || condition2) {
            
            for (std::int64_t j = integer_list.size(); j < system_size_; ++j) {
               integer_list.push_back(0);
            }
            
            std::sort(integer_list.begin(), integer_list.end());
            
            do {
               std::int64_t basis_global = 0;
               for (std::size_t j = 0; j < integer_list.size(); ++j) {
                  basis_global += integer_list[j]*site_constant[j];
               }
               basis.push_back(basis_global);
            } while (std::next_permutation(integer_list.begin(), integer_list.end()));
         }
      }
      
#endif
      
      if (static_cast<std::int64_t>(basis.size()) != dim_target) {
         std::stringstream ss;
         ss << "Unknown error detected in " << __FUNCTION__ << " at " << __LINE__ << std::endl;
         throw std::runtime_error(ss.str());
      }
      
      std::sort(basis.begin(), basis.end());
      
      if (flag_display_info) {
         const auto   time_count = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start).count();
         const double time_sec   = static_cast<double>(time_count)/sparse_matrix::TIME_UNIT_CONSTANT;
         std::cout << "\rElapsed time of generating basis:" << time_sec << "[sec]" << std::endl;
      }
      return basis;
   }
   
   //! @brief Check if there is a subspace specified by the input quantum numbers.
   //! @param system_size The system size \f$ N\f$.
   //! @param magnitude_spin The magnitude of the spin \f$ S \f$.
   //! @param total_sz The total sz \f$ \langle\hat{S}^{z}_{\rm tot}\rangle\f$.
   //! @return ture if there exists corresponding subspace, otherwise false.
   static bool isValidQNumber(const int system_size, const HalfInt magnitude_spin, const HalfInt total_sz) {
      const int total_2sz       = 2*total_sz;
      const int magnitude_2spin = 2*magnitude_spin;
      const bool c1 = ((system_size*magnitude_2spin - total_2sz)%2 == 0);
      const bool c2 = (-system_size*magnitude_2spin <= total_2sz);
      const bool c3 = (total_2sz <= system_size*magnitude_2spin);
      if (c1 && c2 && c3) {
         return true;
      }
      else {
         return false;
      }
   }
   
   //! @brief Calculate dimension of the target Hilbert space specified by
   //! the system size \f$ N\f$, the magnitude of the spin \f$ S\f$,
   //! and the total sz \f$ \langle\hat{S}^{z}_{\rm tot}\rangle \f$.
   //! @param system_size The system size \f$ N\f$.
   //! @param magnitude_spin The magnitude of the spin \f$ S \f$.
   //! @param total_sz The total sz \f$ \langle\hat{S}^{z}_{\rm tot}\rangle\f$.
   static std::int64_t CalculateTargetDim(const int system_size, const HalfInt magnitude_spin, const HalfInt total_sz) {
      if (!isValidQNumber(system_size, magnitude_spin, total_sz)) {
         return 0;
      }
      if (system_size <= 0) {
         return 0;
      }
      const int magnitude_2spin = 2*magnitude_spin;
      const int total_2sz       = 2*total_sz;
      const int max_total_2sz   = system_size*magnitude_2spin;
      std::vector<std::vector<std::int64_t>> dim(system_size, std::vector<std::int64_t>(max_total_2sz + 1));
      for (int s = -magnitude_2spin; s <= magnitude_2spin; s += 2) {
         dim[0][(s + magnitude_2spin)/2] = 1;
      }
      for (int site = 1; site < system_size; site++) {
         for (int s = -magnitude_2spin; s <= magnitude_2spin; s += 2) {
            for (int s_prev = -magnitude_2spin*site; s_prev <= magnitude_2spin*site; s_prev += 2) {
               const std::int64_t a = dim[site    ][(s + s_prev + magnitude_2spin*(site + 1))/2];
               const std::int64_t b = dim[site - 1][(s_prev + magnitude_2spin*site)/2];
               if (a >= INT64_MAX - b) {
                  throw std::overflow_error("Overflow detected for sumation using uint64_t");
               }
               dim[site][(s + s_prev + magnitude_2spin*(site + 1))/2] = a + b;
            }
         }
      }
      return dim[system_size - 1][(total_2sz + max_total_2sz)/2];
   }
   
   //! @brief Generate the spin-\f$ S\f$ operator for the x-direction \f$ \hat{s}^{x}\f$.
   //! @param magnitude_spin The magnitude of the spin \f$ S \f$.
   //! @return The matrix of \f$ \hat{s}^{x}\f$.
   static CRS CreateOnsiteOperatorSx(const HalfInt magnitude_spin) {
      return 0.5*(CreateOnsiteOperatorSp(magnitude_spin) + CreateOnsiteOperatorSm(magnitude_spin));
   }
   
   //! @brief Generate the spin-\f$ S\f$ operator for the y-direction
   //! \f$ i\hat{s}^{y}\f$ with \f$ i\f$ being the imaginary unit.
   //! @param magnitude_spin The magnitude of the spin \f$ S \f$.
   //! @return The matrix of \f$ i\hat{s}^{y}\f$.
   static CRS CreateOnsiteOperatoriSy(const HalfInt magnitude_spin) {
      return 0.5*(CreateOnsiteOperatorSp(magnitude_spin) - CreateOnsiteOperatorSm(magnitude_spin));
   }
   
   //! @brief Generate the spin-\f$ S\f$ operator for the z-direction \f$ \hat{s}^{z}\f$.
   //! @param magnitude_spin The magnitude of the spin \f$ S \f$.
   //! @return The matrix of \f$ \hat{s}^{z}\f$.
   static CRS CreateOnsiteOperatorSz(const HalfInt magnitude_spin) {
      const int dim_onsite = 2*magnitude_spin + 1;
      CRS matrix(dim_onsite, dim_onsite);
      
      for (int row = 0; row < dim_onsite; ++row) {
         const RealType val = magnitude_spin - row;
         if (val != 0.0) {
            matrix.val.push_back(val);
            matrix.col.push_back(row);
         }
         matrix.row[row + 1] = matrix.col.size();
      }
      return matrix;
   }
   
   //! @brief Generate the spin-\f$ S\f$ raising operator \f$ \hat{s}^{+}\f$.
   //! @param magnitude_spin The magnitude of the spin \f$ S \f$.
   //! @return The matrix of \f$ \hat{s}^{+}\f$.
   static CRS CreateOnsiteOperatorSp(const HalfInt magnitude_spin) {
      const int dim_onsite = 2*magnitude_spin + 1;
      CRS matrix(dim_onsite, dim_onsite);
      for (int row = 1; row < dim_onsite; ++row) {
         matrix.val.push_back(std::sqrt((magnitude_spin + 1)*2.0*row - row*(row + 1)));
         matrix.col.push_back(row);
         matrix.row[row] = matrix.col.size();
      }
      matrix.row[dim_onsite] = matrix.col.size();
      return matrix;
   }
   
   //! @brief Generate the spin-\f$ S\f$ raising operator \f$ \hat{s}^{-}\f$.
   //! @param magnitude_spin The magnitude of the spin \f$ S \f$.
   //! @return The matrix of \f$ \hat{s}^{-}\f$.
   static CRS CreateOnsiteOperatorSm(const HalfInt magnitude_spin) {
      const int dim_onsite = 2*magnitude_spin + 1;
      CRS matrix(dim_onsite, dim_onsite);
      for (int row = 1; row < dim_onsite; ++row) {
         matrix.val.push_back(std::sqrt((magnitude_spin + 1)*2.0*row - row*(row + 1)));
         matrix.col.push_back(row - 1);
         matrix.row[row + 1] = matrix.col.size();
      }
      return matrix;
   }
   
   //------------------------------------------------------------------
   //----------------------Access Member variables---------------------
   //------------------------------------------------------------------
   //! @brief Get dimension of the onsite Hilbert space, \f$ 2S+1\f$.
   //! @return The dimension of the onsite Hilbert space, \f$ 2S+1\f$.
   inline int GetDimOnsite() const { return dim_onsite_; }
   
   //! @brief Get the total sz \f$ \langle\hat{S}^{z}_{\rm tot}\rangle\f$.
   //! @return The total sz \f$ \langle\hat{S}^{z}_{\rm tot}\rangle\f$.
   inline HalfInt GetTotalSz() const { return total_sz_; }
      
   //! @brief Get the magnitude of the spin \f$ S\f$.
   //! @return The magnitude of the spin \f$ S\f$.
   inline HalfInt GetMagnitudeSpin() const { return magnitude_spin_; }
   
   //! @brief Get the spin-\f$ S\f$ operator for the x-direction \f$ \hat{s}^{x}\f$.
   //! @return The matrix of \f$ \hat{s}^{x}\f$.
   inline const CRS &GetOnsiteOperatorSx () const { return onsite_operator_sx_; }
   
   //! @brief Get the spin-\f$ S\f$ operator for the y-direction \f$ i\hat{s}^{y}\f$ with \f$ i\f$ being the imaginary unit.
   //! @return The matrix of \f$ i\hat{s}^{y}\f$.
   inline const CRS &GetOnsiteOperatoriSy() const { return onsite_operator_isy_; }
   
   //! @brief Get the spin-\f$ S\f$ operator for the z-direction \f$ \hat{s}^{z}\f$.
   //! @return The matrix of \f$ \hat{s}^{z}\f$.
   inline const CRS &GetOnsiteOperatorSz () const { return onsite_operator_sz_; }
   
   //! @brief Get the spin-\f$ S\f$ raising operator \f$ \hat{s}^{+}\f$.
   //! @return The matrix of \f$ \hat{s}^{+}\f$.
   inline const CRS &GetOnsiteOperatorSp () const { return onsite_operator_sp_; }
   
   //! @brief Get the spin-\f$ S\f$ lowering operator \f$ \hat{s}^{-}\f$.
   //! @return The matrix of \f$ \hat{s}^{-}\f$.
   inline const CRS &GetOnsiteOperatorSm () const { return onsite_operator_sm_; }
   
private:
   //------------------------------------------------------------------
   //---------------------Private Member Variables---------------------
   //------------------------------------------------------------------
   //! @brief Twice the number of the total sz \f$ 2\langle\hat{S}^{z}_{\rm tot}\rangle\f$.
   HalfInt total_sz_ = 0;
   
   //! @brief The dimension of the local Hilbert space, \f$ 2S + 1\f$.
   int dim_onsite_ = 2;
   
   //! @brief The magnitude of the spin \f$ S\f$.
   HalfInt magnitude_spin_ = 0.5;
   
   //! @brief The spin-\f$ S\f$ operator for the x-direction \f$ \hat{s}^{x}\f$.
   CRS onsite_operator_sx_;
   
   //! @brief The spin-\f$ S\f$ operator for the y-direction \f$ i\hat{s}^{y}\f$ with \f$ i\f$ being the imaginary unit.
   CRS onsite_operator_isy_;
   
   //! @brief The spin-\f$ S\f$ operator for the z-direction \f$ \hat{s}^{z}\f$.
   CRS onsite_operator_sz_;
   
   //! @brief The spin-\f$ S\f$ raising operator \f$ \hat{s}^{+}\f$.
   CRS onsite_operator_sp_;
   
   //! @brief The the spin-\f$ S\f$ raising operator \f$ \hat{s}^{-}\f$.
   CRS onsite_operator_sm_;
   
   //------------------------------------------------------------------
   //----------------------Private Member Functions---------------------
   //------------------------------------------------------------------
   //! @brief Set onsite operators.
   void SetOnsiteOperator() {
      onsite_operator_sx_  = CreateOnsiteOperatorSx (magnitude_spin_);
      onsite_operator_isy_ = CreateOnsiteOperatoriSy(magnitude_spin_);
      onsite_operator_sz_  = CreateOnsiteOperatorSz (magnitude_spin_);
      onsite_operator_sp_  = CreateOnsiteOperatorSp (magnitude_spin_);
      onsite_operator_sm_  = CreateOnsiteOperatorSm (magnitude_spin_);
   }
   
};



}
}


#endif /* COMPNAL_MODEL_BASE_U1SPIN_HPP_ */
