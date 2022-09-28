#include "examples/paddle/framework.pb.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <cassert>

bool IsAttrVar(const paddle::framework::proto::OpDesc &op,
               const int64_t &attr_id) {
  return op.attrs(attr_id).has_var_name() ||
         op.attrs(attr_id).vars_name_size() > 0;
}

bool ExistsDumplicateTensorName(const std::shared_ptr<paddle::framework::proto::ProgramDesc>& prog) {
  std::set<std::string> names;
  for (auto i = 0; i < prog->blocks(0).ops_size(); ++i) {
    auto& op = prog->blocks(0).ops(i);
    for (auto j = 0; j < op.outputs_size(); ++j) {
      for (auto k = 0; k < op.outputs(j).arguments_size(); ++k) {
        if (op.type() == "fetch") {
          continue;
        }
        if (names.find(op.outputs(j).arguments(k)) != names.end()) {
          std::cout << "There's dumplicate output name: "
                      << op.outputs(j).arguments(k)
                      << " in this model, not supported yet." << std::endl;
          return true;
        }
        names.insert(op.outputs(j).arguments(k));
      }
    }
  }
  return false;
}

enum DataType {
  BOOL,
  INT16,
  INT32,
  INT64,
  FP16,
  FP32,
  FP64,
  X7,
  X8,
  X9,
  X10,
  X11,
  X12,
  X13,
  X14,
  X15,
  X16,
  X17,
  X18,
  X19,
  UINT8,
  INT8,
};
int32_t DataTypeSize(int32_t dtype) {
  if (dtype == DataType::BOOL) {
    return sizeof(bool);
  } else if (dtype == DataType::INT8) {
    return sizeof(int8_t);
  } else if (dtype == DataType::INT16) {
    return sizeof(int16_t);
  } else if (dtype == DataType::INT32) {
    return sizeof(int32_t);
  } else if (dtype == DataType::INT64) {
    return sizeof(int64_t);
  } else if (dtype == DataType::FP32) {
    return sizeof(float);
  } else if (dtype == DataType::FP64) {
    return sizeof(double);
  } else if (dtype == DataType::UINT8) {
    return sizeof(uint8_t);
  } else {
    std::cout <<  "Unexpected data type: " + std::to_string(dtype) << std::endl;
  }
  return -1;
}

struct Weight {
  std::vector<char> buffer;
  std::vector<int32_t> shape;
  int32_t dtype;

  template <typename T>
  void set(int32_t data_type, const std::vector<int64_t>& dims,
           const std::vector<T>& data) {
    buffer.clear();
    shape.clear();
    dtype = data_type;
    buffer.resize(data.size() * DataTypeSize(dtype));
    memcpy(buffer.data(), data.data(), data.size() * DataTypeSize(dtype));
    for (auto& d : dims) {
      shape.push_back(d);
    }
  }
  template <typename T>
  void get(std::vector<T>* data) const {
    int64_t nums = std::accumulate(std::begin(shape), std::end(shape), 1,
                                   std::multiplies<int64_t>());
    data->resize(nums);
    if (dtype == DataType::INT64) {
      std::vector<int64_t> value(nums);
      memcpy(value.data(), buffer.data(), nums * sizeof(int64_t));
      data->assign(value.begin(), value.end());
    } else if (dtype == DataType::INT32) {
      std::vector<int32_t> value(nums);
      memcpy(value.data(), buffer.data(), nums * sizeof(int32_t));
      data->assign(value.begin(), value.end());
    } else if (dtype == DataType::INT8) {
      std::vector<int8_t> value(nums);
      memcpy(value.data(), buffer.data(), nums * sizeof(int8_t));
      data->assign(value.begin(), value.end());
    } else if (dtype == DataType::FP32) {
      std::vector<float> value(nums);
      memcpy(value.data(), buffer.data(), nums * sizeof(float));
      data->assign(value.begin(), value.end());
    } else if (dtype == DataType::FP64) {
      std::vector<double> value(nums);
      memcpy(value.data(), buffer.data(), nums * sizeof(double));
      data->assign(value.begin(), value.end());
    } else {
    //   Assert(false,
    //          "Weight::get() only support int64/int32/int8/float32/float64.");
    }
  }
};
bool GetParamNames(std::vector<std::string>* var_names, const std::shared_ptr<paddle::framework::proto::ProgramDesc>& prog) {
  var_names->clear();
  int block_size = prog->blocks_size();
  for (auto i = 0; i < block_size; ++i) {
    auto block = prog->blocks(i);
    int vars_size = block.vars_size();
    for (auto j = 0; j < vars_size; ++j) {
      auto type = block.vars(j).type().type();
      if (type == paddle::framework::proto::VarType_Type::VarType_Type_SELECTED_ROWS) {
        std::cout
            << "VarType of SELECTED_ROWS is not supported by Paddle2ONNX."
            << std::endl;
        return false;
      }
      if (type == paddle::framework::proto::VarType_Type::VarType_Type_FEED_MINIBATCH) {
        continue;
      }
      if (type == paddle::framework::proto::VarType_Type::
                      VarType_Type_FETCH_LIST) {
        continue;
      }
      if (type ==
          paddle::framework::proto::VarType_Type::VarType_Type_READER) {
        continue;
      }
      if (type ==
          paddle::framework::proto::VarType_Type::VarType_Type_RAW) {
        continue;
      }
      if (!block.vars(j).persistable()) {
        continue;
      }
      var_names->push_back(block.vars(j).name());
    }
  }
  std::sort(var_names->begin(), var_names->end());
  return true;
}

