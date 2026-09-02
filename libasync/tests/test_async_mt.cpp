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
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
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

// Команда в библиотеке завершается '\n' (receive может получать порцию с
// несколькими командами, кусок команды может рваться между вызовами) —
// отправляем команду как строку с завершающим переводом строки.
void recv(const ContextID ctx, const std::string &cmd) {
  const std::string line = cmd + '\n';
  receive(ctx, line.c_str(), line.size());
}

// Имена bulk*.log в текущей директории.
std::vector<std::string> log_file_names() {
  std::vector<std::string> out;
  std::error_code ec;
  for (const auto &e : std::filesystem::directory_iterator(".", ec)) {
    const std::string name = e.path().filename().string();
    if (name.starts_with("bulk") && name.ends_with(".log")) {
      out.push_back(name);
    }
  }
  return out;
}

// Содержимое каждого bulk*.log (одна строка "bulk: ..." на файл).
std::vector<std::string> log_file_contents() {
  std::vector<std::string> out;
  std::error_code ec;
  for (const auto &e : std::filesystem::directory_iterator(".", ec)) {
    const std::string name = e.path().filename().string();
    if (name.starts_with("bulk") && name.ends_with(".log")) {
      std::ifstream in(e.path());
      std::string content((std::istreambuf_iterator<char>(in)),
                          std::istreambuf_iterator<char>());
      while (!content.empty() &&
             (content.back() == '\n' || content.back() == '\r' || content.back() == ' ')) {
        content.pop_back();
      }
      out.push_back(std::move(content));
    }
  }
  return out;
}

// Имя вида bulk{ts}_{postfix}.log или bulk{ts}_{postfix}_{seq}.log.
bool is_valid_log_name(const std::string &name) {
  if (!name.starts_with("bulk") || !name.ends_with(".log")) {
    return false;
  }
  const std::string mid = name.substr(4, name.size() - 8); // без "bulk" и ".log"
  std::vector<std::string> parts;
  std::stringstream ss(mid);
  std::string part;
  while (std::getline(ss, part, '_')) {
    parts.push_back(part);
  }
  if (parts.size() < 2 || parts.size() > 3) {
    return false;
  }
  for (const auto &p : parts) {
    if (p.empty() ||
        !std::all_of(p.begin(), p.end(),
                     [](char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; })) {
      return false;
    }
  }
  return true;
}

// Постфикс файлового потока из имени (второй компонент, после timestamp).
std::string log_name_postfix(const std::string &name) {
  const std::string mid = name.substr(4, name.size() - 8);
  const auto first = mid.find('_');
  const auto second = mid.find('_', first + 1);
  return mid.substr(first + 1, second == std::string::npos ? std::string::npos : second - first - 1);
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
    recv(id, "x");
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
        recv(ctx, cmd);
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
        recv(ctx, "t" + std::to_string(t) + suffix);
      };
      cmd("_pre1");
      recv(ctx, "{"); // открытие динамического блока
      cmd("_b1");
      cmd("_b2");
      recv(ctx, "}"); // закрытие -> flush динамического блока
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
          recv(ctx, cmd);
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
  recv(ctx, "a");
  disconnect(ctx);
  EXPECT_THROW(recv(ctx, "b"), std::invalid_argument);
}

TEST_F(MtFixture, DoubleDisconnectIsSafe) {
  const auto ctx = connect(2);
  recv(ctx, "a");
  disconnect(ctx);
  EXPECT_NO_THROW(disconnect(ctx));
  EXPECT_THROW(recv(ctx, "b"), std::invalid_argument);
}

// ---------------------------------------------------------------------------
// 6. Канонический пример задания: receive() принимает порции, в которых может
//    быть несколько команд ('\n'-разделитель), а кусок команды может рваться
//    между вызовами (receive(h, "1", 1) затем "\n2\n3..."). Эталонный вывод
//    задания (bulk = 5): пять блоков в строгом порядке.
// ---------------------------------------------------------------------------
TEST_F(MtFixture, CanonicalExternalSampleMatchesTaskOutput) {
  const auto h = connect(5);
  const auto h2 = connect(5);

  receive(h, "1", 1);              // хвост без '\n' — буферизуется
  receive(h2, "1\n", 2);           // h2: команда 1 (одна, блок не полон)
  receive(h, "\n2\n3\n4\n5\n6\n{\na\n", 15); // "1" завершается, затем 2..5, 6, {, a
  receive(h, "b\nc\nd\n}\n89\n", 11);        // b c d } -> flush динамического, 89
  disconnect(h);                   // flush статического блока с 89
  disconnect(h2);                  // flush h2: bulk: 1

  const std::vector<std::string> expected = {
      "bulk: 1, 2, 3, 4, 5", //
      "bulk: 6",              //
      "bulk: a, b, c, d",     //
      "bulk: 89",             //
      "bulk: 1",              //
  };
  EXPECT_EQ(bulk_lines(), expected); // один поток-производитель -> строгий порядок

  // По файлу на каждый блок с тем же содержимым.
  auto files = log_file_contents();
  ASSERT_EQ(files.size(), expected.size());
  std::sort(files.begin(), files.end());
  auto sorted_expected = expected;
  std::sort(sorted_expected.begin(), sorted_expected.end());
  EXPECT_EQ(files, sorted_expected);
  for (const auto &n : log_file_names()) {
    EXPECT_TRUE(is_valid_log_name(n)) << n;
  }
}

