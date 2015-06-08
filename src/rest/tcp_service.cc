/**
* Copyright 2015 Telefonica Investigación y Desarrollo, S.A.U
*
* This file is part of iotagent project.
*
* iotagent is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published
* by the Free Software Foundation, either version 3 of the License,
* or (at your option) any later version.
*
* iotagent is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with iotagent. If not, see http://www.gnu.org/licenses/.
*
* For those usages not covered by the GNU Affero General Public License
* please contact with iot_support at tid dot es
*/

#include "tcp_service.h"
#include "services/admin_service.h"
#include <iomanip>

namespace iota {
extern std::string logger;
}

iota::TcpService::TcpService(const boost::asio::ip::tcp::endpoint& endpoint):
  pion::tcp::server(endpoint), m_logger(PION_GET_LOGGER(logger)) {
}

iota::TcpService::~TcpService() {}

boost::shared_ptr<iota::TcpService> iota::TcpService::register_handler(
  std::string client_name,
  iota::TcpService::IotaRequestHandler client_handler) {
  boost::shared_ptr<iota::TcpService> your_tcp_service;
  try {
    c_handlers.insert(std::pair<std::string, iota::TcpService::IotaRequestHandler>
                      (client_name, client_handler));
    your_tcp_service = shared_from_this();
  }
  catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, e.what());
  }
  return your_tcp_service;
}

void iota::TcpService::handle_connection(pion::tcp::connection_ptr& tcp_conn) {
  //tcp_conn->set_lifecycle(pion::tcp::connection::LIFECYCLE_CLOSE);
  tcp_conn->async_read_some(boost::bind(
                              &iota::TcpService::handle_read,
                              this, tcp_conn,
                              boost::asio::placeholders::error,
                              boost::asio::placeholders::bytes_transferred));
}

void iota::TcpService::handle_read(pion::tcp::connection_ptr& tcp_conn,
                                   const boost::system::error_code& read_error,
                                   std::size_t bytes_read) {
  IOTA_LOG_DEBUG(m_logger,
                 " read_error=" << read_error << " bytes_read=" << bytes_read);
  std::string reading_buffer;
  if (!read_error) {
    std::string reading_buffer(tcp_conn->get_read_buffer().data(),
                               bytes_read);
    print_buffer(reading_buffer, bytes_read);
  }

  std::map<std::string, iota::TcpService::IotaRequestHandler>::iterator it =
    c_handlers.begin();
  while (it != c_handlers.end()) {
    iota::TcpService::IotaRequestHandler h = it->second;
    if (h) {
      IOTA_LOG_DEBUG(m_logger, " client=" + it->first);
      h(tcp_conn, reading_buffer, read_error);
    }
    ++it;
  }

  //if (read_error) {
    finish(tcp_conn);
  //}

};

void iota::TcpService::send_response(pion::tcp::connection_ptr& tcp_conn,
                                     std::string& buffer_response,
                                     bool close_connection) {

  print_buffer(buffer_response, buffer_response.size());
  // TODO ¿Pueden mezclarse?
  if (tcp_conn->is_open()) {
    tcp_conn->async_write(boost::asio::buffer(buffer_response.data(),
                          buffer_response.size()),
                          boost::bind(&iota::TcpService::finish, this, tcp_conn, close_connection));
  }
  else {
    IOTA_LOG_ERROR(m_logger, "Connection is closed");
  }

}


void iota::TcpService::print_buffer(std::string& buffer, int bytes_read) {
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (int i = 0; i < bytes_read; ++i) {
    ss << std::setw(2) << static_cast<unsigned>(buffer[i]);
  }
  IOTA_LOG_DEBUG(m_logger, ss.str());
}

void iota::TcpService::finish(pion::tcp::connection_ptr& tcp_conn,
                              bool close_connection) {
  if (close_connection) {
    tcp_conn->set_lifecycle(pion::tcp::connection::LIFECYCLE_CLOSE);
    tcp_conn->finish();
    IOTA_LOG_DEBUG(m_logger, "finish connection " << tcp_conn.use_count());
  }
}

void iota::TcpService::close_connection(pion::tcp::connection_ptr& tcp_conn) {
  finish(tcp_conn);
}





