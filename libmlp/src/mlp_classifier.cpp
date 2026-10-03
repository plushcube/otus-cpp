#include <mnist/mlp_classifier.h>

#include <algorithm>
#include <cmath>

using Eigen::VectorXf;
using mnist::MlpClassifier;

namespace {

template <typename T> auto sigma(T x) { return 1 / (1 + std::exp(-x)); }

VectorXf sigmav(const VectorXf &v) {
  VectorXf res{v.rows()};
  for (Eigen::Index i = 0; i < v.rows(); ++i) {
    res(i) = sigma(v(i));
  }
  return res;
}

VectorXf softmax(const VectorXf &v) {
  VectorXf res{v.rows()};
  float denominator = 0.0f;

  for (Eigen::Index i = 0; i < v.rows(); ++i) {
    denominator += std::exp(v(i));
  }
  for (Eigen::Index i = 0; i < v.rows(); ++i) {
    res(i) = std::exp(v(i)) / denominator;
  }
  return res;
}

} // namespace

MlpClassifier::MlpClassifier(const Eigen::MatrixXf &w1, const Eigen::MatrixXf &w2) : m_w1{w1}, m_w2{w2} {}

size_t MlpClassifier::num_classes() const { return m_w2.rows(); }

size_t MlpClassifier::predict(const features_t &feat) const {
  auto proba = predict_proba(feat);
  auto argmax = std::max_element(proba.begin(), proba.end());
  return std::distance(proba.begin(), argmax);
}

MlpClassifier::probas_t MlpClassifier::predict_proba(const features_t &feat) const {
  VectorXf x{feat.size()};
  for (size_t i = 0; i < feat.size(); ++i) {
    x[i] = feat[i];
  }

  auto o1 = sigmav(m_w1 * x);
  auto o2 = softmax(m_w2 * o1);

  probas_t res;
  for (Eigen::Index i = 0; i < o2.rows(); ++i) {
    res.push_back(o2(i));
  }
  return res;
}
