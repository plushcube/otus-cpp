#pragma once

#include <di/real_container.h>
#include <parser.h>

#include <memory>
#include <unordered_map>

class Scheduler {
public:
  using ID = size_t;

  struct Task {
    ID id;
    std::shared_ptr<Parser> parser;
  };

  static Scheduler &shared() noexcept;

  Task start_task(const size_t &) noexcept;
  void stop_task(const ID &) noexcept;
  std::shared_ptr<Parser> get_value(const ID &id) noexcept { return m_tasks[id]; }

private:
  Scheduler() : p_di(std::make_shared<RealContainer>()) {};

  std::shared_ptr<DI_Container> p_di;
  std::unordered_map<ID, std::shared_ptr<Parser>> m_tasks{};
  ID m_next_id{0};
};
