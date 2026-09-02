#include <async/async.h>

#include <di/real_container.h>

#include <async/scheduler.h>

ContextID connect(const size_t &block_size) {
  auto di = DI_Builder::get_di();
  auto t = di->scheduler()->start_task(block_size);
  return t.id;
}

void receive(const ContextID &cid, const char *buffer, const size_t &size) {
  auto di = DI_Builder::get_di();

  auto parser = di->scheduler()->get_value(cid);
  if (!parser) {
    throw std::invalid_argument("unknown context id");
  }
  parser->feed(buffer, size);
}

void disconnect(const ContextID &cid) {
  auto di = DI_Builder::get_di();
  di->scheduler()->stop_task(cid);
}
