/*
 * @Author: Zhang Jun ewalker@live.cn
 * @Date: 2022-09-25 12:12:57
 * @LastEditors: Zhang Jun ewalker@live.cn
 * @LastEditTime: 2022-09-25 15:57:26
 * @FilePath: /paddle/Users/apple/Downloads/mydev/model_parser/common/global.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
// Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "common/global.h"

namespace model_parser {

Global::Global() {}

// mlir::MLIRContext* Global::context = nullptr;

// mlir::MLIRContext* Global::getMLIRContext() {
//   if (nullptr == context) {
//     context = new mlir::MLIRContext();
//   }
//   return context;
// }

}  // namespace model_parser
