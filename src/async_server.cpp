#include "async_server.h"

#include "session.h"

#include <utility>

using namespace boost::asio::ip;

AsyncServer::AsyncServer(unsigned short port, size_t block_size)
    : m_acceptor(m_io, tcp::endpoint(tcp::v4(), port)), m_dispatcher(), m_router(m_dispatcher, block_size),
      m_port(m_acceptor.local_endpoint().port()) {
  m_acceptor.listen();
  m_dispatcher.start();
  do_accept();
}

void AsyncServer::stop() {
  boost::asio::post(m_io, [this] {
    m_acceptor.close();
    for (const auto &session : m_sessions) {
      session->close();
    }
  });
}

void AsyncServer::do_accept() {
  m_acceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
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
