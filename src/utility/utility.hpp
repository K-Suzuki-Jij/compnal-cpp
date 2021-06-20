//
//  utility.hpp
//  compnal
//
//  Created by Kohei Suzuki on 2021/06/17.
//

#ifndef utility_hpp
#define utility_hpp

#include <vector>

namespace compnal {
namespace utility {

template<typename T1, typename T2>
void QuickSort(std::vector<T1> *array_comp, std::vector<T2> *array2, const int64_t left, const int64_t right) {
   
   if (right - left <= 1) {
      return;
   }
   
   const int64_t index_pivot = (left + right)/2;
   const T1      val_pivot   = (*array_comp)[index_pivot];
   
   std::swap((*array_comp)[index_pivot], (*array_comp)[right - 1]);
   std::swap((*array2)    [index_pivot], (*array2)    [right - 1]);
   
   int64_t index = left;
   for (int64_t i = left; i < right - 1; ++i) {
      if ((*array_comp)[i] < val_pivot) {
         std::swap((*array_comp)[index], (*array_comp)[i]);
         std::swap((*array2)    [index], (*array2)    [i]);
         ++index;
      }
   }

   std::swap((*array_comp)[index], (*array_comp)[right - 1]);
   std::swap((*array2)    [index], (*array2)    [right - 1]);
   
   QuickSort(array_comp, array2, left     , index);
   QuickSort(array_comp, array2, index + 1, right);
   
}





} //utility
} //compnel


#endif /* utility_hpp */