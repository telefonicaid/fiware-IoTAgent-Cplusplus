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
#include "IotaMqttServiceImpl.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include "ngsi/UpdateContext.h"
#include "rest/iot_cb_comm.h"
#include <pion/http/response_writer.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "ngsi/Attribute.h"

#include "util/RiotISO8601.h"

#include "util/iota_exception.h"

iota::esp::ngsi::IotaMqttServiceImpl::IotaMqttServiceImpl(
    std::string iotServiceName)
    : m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  iotService.assign(iotServiceName);

  publisher_ptr = new iota::tt::TTCBPublisher();
}

void iota::esp::ngsi::IotaMqttServiceImpl::set_resthandle(
    iota::RestHandle* service_ptr) {
  resthandle_ptr_ = service_ptr;
}

void iota::esp::ngsi::IotaMqttServiceImpl::set_command_service(
    iota::esp::ngsi::IotaMqttCommands* command_ptr) {
  mqtt_command_ptr_ = command_ptr;
}

iota::esp::ngsi::IotaMqttServiceImpl::~IotaMqttServiceImpl() {
  // dtor
  delete publisher_ptr;
}

void iota::esp::ngsi::IotaMqttServiceImpl::extract_command_id(
    std::string payload, std::string& out_payload, std::string& out_id) {
  // examples of valid payloads:
  // cmdid|2238sdf98234987#dev01@PING#param|value
  // result|ok#cmdid|2238sdf98234987#
  // result|ok#cmdid|2238sdf98234987

  std::size_t cmd_id_pos = payload.find("cmdid|");

  int separator = 0;

  if (cmd_id_pos == std::string::npos) {
    throw new iota::IotaException("Command response not valid",
                                  "Missing cmd-id", 400);
  }

  std::string cmd_id =
      payload.substr(cmd_id_pos + 6, payload.length() - cmd_id_pos);

  std::size_t end_pos = cmd_id.find("#");

  if (end_pos == std::string::npos) {
    // if # not found, it means cmdid it's at the end, so we are done with
    // command id.
    out_id.assign(cmd_id);

  } else {
    out_id.assign(cmd_id.substr(0, end_pos));
    separator = 1;
  }

  IOTA_LOG_DEBUG(m_logger, "extract_command_id: command id found: " << out_id);

  // now let's remove the command id from the payload.

  out_payload.assign(
      payload.erase(cmd_id_pos, 6 + out_id.length() + separator));

  // IOTA_LOG_DEBUG (m_logger,"extract_command_id: start "<< (cmd_id_pos) << "
  // end " << (6+cmd_id.length()));

  IOTA_LOG_DEBUG(m_logger,
                 "extract_command_id: payload trimmed: " << out_payload);
}

void iota::esp::ngsi::IotaMqttServiceImpl::processCommandResponse(
    std::string& apikey, std::string& idDevice, std::string& raw_payload) {
  std::string payload;
  std::string cmd_id;
  std::string command;

  try {
    extract_command_id(raw_payload, payload, cmd_id);
    IOTA_LOG_DEBUG(m_logger,
                   "IotaMqttServiceImpl: processCommandResponse, cmd id "
                       << cmd_id << " apikey: " << apikey);

    mqtt_command_ptr_->respond_mqtt_command(apikey, idDevice, payload, cmd_id);

  } catch (iota::IotaException ex) {
    IOTA_LOG_ERROR(m_logger, "Error while processing response to command:"
                                 << ex.what() << " payload: " << raw_payload
                                 << " apikey " << apikey << " device "
                                 << idDevice);
  } catch (...) {
  }
}

/**
*/
void iota::esp::ngsi::IotaMqttServiceImpl::add_info(
    boost::property_tree::ptree& pt, const std::string& apiKey) {
  try {
    resthandle_ptr_->get_service_by_apiKey(pt, apiKey);
    std::string timeoutSTR = pt.get<std::string>("timeout", "0");
    int timeout = boost::lexical_cast<int>(timeoutSTR);
    std::string service = pt.get<std::string>("service", "");
    std::string service_path = pt.get<std::string>("service_path", "");
    std::string token = pt.get<std::string>("token", "");

    IOTA_LOG_DEBUG(m_logger, "Config retrieved: token: "
                                 << token << " service_path: " << service_path);

    pt.put("timeout", timeout);
    pt.put("service", service);
    pt.put("service_path", service_path);

  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, "Configuration error for service: "
                                 << iotService << " [" << e.what() << "] ");
    throw e;
  }
}

std::string iota::esp::ngsi::IotaMqttServiceImpl::doPublishCB(
    std::string& apikey, std::string& idDevice, std::string& json) {
  if (apikey == "") {
    std::ostringstream what;
    what << "Mandatory field missing: ";
    what << "[apikey]";

    throw iota::IotaException(what.str(), "", 400);
  }

  if (idDevice == "") {
    std::ostringstream what;
    what << "Mandatory field missing: ";
    what << "[idDevice]";
    throw iota::IotaException(what.str(), "", 400);
  }

  std::string cb_response;

  IOTA_LOG_DEBUG(m_logger, "IotaMqttServiceImpl, doPublishCB... json:"
                               << json << " apikey: " << apikey
                               << " device: " << idDevice);

  boost::property_tree::ptree pt_cb;

  try {
    add_info(pt_cb, apikey);
    std::string serviceMQTT = pt_cb.get<std::string>("service", "");

    boost::shared_ptr<iota::Device> dev =
        resthandle_ptr_->get_device(idDevice, serviceMQTT);

    iota::ContextElement ngsi_context_element(idDevice, "", "false");

    ngsi_context_element.set_env_info(pt_cb, dev);

    IOTA_LOG_DEBUG(m_logger,
                   "MQTTService: Creating entity : ["
                       << ngsi_context_element.get_string()
                       << "]");  // call to populate internal fields from cache,
                                 // or by default, etc... not interesated in
                                 // result

    iota::RiotISO8601 timeInstant;

    IOTA_LOG_INFO(m_logger, "Adding attribute to ContextElement: " << json);

    std::vector<std::string> vJsonTT;

    vJsonTT.push_back(json);

    cb_response.assign(publisher_ptr->publishContextBroker(
        ngsi_context_element, vJsonTT, pt_cb, timeInstant));

    IOTA_LOG_DEBUG(m_logger, "ContextBroker RESPONSE: " << cb_response);

  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, "Configuration error : " << e.what());
    std::ostringstream what;
    what << "Can't publish on ContextBroker:";
    what << e.what();
    throw iota::IotaException(what.str(), "", 400);

  } catch (...) {
  }

  return cb_response;
}
