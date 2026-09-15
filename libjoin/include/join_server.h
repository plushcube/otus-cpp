#pragma once

#include <boost/asio.hpp>

#include <memory>
#include <set>

#include "command_executor.h"
#include "join_storage.h"

class Session;

class JoinServer {
public:
  using Context = boost::asio::io_context;
  using Acceptor = boost::asio::ip::tcp::acceptor;

  JoinServer(unsigned short port);
  ~JoinServer();

  void run() { m_io.run(); }
  void stop();

  Context &io_context() { return m_io; }
  unsigned short port() const { return m_port; }

  void on_session_closed(Session *session);

private:
  void do_accept();

  Context m_io;
  Acceptor m_acceptor;
  JoinStorage m_storage;
  CommandExecutor m_executor;
  std::set<std::shared_ptr<Session>> m_sessions;
  unsigned short m_port;
};
