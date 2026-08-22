#include <async/async.h>

#include <gtest/gtest.h>

TEST(lib_test, connect_receive_disconnect) {
  const auto ctx = connect(2);

  receive(ctx, "cmd1", 4);
  receive(ctx, "cmd2", 4);
  receive(ctx, "cmd3", 4);

  disconnect(ctx);
}
