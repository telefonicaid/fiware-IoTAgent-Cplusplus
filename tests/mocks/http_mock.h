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
#ifndef SRC_TESTS_MOCKS_HTTP_MOCK_H_
#define SRC_TESTS_MOCKS_HTTP_MOCK_H_

#include <rest/rest_handle.h>
#include <pion/http/plugin_server.hpp>
#include <boost/thread/mutex.hpp>

class MockService : public iota::RestHandle {
 public:
  MockService(){};
  ~MockService() {}
  void operator()(pion::http::request_ptr& http_request_ptr,
                  pion::tcp::connection_ptr& tcp_conn);

  std::map<std::string, int> _sc;
  std::map<std::string, std::vector<std::string> > _content;
  std::map<std::string, std::vector<std::string> > _received_content;
  std::map<std::string, std::map<std::string, std::string> > _headers;
  std::map<std::string, bool> _extended_echo;
  std::string get_last(std::string test_name);
  void set_response(std::string test_function, int status_code,
                    std::string content,
                    std::map<std::string, std::string> headers =
                        std::map<std::string, std::string>());
  void set_extended_echo(std::string test_function) {
    _extended_echo[test_function] = true;
  };

  void reset(std::string test_function);
  int size(std::string test_function);
  int r_size(std::string test_function) {
    return _content[test_function].size();
  };

 private:
};

#endif
