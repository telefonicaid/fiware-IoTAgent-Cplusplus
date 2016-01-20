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
#ifndef SRC_REST_COMMAND_HANDLE_H_
#define SRC_REST_COMMAND_HANDLE_H_

#include <pion/http/plugin_service.hpp>
#include <pion/http/response_writer.hpp>
#include <util/service.h>
#include <rest/rest_handle.h>
#include <string>
#include <ngsi/UpdateContext.h>
#include <ngsi/QueryContext.h>
#include <ngsi/ContextElement.h>
#include <ngsi/ContextResponses.h>
#include <boost/property_tree/ptree.hpp>
#include <util/command_cache.h>

#include "util/device_collection.h"
#include <ngsi/ContextElement.h>
#include <ngsi/RegisterContext.h>

#define DEFAULT_REG_TIMEOUT 1
#define UNKOWN_PROVIDING_APP "unkown"

namespace iota {

/**
* A CommandHandle class. Command common functiopns for Iot plugins.
*/
class CommandHandle : public iota::RestHandle {
 public:
  typedef struct {
    std::string command_name;
    std::string command_id;
    boost::property_tree::ptree command_to_send;
    std::string sequence;
    int timeout;
    boost::shared_ptr<iota::Device> item_dev;
    std::string entity_type;
    boost::property_tree::ptree service;

  } CommandData;

  CommandHandle();
  virtual ~CommandHandle();

  int get_duration_seconds(std::string data);
  void make_registrations(void);

  int queryContext(iota::QueryContext& queryContext,
                   const boost::property_tree::ptree& service_ptree,
                   iota::ContextResponses& context_responses);

  int updateContext(iota::UpdateContext& updateContext,
                    const boost::property_tree::ptree& service,
                    const std::string& sequence,
                    iota::ContextResponses& response);

  void default_op_ngsi(
      pion::http::request_ptr& http_request_ptr,
      std::map<std::string, std::string>& url_args,
      std::multimap<std::string, std::string>& query_parameters,
      pion::http::response& http_response, std::string& response);

  void default_queryContext_ngsi(
      pion::http::request_ptr& http_request_ptr,
      std::map<std::string, std::string>& url_args,
      std::multimap<std::string, std::string>& query_parameters,
      pion::http::response& http_response, std::string& response);

  void getCommandLine(const std::string& command_name,
                      const std::string& updateCommand_value,
                      const std::string& sequence_id,
                      const boost::shared_ptr<Device>& item_dev,
                      const boost::property_tree::ptree& service,
                      std::string& command_id,
                      boost::property_tree::ptree& command_line);

  /**
   * @name    timeout_f
   * @brief   this function is called when a command in a cache gives timeout
   *
   *
   * @param [in] item  the command timeout.
   *
   */
  boost::shared_ptr<Command> timeout_f(boost::shared_ptr<Command> item);

  /**
   * @name    handle_updateContext
   * @brief   this function is called by asynchronous responses from CB
   *          if error writes an alarm
   *
   * @param [in] response  the response from CB
   * @param [in] response  the response http code response from CB
   *
   */
  void handle_updateContext(const std::string& url, std::string response,
                            int status);

  /**
   * @name    updateCommand
   * @brief   information from one command, recieved in updateConext
   *
   * This API provides certain actions as an example.
   *
   * @param [in] command  name of attribute command.
   * @param [in] parameters  value for attribute command.
   * @param [in] device  registered information for device .
   * @param [in] sequence  id sequence from updateContext .
   * @param [in] service  ptree with configuration information for service
   * (cbroker, service_path, timeout) .
   *
   *
   * Example Usage:
   * @code
   *    updateCommand("ping", "22", dev, "2345fefe4343Ŕ", service);
   * @endcode
   */
  void updateCommand(const std::string& command, const std::string& parameters,
                     boost::shared_ptr<Device> device,
                     const std::string& entity_type,
                     const std::string& sequence,
                     const boost::property_tree::ptree& service);

  void send_all_registrations(void);
  void send_all_registrations_from_mongo();
  void start_thread_registrations(void);
  void send_register_device(Device& device);

  void init_services_by_resource();
  bool find_service_name(std::string srv);

  int send_register(
      std::vector<iota::ContextRegistration> context_registrations,
      boost::property_tree::ptree& service,
      const boost::shared_ptr<Device> device, const std::string& regId,
      std::string& cb_response);

