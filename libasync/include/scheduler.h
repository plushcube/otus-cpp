#pragma once

#include <di/real_container.h>
#include <parser.h>

#include <memory>
#include <shared_mutex>
#include <unordered_map>

class Scheduler {
public:
  using ID = size_t;

  struct Task {
    ID id;
    std::shared_ptr<Parser> parser;
  };

  static Scheduler &shared() noexcept;

  Task start_task(const size_t &);
  void stop_task(const ID &);
  std::shared_ptr<Parser> get_value(const ID &id);

private:
  Scheduler() : p_di(std::make_shared<RealContainer>()) {};

  std::shared_ptr<DI_Container> p_di;
  std::shared_mutex m_mutex;
  std::unordered_map<ID, std::shared_ptr<Parser>> m_tasks{};
  ID m_next_id{0};
};
