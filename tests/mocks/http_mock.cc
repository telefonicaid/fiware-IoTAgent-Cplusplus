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
#include "http_mock.h"
#include <pion/http/response_writer.hpp>
#include <boost/asio.hpp>
#include <pion/error.hpp>
#include <pion/plugin.hpp>
#include <pion/process.hpp>
#include <pion/http/plugin_server.hpp>




void writeDictionaryTerm(pion::http::response_writer_ptr& writer,
                         const pion::ihash_multimap::value_type& val) {
  // text is copied into writer text cache
  writer << val.first << pion::http::types::HEADER_NAME_VALUE_DELIMITER
         << val.second
         << pion::http::types::STRING_CRLF;
}


HttpMock::HttpMock(unsigned short port, std::string url,
                   bool extended_echo,
                   const std::string name):_name(name), _port(port), _url(url) {
  std::cout << "CONTRUCTOR MOCK" << std::endl;
  _service = new MockService();
  _service->_extended_echo = extended_echo;
  pion::process::initialize();
}

HttpMock::HttpMock(std::string url,
            const std::string name):_name(name), _url(url), _port(0) {
  std::cout << "CONTRUCTOR MOCK" << std::endl;
  _name = name;
  pion::process::initialize();
  _service = new MockService();
  _service->_extended_echo = false;
}

void HttpMock::init() {

  std::cout << "Mock HTTP init" << std::endl;
  _thr.reset(new boost::thread(&HttpMock::run, this));
  boost::unique_lock<boost::mutex> lock(m_mutex);
  condition.timed_wait(lock, boost::posix_time::milliseconds(5000));
  std::cout << "Espere" << std::endl;
}


void HttpMock::stop() {
  boost::unique_lock<boost::mutex> lock(m_mutex);
  condition.timed_wait(lock, boost::posix_time::milliseconds(1000));
  std::cout << "STOP" << std::endl;
  if (_ws != NULL) {
    _ws->stop();
    pion::process::shutdown();
    std::cout << "MOCK " << _ws->get_connections() << std::endl;
    _ws->clear();
  }
  _thr->detach();
  _thr.reset();
}


void HttpMock::run() {
  boost::asio::ip::tcp::endpoint cfg_endpoint(boost::asio::ip::tcp::v4(), _port);
  if (_port != 0) {
    _ws = new pion::http::plugin_server(cfg_endpoint);
  }
  else {
    _ws = new pion::http::plugin_server();
  }
  PION_LOG_CONFIG_BASIC;
  PION_LOG_SETLEVEL_DEBUG(PION_GET_LOGGER("pion"));
  _ws->add_service(_url, _service);
  std::cout << "START " << std::endl;
  _ws->start();
  condition.notify_one();
  pion::process::wait_for_shutdown();
  std::cout << "After Stop" << std::endl;
  //condition.notify_one();
}

unsigned short HttpMock::get_port() {
  return _ws->get_port();
}

void HttpMock::set_response(int status_code, std::string content,
                            std::map<std::string, std::string> headers) {

  _service->_sc = status_code;
  //_content.push_back(content);
  _service->_content.insert(_service->_content.begin(), content);
  if (headers.size() > 0) {
    _service->_headers = headers;
  }
  std::cout << "SET RESPONSE " << _service->_content.size() << std::endl;
}



