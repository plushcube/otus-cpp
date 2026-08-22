#include <async/async.h>

#include <parser.h>
#include <scheduler.h>

#include <processor/printer.h>
#include <processor/saver.h>

AsyncContext connect(const size_t &sz) {
  auto t = Scheduler::shared().start_task(sz);
  t.parser->add_processor(std::make_shared<Printer>());
  t.parser->add_processor(std::make_shared<Saver>());
  t.parser->start();
  return t.id;
}

void receive(const AsyncContext &ctx, const Message &s) { Scheduler::shared().get_value(ctx)->process(s); }

void disconnect(const AsyncContext &ctx) { Scheduler::shared().stop_task(ctx); }
