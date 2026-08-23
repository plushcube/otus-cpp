// Многопоточные тесты async-библиотеки.
//
// Стабильный механизм проверки состоит из трёх частей:
//
//  1. Перехват std::cout по строкам (LineCapture): Printer пишет каждый bulk
//     одним operator<< + endl, поэтому мы буферизуем вывод ПОТОКА-локально и
//     атомарно коммитим только полную строку. Параллельные писатели никогда
//     не рвут строки пополам, а порядок строк одного контекста сохраняется
//     (контекст всегда обслуживает один поток). Все проверки ниже — точные
//     равенства (число bulk'ов, размеры bulk'ов, состав команд), а не
//     «не упало».
//
//  2. Барьерная синхронизация (std::barrier) в точках, где нужна
//     одновременность (все connect() стартуют в один момент), и детермини-
//     рованные последовательности команд без случайности — тесты воспроиз-
//     водимы и не флакают.
//
//  3. ThreadSanitizer: собирайте отдельной конфигурацией -DASYNC_TSAN=ON —
//     TSan ловит data race детерминированно при первом же исполнении гонки
//     (functional-проверки выше гонку могут пропустить: например, без
//     синхронизации m_next_id++ может «повезти» и ID не совпадут).

#include <async/async.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <barrier>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {

// ---------------------------------------------------------------------------
// Перехват std::cout: поток-локальная буферизация + атомарный коммит строки.
// ---------------------------------------------------------------------------
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

// "bulk: a, b, c" -> {"a", "b", "c"}; не bulk-строка -> пустой вектор.
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

class MtFixture : public ::testing::Test {
protected:
  void SetUp() override {
    cleanup_logs();
    old_ = std::cout.rdbuf(&capture_);
  }

  void TearDown() override {
    std::cout.rdbuf(old_);
    cleanup_logs();
  }

  // Захваченные bulk-строки (в порядке коммита).
  std::vector<std::string> bulk_lines() const {
    std::vector<std::string> out;
    for (const auto &l : lines_) {
      if (!parse_line(l).empty()) {
        out.push_back(l);
      }
    }
    return out;
  }

  // Все команды из всех строк (порядок между потоками не гарантирован).
  std::vector<std::string> all_tokens() const {
    std::vector<std::string> out;
    for (const auto &l : lines_) {
      for (auto &t : parse_line(l)) {
        out.push_back(std::move(t));
      }
    }
    return out;
  }

private:
  // Saver пишет bulk*.log в текущую директорию — чистим между тестами.
  static void cleanup_logs() {
    std::error_code ec;
    for (const auto &e : std::filesystem::directory_iterator(".", ec)) {
      const std::string name = e.path().filename().string();
      if (name.starts_with("bulk") && name.ends_with(".log")) {
        std::filesystem::remove(e.path(), ec);
      }
    }
  }

  std::vector<std::string> lines_;
  std::mutex mx_;
  LineCapture capture_{lines_, mx_};
  std::streambuf *old_ = nullptr;
};

} // namespace

// ---------------------------------------------------------------------------
// 1. Одновременные connect() дают уникальные контексты, каждый работоспособен.
// ---------------------------------------------------------------------------
TEST_F(MtFixture, ConcurrentConnectYieldsUniqueContexts) {
  constexpr int kThreads = 8;

  std::barrier barrier(kThreads);
  std::vector<ContextID> ids(kThreads, 0);
  std::vector<std::thread> ts;
  ts.reserve(kThreads);
  for (int t = 0; t < kThreads; ++t) {
    ts.emplace_back([&, t] {
      barrier.arrive_and_wait(); // все connect() стартуют одновременно
      ids[t] = connect(2);
    });
  }
  for (auto &th : ts)
    th.join();

  auto sorted = ids;
  std::sort(sorted.begin(), sorted.end());
  ASSERT_EQ(std::adjacent_find(sorted.begin(), sorted.end()), sorted.end())
      << "duplicate context ids from concurrent connect()";

  // Каждый контекст работоспособен: 1 команда + disconnect -> partial bulk.
  for (const auto id : ids) {
    receive(id, "x", 1);
    disconnect(id);
  }
  const auto lines = bulk_lines();
  ASSERT_EQ(lines.size(), static_cast<size_t>(kThreads));
  for (const auto &l : lines) {
    EXPECT_EQ(parse_line(l), (std::vector<std::string>{"x"}));
  }
}

