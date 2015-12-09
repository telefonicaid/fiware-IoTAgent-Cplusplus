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
         << val.second << pion::http::types::STRING_CRLF;
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

  std::string test_function(http_request_ptr->get_resource());

  if (http_request_ptr->get_method().compare("GET") == 0) {
/*
   std::string resp_get(get_last(test_function));
   std::cout << "GET received and responds" << test_function << " " << resp_get << std::endl;
   writer->write(resp_get);
   writer->send();
   return;
*/
  }
  /*
  std::string resource = get_resource();
  std::size_t pos = resource.find_last_of("/");
  if (pos != std::string::npos) {
   test_function = resource.substr(pos+1);
  }
  */
  std::cout << "Executing test " << test_function << " "
            << http_request_ptr->get_method() << std::endl;
  std::cout << "RECIBIDO " << _content[test_function].size() << " "
            << _sc[test_function] << std::endl;
  std::string content;
  // save body received to chech it in unit test
  _received_content[test_function].push_back(http_request_ptr->get_content());

  if (_content[test_function].size() > 0) {
    content.assign(_content[test_function].back());
    _content[test_function].pop_back();
    std::cout << "CONTENT TO SEND " << content << " " << _sc[test_function]
              << std::endl;
    if (_sc[test_function] == 204) {
      content.clear();
    }
    for (std::map<std::string, std::string>::iterator it =
             _headers[test_function].begin();
         it != _headers[test_function].end(); ++it) {
      writer->get_response().add_header(it->first, it->second);
    }
    writer->get_response().set_status_code(_sc[test_function]);
    writer->write_no_copy(content);
    writer->write_no_copy(pion::http::types::STRING_CRLF);
    writer->write_no_copy(pion::http::types::STRING_CRLF);
    std::cout << "SENT " << content << std::endl;
  } else {
    if (!_extended_echo[test_function]) {
      content = http_request_ptr->get_content();
      writer->write_no_copy(content);
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      std::cout << "SENT " << content << std::endl;
    } else {
      writer->write_no_copy(REQUEST_ECHO_TEXT);
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      writer->write_no_copy(pion::http::types::STRING_CRLF);
      writer << "Request method: " << http_request_ptr->get_method()
             << pion::http::types::STRING_CRLF
             << "Resource originally requested: "
             << http_request_ptr->get_original_resource()
             << pion::http::types::STRING_CRLF
             << "Resource delivered: " << http_request_ptr->get_resource()
             << pion::http::types::STRING_CRLF
             << "Query string: " << http_request_ptr->get_query_string()
             << pion::http::types::STRING_CRLF
             << "HTTP version: " << http_request_ptr->get_version_major() << '.'
             << http_request_ptr->get_version_minor()
             << pion::http::types::STRING_CRLF << "Content length: "
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

void MockService::set_response(std::string test_function, int status_code,
                               std::string content,
                               std::map<std::string, std::string> headers) {
  std::cout << "Setting response for " << test_function << " " << content << " " << status_code
            << std::endl;
  std::map<std::string, int>::iterator i_sc = _sc.begin();
  i_sc = _sc.find(test_function);
  if (i_sc != _sc.end()) {
    _sc[test_function] = status_code;
  } else {
    _sc.insert(std::pair<std::string, int>(test_function, status_code));
  }
  std::map<std::string, std::vector<std::string> >::iterator i =
      _content.begin();
  i = _content.find(test_function);
  if (i != _content.end()) {
    _content[test_function].insert(_content[test_function].begin(), content);
  } else {
    std::vector<std::string> test_content;
    test_content.push_back(content);
    _content.insert(std::pair<std::string, std::vector<std::string> >(
        test_function, test_content));
    _content[test_function] = test_content;
  }
  if (headers.size() > 0) {
    _headers.insert(std::pair<std::string, std::map<std::string, std::string> >(
        test_function, headers));
  }
  std::cout << "SET RESPONSE " << _content[test_function].size() << std::endl;
}

std::string MockService::get_last(std::string test_function) {
  std::string result;

  if (_received_content[test_function].size() > 0) {
    result.assign(_received_content[test_function].back());
    _received_content[test_function].pop_back();
  }

  return result;
}
void MockService::reset(std::string test_function) {
  std::cout << "RESET" << std::endl;
  std::string result;

  if (_content[test_function].size() > 0) {
    _content[test_function].clear();
  }
  if (_received_content[test_function].size() > 0) {
    _received_content[test_function].clear();
  }
}

int MockService::size(std::string test_function) {
  return _received_content[test_function].size();
}
