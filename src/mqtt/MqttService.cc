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
#include "MqttService.h"

#include <TDA.h>

#include <ESP_SBC_Command.h>
#include "services/admin_service.h"
#include <pion/http/response_writer.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include "IotaMqttServiceImpl.h"

#include <boost/uuid/uuid.hpp>             // uuid class
#include <boost/uuid/uuid_generators.hpp>  // generators
#include <boost/uuid/uuid_io.hpp>          // streaming operators etc.

ESPLib* iota::esp::MqttService::esplib_instance = NULL;

ESPLib* iota::esp::MqttService::getESPLib() {
  if (MqttService::esplib_instance == NULL) {
    esplib_instance = new ESPLib();
  }

  return esplib_instance;
}

iota::esp::MqttService::MqttService()
    : m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  std::cout << "iota::esp::MqttService " << std::endl;
  IOTA_LOG_DEBUG(m_logger, "iota::esp::MqttService Running...  ");
  iota_mqtt_service_ptr_ = NULL;
  idsensor = -1;
  _protocol_data.description = "MQTT";
  _protocol_data.protocol = "PDI-IoTA-MQTT-UltraLight";
}

iota::esp::MqttService::~MqttService() {
  // dtor
  // is delete contextBrokerPub missing?
  delete iota_mqtt_service_ptr_;  // Be careful when using on as a mock
  /*
  if (idsensor > 0){
    iota::esp::MqttService::getESPLib()->stopSensor(idsensor);
    // SLEEP(1000);//not the best way to allow a task to finish...
    iota::esp::MqttService::getESPLib()->destroySensor(idsensor);

    iota::esp::MqttService::getESPLib()->sensors.clear();
  }
*/
  resetESPSensor();

  if (MqttService::esplib_instance != NULL) {
    delete esplib_instance;
    esplib_instance = NULL;
  }
}

void iota::esp::MqttService::initESPLib(std::string& pathToLog,
                                        std::string& sensorFile) {
  iota::esp::MqttService::getESPLib()->setLoggerPath(pathToLog);

  if (iota::esp::MqttService::getESPLib()->sensors.size() == 0) {
    IOTA_LOG_DEBUG(m_logger, "Loading sensor from file: " << sensorFile);
    idsensor = iota::esp::MqttService::getESPLib()->createSensor(sensorFile);
    IOTA_LOG_DEBUG(m_logger, "Created Sensor, idSensor : " << idsensor);

  } else {
    IOTA_LOG_ERROR(
        m_logger,
        "More than one sensor is not supported on this version. Sorry.");
  }
}

void iota::esp::MqttService::resetESPSensor() {
  if (idsensor > 0) {
    iota::esp::MqttService::getESPLib()->stopSensor(idsensor);
    iota::esp::MqttService::getESPLib()->destroySensor(idsensor);

    iota::esp::MqttService::getESPLib()->sensors.clear();
    idsensor = 0;
  }
}

void iota::esp::MqttService::setIotaMqttService(
    iota::esp::ngsi::IotaMqttService* CBPublisher_ptr) {
  iota_mqtt_service_ptr_ = CBPublisher_ptr;

  userData.clear();
  userData.insert(std::pair<std::string, void*>("contextBrokerPublisher",
                                                (void*)iota_mqtt_service_ptr_));

  iota::esp::MqttService::getESPLib()->setUserData(idsensor, userData);
  iota::esp::MqttService::getESPLib()->registerResultCallback(
      idsensor, this, /*sensorResultCallback*/ NULL);  // NO CALLBACK selected
  IOTA_LOG_DEBUG(
      m_logger,
      "ContextBroker Publisher (IotaMqttService) registered for idsensor:"
          << idsensor);
}

void iota::esp::MqttService::startESP() {
  IOTA_LOG_DEBUG(m_logger, "Starting idsensor: " << idsensor);
  iota::esp::MqttService::getESPLib()->startSensor(idsensor, "main");
  IOTA_LOG_DEBUG(m_logger, "Sensor  idsensor: " << idsensor << " DONE");
}

