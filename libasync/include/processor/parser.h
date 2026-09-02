#pragma once

#include <cstddef>
#include <memory>
#include <string>

#include <async/dispatcher.h>
#include <collector/provider.h>
#include <di/container.h>

class Parser {
public:
  explicit Parser(std::weak_ptr<DI_Container>, const size_t &);

  void feed(const char *data, const size_t size);
  void stop() noexcept;

private:
  void process(const std::string &line) noexcept;
  void flush() noexcept;

  std::shared_ptr<CollectorProvider> p_provider;
  std::shared_ptr<Dispatcher> p_dispatcher;
  std::string m_input; // неполные строки между порциями
};
