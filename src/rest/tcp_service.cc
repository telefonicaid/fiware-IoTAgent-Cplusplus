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
  //_connectionManager.reset(new CommonAsyncManager(1));
  //_connectionManager->run();
}

iota::TcpService::~TcpService() {std::cout << "iota::TcpService::~TcpServic" << std::endl;}

void iota::TcpService::handle_connection(pion::tcp::connection_ptr& tcp_conn) {
  tcp_conn->set_lifecycle(pion::tcp::connection::LIFECYCLE_CLOSE);
  std::cout << "READING" << std::endl;

  tcp_conn->async_read_some(boost::bind(
                              &iota::TcpService::handle_read,
                              this, tcp_conn,
                              boost::asio::placeholders::error,
                              boost::asio::placeholders::bytes_transferred));
  std::cout << "READING" << std::endl;
}

void iota::TcpService::handle_response(pion::tcp::connection_ptr& tcp_conn,
                                       std::string& buffer_response) {
  print_buffer(buffer_response, buffer_response.size());
  tcp_conn->async_write(boost::asio::buffer(buffer_response.data(),
                        buffer_response.size()),
                        boost::bind(&iota::TcpService::finish, this, tcp_conn));

}

void iota::TcpService::handle_read(pion::tcp::connection_ptr& tcp_conn,
                                   const boost::system::error_code& read_error,
                                   std::size_t bytes_read) {
  PION_LOG_DEBUG(m_logger,
                 "|read_error=" << read_error << "|bytes_read=" << bytes_read);
  std::string reading_buffer;
  std::string& response_buffer = create_buffer(tcp_conn);
  if (!read_error) {
    std::string reading_buffer(tcp_conn->get_read_buffer().data(),
                               bytes_read);
    print_buffer(reading_buffer, bytes_read);
  }
  tcp_conn->get_io_service().post(boost::bind(
        &iota::TcpService::handle_data, this, tcp_conn, reading_buffer,
        read_error));

  if (read_error) {
    finish(tcp_conn);
  }
};

void iota::TcpService::send_response(pion::tcp::connection_ptr& tcp_conn,
                                       std::string& buffer_response) {

  tcp_conn->get_io_service().post(boost::bind(
        &iota::TcpService::handle_response, this, tcp_conn, buffer_response));
}


void iota::TcpService::print_buffer(std::string& buffer, int bytes_read) {
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (int i = 0; i < bytes_read; ++i) {
    ss << std::setw(2) << static_cast<unsigned>(buffer[i]);
  }
  PION_LOG_DEBUG(m_logger, ss.str());
}

void iota::TcpService::finish(pion::tcp::connection_ptr& tcp_conn) {
  // Close connection and remove buffer
  clear_buffer(tcp_conn);
  tcp_conn->finish();
  PION_LOG_DEBUG(m_logger, "finish connection " << tcp_conn.use_count());
}


std::string& iota::TcpService::create_buffer(pion::tcp::connection_ptr& conn) {
  boost::mutex::scoped_lock lock(m_mutex);
  std::string buffer;
  _async_buffers[conn] = buffer;
  return _async_buffers[conn];
}

void iota::TcpService::clear_buffer(pion::tcp::connection_ptr& conn) {
  boost::mutex::scoped_lock lock(m_mutex);
  _async_buffers.erase(conn);
}

void iota::TcpService::handle_data(pion::tcp::connection_ptr& tcp_conn,
                                   std::string& request,
                                   const boost::system::error_code& err) {
  PION_LOG_ERROR(m_logger, "No implemented");
  tcp_conn->get_io_service().post(boost::bind(
                                    &iota::TcpService::finish, this, tcp_conn));
}



