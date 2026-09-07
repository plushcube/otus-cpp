#pragma once

#include <boost/asio.hpp>

#include <cstddef>
#include <memory>
#include <set>

#include <async/dispatcher.h>

#include "bulk_router.h"

class Session;

class AsyncServer {
public:
  using Context = boost::asio::io_context;
  using Acceptor = boost::asio::ip::tcp::acceptor;

  AsyncServer(unsigned short port, size_t block_size);
  ~AsyncServer() { m_dispatcher.stop(); }

  void run() { m_io.run(); }
  void stop();

  Context &io_context() { return m_io; }
  unsigned short port() const { return m_port; }

  void on_session_closed(Session *);

private:
  void do_accept();

  Context m_io;
  Acceptor m_acceptor;

  Dispatcher m_dispatcher;
  BulkRouter m_router;

  std::set<std::shared_ptr<Session>> m_sessions;
  unsigned short m_port;
};
