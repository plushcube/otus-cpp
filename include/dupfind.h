#pragma once

#include <config.h>

class DupFinder {
public:
  explicit DupFinder(const Config &cfg) : m_config(cfg) {}

  void run();

private:
  const Config m_config;
};
