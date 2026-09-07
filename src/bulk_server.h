#pragma once

#include <boost/asio.hpp>

#include <cstddef>
#include <memory>
#include <set>
#include <string>

#include <async/dispatcher.h>
#include <collector/dynamic_collector.h>
#include <collector/static_collector.h>

class AsyncServer;
class BulkRouter;
class Session;

class BulkRouter {
public:
  BulkRouter(Dispatcher &dispatcher, size_t block_size);

  void on_line(Session &session, const std::string &line);
  void on_disconnect(Session &session, std::string tail);
  void flush_static();

private:
  Dispatcher &m_dispatcher;
  StaticCollector m_static;
};

class Session : public std::enable_shared_from_this<Session> {
public:
  Session(boost::asio::ip::tcp::socket socket, BulkRouter &router, AsyncServer &server);

  void start();
  void close();

private:
  friend class BulkRouter;

  void do_read();
  void consume_lines();
  void finish(boost::system::error_code ec);

  boost::asio::ip::tcp::socket m_socket;
  boost::asio::streambuf m_buf;
  BulkRouter &m_router;
  AsyncServer &m_server;

  bool m_in_dynamic{false};
  DynamicCollector m_dynamic;
};

class AsyncServer {
public:
  AsyncServer(unsigned short port, size_t block_size);
  ~AsyncServer();

  void run();
  void stop();

  boost::asio::io_context &io_context() { return m_io; }
  unsigned short port() const { return m_port; }

  void on_session_closed(Session *session);

private:
  void do_accept();

  boost::asio::io_context m_io;
  boost::asio::ip::tcp::acceptor m_acceptor;
  Dispatcher m_dispatcher;
  BulkRouter m_router;
  std::set<std::shared_ptr<Session>> m_sessions;
  unsigned short m_port;
};
