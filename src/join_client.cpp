#include "utils/utils.h"

#include <boost/asio.hpp>
#include <iostream>

using namespace boost::asio::ip;

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
    boost::asio::io_context io;
    tcp::socket socket(io);
    socket.connect(tcp::endpoint(address_v4::loopback(), port));

    boost::asio::streambuf buf;
    std::string command;
    while (std::getline(std::cin, command)) {
      boost::asio::write(socket, boost::asio::buffer(command + "\n"));
      for (;;) {
        boost::system::error_code ec;
        boost::asio::read_until(socket, buf, '\n', ec);
        if (ec) {
          std::cerr << "Error: " << ec.message() << std::endl;
          return 1;
        }
        std::istream is(&buf);
        std::string line;
        std::getline(is, line);
        if (!line.empty() && line.back() == '\r') {
          line.pop_back();
        }
        std::cout << line << '\n';
        if (Utils::is_reply_end(line)) {
          break;
        }
      }
      std::cout.flush();
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
