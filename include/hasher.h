#pragma once

#include <memory>
#include <string>
#include <vector>

struct Config;

class Hasher {
public:
  using Hash = std::string;

  static std::unique_ptr<Hasher> make_hasher(const Config &);

  virtual ~Hasher() = default;

  virtual Hash get_hash(const std::vector<char> &) = 0;
};

class CRC_Hasher : public Hasher {
public:
  Hash get_hash(const std::vector<char> &) override;

private:
};

class MD5_Hasher : public Hasher {
public:
  Hash get_hash(const std::vector<char> &) override;
};

class SHA1_Hasher : public Hasher {
public:
  Hash get_hash(const std::vector<char> &) override;
};
