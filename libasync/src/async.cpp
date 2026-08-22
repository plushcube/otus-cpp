#include <async/async.h>

#include <parser.h>

#include <di/real_container.h>
#include <processor/printer.h>
#include <processor/saver.h>

AsyncContext connect(const size_t &sz) {
  auto di = std::make_shared<RealContainer>();
  Parser p(di, sz);
  p.add_processor(std::make_shared<Printer>());
  p.add_processor(std::make_shared<Saver>());
  p.start();
  return p;
}

void receive(const AsyncContext &ctx, const Message &s) { ctx.process(s); }

void disconnect(const AsyncContext &ctx) { ctx.stop(); }
