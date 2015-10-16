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
#include "http_client.h"
#include "iota_exception.h"
#include "rest/types.h"
#include "rest/process.h"
#include "FuncUtil.h"
#include <boost/bind.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <pion/http/request_writer.hpp>

iota::HttpClient::HttpClient(std::string server, unsigned int port,
                             std::string id)
    : _synch_http_client(true),
      _remote_ip(server),
      _remote_port(port),
      _port(port),
      _proxy(false),
      _id(id),
      m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  _connection.reset(new pion::tcp::connection(_local_io));
  _connection->set_lifecycle(pion::tcp::connection::LIFECYCLE_CLOSE);
  _strand.reset(new boost::asio::io_service::strand(_local_io));

  _timer.reset(new boost::asio::deadline_timer(_local_io));
  resolve(_remote_ip, boost::lexical_cast<std::string>(_remote_port));

  _ec = boost::system::errc::make_error_code(boost::system::errc::success);

  generate_identifier();
}

iota::HttpClient::HttpClient(boost::asio::io_service& io, std::string server,
                             unsigned int port, std::string id)
    : _synch_http_client(false),
      _remote_ip(server),
      _remote_port(port),
      _id(id),
      _port(port),
      _proxy(false),
      m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {

  _connection.reset(new pion::tcp::connection(io));
  _connection->set_lifecycle(pion::tcp::connection::LIFECYCLE_CLOSE);
  _strand.reset(new boost::asio::io_service::strand(io));

  _timer.reset(new boost::asio::deadline_timer(io));
  resolve(_remote_ip, boost::lexical_cast<std::string>(_remote_port));

  _ec = boost::system::errc::make_error_code(boost::system::errc::success);

  generate_identifier();
}

iota::HttpClient::~HttpClient() {
  // TODO Remove
  IOTA_LOG_DEBUG(m_logger, "Destructor " + get_identifier());
}

pion::http::response_ptr iota::HttpClient::send(
    pion::http::request_ptr request, unsigned int timeout, std::string proxy,
    application_callback_t callback) {
  // Si proxy, entonces hay que modificar los datos a usar para la conexion
  // set_error(boost::asio::error::would_block);
  set_error(boost::system::errc::make_error_code(boost::system::errc::success));

  _callback = callback;
  _response.reset();
  _request.reset();
  _request = request;
  _proxy = false;

  set_proxy(proxy);

  pion::http::response_ptr response_ptr;

  _timer->expires_from_now(boost::posix_time::seconds(timeout));
  _timer->async_wait(_strand->wrap(
      boost::bind(&HttpClient::timeout_connection, shared_from_this(),
                  boost::asio::placeholders::error)));

  connect();
  if (_synch_http_client) {
    // End io_service
    while (_local_io.run() > 0);
    _local_io.stop();
  }
  return _response;
}

void iota::HttpClient::async_send(pion::http::request_ptr request,
                                  unsigned int timeout, std::string proxy,
                                  application_callback_t callback) {
  // Si proxy, entonces hay que modificar los datos a usar para la conexion
  // set_error(boost::asio::error::would_block);
  set_error(boost::system::errc::make_error_code(boost::system::errc::success));
  _callback = callback;
  _response.reset();
  _request.reset();
  _request = request;
  _proxy = false;

  set_proxy(proxy);
  _timer->expires_from_now(boost::posix_time::seconds(timeout));
  _timer->async_wait(_strand->wrap(
      boost::bind(&HttpClient::timeout_connection, shared_from_this(),
                  boost::asio::placeholders::error)));
  connect();

  return;
}

void iota::HttpClient::set_proxy(std::string proxy) {
  try {
    if (proxy.empty() == false) {
      std::string proxy_server(proxy);
      unsigned int proxy_port = 80;
      size_t n_p = proxy.find(":");
      if (n_p != std::string::npos) {
        std::string p;
        p.append(proxy, n_p + 1, std::string::npos);
        proxy_port = boost::lexical_cast<unsigned int>(p);
      }
      proxy_server.assign(proxy, 0, n_p);
      _port = proxy_port;
      resolve(proxy_server, boost::lexical_cast<std::string>(_port));
      _proxy = true;
    }
  } catch (std::exception& e) {
    // set_error(boost::asio::error::address_not_available);
  }
  std::string host_header(_remote_ip);
  host_header.append(":");
  host_header.append(boost::lexical_cast<std::string>(_remote_port));
  _request->change_header(pion::http::types::HEADER_HOST, host_header);
}

std::string iota::HttpClient::get_identifier() { return _id; }

pion::http::response_ptr iota::HttpClient::get_response() {
  pion::http::response_ptr response_ptr = _response;
  return response_ptr;
}

pion::http::request_ptr iota::HttpClient::get_request() { return _request; }

void iota::HttpClient::connect() {
  _connection->async_connect(
      _ip, _port,
      _strand->wrap(boost::bind(&HttpClient::connectHandle, shared_from_this(),
                                boost::asio::placeholders::error)));
  if (_synch_http_client) {
    _local_io.run_one();
  }
}

void iota::HttpClient::write() {
  pion::http::request_writer_ptr request_writer_ptr(
      pion::http::request_writer::create(
          _connection, _request,
          _strand->wrap(boost::bind(&HttpClient::readResponse,
                                    shared_from_this(),
                                    boost::asio::placeholders::error))));

  request_writer_ptr->send();
  if (_synch_http_client) {
    _local_io.run_one();
  }
}

