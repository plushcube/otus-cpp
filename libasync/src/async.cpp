#include <async/async.h>

#include <di/real_container.h>

#include <async/dispatcher.h>
#include <async/scheduler.h>

#include <processor/parser.h>
#include <processor/printer.h>
#include <processor/saver.h>

ContextID connect(const size_t &block_size) {
  auto di = DI_Builder::get_di();

  auto t = di->scheduler()->start_task(block_size);
  t.parser->add_processor(std::make_shared<Printer>());
  t.parser->add_processor(std::make_shared<Saver>());
  t.parser->start();
  return t.id;
}

void receive(const ContextID &cid, const char *buffer, const size_t &size) {
  auto di = DI_Builder::get_di();

  auto parser = di->scheduler()->get_value(cid);
  if (!parser) {
    throw std::invalid_argument("unknown context id");
  }
  parser->process(std::string(buffer, size));
}

void disconnect(const ContextID &cid) {
  auto di = DI_Builder::get_di();
  di->scheduler()->stop_task(cid);
}
