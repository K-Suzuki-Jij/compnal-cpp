//
//  xxz_1d.hpp
//  compnal
//
//  Created by Kohei Suzuki on 2021/11/06.
//

#ifndef COMPNAL_MODEL_XXZ_1D_HPP_
#define COMPNAL_MODEL_XXZ_1D_HPP_

#include "../sparse_matrix/all.hpp"
#include "../utility/all.hpp"

#include <unordered_map>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace compnal {
namespace model {

template<typename RealType>
class XXZ_1D {
   
   using CRS = sparse_matrix::CRS<RealType>;
   
public:
   
   using ValueType = RealType;
   
   XXZ_1D() {
      SetOnsiteOperator();
   };
   
   explicit XXZ_1D(const int system_size): XXZ_1D() {
      SetSystemSize(system_size);
   }
   
   XXZ_1D(const int system_size, const int magnitude_2spin): XXZ_1D(system_size) {
      SetMagnitude2Spin(magnitude_2spin);
   }
   
   XXZ_1D(const int system_size, const utility::BoundaryCondition bc): XXZ_1D(system_size) {
      SetBoundaryCondition(bc);
   }
   
   XXZ_1D(const int system_size, const int magnitude_2spin, const utility::BoundaryCondition bc): XXZ_1D(system_size, magnitude_2spin) {
      SetBoundaryCondition(bc);
   }
   
   void SetSystemSize(const int system_size) {
      if (system_size <= 0) {
         std::stringstream ss;
         ss << "Error in " << __FUNCTION__ << std::endl;
         ss << "system_size must be a non-negative integer" << std::endl;
         ss << "system_size=" << system_size << "is not allowed" << std::endl;
         throw std::runtime_error(ss.str());
      }
      if (system_size_ != system_size) {
         system_size_ = system_size;
         flag_recalc_basis_ = true;
         flag_recalc_ham_   = true;
      }
   }
   
   void SetMagnitude2Spin(const int magnitude_2spin) {
      if (magnitude_2spin <= 0) {
         std::stringstream ss;
         ss << "Error in " << __FUNCTION__ << std::endl;
         ss << "Please set magnitude_2spin > 0" << std::endl;
         throw std::runtime_error(ss.str());
      }
      if (magnitude_2spin_ != magnitude_2spin) {
         magnitude_2spin_ = magnitude_2spin;
         dim_onsite_      = magnitude_2spin + 1;
         SetOnsiteOperator();
         flag_recalc_basis_ = true;
         flag_recalc_ham_   = true;
      }
   }
   
   void SetTotalSz(const int total_2sz) {
      if (total_2sz < -system_size_*magnitude_2spin_ || system_size_*magnitude_2spin_ < total_2sz) {
         std::stringstream ss;
         ss << "Error in " << __FUNCTION__  << std::endl;
         ss << "There is no target space specified by total_2sz = " << total_2sz << std::endl;
         ss << "Please set as follows:" << std::endl;
         ss << -system_size_*magnitude_2spin_ << " <= total_2sz <= " << system_size_*magnitude_2spin_;
         throw std::runtime_error(ss.str());
      }
      if (total_2sz_ != total_2sz) {
         total_2sz_ = total_2sz;
         flag_recalc_basis_ = true;
         flag_recalc_ham_   = true;
      }
   }
   
   void SetBoundaryCondition(const utility::BoundaryCondition bc) {
      boundary_condition_ = bc;
      flag_recalc_ham_ = true;
   }
   
   void SetJz(const std::vector<RealType> &J_z) {
      if (J_z_ != J_z) {
         J_z_ = J_z;
         flag_recalc_ham_ = true;
      }
   }
   
   void SetJz(const RealType J_z) {
      if (J_z_.size() == 0) {
         J_z_.push_back(J_z);
         flag_recalc_ham_ = true;
      }
      else if (J_z_[0] != J_z) {
         J_z_[0] = J_z;
         flag_recalc_ham_ = true;
      }
   }
   
   void SetJxy(const std::vector<RealType> &J_xy) {
      if (J_xy_ != J_xy) {
         J_xy_ = J_xy;
         flag_recalc_ham_ = true;
      }
   }
   
   void SetJxy(const RealType J_xy) {
      if (J_xy_.size() == 0) {
         J_xy_.push_back(J_xy);
         flag_recalc_ham_ = true;
      }
      else if (J_xy_[0] != J_xy) {
         J_xy_[0] = J_xy;
         flag_recalc_ham_ = true;
      }
   }
   
