#include "join_storage.h"

#include <set>
#include <sstream>
#include <vector>

namespace {

std::vector<std::string> split(const std::string &line) {
  std::vector<std::string> out;
  std::stringstream ss(line);
  std::string tok;
  while (ss >> tok) {
    out.push_back(tok);
  }
  return out;
}

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

  const auto tokens = split(line);
  if (tokens.empty()) {
    return "OK\n";
  }

  if (tokens[0] == "INSERT") {
    TableId table{};
    int id = 0;
    if (tokens.size() != 4 || !parse_table(tokens[1], table) || !parse_id(tokens[2], id)) {
      return "ERR invalid arguments\n";
    }
    return insert(table, id, tokens[3]);
  }

  if (tokens[0] == "TRUNCATE") {
    TableId table{};
    if (tokens.size() != 2 || !parse_table(tokens[1], table)) {
      return "ERR invalid arguments\n";
    }
    return truncate(table);
  }

  if (tokens[0] == "INTERSECTION") {
    if (tokens.size() != 1) {
      return "ERR invalid arguments\n";
    }
    return intersection();
  }

  if (tokens[0] == "SYMMETRIC_DIFFERENCE") {
    if (tokens.size() != 1) {
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
