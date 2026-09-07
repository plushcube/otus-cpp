#include "bulk_router.h"

#include "session.h"

#include <async/dispatcher.h>

#include <commands/cmd_builder.h>
#include <commands/command.h>

BulkRouter::BulkRouter(Dispatcher &dispatcher, size_t block_size) : m_dispatcher(dispatcher), m_static(block_size) {}

void BulkRouter::on_line(Session &session, const std::string &line) {
  const Command cmd = CommandBuilder::make_command(line);

  switch (cmd.type) {
  case Command::Type::Command:
    if (session.m_in_dynamic) {
      session.m_dynamic.collect(cmd);
    } else {
      m_static.collect(cmd);
      if (m_static.is_full()) {
        flush_static();
      }
    }
    break;

  case Command::Type::BlockStart:
    if (!session.m_in_dynamic) {
      flush_static();
      session.m_in_dynamic = true;
    }
    session.m_dynamic.collect(cmd);
    break;

  case Command::Type::BlockEnd:
    if (!session.m_in_dynamic) {
      break;
    }
    session.m_dynamic.collect(cmd);
    if (session.m_dynamic.is_full()) {
      const Collector::Bulk bulk = session.m_dynamic.flush();
      session.m_in_dynamic = false;
      if (!bulk.commands.empty()) {
        m_dispatcher.dispatch(bulk);
      }
    }
    break;
  }
}

void BulkRouter::on_disconnect(Session &session, std::string tail) {
  if (!tail.empty()) {
    on_line(session, tail);
  }
}

void BulkRouter::flush_static() {
  const Collector::Bulk bulk = m_static.flush();
  if (!bulk.commands.empty()) {
    m_dispatcher.dispatch(bulk);
  }
}