  /**
    * @name    send_unregister
    * @brief   send a register to Context Broker with 1 second expiration
    *
    * This API provides certain actions as an example.
    *
    * @param [in] service,  service with ContextBroker data.
    * @param [in] device  device to unregister.
    * @param [in] regId  registrationId to be unregister, it cannot be empty .
    * @param [out] cb_response  response from context broker .
    *
    *
    */
  int send_unregister(boost::property_tree::ptree& service,
                      const boost::shared_ptr<Device> device,
                      const std::string& regId, std::string& cb_response);

  int send(iota::ContextElement ngsi_context_element, const std::string& opSTR,
           const boost::property_tree::ptree& service,
           std::string& cb_response);

  /**
  * @name    send_updateContext
  * @brief   end to contect broker and updateContext.
    *  @param [in] command_name,   the name of command, for example ping, set,
  *...
    *  @param [in] command_att, specificied attribue command ( command, status,
  *info)
    *  @param [in] type, specificied type attribue command ( string )
    *  @param [in] value, message save in attribute status of the command
    *  @param [in] device_name,  the name of device
    *  @param [in] entity_type,  entity_type to send to cb
    *  @param [in] service, config info for the service (cbroker, service_path,
  *timeout, ...)
    *  @param [in] opSTR es la operacion del context broker, por ejemplo APPEND
    *
    * @retval http code response,  response from http communication with Context
  *Broker.
    **/
  int send_updateContext(const std::string& command_name,
                         const std::string& command_att,
                         const std::string& type, const std::string& value,
                         const boost::shared_ptr<Device>& item_dev,
                         const boost::property_tree::ptree& service,
                         const std::string& opSTR);

  int send_updateContext(const std::string& command_name,
                         const std::string& command_att,
                         const std::string& type, const std::string& value,
                         const std::string& command_att2,
                         const std::string& type2, const std::string& value2,
                         const boost::shared_ptr<Device>& item_dev,
                         const boost::property_tree::ptree& service,
                         const std::string& opSTR);

  /**
  * @name    execute_command
  * @brief   virtual function must be implemented in every Iot plugin, execute a
  * command (send http, mqtt, sms...)
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
  * @retval pion::http::types::RESPONSE_CODE_OK   Successfully command executed.
  * @retval other  error.
  *
  */
  virtual int execute_command(
      const std::string& endpoint, const std::string& command_id,
      const boost::property_tree::ptree& command_to_send, int timeout,
      const boost::shared_ptr<Device>& item_dev,
      const boost::property_tree::ptree& service, std::string& response,
      iota::HttpClient::application_callback_t callback = NULL) = 0;

  void receive_command_response(CommandData cmd_data,
                                boost::shared_ptr<iota::HttpClient> http_client,
                                pion::http::response_ptr http_response,
                                const boost::system::error_code& error);

  /**
  * @name is_push_type_of_command
  * @brief virtual function that will return true if PUSH commands are enabled
  * for a particular device. It's virtual
  * so other plugins may implement a different behaviour. A default
  * implementation is provided where the presence of an
  * endpoint will determine that PUSH is the type for commands.
  * @param [in] device, pointer to the device for checking type of command.
  * @return bool: TRUE for PUSH commands, FALSE for PULL.
  */
  virtual bool is_push_type_of_command(boost::shared_ptr<Device> device);

  /**
  * @name    transform_command
  * @brief   virtual function, default implementation but every pluging can
  * overwrite it.
  *
  * This API provides certain actions as an example.
  *
  * @param [in] command_name,  command name registered in Device.
  * @param [in] command_value,  command value registered in Device.
  * @param [in] updateCommand_value, value that user sends in the updateContext
  * operation.
  * @param [in] sequence_id , operation identifier used by Context Broker .
  * @param [in] item_dev, device to execute the command
  * @param [in] service, device service
  * @param [out] command_id, command identifier, needed to associate the result
  * of command
  * @param [out] command_line, device service
  *
  *
  */
  virtual void transform_command(const std::string& command_name,
                                 const std::string& command_value,
                                 const std::string& updateCommand_value,
                                 const std::string& sequence_id,
                                 const boost::shared_ptr<Device>& item_dev,
                                 const boost::property_tree::ptree& service,
                                 std::string& command_id,
                                 boost::property_tree::ptree& command_line);

  /**
  * @name    transform_response
  * @brief   virtual function, default implementation but every pluging can
  * overwrite it. It is
  *          used in asynchronous commands.
  *
  * This API provides certain actions as an example.
  *
  * @param [in] str_command_resp,  response received (if HTTP is response body,
  * for example) when command is finished.
  * @param [in] cmd_status,  status code in response.
  * @param [out] command_response, user data (after str_command_resp is
  * processed) for executed command.
  * @param [out] command_id, command identifier, needed to associate the result
  * of command with command requested.
  *
  *
  */
  virtual int transform_response(const std::string& str_command_resp,
                                 const int& cmd_status,
                                 std::string& command_response,
                                 std::string& id_command);

