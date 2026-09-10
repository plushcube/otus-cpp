#include "join_storage.h"

#include <set>
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
  for (const auto &[id, name_a] : m_a) {
    const auto it = m_b.find(id);
    if (it != m_b.end()) {
      rows.push_back(std::to_string(id) + "," + name_a + "," + it->second);
    }
  }
  return join_reply(rows);
}

std::string JoinStorage::symmetric_difference() const {
  std::set<int> ids;
  for (const auto &entry : m_a) {
    ids.insert(entry.first);
  }
  for (const auto &entry : m_b) {
    ids.insert(entry.first);
  }

  std::vector<std::string> rows;
  for (const int id : ids) {
    const auto in_a = m_a.find(id);
    const auto in_b = m_b.find(id);
    const bool has_a = in_a != m_a.end();
    const bool has_b = in_b != m_b.end();
    if (has_a && has_b) {
      continue;
    }
    rows.push_back(std::to_string(id) + "," + (has_a ? in_a->second : "") + "," + (has_b ? in_b->second : ""));
  }
  return join_reply(rows);
}
