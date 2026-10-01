#include "helpers.h"

#include <fstream>
#include <sstream>

namespace mnist {

Eigen::MatrixXf read_mat_from_stream(size_t rows, size_t cols, std::istream &stream) {
  Eigen::MatrixXf res(rows, cols);
  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      float val = 0.0f;
      stream >> val;
      res(i, j) = val;
    }
  }
  return res;
}

Eigen::MatrixXf read_mat_from_file(size_t rows, size_t cols, const std::string &filepath) {
  std::ifstream stream{filepath};
  return read_mat_from_stream(rows, cols, stream);
}

bool read_features(std::istream &stream, Classifier::features_t &features) {
  std::string line;
  if (!std::getline(stream, line)) {
    return false;
  }

  features.clear();
  std::istringstream linestream{line};
  double value;
  while (linestream >> value) {
    features.push_back(value / 255.0f);
  }
  return !features.empty();
}

} // namespace mnist
