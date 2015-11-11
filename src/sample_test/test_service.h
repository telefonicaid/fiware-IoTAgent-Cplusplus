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
#ifndef SRC_SAMPLE_TEST_SERVICE_H_
#define SRC_SAMPLE_TEST_SERVICE_H_

#include <pion/http/plugin_service.hpp>
#include <rest/rest_handle.h>
#include <boost/property_tree/ptree.hpp>

namespace iota {

class TestService : public iota::RestHandle {
 public:
  TestService();
  virtual ~TestService();

  // Function called when plugin is started
  void start();

  // Callback function to NGSI interface
  void op_ngsi(pion::http::request_ptr& http_request_ptr,
               std::map<std::string, std::string>& url_args,
               std::multimap<std::string, std::string>& query_parameters,
               pion::http::response& http_response, std::string& response);

  // Callback function to SouthBound interface
  void service(pion::http::request_ptr& http_request_ptr,
               std::map<std::string, std::string>& url_args,
               std::multimap<std::string, std::string>& query_parameters,
               pion::http::response& http_response, std::string& response);

  // Function to send a NGSI updateContext to contextBroker
  int send_update_context(const std::string& dev, const std::string& apikey,
                          const std::string& content, std::string& cb_response);

  virtual iota::ProtocolData get_protocol_data();

  virtual std::string get_protocol_commands();

 private:
  pion::logger m_logger;
};

}  // end namespace example

#endif
