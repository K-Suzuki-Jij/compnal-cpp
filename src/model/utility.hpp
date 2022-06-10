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
//  Created by Kohei Suzuki on 2022/01/26.
//

#ifndef COMPNAL_MODEL_UTILITY_HPP_
#define COMPNAL_MODEL_UTILITY_HPP_

#include <cmath>

namespace compnal {
namespace model {

enum BoundaryCondition {

   OBC = 0,
   PBC = 1,
   SSD = 2

};

}
}  // namespace compnal

#endif /* COMPNAL_MODEL_UTILITY_HPP_ */