   void SetHz(const RealType h_z) {
      if (h_z_ != h_z) {
         h_z_ = h_z;
         onsite_operator_ham_ = CreateOnsiteOperatorHam(magnitude_2spin_, h_z_, D_z_);
         flag_recalc_ham_ = true;
      }
   }
   
   void SetDz(const RealType D_z) {
      if (D_z_ != D_z) {
         D_z_ = D_z;
         onsite_operator_ham_ = CreateOnsiteOperatorHam(magnitude_2spin_, h_z_, D_z_);
         flag_recalc_ham_ = true;
      }
   }
   
   void SetFlagRecalcBasis(const bool flag) {
      flag_recalc_basis_ = flag;
   }
   
   void SetFlagRecalcHam(const bool flag) {
      flag_recalc_ham_ = flag;
   }
   
   void PrintInfo() const {
      std::string bc = "None";
      if (boundary_condition_ == utility::BoundaryCondition::OBC) {
         bc = "OBC";
      }
      else if (boundary_condition_ == utility::BoundaryCondition::PBC) {
         bc = "PBC";
      }
      else if (boundary_condition_ == utility::BoundaryCondition::SSD) {
         bc = "SSD";
      }
      std::cout << "Print Heisenberg Model Infomation:" << std::endl;
      std::cout << "boundary_condition     = " << bc                      << std::endl;
      std::cout << "system_size            = " << system_size_            << std::endl;
      std::cout << "magnitute_2spin        = " << magnitude_2spin_        << std::endl;
      std::cout << "total_2sz              = " << total_2sz_              << std::endl;
      std::cout << "dim_target             = " << CalculateTargetDim()    << std::endl;
      std::cout << "dim_onsite             = " << dim_onsite_             << std::endl;
      std::cout << "num_conserved_quantity = " << num_conserved_quantity_ << std::endl;
      
      std::cout << "Print Heisenberg Interaction" << std::endl;
      std::cout << "Sz-Sz Interaction: J_z =" << std::endl;
      for (std::size_t i = 0; i < J_z_.size(); ++i) {
         std::cout << i + 1 << "-th neighber: " << J_z_.at(i) << std::endl;
      }
      std::cout << "Sx-Sx, Sy-Sy Interactions: J_xy =" << std::endl;
      for (std::size_t i = 0; i < J_xy_.size(); ++i) {
         std::cout << i + 1 << "-th neighber: " << J_xy_.at(i) << std::endl;
      }
      std::cout << "External Magnetic Fields for the z-direction: h_z =" << h_z_ << std::endl;
      std::cout << "Uniaxial Anisotropy for the z-direction: D_z =" << D_z_ << std::endl;
   }
   
   void PrintBasisOnsite() const {
      const double magnitude_spin = magnitude_2spin_/2.0;
      for (int row = 0; row < dim_onsite_; ++row) {
         std::cout << "row " << row << ": |Sz=" << magnitude_spin - row << ">" << std::endl;
      }
   }
   
   std::size_t CalculateTargetDim() const {
      const int max_total_2sz = system_size_*magnitude_2spin_;
      std::vector<std::vector<std::size_t>> dim(system_size_, std::vector<std::size_t>(max_total_2sz + 1));
      for (int s = -magnitude_2spin_; s <= magnitude_2spin_; s += 2) {
         dim[0][(s + magnitude_2spin_)/2] = 1;
      }
      for (int site = 1; site < system_size_; site++) {
         for (int s = -magnitude_2spin_; s <= magnitude_2spin_; s += 2) {
            for (int s_prev = -magnitude_2spin_*site; s_prev <= magnitude_2spin_*site; s_prev += 2) {
               const std::size_t a = dim[site    ][(s + s_prev + magnitude_2spin_*(site + 1))/2];
               const std::size_t b = dim[site - 1][(s_prev + magnitude_2spin_*site)/2];
               if (a >= UINT64_MAX - b) {
                  throw std::runtime_error("Overflow detected for sumation using uint64_t");
               }
               dim[site][(s + s_prev + magnitude_2spin_*(site + 1))/2] = a + b;
            }
         }
      }
      return dim[system_size_ - 1][(total_2sz_ + max_total_2sz)/2];
   }
   
