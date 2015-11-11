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
#ifndef _TESTCOMMANDSERVICE_H_
#define _TESTCOMMANDSERVICE_H_

#include <pion/http/plugin_service.hpp>
#include <rest/command_handle.h>
#include <boost/property_tree/ptree.hpp>

namespace iota {

class TestCommandService : public iota::CommandHandle {
 public:
  TestCommandService();
  virtual ~TestCommandService();

  // Function called when plugin is started
  /**
  * @name    start
  * @brief   Callback function called by pion to init the plugin.
  *
  */
  void start();

  /**
  * @name    start
  * @brief   Callback function to SouthBound interface (called from devices).
  *
  */
  void service(pion::http::request_ptr& http_request_ptr,
               std::map<std::string, std::string>& url_args,
               std::multimap<std::string, std::string>& query_parameters,
               pion::http::response& http_response, std::string& response);

  /**
  * @name    receive_observations
  * @brief   receive observations from devices, translate them,
  *          and send results to contest broker.
  *
  *  This is an example, and read data in http_request_ptr, but it can be other
  * protocol.
  *  Must create an iota::Attribute (attribute_name, attribute_type, value
  *  this value is translate with data provides by API rest
  *  if device exists and have entity_name, device_name is changed with
  * entity_name
  *  if attribute is provisioned in device, attribute_name is changed with
  * provisioned name
  *
  * @param [in] http_request_ptr  from pion http in.
  * @param [in] url_args, for rest .
  * @param [in] query_parameters , query data from uri in a map .
  * @param [out] http_response , command id .
  * @param [out] response , command id .
  *
  */
  void receive_observations(
      pion::http::request_ptr& http_request_ptr,
      std::map<std::string, std::string>& url_args,
      std::multimap<std::string, std::string>& query_parameters,
      pion::http::response& http_response, std::string& response);

  /**
       * @name    execute_command
       * @brief   virtual function must be implemented in every Iot plugin,
   * execute a command (send http, mqtt, sms...)
       *
       * This API provides certain actions as an example.
       *
       * @param [in] endpoint  defined in  device registration.
       * @param [in] command_id,  identifier for the command (not name in device
   * registration, identifier to receive results).
       * @param [in] command_to_send, command text once executed the
   * transform_command.
       * @param [in] timeout  max time in miliseconds.
       * @param [in] item_dev  device information.
       * @param [in] service   service information( apikey included)
       * @param [out] response  text with information (results  or error ).
       *
       * @retval pion::http::types::RESPONSE_CODE_OK   Successfully command
   * executed.
       * @retval other  error.
       *
       */
  int execute_command(const std::string& endpoint,
                      const std::string& command_id,
                      const boost::property_tree::ptree& command_to_send,
                      int timeout,
                      const boost::shared_ptr<iota::Device>& item_dev,
                      const boost::property_tree::ptree& service,
                      std::string& response,
                      iota::HttpClient::application_callback_t callback = NULL);

  /**
  * @name    receive_command_results
  * @brief   device sends commands results, iotagent check them and send the
  * result to contest broker.
  *
  *  This is for polling commands.
  *  Iotagent return all commands in READY_FOR_READ status and for this device
  *  and change status command  to DELIVERY (waiting for results)
  *
  * @param [in] http_request_ptr  from pion http in.
  * @param [in] url_args, for rest .
  * @param [in] query_parameters , query data from uri in a map .
  * @param [out] http_response , command id .
  * @param [out] response , command id .
  */
  void receive_command_results(
      pion::http::request_ptr& http_request_ptr,
      std::map<std::string, std::string>& url_args,
      std::multimap<std::string, std::string>& query_parameters,
      pion::http::response& http_response, std::string& response);

  /**
  * @name    get_commands_polling
  * @brief   device asks for outstanding commands to execute.
  *
  * This is for polling commands. The device wake up and check with iotagent if
  * there are
  *     outstanding commands to execute
  * It is mandatory remove command from cache.
  *
  * @param [in] http_request_ptr  from pion http in.
  * @param [in] url_args, for rest .
  * @param [in] query_parameters , query data from uri in a map .
  * @param [out] http_response , command id .
  * @param [out] response , command id .
  */
  void get_commands_polling(
      pion::http::request_ptr& http_request_ptr,
      std::map<std::string, std::string>& url_args,
      std::multimap<std::string, std::string>& query_parameters,
      pion::http::response& http_response, std::string& response);

  virtual iota::ProtocolData get_protocol_data();

  virtual std::string get_protocol_commands();

  virtual int get_timeout_commands();

 private:
  pion::logger m_logger;
};

}  // end namespace example

#endif
