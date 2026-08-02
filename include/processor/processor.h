#pragma once

#include "../collector/collector.h"

class Processor {
public:
  virtual ~Processor() = default;
  virtual void process(const Collector::Bulk &) const noexcept = 0;
};
