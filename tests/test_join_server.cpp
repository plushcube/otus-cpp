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
  boost::system::error_code ec;
  sock.shutdown(tcp::socket::shutdown_send, ec);
  sock.close(ec);
  (void)ec;

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

TEST_F(JoinServerFixture, LargeOperationDoesNotBlockOtherClients) {
  constexpr int kCount = 2000;
  boost::asio::io_context io;
  auto seeder = connect(io);
  boost::asio::streambuf seed_buf;

  std::string batch;
  for (int i = 0; i < kCount; ++i) {
    batch += "INSERT A " + std::to_string(i) + " a" + std::to_string(i) + "\n";
  }
  for (int i = 0; i < kCount; ++i) {
    batch += "INSERT B " + std::to_string(i) + " b" + std::to_string(i) + "\n";
  }
  boost::asio::write(seeder, boost::asio::buffer(batch));
  for (int i = 0; i < 2 * kCount; ++i) {
    EXPECT_EQ(read_line(seeder, seed_buf), "OK");
  }

  boost::asio::io_context io_slow;
  auto slow = connect(io_slow);
  boost::asio::write(slow, boost::asio::buffer(std::string("INTERSECTION\n")));
  boost::asio::streambuf slow_buf;
  EXPECT_EQ(read_line(slow, slow_buf), "0,a0,b0");

  boost::asio::io_context io_fast;
  auto fast = connect(io_fast);
  EXPECT_EQ(send_command(fast, "INSERT A 5000 five"), "OK\n");
  EXPECT_EQ(send_command(fast, "SYMMETRIC_DIFFERENCE"), "5000,five,\nOK\n");
  boost::asio::streambuf fast_buf;
  boost::asio::write(fast, boost::asio::buffer(std::string("INSERT A 5001 six\n")));
  EXPECT_EQ(read_line(fast, fast_buf), "OK");

  std::string tail;
  int rows = 1;
  for (;;) {
    const std::string line = read_line(slow, slow_buf);
    if (line == "OK") {
      break;
    }
    ++rows;
    tail = line;
  }
  EXPECT_EQ(rows, kCount);
  EXPECT_EQ(tail, std::to_string(kCount - 1) + ",a" + std::to_string(kCount - 1) + ",b" +
                      std::to_string(kCount - 1));

  EXPECT_EQ(send_command(fast, "SYMMETRIC_DIFFERENCE"), "5000,five,\n5001,six,\nOK\n");
}

TEST_F(JoinServerFixture, ConcurrentClientsSurviveLargeIntersection) {
  constexpr int kCount = 1500;
  boost::asio::io_context io;
  auto seeder = connect(io);
  boost::asio::streambuf seed_buf;

  std::string batch;
  for (int i = 0; i < kCount; ++i) {
    batch += "INSERT A " + std::to_string(i) + " x" + std::to_string(i) + "\n";
  }
  for (int i = 0; i < kCount; ++i) {
    batch += "INSERT B " + std::to_string(i) + " y" + std::to_string(i) + "\n";
  }
  boost::asio::write(seeder, boost::asio::buffer(batch));
  for (int i = 0; i < 2 * kCount; ++i) {
    EXPECT_EQ(read_line(seeder, seed_buf), "OK");
  }

  auto query_worker = [this] {
    boost::asio::io_context local_io;
    auto sock = connect(local_io);
    boost::asio::streambuf buf;
    boost::asio::write(sock, boost::asio::buffer(std::string("INTERSECTION\n")));
    int rows = 0;
    for (;;) {
      const std::string line = read_line(sock, buf);
      if (line == "OK") {
        break;
      }
      ++rows;
    }
    return rows;
  };

  auto write_worker = [this] {
    boost::asio::io_context local_io;
    auto sock = connect(local_io);
    boost::asio::streambuf buf;
    for (int i = 0; i < 100; ++i) {
      const int id = 100000 + i;
      boost::asio::write(sock, boost::asio::buffer("INSERT B " + std::to_string(id) + " z\n"));
      EXPECT_EQ(read_line(sock, buf), "OK");
    }
  };

  std::thread query(query_worker);
  std::thread writer(write_worker);
  query.join();
  writer.join();

  boost::asio::io_context io2;
  auto sock = connect(io2);
  const std::string reply = send_command(sock, "SYMMETRIC_DIFFERENCE");
  std::string expected;
  for (int i = 100000; i < 100100; ++i) {
    expected += std::to_string(i) + ",,z\n";
  }
  expected += "OK\n";
  EXPECT_EQ(reply, expected);
}

TEST_F(JoinServerFixture, NameWithSpacesOverNetwork) {
  boost::asio::io_context io;
  auto sock = connect(io);
  EXPECT_EQ(send_command(sock, "INSERT A 42 John Doe"), "OK\n");
  EXPECT_EQ(send_command(sock, "INSERT B 42 Jane Roe"), "OK\n");
  EXPECT_EQ(send_command(sock, "INSERT A 43 Solo"), "OK\n");
  EXPECT_EQ(send_command(sock, "INTERSECTION"), "42,John Doe,Jane Roe\nOK\n");
  EXPECT_EQ(send_command(sock, "SYMMETRIC_DIFFERENCE"), "43,Solo,\nOK\n");
  EXPECT_EQ(send_command(sock, "INSERT  A 44 bad"), "ERR invalid arguments\n");
  EXPECT_EQ(send_command(sock, "INSERT A 44"), "ERR invalid arguments\n");
}
