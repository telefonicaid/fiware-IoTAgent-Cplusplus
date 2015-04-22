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
namespace iota {

template<class Derived>
class enable_shared_from_this_wrapper {
  public:
    boost::shared_ptr<Derived> shared_from_this_wrapper() {
      return boost::static_pointer_cast<Derived>(static_cast<Derived*>(this)->shared_from_this());
    }

    boost::shared_ptr<Derived> shared_from_this_wrapper() const {
      return boost::static_pointer_cast<Derived>(static_cast<Derived*>(this)->shared_from_this());
    }
};
class TcpService: public pion::tcp::server {
                   //public iota::enable_shared_from_this_wrapper<iota::TcpService> {
  public:
    // Application handler
    typedef boost::function<void (
      pion::tcp::connection_ptr&,
      const std::string&,
      std::string&,
      boost::system::error_code&)> IotaRequestHandler;

    TcpService(const boost::asio::ip::tcp::endpoint& endpoint);
    virtual ~TcpService();
    virtual void handle_data(pion::tcp::connection_ptr& tcp_conn,
                     std::string& request,
                     const boost::system::error_code& err);

  protected:
    void send_response(pion::tcp::connection_ptr& tcp_conn,
                       std::string& buffer_response);

  private:
    virtual void handle_connection(pion::tcp::connection_ptr& tcp_conn);
    void handle_read(pion::tcp::connection_ptr& tcp_conn,
                     const boost::system::error_code& read_error,
                     std::size_t bytes_read);
    void handle_response(pion::tcp::connection_ptr& tcp_conn,
                       std::string& buffer_response);
    void print_buffer(std::string& buffer, int bytes_read);
    std::string& create_buffer(pion::tcp::connection_ptr& tcp_conn);
    void finish(pion::tcp::connection_ptr& tcp_conn);
    void clear_buffer(pion::tcp::connection_ptr& tcp_conn);
    // Buffers to answers (async)
    std::map<pion::tcp::connection_ptr, std::string> _async_buffers;

    // Lock buffer map
    boost::mutex m_mutex;

    // For async communications. It provides a parallel event loop.
    //boost::shared_ptr<CommonAsyncManager> _connectionManager;

    pion::logger m_logger;
};
}
#endif