void MockService::operator()(pion::http::request_ptr& http_request_ptr,
                             pion::tcp::connection_ptr& tcp_conn) {
  pion::http::response_writer_ptr writer(pion::http::response_writer::create(
      tcp_conn, *http_request_ptr,
      boost::bind(&pion::tcp::connection::finish, tcp_conn)));

  static const std::string REQUEST_ECHO_TEXT("[Request Echo]");
  static const std::string REQUEST_HEADERS_TEXT("[Request Headers]");
  static const std::string QUERY_PARAMS_TEXT("[Query Parameters]");
  static const std::string COOKIE_PARAMS_TEXT("[Cookie Parameters]");
  static const std::string POST_CONTENT_TEXT("[POST Content]");
  static const std::string USER_INFO_TEXT("[USER Info]");

  std::cout << "RECIBIDO " << _content.size() << std::endl;
  std::string content;
  // save body received to chech it in unit test
  _received_content.push_back(http_request_ptr->get_content());

  if (_content.size() > 0) {
    content.assign(_content.back());
    _content.pop_back();
    std::cout << "CONTENT TO SEND " << content << std::endl;

    for (std::map<std::string, std::string>::iterator it = _headers.begin();
         it != _headers.end(); ++it) {

      writer->get_response().add_header(it->first, it->second);
    }
    writer->get_response().set_status_code(_sc);
    writer->write_no_copy(content);
    writer->write_no_copy(pion::http::types::STRING_CRLF);
    writer->write_no_copy(pion::http::types::STRING_CRLF);
    std::cout << "SENT " << content << std::endl;
  }
  else {
    if (!_extended_echo) {
      content = http_request_ptr->get_content();
      writer->write_no_copy(content);
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      std::cout << "SENT " << content << std::endl;
    }
    else {
      writer->write_no_copy(REQUEST_ECHO_TEXT);
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      writer
          << "Request method: "
          << http_request_ptr->get_method()
          << pion::http::types::STRING_CRLF
          << "Resource originally requested: "
          << http_request_ptr->get_original_resource()
          << pion::http::types::STRING_CRLF
          << "Resource delivered: "
          << http_request_ptr->get_resource()
          << pion::http::types::STRING_CRLF
          << "Query string: "
          << http_request_ptr->get_query_string()
          << pion::http::types::STRING_CRLF
          << "HTTP version: "
          << http_request_ptr->get_version_major() << '.' <<
          http_request_ptr->get_version_minor()
          << pion::http::types::STRING_CRLF
          << "Content length: "
          << (unsigned long)http_request_ptr->get_content_length()
          << pion::http::types::STRING_CRLF
          << pion::http::types::STRING_CRLF;

      // write request headers
      writer->write_no_copy(REQUEST_HEADERS_TEXT);
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      std::for_each(http_request_ptr->get_headers().begin(),
                    http_request_ptr->get_headers().end(),
                    boost::bind(&writeDictionaryTerm, writer, _1));
      writer->write_no_copy(pion::http::types::STRING_CRLF);

      // write query parameters
      writer->write_no_copy(QUERY_PARAMS_TEXT);
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      std::for_each(http_request_ptr->get_queries().begin(),
                    http_request_ptr->get_queries().end(),
                    boost::bind(&writeDictionaryTerm, writer, _1));
      writer->write_no_copy(pion::http::types::STRING_CRLF);

      // write cookie parameters
      writer->write_no_copy(COOKIE_PARAMS_TEXT);
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      std::for_each(http_request_ptr->get_cookies().begin(),
                    http_request_ptr->get_cookies().end(),
                    boost::bind(&writeDictionaryTerm, writer, _1));
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      // write POST content
      writer->write_no_copy(POST_CONTENT_TEXT);
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      if (http_request_ptr->get_content_length() != 0) {
        writer->write(http_request_ptr->get_content(),
                      http_request_ptr->get_content_length());
        writer->write_no_copy(pion::http::types::STRING_CRLF);
        writer->write_no_copy(pion::http::types::STRING_CRLF);
      }

    }
  }

  writer->send();


}

std::string HttpMock::get_last() {
  std::string result;

  if (_service->_received_content.size() > 0) {
    result.assign(_service->_received_content.back());
    _service->_received_content.pop_back();
  }

  return result;
}

void HttpMock::reset() {
  std::cout << "RESET" << std::endl;
  std::string result;

  if (_service->_content.size() > 0) {
    _service->_content.clear();
  }
  if (_service->_received_content.size() > 0) {
    _service->_received_content.clear();
  }
}

int HttpMock::size() {
  return   _service->_received_content.size();

}
