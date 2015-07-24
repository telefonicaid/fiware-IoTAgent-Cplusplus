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

#include <pion/http/plugin_service.hpp>
#include <pion/http/plugin_server.hpp>
#include <boost/thread/mutex.hpp>

class MockService: public pion::http::plugin_service {
  public:
    MockService() {};
    ~MockService() {
    }
    void operator()(pion::http::request_ptr& http_request_ptr,
                    pion::tcp::connection_ptr& tcp_conn);
    int _sc;
    std::vector<std::string> _content;
    std::vector<std::string> _received_content;
    std::map<std::string, std::string> _headers;
    bool _extended_echo;
    std::string get_last();
  private:
};
class HttpMock {
  public:
    HttpMock(std::string url);
    HttpMock(unsigned short port, std::string url, bool extended_echo = false);
    virtual ~HttpMock() {
      delete _ws;
    };
    void init();
    void stop();
    unsigned short get_port();
    void set_response(int status_code, std::string content,
                      std::map<std::string, std::string> headers =
                        std::map<std::string, std::string>());
    void set_extended_echo() {
      _service->_extended_echo = true;
    };

    boost::asio::ip::tcp::endpoint get_endpoint() {
      return _ws->get_endpoint();
    };

    /** return the last content **/
    std::string get_last();

    void reset();
    int size();
    int r_size() {
      return _service->_content.size();
    };


  protected:

  private:
    void run();
    unsigned short _port;
    std::string _url;
    pion::http::plugin_server* _ws;
    MockService* _service;
    boost::condition_variable condition;
    boost::mutex m_mutex;
    boost::shared_ptr<boost::thread> _thr;
};


#endif
