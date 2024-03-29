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

#include "paddle/scope.h"

#include "common/common.h"

namespace model_parser {
namespace paddle {

_Variable* Scope::FindVar(const std::string& name) const {
  auto it = data_.find(name);
  if (it != data_.end()) return it->second.get();
  return nullptr;
}

Tensor Scope::GetTensor(const std::string& name) const {
  CheckVarNameValid(name);
  auto* var = FindVar(name);
  CHECK(var) << "No variable called [" << name << "] found";
  return var->get<Tensor>();
}

std::vector<std::string> Scope::var_names() const {
  std::vector<std::string> names;
  for (auto& item : data_) {
    names.push_back(item.first);
  }
  return names;
}

}  // namespace paddle
}  // namespace model_parser
