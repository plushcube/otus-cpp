#include <filelist.h>

#include <algorithm>
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>

using namespace std;

FileList::Iterator::Iterator(const Config &cfg) : m_cfg(&cfg), m_is_end(false) {
  vector<filesystem::path> roots;
  for (const auto &dir : cfg.dirs) {
    const auto d = filesystem::absolute(dir).lexically_normal();
    roots.erase(remove_if(roots.begin(), roots.end(), [&](const filesystem::path &r) { return is_within(r, d); }),
                roots.end());
    const bool nested = any_of(roots.begin(), roots.end(), [&](const filesystem::path &r) { return is_within(d, r); });
    if (!nested) {
      roots.push_back(d);
    }
  }

  for (auto it = roots.rbegin(); it != roots.rend(); ++it) {
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

    if (current.depth() >= static_cast<int>(m_cfg->depth)) {
      current.disable_recursion_pending();
    }

    if (entry.is_directory()) {
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
      string name = entry.path().filename().string();
      transform(name.begin(), name.end(), name.begin(),
                [](auto c) { return ::tolower(static_cast<unsigned char>(c)); });

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

bool FileList::is_within(const filesystem::path &p, const filesystem::path &root) {
  auto r = root.begin();
  auto q = p.begin();
  for (; r != root.end(); ++r, ++q) {
    if (q == p.end() || *r != *q) {
      return false;
    }
  }
  return true;
}
