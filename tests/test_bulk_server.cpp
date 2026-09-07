#include "async_server.h"

#include <boost/asio.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace {

using namespace boost::asio::ip;
using Clock = std::chrono::steady_clock;

class LineCapture : public std::streambuf {
public:
  LineCapture(std::vector<std::string> &lines, std::mutex &mx) : lines_(lines), mx_(mx) {}

protected:
  int_type overflow(int_type c) override {
    if (c != traits_type::eof()) {
      const char ch = static_cast<char>(c);
      return xsputn(&ch, 1) == 1 ? c : traits_type::eof();
    }
    return traits_type::not_eof(c);
  }

  std::streamsize xsputn(const char *s, std::streamsize n) override {
    std::string &buf = thread_buf();
    for (std::streamsize i = 0; i < n; ++i) {
      if (s[i] == '\n') {
        std::lock_guard<std::mutex> lk(mx_);
        lines_.push_back(std::move(buf));
        buf.clear();
      } else {
        buf.push_back(s[i]);
      }
    }
    return n;
  }

private:
  static std::string &thread_buf() {
    thread_local std::string buf;
    return buf;
  }

  std::vector<std::string> &lines_;
  std::mutex &mx_;
};

std::vector<std::string> parse_line(const std::string &line) {
  constexpr std::string_view kPrefix = "bulk: ";
  if (line.rfind(kPrefix, 0) != 0) {
    return {};
  }
  std::vector<std::string> out;
  std::stringstream ss(line.substr(kPrefix.size()));
  std::string tok;
  while (std::getline(ss, tok, ',')) {
    const auto b = tok.find_first_not_of(' ');
    const auto e = tok.find_last_not_of(' ');
    if (b != std::string::npos) {
      out.push_back(tok.substr(b, e - b + 1));
    }
  }
  return out;
}

void cleanup_logs() {
  std::error_code ec;
  for (const auto &e : std::filesystem::directory_iterator(".", ec)) {
    const std::string name = e.path().filename().string();
    if (name.starts_with("bulk") && name.ends_with(".log")) {
      std::filesystem::remove(e.path(), ec);
    }
  }
}

std::vector<std::string> log_file_contents() {
  std::vector<std::string> out;
  std::error_code ec;
  for (const auto &e : std::filesystem::directory_iterator(".", ec)) {
    const std::string name = e.path().filename().string();
    if (name.starts_with("bulk") && name.ends_with(".log")) {
      std::ifstream in(e.path());
      std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
      while (!content.empty() &&
             (content.back() == '\n' || content.back() == '\r' || content.back() == ' ')) {
        content.pop_back();
      }
      out.push_back(std::move(content));
    }
  }
  return out;
}

class BulkServerFixture : public ::testing::Test {
protected:
  void SetUp() override {
    cleanup_logs();
    old_ = std::cout.rdbuf(&capture_);
  }

  void TearDown() override {
    stop_server();
    std::cout.rdbuf(old_);
    cleanup_logs();
  }

  void start_server(size_t block_size) {
    m_server = std::make_unique<AsyncServer>(0, block_size);
    m_port = m_server->port();
    m_thread = std::thread([this] { m_server->run(); });
  }

  void stop_server() {
    if (m_server) {
      m_server->stop();
    }
    if (m_thread.joinable()) {
      m_thread.join();
    }
    m_server.reset();
    m_thread = std::thread();
  }

  std::vector<std::string> captured() const {
    std::lock_guard<std::mutex> lk(mx_);
    std::vector<std::string> out;
    for (const auto &l : lines_) {
      if (l.starts_with("bulk: ")) {
        out.push_back(l);
      }
    }
    return out;
  }