void iota::HttpClient::read() {
  pion::http::response_reader_ptr reader_ptr(
      pion::http::response_reader::create(
          _connection, *_request,
          _strand->wrap(boost::bind(&HttpClient::checkResponse,
                                    shared_from_this(), _1, _2, _3))));
  reader_ptr->receive();

  if (_synch_http_client) {
    while (_response.get() == NULL && check_connection()) {
      _local_io.run_one();
    }
  }
}

void iota::HttpClient::connectHandle(const boost::system::error_code& ec) {

  IOTA_LOG_DEBUG(m_logger, get_identifier());
  set_error(ec);
  if (check_connection()) {
    // Si se esta utilizando proxy

    if (_proxy) {
      std::stringstream request_CONNECT;
      request_CONNECT << "CONNECT " << _remote_ip << ":" << _remote_port
                      << " HTTP/1.1\r\n";
      std::string h_outgoing =
          _request->get_header(iota::types::HEADER_OUTGOING_ROUTE);
      if (!h_outgoing.empty()) {
        request_CONNECT << iota::types::HEADER_OUTGOING_ROUTE << ": "
                        << h_outgoing << "\r\n";
      }
      request_CONNECT << "\r\n";
      boost::system::error_code error_code;
      _connection->write(boost::asio::buffer(request_CONNECT.str()),
                         error_code);
      _connection->async_read_some(_strand->wrap(
          boost::bind(&HttpClient::endConnectProxy, shared_from_this(),
                      boost::asio::placeholders::error,
                      boost::asio::placeholders::bytes_transferred)));
      if (_synch_http_client) {
        _local_io.run_one();
      }
    } else {
      write();
    }
  }
}

void iota::HttpClient::endWriteProxy(const boost::system::error_code& ec,
                                     std::size_t bytes_writen) {
  set_error(ec);
  if (check_connection()) {
    _connection->async_read_some(_strand->wrap(
        boost::bind(&HttpClient::endConnectProxy, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
    if (_synch_http_client) {
      _local_io.run_one();
    }
  }
}

void iota::HttpClient::endConnectProxy(const boost::system::error_code& ec,
                                       std::size_t bytes_read) {
  std::string resp_proxy(_connection->get_read_buffer().data());
  boost::system::error_code ec_proxy = ec;
  size_t pos_OK = resp_proxy.find(" 200 ");
  if ((pos_OK == std::string::npos) && (!ec)) {
    ec_proxy = boost::asio::error::host_unreachable;
  }
  set_error(ec_proxy);
  if (check_connection()) {
    write();
  }
}

void iota::HttpClient::readResponse(const boost::system::error_code& ec) {
  // std::size_t bytes_written) {
  IOTA_LOG_DEBUG(m_logger, get_identifier());
  set_error(ec);
  if (check_connection()) {
    read();
  }
}

void iota::HttpClient::checkResponse(
    const pion::http::response_ptr& http_response_ptr,
    const pion::tcp::connection_ptr& conn_ptr,
    const boost::system::error_code& ec) {
  IOTA_LOG_DEBUG(m_logger, get_identifier());
  _response = http_response_ptr;
  if (_timer) {
    _timer->cancel();
    _timer.reset();
  }
  if (_callback) {
    _callback(shared_from_this(), _response, ec);
    _callback = NULL;
  }
  stop();
}

void iota::HttpClient::timeout_connection(const boost::system::error_code& ec) {

  if ((!ec) || (ec != boost::asio::error::operation_aborted)) {
    IOTA_LOG_DEBUG(m_logger, get_identifier());
    set_error(boost::asio::error::timed_out);
    check_connection();
  }
}

void iota::HttpClient::set_error(boost::system::error_code ec) {
  boost::mutex::scoped_lock l(_m);
  _ec = ec;
}

boost::system::error_code iota::HttpClient::get_error() {
  boost::mutex::scoped_lock l(_m);
  return _ec;
}

bool iota::HttpClient::check_connection() {
  bool success = true;
  boost::system::error_code ec = get_error();
  if ((ec && ec != boost::asio::error::would_block) ||
      (!_connection->is_open())) {
    success = false;
    if (_callback) {
      _callback(shared_from_this(), _response, ec);
      _callback = NULL;
    }
    stop();
  }
  return success;
}

void iota::HttpClient::generate_identifier() {
  if (_id.empty() == true) {
    _id = iota::riot_uuid();
  }
}

std::string iota::HttpClient::getRemoteEndpoint() {
  std::stringstream ss;
  ss << "http://";
  ss << _remote_ip;
  ss << ":";
  ss << _remote_port;
  if (_request) {
    ss << _request->get_resource();
    ss << " ";
    ss << _request->get_query_string();
  }
  return ss.str();
}

void iota::HttpClient::resolve(std::string address, std::string port) {
  try {
    boost::asio::ip::tcp::resolver resolver(_timer->get_io_service());
    boost::asio::ip::tcp::resolver::query query(address, port);
    boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
    boost::asio::ip::tcp::endpoint e = (*iterator);
    _ip = e.address();
  } catch (std::exception& e) {
    std::string msg_exception("Invalid endpoint ");
    msg_exception.append(address);
    msg_exception.append(":");
    msg_exception.append(port);
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER +
                                  " " + "[" + address + "]",
                              e.what(), iota::types::RESPONSE_CODE_BAD_REQUEST);
  }
}

void iota::HttpClient::stop() {
  IOTA_LOG_DEBUG(m_logger, get_identifier());
  _timer.reset();
  if (_connection) {
    _connection->close();
  }
}