// ---------------------------------------------------------------------------
// 2. Параллельные receive() на разных контекстах не мешают друг другу:
//    точная структура bulk'ов по каждому контексту, ни одна команда не
//    потеряна/задвоена/переставлена.
// ---------------------------------------------------------------------------
TEST_F(MtFixture, ConcurrentReceiveKeepsContextsIsolated) {
  constexpr int kThreads = 8;
  constexpr int kCommands = 50; // команд на контекст
  constexpr size_t kBlock = 3;  // 50 = 16*3 + 2 -> 17 bulk'ов, хвост 2

  std::barrier barrier(kThreads);
  std::vector<std::thread> ts;
  ts.reserve(kThreads);
  for (int t = 0; t < kThreads; ++t) {
    ts.emplace_back([&, t] {
      barrier.arrive_and_wait();
      const auto ctx = connect(kBlock);
      for (int c = 0; c < kCommands; ++c) {
        const std::string cmd = "t" + std::to_string(t) + "_c" + std::to_string(c);
        receive(ctx, cmd.c_str(), cmd.size());
      }
      disconnect(ctx);
    });
  }
  for (auto &th : ts)
    th.join();

  const auto lines = bulk_lines();
  int total = 0;
  for (int t = 0; t < kThreads; ++t) {
    const std::string prefix = "t" + std::to_string(t) + "_";

    // Строки этого контекста, в порядке вывода.
    std::vector<std::string> ctx_lines;
    for (const auto &l : lines) {
      const auto toks = parse_line(l);
      if (!toks.empty() && toks.front().starts_with(prefix)) {
        ctx_lines.push_back(l);
      }
    }

    // 16 полных bulk'ов по kBlock + 1 хвостовой (kCommands % kBlock).
    const size_t expected_lines = kCommands / kBlock + 1;
    ASSERT_EQ(ctx_lines.size(), expected_lines) << "context t" << t;

    std::vector<std::string> flattened;
    for (size_t i = 0; i < ctx_lines.size(); ++i) {
      const auto toks = parse_line(ctx_lines[i]);
      if (i + 1 < ctx_lines.size()) {
        EXPECT_EQ(toks.size(), kBlock) << ctx_lines[i];
      } else {
        EXPECT_EQ(toks.size(), kCommands % kBlock) << ctx_lines[i];
      }
      for (const auto &tok : toks) {
        EXPECT_TRUE(tok.starts_with(prefix)) << "foreign token in context: " << tok;
        flattened.push_back(tok);
      }
    }

    // Команды контекста не потеряны, не задвоены и не переставлены.
    std::vector<std::string> expected;
    expected.reserve(kCommands);
    for (int c = 0; c < kCommands; ++c) {
      expected.push_back(prefix + "c" + std::to_string(c));
    }
    EXPECT_EQ(flattened, expected) << "context t" << t;
    total += static_cast<int>(flattened.size());
  }
  EXPECT_EQ(total, kThreads * kCommands);
}

// ---------------------------------------------------------------------------
// 3. Динамические блоки {} параллельно: по каждому контексту ровно
//    "bulk: tN_pre1", "bulk: tN_b1, tN_b2", "bulk: tN_post1".
// ---------------------------------------------------------------------------
TEST_F(MtFixture, ConcurrentBlocksAreGroupedPerContext) {
  constexpr int kThreads = 6;
  constexpr size_t kBlock = 2;

  std::barrier barrier(kThreads);
  std::vector<std::thread> ts;
  ts.reserve(kThreads);
  for (int t = 0; t < kThreads; ++t) {
    ts.emplace_back([&, t] {
      barrier.arrive_and_wait();
      const auto ctx = connect(kBlock);
      const auto cmd = [&](const std::string &suffix) {
        const std::string c = "t" + std::to_string(t) + suffix;
        receive(ctx, c.c_str(), c.size());
      };
      cmd("_pre1");
      receive(ctx, "{", 1); // открытие динамического блока
      cmd("_b1");
      cmd("_b2");
      receive(ctx, "}", 1); // закрытие -> flush динамического блока
      cmd("_post1");
      disconnect(ctx); // flush хвостового static-bulk'а
    });
  }
  for (auto &th : ts)
    th.join();

  const auto lines = bulk_lines();
  for (int t = 0; t < kThreads; ++t) {
    const std::string prefix = "t" + std::to_string(t) + "_";
    std::vector<std::string> ctx_lines;
    for (const auto &l : lines) {
      const auto toks = parse_line(l);
      if (!toks.empty() && toks.front().starts_with(prefix)) {
        ctx_lines.push_back(l);
      }
    }
    const std::vector<std::string> expected = {
        "bulk: " + prefix + "pre1",
        "bulk: " + prefix + "b1, " + prefix + "b2",
        "bulk: " + prefix + "post1",
    };
    EXPECT_EQ(ctx_lines, expected) << "context t" << t;
  }
}