// ---------------------------------------------------------------------------
// 7. Файлы: каждый блок -> ровно один файл (в т.ч. два блока за одну секунду
//    не сливаются), содержимое файлов совпадает с консолью, имена уникальны.
// ---------------------------------------------------------------------------
TEST_F(MtFixture, AsyncFilesMatchBlocksOneFileEach) {
  constexpr size_t kBlock = 3;
  constexpr int kCommands = 12; // ровно 4 блока, без хвоста
  const auto ctx = connect(kBlock);
  for (int i = 0; i < kCommands; ++i) {
    recv(ctx, "c" + std::to_string(i));
  }
  disconnect(ctx);

  std::vector<std::string> expected_lines;
  for (int b = 0; b < kCommands / static_cast<int>(kBlock); ++b) {
    std::string line = "bulk: ";
    for (int c = 0; c < static_cast<int>(kBlock); ++c) {
      if (c != 0) {
        line += ", ";
      }
      line += "c" + std::to_string(b * static_cast<int>(kBlock) + c);
    }
    expected_lines.push_back(line);
  }
  EXPECT_EQ(bulk_lines(), expected_lines); // консоль в порядке отправки

  const auto files = log_file_contents();
  ASSERT_EQ(files.size(), expected_lines.size()); // файл на блок, слияний нет
  auto sorted_files = files;
  std::sort(sorted_files.begin(), sorted_files.end());
  auto sorted_expected = expected_lines;
  std::sort(sorted_expected.begin(), sorted_expected.end());
  EXPECT_EQ(sorted_files, sorted_expected);

  int total = 0;
  for (const auto &f : files) {
    total += static_cast<int>(parse_line(f).size());
  }
  EXPECT_EQ(total, kCommands); // команды не потеряны и не задвоены

  const auto names = log_file_names();
  auto sorted_names = names;
  std::sort(sorted_names.begin(), sorted_names.end());
  EXPECT_EQ(std::adjacent_find(sorted_names.begin(), sorted_names.end()), sorted_names.end())
      << "duplicate file names";
  for (const auto &n : names) {
    EXPECT_TRUE(is_valid_log_name(n)) << n;
  }
}

// ---------------------------------------------------------------------------
// 8. Большой объём без пауз: 100 блоков подряд (наверняка несколько за одну
//    секунду) — файлов ровно по числу блоков, команды сохраняются, и оба
//    файловых потока реально пишут (постфиксы "1" и "2"). Распределение
//    между потоками недетерминировано, поэтому присутствие обоих постфиксов
//    проверяем на объёме, где вероятность «всё ушло одному» исчезающе мала.
// ---------------------------------------------------------------------------
TEST_F(MtFixture, AsyncLargeVolumeBothFileThreadsWrite) {
  constexpr size_t kBlock = 2;
  constexpr int kCommands = 200; // ровно 100 блоков
  const auto ctx = connect(kBlock);
  for (int i = 0; i < kCommands; ++i) {
    recv(ctx, "x" + std::to_string(i));
  }
  disconnect(ctx);

  EXPECT_EQ(bulk_lines().size(), static_cast<size_t>(kCommands / kBlock));

  const auto names = log_file_names();
  ASSERT_EQ(names.size(), static_cast<size_t>(kCommands / kBlock)); // файл на блок

  int total = 0;
  std::unordered_set<std::string> postfixes;
  for (const auto &n : names) {
    EXPECT_TRUE(is_valid_log_name(n)) << n;
    postfixes.insert(log_name_postfix(n));
  }
  EXPECT_GE(postfixes.size(), 2u) << "expected both file threads to write";
  EXPECT_TRUE(postfixes.contains("1")) << "postfix 1 absent";
  EXPECT_TRUE(postfixes.contains("2")) << "postfix 2 absent";

  for (const auto &content : log_file_contents()) {
    total += static_cast<int>(parse_line(content).size());
  }
  EXPECT_EQ(total, kCommands); // без потерь и дублей
}
