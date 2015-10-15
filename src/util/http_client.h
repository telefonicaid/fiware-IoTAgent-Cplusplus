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
#ifndef SRC_UTIL_HTTP_CLIENT_H_
#define SRC_UTIL_HTTP_CLIENT_H_

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>

#include <pion/tcp/connection.hpp>
#include <pion/error.hpp>
#include <pion/http/response_reader.hpp>

namespace iota {

class HttpClient : public boost::enable_shared_from_this<HttpClient> {
 public:
  typedef boost::function3<
      void, boost::shared_ptr<HttpClient>, pion::http::response_ptr,
      const boost::system::error_code&> application_callback_t;

  HttpClient(std::string server, unsigned int port,
             std::string id = std::string());

  HttpClient(boost::asio::io_service& io, std::string server, unsigned int port,
             std::string id = std::string());

  virtual ~HttpClient();

  pion::http::response_ptr send(
      pion::http::request_ptr request, unsigned int timeout, std::string proxy,
      application_callback_t callback = application_callback_t());
  void async_send(pion::http::request_ptr request, unsigned int timeout,
                  std::string proxy, application_callback_t callback);

  pion::http::request_ptr get_request();

  std::string get_identifier();

  pion::http::response_ptr get_response();

  boost::system::error_code get_error();

  std::string getRemoteEndpoint();

  void stop();

 protected:
 private:

  // If synchronous use a local io_service
  boost::asio::io_service _local_io;
  bool _synch_http_client;

  // Remote endpoint (may be different to connection data, if proxy is used)
  std::string _remote_ip;
  unsigned int _remote_port;

  // Connection
  bool _proxy;
  boost::asio::ip::address _ip;
  unsigned int _port;
  std::string _id;
  pion::tcp::connection_ptr _connection;
  boost::shared_ptr<boost::asio::deadline_timer> _timer;
  boost::shared_ptr<boost::asio::io_service::strand> _strand;
  boost::system::error_code _ec;
  boost::mutex _m;
  pion::http::response_ptr _response;
  pion::http::request_ptr _request;
  application_callback_t _callback;
  pion::logger m_logger;

  void generate_identifier();

  void connect();

  void write();

  void read();

  void connectHandle(const boost::system::error_code& ec);

  void readResponse(const boost::system::error_code& ec);
  // std::size_t bytes_written);

  void checkResponse(const pion::http::response_ptr& http_response_ptr,
                     const pion::tcp::connection_ptr& conn_ptr,
                     const boost::system::error_code& ec);
  void endWriteProxy(const boost::system::error_code& ec,
                     std::size_t bytes_writen);
  void endConnectProxy(const boost::system::error_code& ec,
                       std::size_t bytes_read);

  void timeout_connection(const boost::system::error_code& ec);

  void set_error(boost::system::error_code ec);

  bool check_connection();

  void resolve(std::string address, std::string port);

  void set_proxy(std::string proxy);
};
};
#endif
