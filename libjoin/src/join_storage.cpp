#include "join_storage.h"

#include <algorithm>
#include <vector>

namespace {

std::string join_reply(const std::vector<std::string> &rows) {
  std::string out;
  for (const auto &row : rows) {
    out += row;
    out += '\n';
  }
  out += "OK\n";
  return out;
}

} // namespace

std::string JoinStorage::execute(const std::string &line) {
  std::lock_guard lk(m_mutex);

  if (line.empty()) {
    return "OK\n";
  }

  const auto sp1 = line.find(' ');
  const std::string command = line.substr(0, sp1);

  if (command == "INSERT") {
    if (sp1 == std::string::npos) {
      return "ERR invalid arguments\n";
    }
    const auto sp2 = line.find(' ', sp1 + 1);
    if (sp2 == std::string::npos) {
      return "ERR invalid arguments\n";
    }
    const auto sp3 = line.find(' ', sp2 + 1);
    if (sp3 == std::string::npos) {
      return "ERR invalid arguments\n";
    }

    const std::string table_token = line.substr(sp1 + 1, sp2 - sp1 - 1);
    const std::string id_token = line.substr(sp2 + 1, sp3 - sp2 - 1);
    const std::string name = line.substr(sp3 + 1);

    TableId table{};
    int id = 0;
    if (!parse_table(table_token, table) || !parse_id(id_token, id) || name.empty()) {
      return "ERR invalid arguments\n";
    }
    return insert(table, id, name);
  }

  if (command == "TRUNCATE") {
    if (sp1 == std::string::npos) {
      return "ERR invalid arguments\n";
    }
    TableId table{};
    if (!parse_table(line.substr(sp1 + 1), table)) {
      return "ERR invalid arguments\n";
    }
    return truncate(table);
  }

  if (command == "INTERSECTION") {
    if (sp1 != std::string::npos) {
      return "ERR invalid arguments\n";
    }
    return intersection();
  }

  if (command == "SYMMETRIC_DIFFERENCE") {
    if (sp1 != std::string::npos) {
      return "ERR invalid arguments\n";
    }
    return symmetric_difference();
  }

  return "ERR unknown command\n";
}

bool JoinStorage::parse_table(const std::string &s, TableId &out) {
  if (s == "A") {
    out = TableId::A;
    return true;
  }
  if (s == "B") {
    out = TableId::B;
    return true;
  }
  return false;
}

bool JoinStorage::parse_id(const std::string &s, int &out) {
  if (s.empty()) {
    return false;
  }
  std::size_t pos = 0;
  try {
    out = std::stoi(s, &pos);
  } catch (const std::exception &) {
    return false;
  }
  return pos == s.size();
}

std::string JoinStorage::insert(TableId id, int key, const std::string &name) {
  Table &t = table(id);
  if (t.find(key) != t.end()) {
    return "ERR duplicate " + std::to_string(key) + "\n";
  }
  t[key] = name;
  return "OK\n";
}

std::string JoinStorage::truncate(TableId id) {
  table(id).clear();
  return "OK\n";
}

std::string JoinStorage::intersection() const {
  std::vector<std::string> rows;
  rows.reserve(std::min(m_a.size(), m_b.size()));

  auto a = m_a.begin();
  auto b = m_b.begin();
  while (a != m_a.end() && b != m_b.end()) {
    if (a->first < b->first) {
      ++a;
    } else if (b->first < a->first) {
      ++b;
    } else {
      rows.push_back(std::to_string(a->first) + "," + a->second + "," + b->second);
      ++a;
      ++b;
    }
  }
  return join_reply(rows);
}

std::string JoinStorage::symmetric_difference() const {
  std::vector<std::string> rows;
  rows.reserve(m_a.size() + m_b.size());

  auto a = m_a.begin();
  auto b = m_b.begin();
  while (a != m_a.end() || b != m_b.end()) {
    if (b == m_b.end() || (a != m_a.end() && a->first < b->first)) {
      rows.push_back(std::to_string(a->first) + "," + a->second + ",");
      ++a;
    } else if (a == m_a.end() || b->first < a->first) {
      rows.push_back(std::to_string(b->first) + ",," + b->second);
      ++b;
    } else {
      ++a;
      ++b;
    }
  }
  return join_reply(rows);
}
