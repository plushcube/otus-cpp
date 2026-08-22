#include <async/async.h>

#include <parser.h>
#include <scheduler.h>

#include <processor/printer.h>
#include <processor/saver.h>

ContextID connect(const size_t &block_size) {
  auto t = Scheduler::shared().start_task(block_size);
  t.parser->add_processor(std::make_shared<Printer>());
  t.parser->add_processor(std::make_shared<Saver>());
  t.parser->start();
  return t.id;
}

void receive(const ContextID &cid, const char *buffer, const size_t &size) {
  std::string s(buffer, size);
  Scheduler::shared().get_value(cid)->process(s);
}

void disconnect(const ContextID &cid) { Scheduler::shared().stop_task(cid); }