// uint32_t version;
// uint64_t lod_level;
// uint32_t version;
// int32_t size;        // size of TensorDesc
// TensorDesc
// weight data
bool LoadParams(const std::string& path, const std::shared_ptr<paddle::framework::proto::ProgramDesc>& prog) {
  std::map<std::string, Weight> params;
  std::ifstream is(path, std::ios::in | std::ios::binary);
  if (!is.is_open()) {
    std::cout << "Cannot open file " << path << " to read." << std::endl;
    return false;
  }
  is.seekg(0, std::ios::end);
  int total_size = is.tellg();
  is.seekg(0, std::ios::beg);

  std::vector<std::string> var_names;
  GetParamNames(&var_names, prog);

  int read_size = 0;
  while (read_size < total_size) {
    {
      // read version, we don't need this
      uint32_t version;
      read_size += sizeof(version);
      is.read(reinterpret_cast<char*>(&version), sizeof(version));
    }
    {
      // read lod_level, we don't use it
      // this has to be zero, otherwise not support
      uint64_t lod_level;
      read_size += sizeof(lod_level);
      is.read(reinterpret_cast<char*>(&lod_level), sizeof(lod_level));
    //   Aassert(lod_level == 0,
    //          "Paddle2ONNX: Only support weight with lod_level = 0.");
    }
    {
      // Another version, we don't use it
      uint32_t version;
      read_size += sizeof(version);
      is.read(reinterpret_cast<char*>(&version), sizeof(version));
    }
    {
      // read size of TensorDesc
      int32_t size;
      read_size += sizeof(size);
      is.read(reinterpret_cast<char*>(&size), sizeof(size));
      // read TensorDesc
      std::unique_ptr<char[]> buf(new char[size]);
      read_size += size;
      is.read(reinterpret_cast<char*>(buf.get()), size);

      std::unique_ptr<paddle::framework::proto::VarType_TensorDesc>
          tensor_desc(new paddle::framework::proto::VarType_TensorDesc());
      tensor_desc->ParseFromArray(buf.get(), size);

      Weight weight;

      int32_t numel = 1;
      int32_t data_type = tensor_desc->data_type();
      weight.dtype = data_type;
      for (auto i = 0; i < tensor_desc->dims().size(); ++i) {
        numel *= tensor_desc->dims()[i];
        weight.shape.push_back(tensor_desc->dims()[i]);
      }

      // read weight data
      weight.buffer.resize(numel * DataTypeSize(data_type));
      read_size += numel * DataTypeSize(data_type);
      is.read(weight.buffer.data(), numel * DataTypeSize(data_type));
      auto index = params.size();
      if (index >= var_names.size()) {
        std::cout << "Unexcepted situation happend while reading parameters "
                       "of PaddlePaddle model."
                    << std::endl;
        return false;
      }
      params[var_names[index]] = weight;
    }
  }
  is.close();
  return true;
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
    std::string not_persistable_var;
    for (int var_idx = 0; var_idx < vars_num; var_idx++) {
      if (prog->blocks(block_idx).vars(var_idx).persistable()) {
        std::cout << prog->blocks(block_idx).vars(var_idx).name() << " ";
      } else {
        not_persistable_var += prog->blocks(block_idx).vars(var_idx).name() + " ";
      }
    }
    std::cout << std::endl;
    std::cout << "[not persistable var] " << not_persistable_var << std::endl;

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
      std::cout << "  attr: ";
      for (auto i = 0; i < op.attrs_size(); ++i) {
        if(op.type() == "feed" || op.type() == "fetch") {
            std::cout << op.attrs(i).name() << " ";
        }
        // if (op.attrs(i).name() == name && IsAttrVar(op, i)) {
        //     break;
        // }
      }
      std::cout << std::endl;
    }
  }
  return true;
}