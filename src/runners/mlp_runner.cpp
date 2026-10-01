#include "mlp_runner.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>

#include <mnist/mlp_classifier.h>

#include "exe_dir.h"

using namespace std;

namespace {

template <typename T> vector<T> parse_line(const string &s) {
  stringstream ss(s);
  vector<T> v;
  for (T f; ss >> f;) {
    v.push_back(f);
    if (ss.peek() == ',') {
      ss.ignore();
    }
  }
  return v;
}

template <size_t Z> bool read_bytes(const string s, size_t *x, vector<float> &d) {
  const auto v = parse_line<int>(s);
  if (v.size() != Z + 1) {
    return false;
  }
  *x = v[0];
  for (size_t i = 0; i < Z; ++i) {
    d[i] = static_cast<float>(v[i + 1]) / 255.0;
  }
  return true;
}

template <size_t Z> int read_data(const string &s, function<void(const size_t, const vector<float>)> callback) {
  string line;
  vector<float> v(Z);

  ifstream fs(s);
  if (!fs.is_open()) {
    cerr << "Ошибка: Не удалось открыть файл!" << endl;
    return 1;
  }

  while (getline(fs, line)) {
    size_t x;
    if (read_bytes<Z>(line, &x, v)) {
      callback(x, v);
    } else {
      cerr << "Ошибка: тестовые данные не соответствуют формату!" << endl;
      fs.close();
      return 1;
    }
  }

  fs.close();
  return 0;
}

Eigen::MatrixXf read_weights(const filesystem::path &p) {
  ifstream fs(p);
  if (!fs.is_open()) {
    cerr << "Ошибка: Не удалось открыть файл " << p << endl;
    return {};
  }

  size_t rows = 0, cols = 0;
  string line;
  vector<vector<float>> w;

  while (getline(fs, line)) {
    const auto v = parse_line<float>(line);
    if (cols == 0) {
      cols = v.size();
    } else if (cols != v.size()) {
      w.clear();
      break;
    }
    w.push_back(v);
    ++rows;
  }
  fs.close();

  if (w.empty()) {
    cerr << "Ошибка: Не удалось прочитать веса из " << p << endl;
    return {};
  }

  Eigen::MatrixXf result(rows, cols);
  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      result(i, j) = w[i][j];
    }
  }
  return result.transpose();
}

} // namespace

int MlpRunner::run(const string &s) {
  const auto w1 = read_weights(app::resource_path("w1.txt"));
  const auto w2 = read_weights(app::resource_path("w2.txt"));
  if (w1.cols() == 0 || w2.cols() == 0) {
    cerr << "Ошибка: веса модели не загружены (каталог: " << app::executable_dir() << ")" << endl;
    return 1;
  }

  const auto model = mnist::MlpClassifier(w1, w2);

  constexpr size_t sz = 28 * 28;

  size_t total = 0;
  size_t match = 0;

  int result = read_data<sz>(s, [&model, &total, &match](const size_t x, const vector<float> v) {
    if (model.predict(v) == x) {
      ++match;
    }
    ++total;
  });
  if (result == 0) {
    cout << fixed << setprecision(2) << static_cast<float>(match) / static_cast<float>(total) << endl;
  }

  return result;
}