  /**
  * @name    save_command
  * @brief   a 202 or poling command is saved in cache.
  *
  *
  * @param [in] command_name  name of the device that ask for the commands.
  * @param [in] command_id, id for the command.
  *              if it is empty, one will be created and returned in this field
  * @param [in] timeout , timeout .
  * @param [in] command_to_send , command ready to send to device .
  * @param [in] device_name , name of th device .
  * @param [in] service , name of the service .
  * @param [in] status , status of the command  (for ex, READY_TO_READ) .
  *
  *
  */
  void save_command(const std::string& command_name,
                    const std::string& command_id, int timeout,
                    const boost::property_tree::ptree& command_to_send,
                    const boost::shared_ptr<Device>& item_dev,
                    const std::string& entity_type, const std::string& endpoint,
                    const boost::property_tree::ptree& service_ptree,
                    const std::string& sequence, int status);

  /**
  * @name    get_all_command
  * @brief   return all commands that they are ready to read.
  *
  *
  * @param [in] device  name of the device that ask for the commands.
  * @param [in] entity_type,  .
  * @param [in] service, name of the service of the device (read from apikey).
  * @param [out] commands , all commands ready for read  in this device .
  *
  *
  */
  CommandVect get_all_command(const boost::shared_ptr<Device>& device,
                              const boost::property_tree::ptree& service);

  CommandVect get_all_command(const std::string& device_id,
                              const std::string& apikey);

  int remove_command(const std::string& command_id, const std::string& service,
                     const std::string& service_path);

  void remove_all_command();

  CommandPtr get_command(const std::string& command_id,
                         const std::string& service,
                         const std::string& service_path);

  CommandPtr get_command_from_mongo(const std::string& command_id,
                         const std::string& service,
                         const std::string& service_path);

  int get_cache_size();

  /**
  * @name    set_async_commands
  * @brief   set command interface to be asynchronous.
  */
  void set_async_commands();

  void set_myProvidingApp(const std::string& myapp) { _myProvidingApp = myapp; }

  void find_devices_with_commands(iota::DeviceCollection& dev_table,
                                  const std::string& service,
                                  const std::string& service_path,
                                  const std::string& protocol);

 protected:
  void enable_ngsi_service(
      std::map<std::string, std::string>& filters,
      iota::RestHandle::HandleFunction_t handle, iota::RestHandle* context,
      iota::RestHandle::HandleFunction_t handle_query = NULL);

  std::string get_ngsi_operation(const std::string& operation);

  /**
  * @name    response_command
           * @brief   a response from command is received, change status and
  * info in CB and  in command cache
           *
           *
           * @param [in] id_command  name of the device that ask for the
  * commands.
           * @param [in] response, name of the service of the device (read from
  * apikey).
           *
           *
           */
  void response_command(const std::string& id_command,
                        const std::string& response,
                        const boost::shared_ptr<Device>& device,
                        const boost::property_tree::ptree& service);

  std::string create_ngsi_response(int code, const std::string& reason,
                                   const std::string& details);

  void process_command_response(CommandData& cmd_data, int& res_code,
                                std::string& resp_cmd);

  /**
  * @name json_value_to_ul
  * @brief for commands that have parameters formatted in JSON, this function
  *will convert that JSON string into
  * UltraLight-type sequence of params (param-name=value|param-name2=value...).
  *This is used at least by MQTTService and UL20Service
  * @param [in] json_value, std::string. This is the JSON object with commands
  *"{ "param1" : "value1" , ...}"
  * @return std::string, the formatted sequence: "param1=value|param2=value..."
  *IF input value is JSON, otherwise it will just bypass whatever comes
  * as input.
  *
  **/
  std::string json_value_to_ul(const std::string& json_value);

  std::string _myProvidingApp;

  int _reg_timeout;

  std::vector<std::string> _services_names;

 private:
  pion::logger m_logger;

  // cache with asyncronous command
  CommandCache m_asyncCommands;

  // Asynchronous operation
  bool _callback;

  /**
   *  fill entity_context_element  with command of device
   **/
  void populate_command_attributes(
      const boost::shared_ptr<Device>& device,
      iota::ContextElement& entity_context_element);
};
}

#endif  // COMMANDJANDLER_H