// ---------------------------------------------------------------------------
// 4. Стресс: интенсивная смена контекстов (insert/erase карты) параллельно
//    с receive() других контекстов. Точный подсчёт уникальных команд
//    детектирует потерю/дублирование; TSan — гонки в Scheduler.
// ---------------------------------------------------------------------------
TEST_F(MtFixture, StressConnectReceiveDisconnectChurn) {
  constexpr int kThreads = 4;
  constexpr int kIterations = 60; // циклов connect->...->disconnect на поток
  constexpr int kCmds = 7;        // команд на контекст (7 = 2*3 + 1)
  constexpr size_t kBlock = 3;

  std::barrier barrier(kThreads);
  std::vector<std::thread> ts;
  ts.reserve(kThreads);
  for (int t = 0; t < kThreads; ++t) {
    ts.emplace_back([&, t] {
      barrier.arrive_and_wait();
      for (int it = 0; it < kIterations; ++it) {
        const auto ctx = connect(kBlock);
        for (int c = 0; c < kCmds; ++c) {
          const std::string cmd = "t" + std::to_string(t) + "_i" + std::to_string(it) + "_c" + std::to_string(c);
          receive(ctx, cmd.c_str(), cmd.size());
        }
        disconnect(ctx);
      }
    });
  }
  for (auto &th : ts)
    th.join();

  const int expected_commands = kThreads * kIterations * kCmds;
  const auto tokens = all_tokens();
  std::unordered_set<std::string> unique(tokens.begin(), tokens.end());
  EXPECT_EQ(tokens.size(), static_cast<size_t>(expected_commands));
  EXPECT_EQ(unique.size(), static_cast<size_t>(expected_commands));

  // Точная структура bulk'ов по каждому контексту: [3,3,1] на итерацию.
  const auto lines = bulk_lines();
  EXPECT_EQ(lines.size(), static_cast<size_t>(kThreads * kIterations * 3));
  for (int t = 0; t < kThreads; ++t) {
    const std::string prefix = "t" + std::to_string(t) + "_";
    std::vector<size_t> sizes;
    for (const auto &l : lines) {
      const auto toks = parse_line(l);
      if (!toks.empty() && toks.front().starts_with(prefix)) {
        sizes.push_back(toks.size());
      }
    }
    ASSERT_EQ(sizes.size(), static_cast<size_t>(kIterations * 3)) << "context t" << t;
    for (int it = 0; it < kIterations; ++it) {
      EXPECT_EQ(sizes[it * 3 + 0], kBlock) << "context t" << t;
      EXPECT_EQ(sizes[it * 3 + 1], kBlock) << "context t" << t;
      EXPECT_EQ(sizes[it * 3 + 2], kCmds % kBlock) << "context t" << t;
    }
  }
}

// ---------------------------------------------------------------------------
// 5. Контрактные проверки: receive() после disconnect() и повторный
//    disconnect() не должны приводить к UB/краху.
// ---------------------------------------------------------------------------
TEST_F(MtFixture, ReceiveOnDestroyedContextThrows) {
  const auto ctx = connect(2);
  receive(ctx, "a", 1);
  disconnect(ctx);
  EXPECT_THROW(receive(ctx, "b", 1), std::invalid_argument);
}

TEST_F(MtFixture, DoubleDisconnectIsSafe) {
  const auto ctx = connect(2);
  receive(ctx, "a", 1);
  disconnect(ctx);
  EXPECT_NO_THROW(disconnect(ctx));
  EXPECT_THROW(receive(ctx, "b", 1), std::invalid_argument);
}
