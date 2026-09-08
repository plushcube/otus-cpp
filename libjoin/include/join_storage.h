#pragma once

#include <map>
#include <mutex>
#include <string>

class JoinStorage {
public:
  std::string execute(const std::string &line);

private:
  enum class TableId { A, B };

  using Table = std::map<int, std::string>;

  static bool parse_table(const std::string &s, TableId &out);
  static bool parse_id(const std::string &s, int &out);

  Table &table(TableId id) { return id == TableId::A ? m_a : m_b; }
  const Table &table(TableId id) const { return id == TableId::A ? m_a : m_b; }

  std::string insert(TableId id, int key, const std::string &name);
  std::string truncate(TableId id);
  std::string intersection() const;
  std::string symmetric_difference() const;

  mutable std::mutex m_mutex;
  Table m_a;
  Table m_b;
};
