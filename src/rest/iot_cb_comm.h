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
#ifndef SRC_REST_IOT_CB_COMM_H_
#define SRC_REST_IOT_CB_COMM_H_

#include <string>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "util/http_client.h"
#include "rest/process.h"

#include <ngsi/ContextElement.h>
#include <ngsi/UpdateContext.h>

namespace iota {

class ContextBrokerCommunicator
    : public boost::enable_shared_from_this<ContextBrokerCommunicator> {
 public:
  typedef boost::function<void(std::string, int)> app_callback_t;
  ContextBrokerCommunicator();
  ContextBrokerCommunicator(boost::asio::io_service& io_service);
  virtual ~ContextBrokerCommunicator();
  // TODO void start();
  void receive_event(std::string url, std::string content,
                     boost::property_tree::ptree additional_info,
                     boost::shared_ptr<iota::HttpClient> connection,
                     pion::http::response_ptr response_ptr,
                     const boost::system::error_code& error);
  bool async_send(std::string url, std::string content,
                  boost::property_tree::ptree additional_info,
                  app_callback_t callback = app_callback_t(),
                  int status_code = pion::http::types::RESPONSE_CODE_OK);
  std::string send(std::string url, std::string content,
                   boost::property_tree::ptree additional_info,
                   int status_code = pion::http::types::RESPONSE_CODE_OK);

  int send_updateContext(const std::string& command_name,
                         const std::string& command_att,
                         const std::string& type, const std::string& value,
                         const boost::shared_ptr<iota::Device>& item_dev,
                         const boost::property_tree::ptree& service,
                         const std::string& opSTR);

  static void add_updateContext(const std::string& command_name,
                                const std::string& command_att,
                                const std::string& type,
                                const std::string& value,
                                const boost::shared_ptr<iota::Device>& item_dev,
                                const boost::property_tree::ptree& service,
                                iota::ContextElement& ngsi_context_element);

  int send(iota::ContextElement ngsi_context_element, const std::string& opSTR,
           const boost::property_tree::ptree& service,
           std::string& cb_response);

  std::string get_ngsi_operation(const std::string& operation);

  // Default method is POST
  void method(std::string http_method);

 protected:
 private:
  //
  boost::mutex _m;

  pion::logger m_logger;

  // Connection
  std::map<std::string, boost::shared_ptr<iota::HttpClient> > _connections;

  // io_service when external event loop is used
  boost::asio::io_service& _io_service;

  boost::shared_ptr<HttpClient> get_connection(
      const std::string& id_connection);

  void add_connection(boost::shared_ptr<iota::HttpClient> connection);

  void remove_connection(boost::shared_ptr<iota::HttpClient> connection);

  pion::http::request_ptr create_request(
      std::string& server, std::string& resource, std::string& content,
      std::string& query, boost::property_tree::ptree& additional_info);

  std::string process_response(
      const std::string& url, boost::shared_ptr<iota::HttpClient> connection,
      pion::http::response_ptr resp, const std::string& content,
      const boost::property_tree::ptree& additional_info,
      const boost::system::error_code& error);
  app_callback_t _callback;

  static const std::string NUMBER_OF_TRIES;

  std::string _method;
};
};
#endif
