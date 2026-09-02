#pragma once

#include <processor/parser.h>

#include <memory>
#include <shared_mutex>
#include <unordered_map>

class DI_Container;
class Dispatcher;

class Scheduler {
public:
  using ID = size_t;

  struct Task {
    ID id;
    std::shared_ptr<Parser> parser;
  };

  explicit Scheduler(std::shared_ptr<DI_Container> di) : p_di(di), p_gcd(di->dispatcher()) {};

  Task start_task(const size_t &);
  void stop_task(const ID &);
  std::shared_ptr<Parser> get_value(const ID &id);

private:
  std::shared_ptr<DI_Container> p_di;
  std::shared_ptr<Dispatcher> p_gcd;

  std::shared_mutex m_mutex;
  std::unordered_map<ID, std::shared_ptr<Parser>> m_tasks{};
  ID m_next_id{0};
};
