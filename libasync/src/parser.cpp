#include <processor/parser.h>

#include <collector/dynamic_collector.h>
#include <collector/static_collector.h>
#include <commands/cmd_builder.h>
#include <commands/command.h>

Parser::Parser(std::weak_ptr<DI_Container> di, const size_t &n) {
  auto locked = di.lock();
  if (!locked) {
    throw std::runtime_error("DI container expired!");
  }
  p_provider = locked->collector_provider(n);
  p_dispatcher = locked->dispatcher();
}

void Parser::feed(const char *data, const size_t size) {
  m_input.append(data, size);

  // Разбираем все завершённые строки; хвост без '\n' остаётся в m_input.
  std::size_t consumed = 0;
  for (;;) {
    const std::size_t nl = m_input.find('\n', consumed);
    if (nl == std::string::npos) {
      break;
    }
    std::string line = m_input.substr(consumed, nl - consumed);
    consumed = nl + 1;

    if (!line.empty() && line.back() == '\r') { // допустим CRLF
      line.pop_back();
    }
    if (!line.empty()) {
      process(line);
    }
  }
  if (consumed != 0) {
    m_input.erase(0, consumed);
  }
}

void Parser::process(const std::string &s) noexcept {
  const Command cmd = CommandBuilder::make_command(s);

  switch (cmd.type) {
  case Command::Type::Command: {
    const auto collector = p_provider->collector();
    collector->collect(cmd);
    if (collector->is_full()) {
      flush();
    }
    break;
  }

  case Command::Type::BlockStart: {
    if (!p_provider->is_dynamic()) {
      flush();
      p_provider->set_dynamic(true);
    }
    const auto collector = p_provider->collector();
    collector->collect(cmd);
    break;
  }

  case Command::Type::BlockEnd: {
    const auto collector = p_provider->collector();
    collector->collect(cmd);
    if (collector->is_full()) {
      flush();
      p_provider->set_dynamic(false);
    }
    break;
  }
  }
}

void Parser::stop() noexcept {
  if (!m_input.empty()) {
    std::string tail = std::move(m_input);
    m_input.clear();
    if (tail.back() == '\r') {
      tail.pop_back();
    }
    if (!tail.empty()) {
      process(tail);
    }
  }
  flush();
}

void Parser::flush() noexcept {
  const auto collector = p_provider->collector();
  const Collector::Bulk bulk = collector->flush();

  if (bulk.commands.empty()) {
    return;
  }

  p_dispatcher->dispatch(bulk);
}
