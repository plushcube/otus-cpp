#pragma once

#include <config.h>

#include <filesystem>

class DupFinder {
public:
  explicit DupFinder(const Config &cfg) : m_config(cfg) {}

  void run();

private:
  const Config m_config;

  static bool is_match(const Config &, const std::filesystem::directory_entry &);
};
