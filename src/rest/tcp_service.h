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

#ifndef SRC_REST_TCP_SERVICE_H_
#define SRC_REST_TCP_SERVICE_H_

#include <pion/tcp/server.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
namespace iota {

template <class Derived>
class enable_shared_from_this_wrapper {
 public:
  boost::shared_ptr<Derived> shared_from_this_wrapper() {
    return boost::static_pointer_cast<Derived>(
        static_cast<Derived*>(this)->shared_from_this());
  }

  boost::shared_ptr<Derived> shared_from_this_wrapper() const {
    return boost::static_pointer_cast<Derived>(
        static_cast<Derived*>(this)->shared_from_this());
  }
};
class TcpService : public pion::tcp::server,
                   public boost::enable_shared_from_this<TcpService> {
 public:
  // Application handler sync
  typedef boost::function<void(
      pion::tcp::connection_ptr&, const std::vector<unsigned char>&,
      std::vector<unsigned char>&, const boost::system::error_code&)>
      IotaRequestHandler;

  TcpService(const boost::asio::ip::tcp::endpoint& endpoint);
  TcpService(pion::scheduler& scheduler,
             const boost::asio::ip::tcp::endpoint& endpoint);
  virtual ~TcpService();
  boost::shared_ptr<iota::TcpService> register_handler(
      std::string client_name,
      iota::TcpService::IotaRequestHandler client_handler);
  void read(pion::tcp::connection_ptr& tcp_conn);
  void send(pion::tcp::connection_ptr& tcp_conn,
            std::vector<unsigned char>& buffer_response,
            bool wait_data = false);
  void close_connection(pion::tcp::connection_ptr& tcp_conn);

 private:
  virtual void handle_connection(pion::tcp::connection_ptr& tcp_conn);
  void handle_read(pion::tcp::connection_ptr& tcp_conn,
                   const boost::system::error_code& read_error,
                   std::size_t bytes_read);
  void print_buffer(std::vector<unsigned char>& buffer, int bytes_read);
  void finish_write(pion::tcp::connection_ptr& tcp_conn);
  void finish(pion::tcp::connection_ptr& tcp_conn);

  // Lock buffer map. Every connection buffer to send
  boost::mutex m_mutex;
  std::map<pion::tcp::connection_ptr, std::vector<unsigned char> >
      _async_buffers;
  void clear_buffer(pion::tcp::connection_ptr& tcp_conn);
  std::vector<unsigned char>& create_buffer(
      pion::tcp::connection_ptr& tcp_conn);

  // Call client
  void call_client(pion::tcp::connection_ptr& tcp_conn,
                   const std::vector<unsigned char>& buffer_read,
                   const boost::system::error_code& error);

  // Logger
  pion::logger m_logger;

  // Clients
  std::map<std::string, iota::TcpService::IotaRequestHandler> c_handlers;
};
}
#endif