   void GenerateBasis(std::vector<std::size_t> *basis, std::unordered_map<std::size_t, std::size_t> *basis_inv) const {
      if ((system_size_*magnitude_2spin_ - total_2sz_)%2 == 1) {
         std::stringstream ss;
         ss << "Error in " << __FUNCTION__ << std::endl;
         ss << "Invalid parameters (system_size or magnitude_spin or total_sz)" << std::endl;
         throw std::runtime_error(ss.str());
      }
      
      const int shifted_2sz = (system_size_*magnitude_2spin_ - total_2sz_)/2;
      const std::size_t dim_target = CalculateTargetDim();
      std::vector<std::vector<int>> partition_integers;
      utility::GenerateIntegerPartition(&partition_integers, shifted_2sz, magnitude_2spin_);
      
      std::vector<std::size_t> site_constant(system_size_);
      for (int site = 0; site < system_size_; ++site) {
         site_constant[site] = static_cast<std::size_t>(std::pow(dim_onsite_, site));
      }
      
      std::vector<std::size_t>().swap(*basis);
      
#ifdef _OPENMP
      const int num_threads = omp_get_max_threads();
      std::vector<std::vector<std::size_t>> temp_basis(num_threads);
      
      for (std::size_t i = 0; i < partition_integers.size(); ++i) {
         const bool condition1 = (0 < partition_integers[i].size()) && (partition_integers[i].size() <= system_size_);
         const bool condition2 = (partition_integers[i].size() == 0) && (shifted_2sz  == 0);
         if (condition1 || condition2) {
            
            for (std::size_t j = partition_integers[i].size(); j < system_size_; ++j) {
               partition_integers[i].push_back(0);
            }
            
            const std::size_t size = utility::CalculateNumCombination(partition_integers[i]);
            
#pragma omp parallel
            {
               const int thread_num = omp_get_thread_num();
               const std::size_t loop_begin = thread_num*size/num_threads;
               const std::size_t loop_end   = (thread_num + 1)*size/num_threads;
               
               std::vector<int> temp_partition_integer = partition_integers[i];
               utility::CalculateNthPermutation(&temp_partition_integer, loop_begin);
               
               for (std::size_t j = loop_begin; j < loop_end; ++j) {
                  std::size_t basis_onsite = 0;
                  for (std::size_t k = 0; k < partition_integers[i].size(); ++k) {
                     basis_onsite += temp_partition_integer[k]*site_constant[k];
                  }
                  temp_basis[thread_num].push_back(basis_onsite);
                  std::next_permutation(temp_partition_integer.begin(), temp_partition_integer.end());
               }
            }
         }
      }
      
      for (std::size_t i = 0; i < temp_basis.size(); ++i) {
         basis->insert(basis->end(), temp_basis[i].begin(), temp_basis[i].end());
         std::vector<std::size_t>().swap(temp_basis[i]);
      }
      
#else
      basis->reserve(dim_target);
      
      for (std::size_t i = 0; i < partition_integers.size(); ++i) {
         const bool condition1 = (0 < partition_integers[i].size()) && (partition_integers[i].size() <= system_size_);
         const bool condition2 = (partition_integers[i].size() == 0) && (shifted_2sz  == 0);
         if (condition1 || condition2) {
            
            for (std::size_t j = partition_integers[i].size(); j < system_size_; ++j) {
               partition_integers[i].push_back(0);
            }
            
            std::sort(partition_integers[i].begin(), partition_integers[i].end());
            
            do {
               std::size_t basis_onsite = 0;
               for (std::size_t j = 0; j < partition_integers[i].size(); ++j) {
                  basis_onsite += partition_integers[i][j]*site_constant[j];
               }
               basis->push_back(basis_onsite);
            } while (std::next_permutation(partition_integers[i].begin(), partition_integers[i].end()));
         }
      }
#endif
      
      if (basis->size() != dim_target) {
         std::stringstream ss;
         ss << "Unknown error detected in " << __FUNCTION__ << std::endl;
         throw std::runtime_error(ss.str());
      }
      
      std::sort(basis->begin(), basis->end());
      basis_inv->clear();
      
      for (std::size_t i = 0; i < basis->size(); ++i) {
         (*basis_inv)[(*basis)[i]] = i;
      }
      flag_recalc_basis_ = false;
   }
   
