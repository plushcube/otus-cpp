#include <hasher.h>

#include <config.h>

#include <boost/crc.hpp>
#include <boost/hash2/md5.hpp>
#include <boost/hash2/sha1.hpp>
#include <utility>

std::unique_ptr<Hasher> Hasher::make_hasher(const Config &cfg) {
  switch (cfg.hash) {
  case Config::Hash::crc32:
    return std::make_unique<CRC_Hasher>();
  case Config::Hash::md5:
    return std::make_unique<MD5_Hasher>();
  case Config::Hash::sha1:
    return std::make_unique<SHA1_Hasher>();
  }
  std::unreachable();
}

Hasher::Hash CRC_Hasher::get_hash(const std::vector<char> &msg) {
  boost::crc_32_type a{};
  a.process_bytes(msg.data(), msg.size());
  return std::to_string(a.checksum());
}

Hasher::Hash MD5_Hasher::get_hash(const std::vector<char> &msg) {
  boost::hash2::md5_128 a{};
  a.update(msg.data(), msg.size());
  return to_string(a.result());
}

Hasher::Hash SHA1_Hasher::get_hash(const std::vector<char> &msg) {
  boost::hash2::sha1_160 a{};
  a.update(msg.data(), msg.size());
  return to_string(a.result());
}
