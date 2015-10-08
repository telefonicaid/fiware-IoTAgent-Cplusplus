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
#ifndef SRC_MQTT_MQTTSERVICE_H_
#define SRC_MQTT_MQTTSERVICE_H_

#include <pion/http/plugin_service.hpp>

#include <boost/property_tree/ptree.hpp>
#include <ESPLib.h>

#include "IotaMqttService.h"
#include "IotaMqttCommands.h"
#include "util/iota_exception.h"

namespace iota {
namespace esp {

class MqttService : public iota::esp::ngsi::IotaMqttCommands {
 public:
  int idsensor;
  static MqttService* instance;
  static ESPLib* getESPLib();

  MqttService();
  MqttService(iota::esp::ngsi::IotaMqttService* CBPublisher_ptr);

  void setIotaMqttService(iota::esp::ngsi::IotaMqttService* CBPublisher_ptr);
  void startESP();

  void resetESPSensor();

  virtual ~MqttService();
  void start();
  void op_mqtt(pion::http::request_ptr& http_request_ptr,
               std::map<std::string, std::string>& url_args,
               std::multimap<std::string, std::string>& query_parameters,
               pion::http::response& http_response, std::string& response);
  virtual void set_option(const std::string& name, const std::string& value);

  void initESPLib(std::string& pathToLog, std::string& sensorFile);

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
  virtual int execute_command(
      const std::string& endpoint, const std::string& command_id,
      const boost::property_tree::ptree& command_to_send, int timeout,
      const boost::shared_ptr<iota::Device>& item_dev,
      const boost::property_tree::ptree& service, std::string& response,
      iota::HttpClient::application_callback_t callback = NULL);

  bool is_push_type_of_command(boost::shared_ptr<Device> device);

  void transform_command(const std::string& command_name,
                         const std::string& command_value,
                         const std::string& updateCommand_value,
                         const std::string& sequence_id,
                         const boost::shared_ptr<iota::Device>& item_dev,
                         const boost::property_tree::ptree& service,
                         std::string& command_id,
                         boost::property_tree::ptree& command_line);

  /**
  Maybe I don't need to use this anymore.
  */
  std::string serializeMqttCommand(std::string apikey, std::string device,
                                   std::string command, std::string payload);

  int execute_mqtt_command(std::string apikey, std::string device,
                           std::string name, std::string command_payload,
                           std::string command_id);

  void respond_mqtt_command(std::string apikey, std::string device,
                            std::string command_payload,
                            std::string command_id);

  void op_ngsi(pion::http::request_ptr& http_request_ptr,
               std::map<std::string, std::string>& url_args,
               std::multimap<std::string, std::string>& query_parameters,
               pion::http::response& http_response, std::string& response);

 protected:
  pion::logger m_logger;

 private:
  std::map<std::string, void*> userData;
  iota::esp::ngsi::IotaMqttService* iota_mqtt_service_ptr_;

  static ESPLib* esplib_instance;
  std::string resource;
  std::string pathLog;
  std::string strSensorFile;
  boost::property_tree::ptree _service_configuration;
};
}
}
#endif  // MQTTSERVICE_H
