#pragma once

#include <boost/asio.hpp>

#include <memory>
#include <string>

class JoinServer;
class JoinStorage;

class Session : public std::enable_shared_from_this<Session> {
public:
  using Buffer = boost::asio::streambuf;
  using Socket = boost::asio::ip::tcp::socket;

  Session(Socket socket, JoinStorage &storage, JoinServer &server);

  void start() { do_read(); }
  void close();

private:
  void do_read();
  void handle_lines();
  void do_write();
  void finish(boost::system::error_code ec);

  Socket m_socket;
  Buffer m_buf;
  JoinStorage &m_storage;
  JoinServer &m_server;

  std::string m_outbox;
};
