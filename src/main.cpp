#include "bulk_server.h"

#include <boost/asio.hpp>

#include <csignal>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

unsigned long parse_uint(const std::string &s) {
  std::size_t pos = 0;
  const unsigned long v = std::stoul(s, &pos);
  if (pos != s.size()) {
    throw std::invalid_argument("not a number: " + s);
  }
  return v;
}

} // namespace

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: bulk_server <port> <bulk_size>" << std::endl;
    return 1;
  }

  unsigned short port = 0;
  size_t block_size = 0;
  try {
    port = static_cast<unsigned short>(parse_uint(argv[1]));
    block_size = parse_uint(argv[2]);
  } catch (const std::exception &) {
    std::cerr << "Usage: bulk_server <port> <bulk_size>" << std::endl;
    return 1;
  }

  if (port == 0 || block_size == 0) {
    std::cerr << "Usage: bulk_server <port> <bulk_size>" << std::endl;
    return 1;
  }

  try {
    AsyncServer server(port, block_size);

    boost::asio::signal_set signals(server.io_context(), SIGINT, SIGTERM);
    signals.async_wait([&server](const boost::system::error_code &, int) { server.stop(); });

    server.run();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
