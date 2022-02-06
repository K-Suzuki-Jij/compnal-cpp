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
//  Created by Kohei Suzuki on 2022/01/23.
//

#ifndef COMPNAL_TEST_MODEL_BASE_U1_SPIN_ELECTRON_1D_HPP_
#define COMPNAL_TEST_MODEL_BASE_U1_SPIN_ELECTRON_1D_HPP_

#include "../../src/model/base_u1_spin_electron_1d.hpp"
#include "../include/all.hpp"
#include <gtest/gtest.h>

namespace compnal {
namespace test {

TEST(ModelBaseU1SpinElectron1D, ConstructorDefault) {
   model::BaseU1SpinElectron_1D<double> model;
   TestSpinOneHalf(model);
   
   EXPECT_EQ(model.GetSystemSize()    , 0  );
   EXPECT_EQ(model.GetTotalSz()       , 0.0);
   EXPECT_EQ(model.GetTotalElectron() , 0  );
   EXPECT_EQ(model.GetMagnitudeLSpin(), 0.5);
   EXPECT_EQ(model.GetCalculatedEigenvectorSet(), std::unordered_set<int>());
   
   ExpectEQ(model.GetBases(), std::unordered_map<std::pair<int, int>, std::vector<std::int64_t>, compnal::utility::PairHash>{});
   ExpectEQ(model.GetBasesInv(), std::unordered_map<std::pair<int, int>, std::unordered_map<std::int64_t, std::int64_t>, compnal::utility::PairHash>{});
   
}

}
}

#endif /* COMPNAL_TEST_MODEL_BASE_U1_SPIN_1D_HPP_ */
   
