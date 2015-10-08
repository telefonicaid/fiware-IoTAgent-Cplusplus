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
#ifndef SRC_UTIL_ACCESS_CONTROL_H_
#define SRC_UTIL_ACCESS_CONTROL_H_

#include "rest/process.h"
#include "http_client.h"
#include <boost/thread/mutex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <string>

namespace iota {
class AccessControl : public boost::enable_shared_from_this<AccessControl> {
 public:
  typedef boost::function<void(boost::shared_ptr<AccessControl>, bool)>
      app_callback_t;
  AccessControl();
  AccessControl(std::string endpoint, int timeout,
                boost::asio::io_service& io_service);
  virtual ~AccessControl();
  void set_timeout(int timeout);
  int get_timeout();
  void set_endpoint_ac(std::string endpoint_ac);
  std::string get_endpoint_ac();
  bool authorize(std::vector<std::string> roles, std::string resource,
                 std::string action,
                 boost::property_tree::ptree additional_info,
                 app_callback_t callback = app_callback_t());

  void receive_event(boost::shared_ptr<iota::HttpClient> connection,
                     pion::http::response_ptr response,
                     const boost::system::error_code& error);

  void set_identifier(std::string id) { _id = id; };

  std::string get_identifier() { return _id; };

 protected:
 private:
  std::string _endpoint_ac;
  int _timeout;
  pion::logger m_logger;
  boost::mutex _m;
  app_callback_t _application_callback;
  boost::asio::io_service& _io_service;
  std::string _id;

  pion::http::request_ptr create_request(
      std::string server, std::string resource, std::string content,
      std::string query, boost::property_tree::ptree additional_info);

  std::string create_xml_access_control(std::vector<std::string>& roles,
                                        std::string& resource_id,
                                        std::string& action);
};
};
#endif
