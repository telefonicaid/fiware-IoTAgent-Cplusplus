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
#ifndef _HTTPFILTER_H_
#define _HTTPFILTER_H_

#include "riot_conf.h"
#include <pion/http/request.hpp>
#include <pion/tcp/connection.hpp>
#include <pion/http/response_writer.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace iota {
/*
typedef boost::function<void (
  pion::http::request_ptr&,
  pion::tcp::connection_ptr&,
  int status)> filter_handle_t;
*/
typedef boost::function<void(int)> filter_handle_t;
class HTTPFilter : public boost::enable_shared_from_this<HTTPFilter> {
 public:
  HTTPFilter(boost::asio::io_service& io_service) : _io_service(io_service){};
  virtual ~HTTPFilter(){
      // TODO std::cout << "HTTPFilter DESTRUCTOR " << _io_service.use_count()
      // << std::endl;
      // TODO Con este reset no problema en destruccion ¿por que?
      // _io_service.reset();
  };

  boost::asio::io_service& get_io_service() { return _io_service; }

  virtual bool handle_request(pion::http::request_ptr& http_request_ptr,
                              pion::tcp::connection_ptr& tcp_conn) = 0;
  virtual void handle_no_allowed(
      pion::http::request_ptr& http_request_ptr,
      pion::tcp::connection_ptr& tcp_conn,
      int status = pion::http::types::RESPONSE_CODE_UNAUTHORIZED) {
    IOTA_LOG_INFO(m_logger,
                  "|request=" << http_request_ptr->get_header(
                                     iota::types::HEADER_TRACE_MESSAGES)
                              << "|status=" << status);

    tcp_conn->get_io_service().post(boost::bind(
        &iota::HTTPFilter::call_to_callback, shared_from_this(),
        http_request_ptr->get_header(iota::types::HEADER_TRACE_MESSAGES),
        status));
  };
  void set_async_filter(std::string uuid_request, filter_handle_t callback) {
    boost::mutex::scoped_lock lock(_m_callbacks);
    _callbacks.insert(
        std::make_pair<std::string, filter_handle_t>(uuid_request, callback));
  };
  virtual void call_to_callback(std::string id_request, int s) {
    filter_handle_t my_callback;
    boost::mutex::scoped_lock lock(_m_callbacks);
    std::map<std::string, filter_handle_t>::iterator it = _callbacks.begin();
    it = _callbacks.find(id_request);
    if (it != _callbacks.end() && it->second) {
      my_callback = it->second;
      _callbacks.erase(it);
    }
    lock.unlock();
    my_callback(s);
  };

 protected:
  HTTPFilter(boost::asio::io_service& io_service, pion::logger logger)
      : _io_service(io_service), m_logger(logger){};
  boost::asio::io_service& _io_service;
  std::map<std::string, filter_handle_t> _callbacks;
  pion::logger m_logger;
  boost::mutex _m_callbacks;

 private:
};
}
#endif
