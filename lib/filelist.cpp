#include <filelist.h>

#include <filesystem>
#include <string>
#include <system_error>
#include <utility>

using namespace std;

FileList::Iterator::Iterator(const Config &cfg) : m_cfg(&cfg), m_is_end(false) {
  for (auto it = cfg.dirs.rbegin(); it != cfg.dirs.rend(); ++it) {
    error_code ec;
    auto dit = filesystem::recursive_directory_iterator(*it, ec);
    if (ec) {
      continue;
    }
    m_stack.push(std::move(dit));
  }
  advance();
}

FileList::Iterator &FileList::Iterator::operator++() {
  advance();
  return *this;
}

bool FileList::Iterator::operator==(const Iterator &other) const {
  return m_is_end == other.m_is_end && (m_is_end || m_current_value == other.m_current_value);
}

void FileList::Iterator::advance() {
  if (m_cfg == nullptr) {
    m_is_end = true;
    return;
  }

  while (!m_stack.empty()) {
    auto &current = m_stack.top();
    if (current == m_end_it) {
      m_stack.pop();
      continue;
    }

    const auto &entry = *current;

    // depth() отсчитывается от прямых детей корня (0, 1, ...), сам корень
    // итератором не выдаётся. '>=' (а не '>') даёт «файлы не глубже depth
    // уровней» без off-by-one: каталоги на глубине >= depth не обходим.
    if (m_cfg->depth > 0 && current.depth() >= static_cast<int>(m_cfg->depth)) {
      current.disable_recursion_pending();
    }

    if (entry.is_directory()) {
      // Исключённые каталоги не обходим и не выдаём. Сравнение по
      // нормализованному абсолютному пути: filesystem::equivalent бросает
      // исключение для несуществующих путей, а тут такого быть не должно.
      const auto norm = [](const filesystem::path &p) { return filesystem::absolute(p).lexically_normal(); };
      const auto dir = norm(entry.path());
      bool excluded = false;
      for (const auto &e : m_cfg->exclude) {
        if (norm(e) == dir) {
          excluded = true;
          break;
        }
      }
      if (excluded) {
        current.disable_recursion_pending();
        ++current;
        continue;
      }
    } else if (entry.is_regular_file()) {
      if (entry.file_size() < m_cfg->min_size) {
        ++current;
        continue;
      }
      bool match = m_cfg->masks.empty();
      const string name = entry.path().filename().string();
      for (const auto &m : m_cfg->masks) {
        if (matches_glob(name.c_str(), m.c_str())) {
          match = true;
          break;
        }
      }
      if (match) {
        m_current_value = entry;
        ++current;
        return;
      }
    }

    ++current;
  }

  m_is_end = true;
}

bool FileList::matches_glob(const char *str, const char *pat) {
  const char *s = str;
  const char *p = pat;
  const char *last_star_s = nullptr;
  const char *last_star_p = nullptr;

  while (*s != '\0') {
    if (*p == '?' || *p == *s) {
      ++s;
      ++p;
    } else if (*p == '*') {
      // Запоминаем позицию звездочки и текущую позицию в строке
      last_star_p = p;
      last_star_s = s;
      ++p; // Пытаемся сопоставить '*' с пустой последовательностью
    } else if (last_star_p != nullptr) {
      // Не совпало, но у нас была звездочка ранее.
      // Откатываемся: считаем, что звездочка поглотила еще один символ из строки
      p = last_star_p + 1;
      ++last_star_s;
      s = last_star_s;
    } else {
      return false; // Совпадений нет и откатываться некуда
    }
  }

  // Если в конце строки остались лишние символы в паттерне,
  // они должны быть только звездочками
  while (*p == '*') {
    ++p;
  }

  return (*p == '\0');
}
