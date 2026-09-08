#include <boost/asio.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

using namespace boost::asio::ip;

namespace {

unsigned long parse_uint(const std::string &s) {
  std::size_t pos = 0;
  const unsigned long v = std::stoul(s, &pos);
  if (pos != s.size()) {
    throw std::invalid_argument("not a number: " + s);
  }
  return v;
}

bool is_reply_end(const std::string &line) {
  return line == "OK" || line.rfind("ERR", 0) == 0;
}

} // namespace

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: join_client <port>" << std::endl;
    return 1;
  }

  unsigned short port = 0;
  try {
    port = static_cast<unsigned short>(parse_uint(argv[1]));
  } catch (const std::exception &) {
    std::cerr << "Usage: join_client <port>" << std::endl;
    return 1;
  }
  if (port == 0) {
    std::cerr << "Usage: join_client <port>" << std::endl;
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
        if (is_reply_end(line)) {
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