void iota::esp::MqttService::set_option(const std::string& name,
                                        const std::string& value) {
  if (name.compare("ConfigFile") == 0) {
    IOTA_LOG_DEBUG(m_logger, "Reading Config File: " << value);

    try {
      read_xml(value, _service_configuration,
               boost::property_tree::xml_parser::no_comments);
      IOTA_LOG_DEBUG(m_logger, "XML READ");

      strSensorFile = _service_configuration.get<std::string>("Sensors");
      IOTA_LOG_DEBUG(m_logger, "Sensor File: " << strSensorFile);

      // Set LogPath
      pathLog = _service_configuration.get<std::string>("LogPath");
      initESPLib(pathLog, strSensorFile);

    } catch (boost::exception& e) {
      IOTA_LOG_DEBUG(m_logger, "boost::exception ");
    } catch (std::exception& e) {
      IOTA_LOG_DEBUG(m_logger, "std::exception: " << e.what());
    }
  } else {
    IOTA_LOG_DEBUG(m_logger, "OPTION:  " << name << " IS UNKNOWN");
    iota::RestHandle::set_option(name, value);
  }
}

void iota::esp::MqttService::start() {
  std::cout << "START PLUGIN MQtt" << std::endl;
  std::map<std::string, std::string> filters;

  iota::esp::ngsi::IotaMqttServiceImpl* cbImpl_ptr =
      new iota::esp::ngsi::IotaMqttServiceImpl(get_resource());
  cbImpl_ptr->set_resthandle(this);
  cbImpl_ptr->set_command_service(this);

  setIotaMqttService(cbImpl_ptr);

  add_url("", filters, REST_HANDLE(&iota::esp::MqttService::op_ngsi), this);

  startESP();

  enable_ngsi_service(filters, REST_HANDLE(&iota::esp::MqttService::op_ngsi),
                      this);
}

void iota::esp::MqttService::op_ngsi(
    pion::http::request_ptr& http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response, std::string& response) {
  default_op_ngsi(http_request_ptr, url_args, query_parameters, http_response,
                  response);
}

void iota::esp::MqttService::op_mqtt(
    pion::http::request_ptr& http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response, std::string& response) {
  response.assign("Executing iota::esp::MqttService::op_mqtt MqttService");
}

/**
  This means that MQTT will always support PUSH command only.
*/
bool iota::esp::MqttService::is_push_type_of_command(
    boost::shared_ptr<Device> device) {
  IOTA_LOG_DEBUG(m_logger, "MqttService::is_push_type_of_command : TRUE");
  return true;
}

int iota::esp::MqttService::execute_mqtt_command(std::string apikey,
                                                 std::string device,
                                                 std::string name,
                                                 std::string command_payload,
                                                 std::string command_id) {
  int code = 400;

  std::map<std::string, std::string> cparams;

  std::string payload("cmdid|");
  payload.append(command_id);
  payload.append("#");
  payload.append(command_payload);

  cparams.insert(std::pair<std::string, std::string>("apikey", apikey));
  cparams.insert(std::pair<std::string, std::string>("sensorid", device));
  cparams.insert(std::pair<std::string, std::string>("cmdname", name));

  cparams.insert(std::pair<std::string, std::string>("cmdparams", payload));

  IOTA_LOG_DEBUG(m_logger, "MqttService::execute_mqtt_command :/" + apikey +
                               " / " + device + " / " + "cmd / " + name);
  ESP_Input_Base* input =
      iota::esp::MqttService::getESPLib()->sensors[idsensor]->getInputFromName(
          "mqttwriter");

  if (NULL != input) {
    std::vector<ESP_Attribute> result =
        iota::esp::MqttService::getESPLib()
            ->executeRunnerFromInput(idsensor, "sendcmd", cparams, input)
            .ppresults;
    // TODO: how to check it was published?
    code = 202;
  }

  return code;
}

void iota::esp::MqttService::respond_mqtt_command(std::string apikey,
                                                  std::string device,
                                                  std::string command_payload,
                                                  std::string command_id) {
  std::string command;

  boost::property_tree::ptree service_ptree;
  boost::shared_ptr<iota::Device> dev;

  get_service_by_apiKey(service_ptree, apikey);

  std::string srv =
      service_ptree.get<std::string>(iota::store::types::SERVICE, "");
  std::string srv_path =
      service_ptree.get<std::string>(iota::store::types::SERVICE_PATH, "");

  dev = get_device(device, srv, srv_path);

  iota::CommandPtr commandPtr = get_command(command_id, srv, srv_path);

  if (commandPtr.get() == NULL) {
    IOTA_LOG_ERROR(
        m_logger,
        "processCommandResponse: already responded, command not in cache id: "
            << command_id << " service:" << srv
            << " service path: " << srv_path);
  } else {
    IOTA_LOG_DEBUG(m_logger, "processCommandResponse: command in cache id: "
                                 << command_id << "  service:" << srv
                                 << " service path: " << srv_path);
    command = commandPtr->get_name();
    commandPtr->cancel();
    // It is very important to remove command
    remove_command(command_id, srv, srv_path);

    response_command(command, command_payload, dev, service_ptree);
  }
}

