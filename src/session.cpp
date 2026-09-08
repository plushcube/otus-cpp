#include "session.h"

#include "command_executor.h"
#include "join_server.h"
#include "join_storage.h"

#include <boost/asio/buffers_iterator.hpp>

#include <algorithm>
#include <string>
#include <utility>

Session::Session(Socket socket, JoinStorage &storage, CommandExecutor &executor, Context &io,
                 JoinServer &server)
    : m_socket(std::move(socket)), m_storage(storage), m_executor(executor), m_io(io),
      m_server(server) {}

void Session::do_read() {
  auto self = shared_from_this();
  boost::asio::async_read_until(m_socket, m_buf, '\n', [this, self](boost::system::error_code ec, std::size_t) {
    if (ec) {
      finish(ec);
      return;
    }

    std::string command = take_line();
    if (command.empty()) {
      do_read();
      return;
    }

    m_executor.submit([this, self, command] {
      const std::string reply = m_storage.execute(command);
      boost::asio::post(m_io, [this, self, reply] { deliver(reply); });
    });
  });
}

std::string Session::take_line() {
  const auto begin = boost::asio::buffers_begin(m_buf.data());
  const auto end = boost::asio::buffers_end(m_buf.data());
  const auto nl = std::find(begin, end, '\n');
  if (nl == end) {
    return {};
  }
  std::string line(begin, nl);
  m_buf.consume(static_cast<std::size_t>(std::distance(begin, nl)) + 1);
  if (!line.empty() && line.back() == '\r') {
    line.pop_back();
  }
  return line;
}

void Session::deliver(const std::string &reply) {
  m_outbox = reply;
  do_write();
}

void Session::do_write() {
  if (m_outbox.empty()) {
    do_read();
    return;
  }
  auto self = shared_from_this();
  boost::asio::async_write(m_socket, boost::asio::buffer(m_outbox),
                           [this, self](boost::system::error_code ec, std::size_t) {
                             m_outbox.clear();
                             if (!ec) {
                               do_read();
                             } else {
                               finish(ec);
                             }
                           });
}

void Session::finish(boost::system::error_code) {
  m_server.on_session_closed(this);
  boost::system::error_code ignored;
  m_socket.close(ignored);
}

void Session::close() {
  boost::system::error_code ignored;
  m_socket.close(ignored);
}
