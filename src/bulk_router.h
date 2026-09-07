#pragma once

#include <cstddef>
#include <string>

#include <collector/static_collector.h>

class Dispatcher;
class Session;

class BulkRouter {
public:
  BulkRouter(Dispatcher &, size_t);

  void on_line(Session &, const std::string &);
  void on_disconnect(Session &, std::string);
  void flush_static();

private:
  Dispatcher &m_dispatcher;
  StaticCollector m_static;
};