  bool wait_for(const std::function<bool()> &pred, int timeout_ms = 5000) const {
    const auto deadline = Clock::now() + std::chrono::milliseconds(timeout_ms);
    while (!pred()) {
      if (Clock::now() >= deadline) {
        return false;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return true;
  }

  tcp::socket connect_client(boost::asio::io_context &io) {
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

  void send_and_close(const std::string &data) {
    boost::asio::io_context io;
    auto sock = connect_client(io);
    boost::asio::write(sock, boost::asio::buffer(data));
    boost::system::error_code ignored;
    sock.shutdown(tcp::socket::shutdown_send, ignored);
    sock.close(ignored);
  }

  unsigned short m_port = 0;
  std::unique_ptr<AsyncServer> m_server;
  std::thread m_thread;

private:
  std::vector<std::string> lines_;
  mutable std::mutex mx_;
  LineCapture capture_{lines_, mx_};
  std::streambuf *old_ = nullptr;
};

} // namespace

TEST_F(BulkServerFixture, SingleClientStaticBlocksMatchTaskOutput) {
  start_server(3);
  send_and_close("0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n");

  EXPECT_TRUE(wait_for([&] { return captured().size() == 4; }));
  stop_server();

  const std::vector<std::string> expected = {
      "bulk: 0, 1, 2", //
      "bulk: 3, 4, 5", //
      "bulk: 6, 7, 8", //
      "bulk: 9",        //
  };
  EXPECT_EQ(captured(), expected);

  auto files = log_file_contents();
  ASSERT_EQ(files.size(), expected.size());
  std::sort(files.begin(), files.end());
  auto sorted_expected = expected;
  std::sort(sorted_expected.begin(), sorted_expected.end());
  EXPECT_EQ(files, sorted_expected);
}

TEST_F(BulkServerFixture, DynamicBlocksAreGroupedPerConnection) {
  start_server(3);
  send_and_close("cmd1\n{\na\nb\n}\ncmd2\n");

  EXPECT_TRUE(wait_for([&] { return captured().size() == 3; }));
  stop_server();

  const std::vector<std::string> expected = {
      "bulk: cmd1",   //
      "bulk: a, b",   //
      "bulk: cmd2",   //
  };
  EXPECT_EQ(captured(), expected);
  EXPECT_EQ(log_file_contents().size(), expected.size());
}

TEST_F(BulkServerFixture, TwoClientsStaticCommandsMixWithoutLoss) {
  start_server(3);

  auto send_range = [this](int from, int to) {
    boost::asio::io_context io;
    auto sock = connect_client(io);
    std::string data;
    for (int i = from; i <= to; ++i) {
      data += std::to_string(i) + "\n";
    }
    boost::asio::write(sock, boost::asio::buffer(data));
    boost::system::error_code ignored;
    sock.shutdown(tcp::socket::shutdown_send, ignored);
    sock.close(ignored);
  };

  std::thread a([&] { send_range(0, 9); });
  std::thread b([&] { send_range(10, 19); });
  a.join();
  b.join();

  EXPECT_TRUE(wait_for([&] { return captured().size() == 7; }));
  stop_server();

  const auto lines = captured();
  ASSERT_EQ(lines.size(), 7u);
  std::vector<size_t> sizes;
  for (const auto &l : lines) {
    sizes.push_back(parse_line(l).size());
  }
  EXPECT_EQ(sizes, (std::vector<size_t>{3, 3, 3, 3, 3, 3, 2}));

  std::vector<std::string> tokens;
  for (const auto &l : lines) {
    for (auto &t : parse_line(l)) {
      tokens.push_back(std::move(t));
    }
  }
  std::sort(tokens.begin(), tokens.end());
  std::vector<std::string> expected_tokens;
  for (int i = 0; i < 20; ++i) {
    expected_tokens.push_back(std::to_string(i));
  }
  std::sort(expected_tokens.begin(), expected_tokens.end());
  EXPECT_EQ(tokens, expected_tokens);

  auto files = log_file_contents();
  ASSERT_EQ(files.size(), 7u);
  std::vector<std::string> file_tokens;
  for (const auto &f : files) {
    for (auto &t : parse_line(f)) {
      file_tokens.push_back(std::move(t));
    }
  }
  std::sort(file_tokens.begin(), file_tokens.end());
  EXPECT_EQ(file_tokens, expected_tokens);
}

TEST_F(BulkServerFixture, DynamicBlockIsIsolatedFromOtherConnections) {
  start_server(5);

  auto sender = [this](const std::vector<std::string> &lines, int delay_ms) {
    boost::asio::io_context io;
    auto sock = connect_client(io);
    for (const auto &line : lines) {
      boost::asio::write(sock, boost::asio::buffer(line + "\n"));
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
    boost::system::error_code ignored;
    sock.shutdown(tcp::socket::shutdown_send, ignored);
    sock.close(ignored);
  };

  std::vector<std::string> dynamic_lines;
  std::vector<std::string> static_lines;
  for (int i = 0; i < 5; ++i) {
    dynamic_lines.push_back("{");
    dynamic_lines.push_back("a1");
    dynamic_lines.push_back("a2");
    dynamic_lines.push_back("}");
  }
  for (int i = 0; i < 10; ++i) {
    static_lines.push_back("b" + std::to_string(i));
  }

  std::thread a([&] { sender(dynamic_lines, 10); });
  std::thread b([&] { sender(static_lines, 8); });
  a.join();
  b.join();

  EXPECT_TRUE(wait_for([&] { return captured().size() >= 5; }));
  stop_server();

  const auto lines = captured();
  int dynamic_blocks = 0;
  int a1 = 0;
  int a2 = 0;
  int b_tokens = 0;
  for (const auto &l : lines) {
    const auto toks = parse_line(l);
    bool has_a = false;
    bool has_b = false;
    for (const auto &t : toks) {
      has_a = has_a || t.starts_with("a");
      has_b = has_b || t.starts_with("b");
      if (t == "a1") {
        ++a1;
      }
      if (t == "a2") {
        ++a2;
      }
      if (t.starts_with("b")) {
        ++b_tokens;
      }
    }
    EXPECT_FALSE(has_a && has_b) << "mixed line: " << l;
    if (has_a) {
      ++dynamic_blocks;
      EXPECT_EQ(toks.size(), 2u) << l;
    }
  }
  EXPECT_EQ(dynamic_blocks, 5);
  EXPECT_EQ(a1, 5);
  EXPECT_EQ(a2, 5);
  EXPECT_EQ(b_tokens, 10);
}

TEST_F(BulkServerFixture, CommandsBeforeCloseWithoutNewlineAreNotLost) {
  start_server(3);
  send_and_close("1\n2\n3\n4\n5");

  EXPECT_TRUE(wait_for([&] { return captured().size() == 2; }));
  stop_server();

  const std::vector<std::string> expected = {
      "bulk: 1, 2, 3", //
      "bulk: 4, 5",    //
  };
  EXPECT_EQ(captured(), expected);
  EXPECT_EQ(log_file_contents().size(), 2u);
}

TEST_F(BulkServerFixture, ClientWavesDoNotMixPartialBlocks) {
  start_server(3);

  send_and_close("1\n2\n3\n4\n");
  EXPECT_TRUE(wait_for([&] { return captured().size() == 2; }));

  send_and_close("5\n6\n7\n");
  EXPECT_TRUE(wait_for([&] { return captured().size() == 3; }));
  stop_server();

  const std::vector<std::string> expected = {
      "bulk: 1, 2, 3", //
      "bulk: 4",        //
      "bulk: 5, 6, 7",  //
  };
  EXPECT_EQ(captured(), expected);
  EXPECT_EQ(log_file_contents().size(), 3u);
}

TEST_F(BulkServerFixture, UnclosedDynamicBlockIsDroppedOnClose) {
  start_server(3);
  send_and_close("cmd1\n{\na\n");

  EXPECT_TRUE(wait_for([&] { return captured().size() == 1; }));
  stop_server();

  const std::vector<std::string> expected = {
      "bulk: cmd1", //
  };
  EXPECT_EQ(captured(), expected);
  EXPECT_EQ(log_file_contents().size(), 1u);
}
