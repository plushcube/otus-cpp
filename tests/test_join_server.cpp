#include "join_server.h"

#include <boost/asio.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <string>
#include <thread>

namespace {

using namespace boost::asio::ip;
using Clock = std::chrono::steady_clock;

std::string read_line(tcp::socket &s, boost::asio::streambuf &buf) {
  boost::asio::read_until(s, buf, '\n');
  std::istream is(&buf);
  std::string line;
  std::getline(is, line);
  if (!line.empty() && line.back() == '\r') {
    line.pop_back();
  }
  return line;
}

std::string send_command(tcp::socket &s, const std::string &command) {
  boost::asio::write(s, boost::asio::buffer(command + "\n"));
  std::string reply;
  boost::asio::streambuf buf;
  for (;;) {
    const std::string line = read_line(s, buf);
    reply += line;
    reply += '\n';
    if (line == "OK" || line.rfind("ERR", 0) == 0) {
      break;
    }
  }
  return reply;
}

class JoinServerFixture : public ::testing::Test {
protected:
  void SetUp() override {
    m_server = std::make_unique<JoinServer>(0);
    m_port = m_server->port();
    m_thread = std::thread([this] { m_server->run(); });
  }

  void TearDown() override {
    if (m_server) {
      m_server->stop();
    }
    if (m_thread.joinable()) {
      m_thread.join();
    }
    m_server.reset();
  }

  tcp::socket connect(boost::asio::io_context &io) {
    const auto deadline = Clock::now() + std::chrono::seconds(5);
    for (;;) {
      tcp::socket sock(io);
      boost::system::error_code ec;
      sock.connect(tcp::endpoint(address_v4::loopback(), m_port), ec);
      if (!ec) {
        return sock;
      }
      if (Clock::now() >= deadline) {
        throw std::runtime_error("connect timeout");
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }

  unsigned short m_port = 0;
  std::unique_ptr<JoinServer> m_server;
  std::thread m_thread;
};

} // namespace

TEST_F(JoinServerFixture, FullTaskScenarioOverNetwork) {
  boost::asio::io_context io;
  auto sock = connect(io);

  EXPECT_EQ(send_command(sock, "INSERT A 0 lean"), "OK\n");
  EXPECT_EQ(send_command(sock, "INSERT A 0 understand"), "ERR duplicate 0\n");
  EXPECT_EQ(send_command(sock, "INSERT A 1 sweater"), "OK\n");
  EXPECT_EQ(send_command(sock, "INSERT A 2 frank"), "OK\n");
  EXPECT_EQ(send_command(sock, "INSERT A 3 violation"), "OK\n");
  EXPECT_EQ(send_command(sock, "INSERT A 4 quality"), "OK\n");
  EXPECT_EQ(send_command(sock, "INSERT A 5 precision"), "OK\n");
  EXPECT_EQ(send_command(sock, "INSERT B 3 proposal"), "OK\n");
  EXPECT_EQ(send_command(sock, "INSERT B 4 example"), "OK\n");
  EXPECT_EQ(send_command(sock, "INSERT B 5 lake"), "OK\n");
  EXPECT_EQ(send_command(sock, "INSERT B 6 flour"), "OK\n");
  EXPECT_EQ(send_command(sock, "INSERT B 7 wonder"), "OK\n");
  EXPECT_EQ(send_command(sock, "INSERT B 8 selection"), "OK\n");

  EXPECT_EQ(send_command(sock, "INTERSECTION"),
            "3,violation,proposal\n4,quality,example\n5,precision,lake\nOK\n");
  EXPECT_EQ(send_command(sock, "SYMMETRIC_DIFFERENCE"),
            "0,lean,\n1,sweater,\n2,frank,\n6,,flour\n7,,wonder\n8,,selection\nOK\n");

  EXPECT_EQ(send_command(sock, "TRUNCATE A"), "OK\n");
  EXPECT_EQ(send_command(sock, "INTERSECTION"), "OK\n");
  EXPECT_EQ(send_command(sock, "SYMMETRIC_DIFFERENCE"),
            "3,,proposal\n4,,example\n5,,lake\n6,,flour\n7,,wonder\n8,,selection\nOK\n");
}

TEST_F(JoinServerFixture, PipelinesCommandsInOneWrite) {
  boost::asio::io_context io;
  auto sock = connect(io);

  const std::string batch1 = "TRUNCATE A\nTRUNCATE B\n";
  boost::asio::write(sock, boost::asio::buffer(batch1));
  boost::asio::streambuf buf;
  EXPECT_EQ(read_line(sock, buf), "OK");
  EXPECT_EQ(read_line(sock, buf), "OK");

  const std::string batch2 = "INSERT A 7 seven\nINSERT A 8 eight\n";
  boost::asio::write(sock, boost::asio::buffer(batch2));
  EXPECT_EQ(read_line(sock, buf), "OK");
  EXPECT_EQ(read_line(sock, buf), "OK");
  EXPECT_EQ(send_command(sock, "SYMMETRIC_DIFFERENCE"), "7,seven,\n8,eight,\nOK\n");
}

TEST_F(JoinServerFixture, ConcurrentClientsShareTables) {
  auto worker = [this](int base, const std::string &name) {
    boost::asio::io_context io;
    auto sock = connect(io);
    for (int i = 0; i < 5; ++i) {
      const int id = base + i;
      EXPECT_EQ(send_command(sock, "INSERT A " + std::to_string(id) + " " + name + std::to_string(id)),
                "OK\n");
    }
  };

  std::thread a(worker, 0, "aa");
  std::thread b(worker, 100, "bb");
  a.join();
  b.join();

  boost::asio::io_context io;
  auto sock = connect(io);
  const std::string reply = send_command(sock, "SYMMETRIC_DIFFERENCE");
  std::string expected;
  for (int i = 0; i < 5; ++i) {
    expected += std::to_string(i) + ",aa" + std::to_string(i) + ",\n";
  }
  for (int i = 0; i < 5; ++i) {
    expected += std::to_string(100 + i) + ",bb" + std::to_string(100 + i) + ",\n";
  }
  expected += "OK\n";
  EXPECT_EQ(reply, expected);
}

TEST_F(JoinServerFixture, IncompleteCommandOnCloseIsNotExecuted) {
  boost::asio::io_context io;
  auto sock = connect(io);
  boost::asio::write(sock, boost::asio::buffer("INSERT A 5 five"));
  boost::system::error_code ignored;
  sock.shutdown(tcp::socket::shutdown_send, ignored);
  sock.close(ignored);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  boost::asio::io_context io2;
  auto check = connect(io2);
  EXPECT_EQ(send_command(check, "SYMMETRIC_DIFFERENCE"), "OK\n");
}

TEST_F(JoinServerFixture, UnknownCommandReturnsError) {
  boost::asio::io_context io;
  auto sock = connect(io);
  EXPECT_EQ(send_command(sock, "DROP A"), "ERR unknown command\n");
  EXPECT_EQ(send_command(sock, "INSERT"), "ERR invalid arguments\n");
}
