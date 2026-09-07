#include "bulk_server.h"

#include <boost/asio/buffers_iterator.hpp>

#include <algorithm>
#include <utility>

#include <commands/cmd_builder.h>
#include <commands/command.h>

// ---------------------------------------------------------------------------
// BulkRouter
// ---------------------------------------------------------------------------

BulkRouter::BulkRouter(Dispatcher &dispatcher, size_t block_size) : m_dispatcher(dispatcher), m_static(block_size) {}

void BulkRouter::on_line(Session &session, const std::string &line) {
  const Command cmd = CommandBuilder::make_command(line);

  switch (cmd.type) {
  case Command::Type::Command:
    if (session.m_in_dynamic) {
      session.m_dynamic.collect(cmd);
    } else {
      m_static.collect(cmd);
      if (m_static.is_full()) {
        flush_static();
      }
    }
    break;

  case Command::Type::BlockStart:
    if (!session.m_in_dynamic) {
      flush_static();
      session.m_in_dynamic = true;
    }
    session.m_dynamic.collect(cmd);
    break;

  case Command::Type::BlockEnd:
    if (!session.m_in_dynamic) {
      break;
    }
    session.m_dynamic.collect(cmd);
    if (session.m_dynamic.is_full()) {
      const Collector::Bulk bulk = session.m_dynamic.flush();
      session.m_in_dynamic = false;
      if (!bulk.commands.empty()) {
        m_dispatcher.dispatch(bulk);
      }
    }
    break;
  }
}

void BulkRouter::on_disconnect(Session &session, std::string tail) {
  if (!tail.empty()) {
    on_line(session, tail);
  }
}

void BulkRouter::flush_static() {
  const Collector::Bulk bulk = m_static.flush();
  if (!bulk.commands.empty()) {
    m_dispatcher.dispatch(bulk);
  }
}

// ---------------------------------------------------------------------------
// Session
// ---------------------------------------------------------------------------

Session::Session(boost::asio::ip::tcp::socket socket, BulkRouter &router, AsyncServer &server)
    : m_socket(std::move(socket)), m_router(router), m_server(server) {}

void Session::start() { do_read(); }

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
    if (!line.empty() && line.back() == '\r') { // допустим CRLF
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

// ---------------------------------------------------------------------------
// AsyncServer
// ---------------------------------------------------------------------------

AsyncServer::AsyncServer(unsigned short port, size_t block_size)
    : m_acceptor(m_io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), m_dispatcher(),
      m_router(m_dispatcher, block_size), m_port(m_acceptor.local_endpoint().port()) {
  m_acceptor.listen();
  m_dispatcher.start();
  do_accept();
}

AsyncServer::~AsyncServer() { m_dispatcher.stop(); }

void AsyncServer::run() { m_io.run(); }

void AsyncServer::stop() {
  boost::asio::post(m_io, [this] {
    m_acceptor.close();
    for (const auto &session : m_sessions) {
      session->close();
    }
  });
}

void AsyncServer::do_accept() {
  m_acceptor.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
    if (!ec) {
      auto session = std::make_shared<Session>(std::move(socket), m_router, *this);
      m_sessions.insert(session);
      session->start();
    }
    if (m_acceptor.is_open()) {
      do_accept();
    }
  });
}

void AsyncServer::on_session_closed(Session *session) {
  for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
    if (it->get() == session) {
      m_sessions.erase(it);
      break;
    }
  }
  if (m_sessions.empty()) {
    m_router.flush_static();
  }
}
