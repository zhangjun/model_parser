#include "examples/paddle/framework.pb.h"
#include <fstream>
#include <iostream>
#include <memory>

bool IsAttrVar(const paddle::framework::proto::OpDesc &op,
               const int64_t &attr_id) {
  return op.attrs(attr_id).has_var_name() ||
         op.attrs(attr_id).vars_name_size() > 0;
}

bool LoadProgram(const void *model_buffer, int model_size) {
  std::shared_ptr<paddle::framework::proto::ProgramDesc> prog =
      std::make_shared<paddle::framework::proto::ProgramDesc>();
  if (!prog->ParseFromArray(model_buffer, model_size)) {
    std::cout << "Failed to parse PaddlePaddle model from memory buffer."
              << std::endl;
    return false;
  }
  return true;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << argv[0] << " model_file." << std::endl;
    return 0;
  }
  std::shared_ptr<paddle::framework::proto::ProgramDesc> prog =
      std::make_shared<paddle::framework::proto::ProgramDesc>();

  std::string model = argv[1];
  std::ifstream fin(model, std::ios::in | std::ios::binary);
  if (!fin.is_open()) {
    std::cout << "Failed to read model file: " << model
              << ", please make sure your model file or file path is valid."
              << std::endl;
    return false;
  }

  std::string contents;
  fin.seekg(0, std::ios::end);
  contents.clear();
  contents.resize(fin.tellg());
  fin.seekg(0, std::ios::beg);
  fin.read(&(contents.at(0)), contents.size());
  fin.close();

  if (!prog->ParseFromString(contents)) {
    std::cout << "Failed to parse paddlepaddle model from read content."
              << std::endl;
    return false;
  }
  int num_block = prog->blocks_size();
  std::cout << "num_block: " << num_block << std::endl;
  for (int block_idx = 0; block_idx < num_block; block_idx++) {
    int vars_num = prog->blocks(block_idx).vars_size();
    int op_num = prog->blocks(block_idx).ops_size();
    std::cout << "### block idx: " << block_idx << ", op num: " << op_num
              << ", var num: " << vars_num << std::endl;
    for (int var_idx = 0; var_idx < vars_num; var_idx++) {
      if (prog->blocks(block_idx).vars(var_idx).persistable()) {
        std::cout << prog->blocks(block_idx).vars(var_idx).name() << " ";
      }
    }
    std::cout << std::endl;
    for (int op_idx = 0; op_idx < op_num; op_idx++) {
      auto &op = prog->blocks(block_idx).ops(op_idx);
      int input_num = op.inputs_size(), output_num = op.outputs_size();
      std::cout << "op idx: " << op_idx << ", op_type: " << op.type()
                << ", input num: " << input_num
                << ", output_num: " << output_num << std::endl;
      for (auto i = 0; i < op.inputs_size(); ++i) {
        std::cout << "  Inputs: " << op.inputs(i).parameter() << ", ";
        for (auto j = 0; j < op.inputs(i).arguments_size(); ++j) {
          std::cout << op.inputs(i).arguments(j) << " ";
        }
        std::cout << std::endl;
      }
      //   op.inputs(i).arguments(j)
      for (auto i = 0; i < op.attrs_size(); ++i) {
        // if (op.attrs(i).name() == name && IsAttrVar(op, i)) {
        //     break;
        // }
      }
    }
  }
  return true;
}