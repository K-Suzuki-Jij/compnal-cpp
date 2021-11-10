//
//  test_model_xxz_1d.hpp
//  compnal
//
//  Created by Kohei Suzuki on 2021/11/06.
//

#ifndef TEST_MODEL_XXZ_1D_HPP_
#define TEST_MODEL_XXZ_1D_HPP_

#include "../../src/model/all.hpp"
#include <gtest/gtest.h>

TEST(XXZ_1D, Basic) {
   
   compnal::model::XXZ_1D<double> model;
   model.GetOnsiteOperatorSz().PrintMatrix("Sz");
   model.GetOnsiteOperatorSz().PrintInfo("Sz");
   model.GetOnsiteOperatorSp().PrintMatrix("Sp");
   model.GetOnsiteOperatorSp().PrintInfo("Sp");
   model.GetOnsiteOperatorSm().PrintMatrix("Sm");
   model.GetOnsiteOperatorSm().PrintInfo("Sm");
   
}


#endif /* TEST_MODEL_XXZ_1D_HPP_ */
