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
//  Created by Kohei Suzuki on 2022/01/08.
//

#ifndef COMPNAL_TEST_MODEL_BASE_U1_SPIN_HPP_
#define COMPNAL_TEST_MODEL_BASE_U1_SPIN_HPP_

#include "../../src/model/base_u1_spin.hpp"
//#include "../include/all.hpp"
#include <gtest/gtest.h>

namespace compnal {
namespace test {

TEST(ModelBaseU1Spin, ConstructorDefault) {
   model::BaseU1Spin<double> model;

   EXPECT_EQ(model.GetDimOnsite(), 2);
   EXPECT_EQ(model.GetMagnitudeSpin(), 0.5);
   
   const sparse_matrix::CRS<double> ref_sp ({{+0.0, +1.0}, {+0.0, +0.0}});
   const sparse_matrix::CRS<double> ref_sm ({{+0.0, +0.0}, {+1.0, +0.0}});
   const sparse_matrix::CRS<double> ref_sx ({{+0.0, +0.5}, {+0.5, +0.0}});
   const sparse_matrix::CRS<double> ref_isy({{+0.0, +0.5}, {-0.5, +0.0}});
   const sparse_matrix::CRS<double> ref_sz ({{+0.5, +0.0}, {+0.0, -0.5}});
   
   EXPECT_EQ(model.GetOnsiteOperatorSp() , ref_sp );
   EXPECT_EQ(model.GetOnsiteOperatorSm() , ref_sm );
   EXPECT_EQ(model.GetOnsiteOperatorSx() , ref_sx );
   EXPECT_EQ(model.GetOnsiteOperatoriSy(), ref_isy);
   EXPECT_EQ(model.GetOnsiteOperatorSz() , ref_sz );
}

TEST(ModelBaseU1Spin, ConstructorSystemSize) {
   model::BaseU1Spin<long double> model(1);
   
   EXPECT_EQ(model.GetDimOnsite(), 3);
   EXPECT_EQ(model.GetMagnitudeSpin(), 1);
  
   const sparse_matrix::CRS<long double> ref_sp ({
      {+0.0, +std::sqrt(2), +0.0},
      {+0.0, +0.0, +std::sqrt(2)},
      {+0.0, +0.0, +0.0}
   });
   
   const sparse_matrix::CRS<long double> ref_sm ({
      {+0.0, +0.0, +0.0},
      {+std::sqrt(2), +0.0, +0.0},
      {+0.0, +std::sqrt(2), +0.0}
   });
   
   const sparse_matrix::CRS<long double> ref_sx ({
      {+0.0, +1.0/std::sqrt(2), +0.0},
      {+1.0/std::sqrt(2), +0.0, +1.0/std::sqrt(2)},
      {+0.0, +1.0/std::sqrt(2), +0.0}
   });
   
   const sparse_matrix::CRS<long double> ref_isy ({
      {+0.0, +1.0/std::sqrt(2), +0.0},
      {-1.0/std::sqrt(2), +0.0, +1.0/std::sqrt(2)},
      {+0.0, -1.0/std::sqrt(2), +0.0}
   });
   
   const sparse_matrix::CRS<long double> ref_sz ({
      {+1.0, +0.0, +0.0},
      {+0.0, +0.0, +0.0},
      {+0.0, +0.0, -1.0}
   });
   
   EXPECT_EQ(model.GetOnsiteOperatorSp() , ref_sp );
   EXPECT_EQ(model.GetOnsiteOperatorSm() , ref_sm );
   EXPECT_EQ(model.GetOnsiteOperatorSx() , ref_sx );
   //EXPECT_EQ(model.GetOnsiteOperatoriSy(), ref_isy);
   EXPECT_EQ(model.GetOnsiteOperatorSz() , ref_sz );
   EXPECT_EQ(0.99999999999999999, 1.0);
   //EXPECT_PRED_FORMAT3(::testing::internal::EqHelper::Compare, model.GetOnsiteOperatorSx(), ref_sx, 0.001);
}


/*
TEST(ModelBaseU1Spin, ConstructorSystemSizeSpin) {
   model::BaseU1Spin<double> model(10, 1);
   TestSpinOne(model);
   EXPECT_EQ(model.GetSystemSize(), 10);
   EXPECT_EQ(model.GetTotalSz(), 0_hi);
   EXPECT_EQ(model.GetCalculatedEigenvectorSet(), std::unordered_set<int>());
}

TEST(ModelBaseU1Spin, ConstructorSystemSizeSpinTotalSz) {
   model::BaseU1Spin<double> model(10, 1, 1);
   TestSpinOne(model);
   EXPECT_EQ(model.GetSystemSize(), 10);
   EXPECT_EQ(model.GetTotalSz(), 1_hi);
   EXPECT_EQ(model.GetCalculatedEigenvectorSet(), std::unordered_set<int>());
}

TEST(ModelBaseU1Spin, SetSystemSize) {
   model::BaseU1Spin<double> model;
   model.SetSystemSize(5);
   EXPECT_EQ(model.GetSystemSize(), 5);
}

TEST(ModelBaseU1Spin, SetTotalSz) {
   model::BaseU1Spin<double> model;
   model.SetTotalSz(2_hi);
   EXPECT_EQ(model.GetTotalSz(), 2_hi);
   EXPECT_THROW(model.SetTotalSz(1.9_hi), std::runtime_error);
}

TEST(ModelBaseU1Spin, SetMagnitudeSpin) {
   model::BaseU1Spin<double> model;
   model.SetMagnitudeSpin(1.5_hi);
   EXPECT_EQ(model.GetMagnitudeSpin(), 1.5_hi);
   EXPECT_THROW(model.SetMagnitudeSpin(1.3_hi), std::runtime_error);
}

TEST(ModelBaseU1Spin, isValidQNumber) {
   EXPECT_TRUE(model::BaseU1Spin<double>::isValidQNumber(10, 0.5_hi, +5.0_hi));
   EXPECT_TRUE(model::BaseU1Spin<double>::isValidQNumber(10, 0.5_hi, +0.0_hi));
   EXPECT_TRUE(model::BaseU1Spin<double>::isValidQNumber(10, 0.5_hi, -5.0_hi));
   EXPECT_TRUE(model::BaseU1Spin<double>::isValidQNumber(9 , 0.5_hi, -4.5_hi));
   EXPECT_TRUE(model::BaseU1Spin<double>::isValidQNumber(9 , 0.5_hi, -0.5_hi));
   EXPECT_TRUE(model::BaseU1Spin<double>::isValidQNumber(9 , 0.5_hi, +4.5_hi));

   EXPECT_TRUE(model::BaseU1Spin<double>::isValidQNumber(10, 1.0_hi, +10.0_hi));
   EXPECT_TRUE(model::BaseU1Spin<double>::isValidQNumber(10, 1.0_hi, +0.0_hi ));
   EXPECT_TRUE(model::BaseU1Spin<double>::isValidQNumber(10, 1.0_hi, -10.0_hi));
   EXPECT_TRUE(model::BaseU1Spin<double>::isValidQNumber(9 , 1.0_hi, -9.0_hi ));
   EXPECT_TRUE(model::BaseU1Spin<double>::isValidQNumber(9 , 1.0_hi, -1.0_hi ));
   EXPECT_TRUE(model::BaseU1Spin<double>::isValidQNumber(9 , 1.0_hi, +9.0_hi ));
   
   EXPECT_FALSE(model::BaseU1Spin<double>::isValidQNumber(10, 0.5_hi, +5.5_hi));
   EXPECT_FALSE(model::BaseU1Spin<double>::isValidQNumber(10, 0.5_hi, +0.5_hi));
   EXPECT_FALSE(model::BaseU1Spin<double>::isValidQNumber(10, 0.5_hi, -5.5_hi));
   EXPECT_FALSE(model::BaseU1Spin<double>::isValidQNumber(9 , 0.5_hi, -5.0_hi));
   EXPECT_FALSE(model::BaseU1Spin<double>::isValidQNumber(9 , 0.5_hi, -0.0_hi));
   EXPECT_FALSE(model::BaseU1Spin<double>::isValidQNumber(9 , 0.5_hi, +4.0_hi));
}

TEST(ModelBaseU1Spin, CalculateTargetDim) {
   //Spin-1/2
   EXPECT_EQ(model::BaseU1Spin<double>::CalculateTargetDim(0, 0.5_hi, +0.0_hi), 0);
   EXPECT_EQ(model::BaseU1Spin<double>::CalculateTargetDim(1, 0.5_hi, +0.5_hi), 1);
   EXPECT_EQ(model::BaseU1Spin<double>::CalculateTargetDim(2, 0.5_hi, +0.0_hi), 2);
   EXPECT_EQ(model::BaseU1Spin<double>::CalculateTargetDim(3, 0.5_hi, +0.5_hi), 3);
   EXPECT_EQ(model::BaseU1Spin<double>::CalculateTargetDim(4, 0.5_hi, +0.0_hi), 6);
   EXPECT_EQ(model::BaseU1Spin<double>::CalculateTargetDim(4, 0.5_hi, +2.0_hi), 1);
   EXPECT_EQ(model::BaseU1Spin<double>::CalculateTargetDim(4, 0.5_hi, -2.0_hi), 1);
}
*/
} //namespace test
} //namespace compnal

#endif /* COMPNAL_TEST_MODEL_BASE_U1_SPIN_HPP_ */