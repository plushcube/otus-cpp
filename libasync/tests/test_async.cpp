#include <async/async.h>

#include <gtest/gtest.h>

#include <string>

namespace {
// Команда завершается '\n' (порция может содержать несколько команд) — поэтому
// отправляем команду как строку с завершающим переводом строки.
void recv(const ContextID ctx, const std::string &cmd) {
  const std::string line = cmd + '\n';
  receive(ctx, line.c_str(), line.size());
}
} // namespace

TEST(lib_test, connect_receive_disconnect) {
  const auto ctx = connect(2);

  recv(ctx, "cmd1");
  recv(ctx, "cmd2");
  recv(ctx, "cmd3");

  disconnect(ctx);
}