   inline utility::BoundaryCondition GetBoundaryCondition()    const { return boundary_condition_;     }
   inline int                        GetSystemSize()           const { return system_size_;            }
   inline int                        GetDimOnsite()            const { return dim_onsite_;             }
   inline double                     GetMagnitude2Spin()       const { return magnitude_2spin_;        }
   inline double                     GetTotal2Sz()             const { return total_2sz_;              }
   inline int                        GetNumConservedQuantity() const { return num_conserved_quantity_; }
   
   inline const CRS &GetOnsiteOperatorHam() const { return onsite_operator_ham_; }
   inline const CRS &GetOnsiteOperatorSx () const { return onsite_operator_sx_ ; }
   inline const CRS &GetOnsiteOperatoriSy() const { return onsite_operator_isy_; }
   inline const CRS &GetOnsiteOperatorSz () const { return onsite_operator_sz_ ; }
   inline const CRS &GetOnsiteOperatorSp () const { return onsite_operator_sp_ ; }
   inline const CRS &GetOnsiteOperatorSm () const { return onsite_operator_sm_ ; }
   
   inline const std::vector<RealType> &GetJz () const { return J_z_ ; }
   inline const std::vector<RealType> &GetJxy() const { return J_xy_; }
   
   inline RealType GetHz() const { return h_z_; }
   inline RealType GetDz() const { return D_z_; }
   
   inline bool GetFlagRecalcBasis() const { return flag_recalc_basis_; }
   inline bool GetFlagRecalcHam  () const { return flag_recalc_ham_; }

   static CRS CreateOnsiteOperatorSx(const int magnitude_2spin) {
      const double magnitude_spin = 0.5*magnitude_2spin;
      const int    dim_onsite     = magnitude_2spin + 1;
      CRS matrix(dim_onsite, dim_onsite);
      int a = 0;
      int b = 1;
      
      matrix.val.push_back(0.5*std::sqrt((magnitude_spin + 1)*(a + b + 1) - (a + 1)*(b + 1)) );
      matrix.col.push_back(b);
      matrix.row[1] = matrix.col.size();
      
      for (int row = 1; row < dim_onsite - 1; ++row) {
         a = row;
         b = row - 1;
         matrix.val.push_back(0.5*std::sqrt((magnitude_spin + 1)*(a + b + 1) - (a + 1)*(b + 1)) );
         matrix.col.push_back(b);
         
         a = row;
         b = row + 1;
         matrix.val.push_back(0.5*std::sqrt((magnitude_spin + 1)*(a + b + 1) - (a + 1)*(b + 1)) );
         matrix.col.push_back(b);
         matrix.row[row + 1] = matrix.col.size();
      }
      
      a = dim_onsite - 1;
      b = dim_onsite - 2;
      
      matrix.val.push_back(0.5*std::sqrt((magnitude_spin + 1)*(a + b + 1) - (a + 1)*(b + 1)) );
      matrix.col.push_back(b);
      matrix.row[dim_onsite] = matrix.col.size();
      
      return matrix;
   }
   
   static CRS CreateOnsiteOperatoriSy(const int magnitude_2spin) {
      const double magnitude_spin = 0.5*magnitude_2spin;
      const int    dim_onsite     = magnitude_2spin + 1;
      CRS matrix(dim_onsite, dim_onsite);
      int a = 0;
      int b = 1;
      
      matrix.val.push_back(0.5*std::sqrt( (magnitude_spin + 1)*(a + b + 1) - (a + 1)*(b + 1) ) );
      matrix.col.push_back(b);
      matrix.row[1] = matrix.col.size();
      
      for (int row = 1; row < dim_onsite - 1; ++row) {
         a = row;
         b = row - 1;
         matrix.val.push_back(-0.5*std::sqrt( (magnitude_spin + 1)*(a + b + 1) - (a + 1)*(b + 1) ) );
         matrix.col.push_back(b);
         
         a = row;
         b = row + 1;
         matrix.val.push_back(0.5*std::sqrt( (magnitude_spin + 1)*(a + b + 1) - (a + 1)*(b + 1) ) );
         matrix.col.push_back(b);
         
         matrix.row[row + 1] = matrix.col.size();
      }
      
      a = dim_onsite - 1;
      b = dim_onsite - 2;
      
      matrix.val.push_back(-0.5*std::sqrt( (magnitude_spin + 1)*(a + b + 1) - (a + 1)*(b + 1) ) );
      matrix.col.push_back(b);
      matrix.row[dim_onsite] = matrix.col.size();
      
      return matrix;
   }
   
