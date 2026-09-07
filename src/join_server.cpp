#include "join_server.h"

#include "session.h"

#include <utility>

using namespace boost::asio::ip;

JoinServer::JoinServer(unsigned short port)
    : m_acceptor(m_io, tcp::endpoint(tcp::v4(), port)), m_port(m_acceptor.local_endpoint().port()) {
  m_acceptor.listen();
  do_accept();
}

void JoinServer::stop() {
  boost::asio::post(m_io, [this] {
    m_acceptor.close();
    for (const auto &session : m_sessions) {
      session->close();
    }
  });
}

void JoinServer::do_accept() {
  m_acceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
    if (!ec) {
      auto session = std::make_shared<Session>(std::move(socket), m_storage, *this);
      m_sessions.insert(session);
      session->start();
    }
    if (m_acceptor.is_open()) {
      do_accept();
    }
  });
}

void JoinServer::on_session_closed(Session *session) {
  for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
    if (it->get() == session) {
      m_sessions.erase(it);
      break;
    }
  }
}
