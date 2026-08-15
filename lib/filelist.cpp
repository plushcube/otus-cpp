#include <filelist.h>
#include <filesystem>

using namespace std;

bool matches_glob(const char *str, const char *pat) {
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

void FileList::Iterator::advance() {
  while (!m_stack.empty()) {
    auto &current = m_stack.top();
    if (current == m_end_it) {
      m_stack.pop();
      continue;
    }

    const auto &entry = *current;

    if (m_cfg.depth > 0 && current.depth() > static_cast<int>(m_cfg.depth)) {
      current.disable_recursion_pending();
    }

    if (entry.is_directory()) {
      bool excluded = false;
      for (const auto &e : m_cfg.exclude) {
        if (std::filesystem::equivalent(entry.path(), e)) {
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
      bool match = m_cfg.masks.empty();
      string name = entry.path().filename().string();
      for (const auto &m : m_cfg.masks) {
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
