#pragma once

#include <boost/asio.hpp>

#include <memory>

#include <collector/dynamic_collector.h>

class AsyncServer;
class BulkRouter;

class Session : public std::enable_shared_from_this<Session> {
public:
  using Buffer = boost::asio::streambuf;
  using Socket = boost::asio::ip::tcp::socket;

  Session(Socket socket, BulkRouter &router, AsyncServer &server);

  void start() { do_read(); }
  void close();

private:
  friend class BulkRouter;

  void do_read();
  void consume_lines();
  void finish(boost::system::error_code ec);

  Socket m_socket;
  Buffer m_buf;

  BulkRouter &m_router;
  AsyncServer &m_server;

  bool m_in_dynamic{false};
  DynamicCollector m_dynamic;
};
