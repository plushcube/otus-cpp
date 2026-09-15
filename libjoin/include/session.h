#pragma once

#include <boost/asio.hpp>

#include <memory>
#include <string>

class CommandExecutor;
class JoinServer;
class JoinStorage;

class Session : public std::enable_shared_from_this<Session> {
public:
  using Buffer = boost::asio::streambuf;
  using Socket = boost::asio::ip::tcp::socket;
  using Context = boost::asio::io_context;

  Session(Socket socket, JoinStorage &storage, CommandExecutor &executor, Context &io, JoinServer &server);

  void start() { do_read(); }
  void close();

private:
  void do_read();
  std::string take_line();
  void deliver(const std::string &reply);
  void do_write();
  void finish(boost::system::error_code ec);

  Socket m_socket;
  Buffer m_buf;
  JoinStorage &m_storage;
  CommandExecutor &m_executor;
  Context &m_io;
  JoinServer &m_server;

  std::string m_outbox;
};