   static CRS CreateOnsiteOperatorSz(const int magnitude_2spin) {
      const double magnitude_spin = 0.5*magnitude_2spin;
      const int    dim_onsite     = magnitude_2spin + 1;
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
   
   static CRS CreateOnsiteOperatorSzSz(const int magnitude_2spin) {
      const double magnitude_spin = 0.5*magnitude_2spin;
      const int    dim_onsite     = magnitude_2spin + 1;
      CRS matrix(dim_onsite, dim_onsite);
      
      for (int row = 0; row < dim_onsite; ++row) {
         const RealType val = magnitude_spin - row;
         if (val != 0.0) {
            matrix.val.push_back(val*val);
            matrix.col.push_back(row);
         }
         matrix.row[row + 1] = matrix.col.size();
      }
      return matrix;
   }
   
   static CRS CreateOnsiteOperatorSp(const int magnitude_2spin) {
      const double magnitude_spin = 0.5*magnitude_2spin;
      const int    dim_onsite     = magnitude_2spin + 1;
      CRS matrix(dim_onsite, dim_onsite);
      for (int row = 1; row < dim_onsite; ++row) {
         matrix.val.push_back(std::sqrt((magnitude_spin + 1)*2*row - row*(row + 1)));
         matrix.col.push_back(row);
         matrix.row[row] = matrix.col.size();
      }
      return matrix;
   }
   
   static CRS CreateOnsiteOperatorSm(const int magnitude_2spin) {
      const double magnitude_spin = 0.5*magnitude_2spin;
      const int    dim_onsite     = magnitude_2spin + 1;
      CRS matrix(dim_onsite, dim_onsite);
      for (int row = 1; row < dim_onsite; ++row) {
         matrix.val.push_back(std::sqrt((magnitude_spin + 1)*2*row - row*(row + 1)));
         matrix.col.push_back(row - 1);
         matrix.row[row] = matrix.col.size();
      }
      return matrix;
   }
   
   static CRS CreateOnsiteOperatorHam(const int magnitude_2spin, const RealType h_z = 0.0, const RealType D_z = 0.0) {
      const double magnitude_spin = 0.5*magnitude_2spin;
      const int    dim_onsite     = magnitude_2spin + 1;
      CRS matrix(dim_onsite, dim_onsite);
      
      for (int row = 0; row < dim_onsite; ++row) {
         const RealType val = h_z*(magnitude_spin - row) + D_z*D_z*(magnitude_spin - row);
         if (val != 0.0) {
            matrix.val.push_back(val);
            matrix.col.push_back(row);
         }
         matrix.row[row + 1] = matrix.col.size();
      }
      return matrix;
   }
   
   
private:
   CRS onsite_operator_ham_;
   CRS onsite_operator_sx_;
   CRS onsite_operator_isy_;
   CRS onsite_operator_sz_;
   CRS onsite_operator_sp_;
   CRS onsite_operator_sm_;
   
   utility::BoundaryCondition boundary_condition_ = utility::BoundaryCondition::OBC;
   
   int system_size_     = 1;
   int total_2sz_       = 0;
   int dim_onsite_      = 2;
   int magnitude_2spin_ = 1;
   
   std::vector<RealType> J_z_  = {1.0};
   std::vector<RealType> J_xy_ = {1.0};
   RealType h_z_ = 0.0;
   RealType D_z_ = 0.0;
   
   const int num_conserved_quantity_ = 1;
   
   bool flag_recalc_basis_ = true;
   bool flag_recalc_ham_   = true;
   
   void SetOnsiteOperator() {
      onsite_operator_ham_ = CreateOnsiteOperatorHam(magnitude_2spin_);
      onsite_operator_sx_  = CreateOnsiteOperatorSx (magnitude_2spin_);
      onsite_operator_isy_ = CreateOnsiteOperatoriSy(magnitude_2spin_);
      onsite_operator_sz_  = CreateOnsiteOperatorSz (magnitude_2spin_);
      onsite_operator_sp_  = CreateOnsiteOperatorSp (magnitude_2spin_);
      onsite_operator_sm_  = CreateOnsiteOperatorSm (magnitude_2spin_);
   }
   
};


}
}


#endif /* COMPNAL_MODEL_XXZ_1D_HPP_ */
