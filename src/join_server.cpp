#include "join_server.h"
#include "utils/utils.h"

#include <boost/asio/signal_set.hpp>

#include <csignal>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  if (argc != 2) {
    Utils::print_usage(argv[0]);
    return 1;
  }

  unsigned short port = 0;
  try {
    port = Utils::parse_port(argv[1]);
  } catch (const std::exception &) {
    Utils::print_usage(argv[0]);
    return 1;
  }

  try {
    JoinServer server(port);

    boost::asio::signal_set signals(server.io_context(), SIGINT, SIGTERM);
    signals.async_wait([&server](const boost::system::error_code &, int) { server.stop(); });

    server.run();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
