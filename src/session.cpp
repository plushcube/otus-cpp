#include "session.h"

#include "async_server.h"
#include "bulk_router.h"

#include <boost/asio/buffers_iterator.hpp>

#include <algorithm>
#include <string>
#include <utility>

Session::Session(Socket socket, BulkRouter &router, AsyncServer &server)
    : m_socket(std::move(socket)), m_router(router), m_server(server) {}

void Session::do_read() {
  auto self = shared_from_this();
  boost::asio::async_read_until(m_socket, m_buf, '\n', [this, self](boost::system::error_code ec, std::size_t) {
    if (!ec) {
      consume_lines();
      do_read();
    } else {
      finish(ec);
    }
  });
}

void Session::consume_lines() {
  for (;;) {
    const auto begin = boost::asio::buffers_begin(m_buf.data());
    const auto end = boost::asio::buffers_end(m_buf.data());
    const auto nl = std::find(begin, end, '\n');
    if (nl == end) {
      break;
    }
    std::string line(begin, nl);
    m_buf.consume(static_cast<std::size_t>(std::distance(begin, nl)) + 1);
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    if (!line.empty()) {
      m_router.on_line(*this, line);
    }
  }
}

void Session::finish(boost::system::error_code) {
  std::string tail(boost::asio::buffers_begin(m_buf.data()), boost::asio::buffers_end(m_buf.data()));
  m_buf.consume(tail.size());
  if (!tail.empty() && tail.back() == '\r') {
    tail.pop_back();
  }

  m_router.on_disconnect(*this, tail);
  m_server.on_session_closed(this);

  boost::system::error_code ignored;
  m_socket.close(ignored);
}

void Session::close() {
  boost::system::error_code ignored;
  m_socket.close(ignored);
}
