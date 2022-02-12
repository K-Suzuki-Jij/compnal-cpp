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
//  Created by Kohei Suzuki on 2021/11/07.
//

#ifndef COMPNAL_UTILITY_INTEGER_HPP_
#define COMPNAL_UTILITY_INTEGER_HPP_

#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>

namespace compnal {
namespace utility {

//! @brief Generate partitions of a positive integer.
//! @tparam IntegerType Integer type
//! @param partitioned_number A positive integer to be partitioned.
//! @param max_number The maximum number in the partition list.
//! @param max_size The maximum list size.
template<typename IntegerType>
std::vector<std::vector<IntegerType>> GenerateIntegerPartition(const IntegerType partitioned_number,
                                                               IntegerType max_number,
                                                               const IntegerType max_size) {
   static_assert(std::is_integral<IntegerType>::value, "Template parameter IntegerType must be integer type");
   
   if (partitioned_number <= 0 || max_number <= 0 || max_size <= 0) {
      std::stringstream ss;
      ss << "Error at " << __LINE__ << " in " << __func__ << " in "<< __FILE__ << std::endl;
      ss << "Invalid input parameters" << std::endl;
      throw std::runtime_error(ss.str());
   }
   
   auto generate_next = [](const std::vector<IntegerType> &vec) -> std::vector<IntegerType> {
      std::int64_t size = static_cast<std::int64_t>(vec.size());
      std::vector<IntegerType> out = vec;
      for (std::int64_t i = size - 1; i >= 0; --i) {
         if (vec[i] > 1) {
            if (i + 1 < size) {
               if (vec[i] - 1 >= vec[i + 1] + 1) {
                  out[i]--;
                  out[i + 1]++;
                  return out;
               }
               else {
                  out[i]--;
                  out.push_back(1);
                  return out;
               }
            }
            else {
               out[i]--;
               out.push_back(1);
               return out;
            }
         }
      }
      return std::vector<IntegerType>();
   };
   
   std::vector<std::vector<IntegerType>> out;
   
   if (max_number >= partitioned_number) {
      max_number = partitioned_number;
      out.push_back({max_number});
   }
   else {
      std::vector<IntegerType> temp = {max_number};
      IntegerType rem = partitioned_number - max_number;
      while (rem != 0) {
         if (rem <= max_number) {
            temp.push_back(rem);
            rem = 0;
         }
         else {
            temp.push_back(max_number);
            rem -= max_number;
         }
      }
      if (static_cast<IntegerType>(temp.size()) > max_size) {
         return std::vector<std::vector<IntegerType>>();
      }
      else {
         out.push_back(temp);
      }
   }
   
   while (true) {
      auto next_partition = generate_next(out.back());
      if (0 < next_partition.size() && static_cast<IntegerType>(next_partition.size()) <= max_size) {
         out.push_back(next_partition);
      }
      else {
         break;
      }
   }
   
   return out;
   
}

//! @brief Generate partitions of a positive integer.
//! @tparam IntegerType Integer type
//! @param partitioned_number A positive integer to be partitioned.
//! @param max_number The maximum number in the partition list.
template<typename IntegerType>
std::vector<std::vector<IntegerType>> GenerateIntegerPartition(const IntegerType partitioned_number,
                                                               const IntegerType max_number) {
   return GenerateIntegerPartition(partitioned_number, max_number, partitioned_number);
}


//! @brief Calculate binomial coefficient
//! @tparam IntegerType Integer type
//! @param n Non-negative integer \f$ n \f$ in \f$ \frac{n!}{k!(n-k)!} \f$.
//! @param k Non-negative integer \f$ k \f$ in \f$ \frac{n!}{k!(n-k)!} \f$.
template<typename IntegerType>
std::int64_t CalculateBinomialCoefficient(IntegerType n, const IntegerType k) {
   static_assert(std::is_integral<IntegerType>::value, "Template parameter IntegerType must be integer type");
   
   if (n < 0 || k < 0) {
      std::stringstream ss;
      ss << "Error at " << __LINE__ << " in " << __func__ << " in "<< __FILE__ << std::endl;
      ss << "Invalid input parameters" << std::endl;
      throw std::runtime_error(ss.str());
   }
   
   std::int64_t r = 1;
   
   for (IntegerType d = 1; d <= k; d++) {
       r *= n--;
       r /= d;
   }
   
   return r;
   
}

//! @brief Generate binomial coefficients \f$ \frac{n!}{k!(n-k)!} \f$ for \f$ n \f$ and all \f$ 0 <= k <= n\f$.
//! @tparam IntegerType Integer type.
//! @param n Non-negative integer \f$ n \f$.
template<typename IntegerType>
std::vector<std::vector<std::int64_t>> GenerateBinomialTable(const IntegerType n) {
   static_assert(std::is_integral<IntegerType>::value, "Template parameter IntegerType must be integer type");
   
   std::vector<std::vector<std::int64_t>> vec(n + 1, std::vector<std::int64_t>(n + 1));
   for (IntegerType i = 0; i <= n; ++i) {
      for (IntegerType j = 0; j <= i; j++) {
         if (j == 0 || j == i) {
            vec[i][j] = 1;
         }
         else {
            vec[i][j] = vec[i - 1][j - 1] + vec[i - 1][j];
         }
      }
   }
   return vec;
}

//! @brief Calculate the number of permutations from list.
//! @n For example, all the possible permutations for {1, 1, 2} are
//! @n {1, 1, 2}, {1, 2, 1}, {2, 1, 1}.
//! @n Thus, this function return the value 3.
//! @tparam T Integer type or string type.
//! @param list The list.
template<typename T>
std::int64_t CalculateNumPermutation(const std::vector<T> &list) {
   static_assert(!std::is_floating_point<T>::value, "Template parameter T must not be floating point");
   
   std::unordered_map<T, std::int64_t> u_map;
   
   for (const auto &it: list) {
      u_map[it]++;
   }
   
   std::int64_t result = 1;
   std::int64_t size_list = static_cast<std::int64_t>(list.size());
   
   for (const auto &it: u_map) {
      result *= CalculateBinomialCoefficient(size_list, it.second);
      size_list -= it.second;
   }
   
   return result;
   
}

//! @brief Calculate \f$ n\f$ -th permutation of the list.
//! @n For example, all the possible permutations for {1, 1, 2} are
//! @n {1, 1, 2}, {1, 2, 1}, {2, 1, 1}.
//! @n Thus, this function GenerateNthPermutation({1, 1, 2}, 2) returns {1, 2, 1}.
//! @tparam T Integer type or string type.
//! @param list The list.
//! @param n Non-negative integer \f$ n\f$.
template<typename T>
std::vector<T> GenerateNthPermutation(const std::vector<T> &list, const std::int64_t n) {
   static_assert(!std::is_floating_point<T>::value, "Template parameter T must not be floating point");
   
   if (n <= 0) {
      std::stringstream ss;
      ss << "Error at " << __LINE__ << " in " << __func__ << " in "<< __FILE__ << std::endl;
      ss << "Invalid input parameters" << std::endl;
      throw std::runtime_error(ss.str());
   }
   
   std::int64_t size_list = static_cast<std::int64_t>(list.size());
   std::int64_t rem = n;
   std::int64_t num_perm = CalculateNumPermutation(list);
   
   if (n > num_perm) {
      std::stringstream ss;
      ss << "Error at " << __LINE__ << " in " << __func__ << " in "<< __FILE__ << std::endl;
      ss << "Invalid input parameters" << std::endl;
      throw std::runtime_error(ss.str());
   }
   
   std::map<T, std::int64_t> map;
   
   for (const auto &it: list) {
      map[it]++;
   }
   
   std::vector<T> out;
   
   while (true) {
      std::size_t negative_count = 0;
      for (auto &&it: map) {
         if (it.second > 0) {
            const std::int64_t count = (num_perm*it.second)/size_list;
            if (rem <= count) {
               out.push_back(it.first);
               num_perm = count;
               it.second--;
               size_list--;
               break;
            }
            rem -= count;
         }
         else {
            negative_count++;
         }
      }
      if (negative_count == map.size()) {
         break;
      }
   }
   
   return out;
   
}

}
}

#endif /* COMPNAL_UTILITY_INTEGER_HPP_ */
