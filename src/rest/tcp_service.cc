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
#include "util/FuncUtil.h"
#include <iomanip>

iota::TcpService::TcpService(const boost::asio::ip::tcp::endpoint& endpoint)
    : pion::tcp::server(endpoint),
      m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {}
iota::TcpService::TcpService(pion::scheduler& scheduler,
                             const boost::asio::ip::tcp::endpoint& endpoint)
    : pion::tcp::server(scheduler, endpoint),
      m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {}

iota::TcpService::~TcpService() {}

boost::shared_ptr<iota::TcpService> iota::TcpService::register_handler(
    std::string client_name,
    iota::TcpService::IotaRequestHandler client_handler) {
  boost::shared_ptr<iota::TcpService> your_tcp_service;
  IOTA_LOG_DEBUG(m_logger, "Registering tcp " + client_name);
  try {
    c_handlers.insert(
        std::pair<std::string, iota::TcpService::IotaRequestHandler>(
            client_name, client_handler));
    your_tcp_service = shared_from_this();
  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, e.what());
  }
  return your_tcp_service;
}

void iota::TcpService::handle_connection(pion::tcp::connection_ptr& tcp_conn) {
  // Empty buffer in read (start connection)
  // Aplication must use b_read to decide if it is a new connection.
  // Flow dependant
  const std::vector<unsigned char> b_read;
  call_client(tcp_conn, b_read, boost::system::error_code());
}

void iota::TcpService::read(pion::tcp::connection_ptr& tcp_conn) {
  tcp_conn->async_read_some(
      boost::bind(&iota::TcpService::handle_read, this, tcp_conn,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

void iota::TcpService::handle_read(pion::tcp::connection_ptr& tcp_conn,
                                   const boost::system::error_code& read_error,
                                   std::size_t bytes_read) {
  IOTA_LOG_DEBUG(m_logger, " read_error=" << read_error
                                          << " bytes_read=" << bytes_read);
  if (!read_error) {
    const std::vector<unsigned char> reading_buffer(
        tcp_conn->get_read_buffer().begin(),
        tcp_conn->get_read_buffer().begin() + bytes_read);
    call_client(tcp_conn, reading_buffer, read_error);
  } else {
    const std::vector<unsigned char> b;
    call_client(tcp_conn, b, read_error);
    //finish(tcp_conn);
  }
};

void iota::TcpService::send(pion::tcp::connection_ptr& tcp_conn,
                            std::vector<unsigned char>& buffer_response,
                            bool wait_data) {
  // print_buffer(buffer_response, buffer_response.size());
  std::ostringstream os;
  os << tcp_conn->get_remote_endpoint();
  std::string endpoint(os.str());
  if (tcp_conn->is_open()) {
    IOTA_LOG_DEBUG(m_logger, "Sending to " + endpoint + " " +
                                 iota::str_to_hex(buffer_response));
    if (!wait_data) {
      tcp_conn->async_write(
          boost::asio::buffer(buffer_response.data(), buffer_response.size()),
          boost::bind(&iota::TcpService::finish_write, this, tcp_conn));
    } else {
      tcp_conn->async_write(
          boost::asio::buffer(buffer_response.data(), buffer_response.size()),
          boost::bind(&iota::TcpService::read, this, tcp_conn));
    }
  } else {
    IOTA_LOG_ERROR(m_logger, endpoint + " connection is closed");
  }
}

void iota::TcpService::print_buffer(std::vector<unsigned char>& buffer,
                                    int bytes_read) {
  /*
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (int i = 0; i < bytes_read; ++i) {
  ss << std::setw(2) << static_cast<unsigned>(buffer[i]);
  }
  */
  IOTA_LOG_DEBUG(m_logger, iota::str_to_hex(buffer));
}
void iota::TcpService::finish_write(pion::tcp::connection_ptr& tcp_conn) {
  // clear_buffer(tcp_conn);
}
void iota::TcpService::finish(pion::tcp::connection_ptr& tcp_conn) {
  tcp_conn->set_lifecycle(pion::tcp::connection::LIFECYCLE_CLOSE);
  tcp_conn->finish();
  clear_buffer(tcp_conn);
  IOTA_LOG_DEBUG(m_logger, "finish connection " << tcp_conn.use_count());
}

void iota::TcpService::close_connection(pion::tcp::connection_ptr& tcp_conn) {
  finish(tcp_conn);
}

void iota::TcpService::clear_buffer(pion::tcp::connection_ptr& conn) {
  boost::mutex::scoped_lock lock(m_mutex);
  _async_buffers.erase(conn);
}

std::vector<unsigned char>& iota::TcpService::create_buffer(
    pion::tcp::connection_ptr& conn) {
  boost::mutex::scoped_lock lock(m_mutex);
  std::vector<unsigned char> buffer;
  _async_buffers[conn] = buffer;
  return _async_buffers[conn];
}

void iota::TcpService::call_client(
    pion::tcp::connection_ptr& tcp_conn,
    const std::vector<unsigned char>& buffer_read,
    const boost::system::error_code& error) {
  std::map<std::string, iota::TcpService::IotaRequestHandler>::iterator it =
      c_handlers.begin();
  while (it != c_handlers.end()) {
    iota::TcpService::IotaRequestHandler h = it->second;
    if (h) {
      IOTA_LOG_DEBUG(m_logger, " client=" + it->first);
      h(tcp_conn, buffer_read, _async_buffers[tcp_conn], error);
    }
    ++it;
  }
}