int iota::esp::MqttService::execute_command(
    const std::string& endpoint, const std::string& command_id,
    const boost::property_tree::ptree& command_to_send, int timeout,
    const boost::shared_ptr<iota::Device>& item_dev,
    const boost::property_tree::ptree& service, std::string& response,
    iota::HttpClient::application_callback_t callback) {
  // deserialize the message
  std::string apikey;
  std::string iddevice;
  std::string cmdname;
  std::string cmdPayload;

  std::string srv = service.get<std::string>(iota::store::types::SERVICE, "");
  std::string srv_path =
      service.get<std::string>(iota::store::types::SERVICE_PATH, "");

  apikey = service.get<std::string>(iota::store::types::APIKEY, "");

  iddevice.assign(item_dev->unique_name());
  cmdPayload = command_to_send.get<std::string>(iota::store::types::BODY, "");

  cmdname = command_to_send.get<std::string>(iota::store::types::NAME, "");
  IOTA_LOG_DEBUG(m_logger, "MqttService::execute_command apikey: ["
                               << apikey << "] device [" << iddevice
                               << "] name: [" << cmdname << "] payload ["
                               << cmdPayload << "]");

  return execute_mqtt_command(apikey, iddevice, cmdname, cmdPayload,
                              command_id);
}

void iota::esp::MqttService::transform_command(
    const std::string& command_name, const std::string& a_command_value,
    const std::string& a_updateCommand_value, const std::string& sequence_id,
    const boost::shared_ptr<iota::Device>& item_dev,
    const boost::property_tree::ptree& service, std::string& command_id,
    boost::property_tree::ptree& command_line) {
  std::string command_value(a_command_value);
  std::string updateCommand_value(a_updateCommand_value);
  boost::trim(command_value);
  boost::trim(updateCommand_value);
  IOTA_LOG_DEBUG(m_logger, "MqttService::transform_command: value ["
                               << command_value << "]");
  IOTA_LOG_DEBUG(m_logger,
                 "MqttService::transform_command: updateCommand Value ["
                     << updateCommand_value << "]");

  updateCommand_value.assign(json_value_to_ul(updateCommand_value));
  IOTA_LOG_DEBUG(m_logger, "MqttService::transform_command: processed: "
                               << updateCommand_value);

  command_line.put(iota::store::types::NAME, command_name);
  std::string result;
  std::string key = "|";

  if (command_value.compare(iota::types::RAW) == 0) {
    result = updateCommand_value;
  } else {
    result.append(item_dev->_name);
    result.append("@");
    result.append(command_name);
    if (!boost::starts_with(key, updateCommand_value)) {
      result.append(key);
    }
    if (!updateCommand_value.empty()) {
      result.append(updateCommand_value);
    }
  }

  if (sequence_id.empty()) {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    command_id.assign(boost::lexical_cast<std::string>(uuid));
  } else {
    command_id.assign(sequence_id);
  }
  command_line.put(iota::store::types::BODY, result);
  IOTA_LOG_DEBUG(m_logger, "MqttService::transform_command result: [" << result
                                                                      << "]");
}

std::string iota::esp::MqttService::serializeMqttCommand(std::string apikey,
                                                         std::string device,
                                                         std::string command,
                                                         std::string payload) {
  std::string result(apikey);
  result.append("/");
  result.append(device);
  result.append("/");
  result.append("cmd");
  result.append("/");
  result.append(command);
  result.append("//");
  result.append(payload);

  return result;
}

// creates new miplugin objects
//
extern "C" PION_PLUGIN iota::esp::MqttService* pion_create_MqttService(void) {
  return new iota::esp::MqttService();
}

/// destroys miplugin objects
extern "C" PION_PLUGIN void pion_destroy_MqttService(
    iota::esp::MqttService* service_ptr) {
  delete service_ptr;
}
