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

#include "IotaMqttService.h"
#include "../util/iota_exception.h"

/**
TODO: this service has some methods for handling  pull commands, but as a new
feture of 1.2.0 version, mqtt will support
push commands only, so it would make sense to remove some of these commands.
*/

#include "rest/process.h"

iota::esp::ngsi::IotaMqttService::IotaMqttService()
    : m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  // ctor
}

iota::esp::ngsi::IotaMqttService::~IotaMqttService() {
  // dtor
}

std::string iota::esp::ngsi::IotaMqttService::publishContextBroker(
    std::string& jsonMsg, std::string& apikey, std::string& idDevice) {
  return doPublishCB(apikey, idDevice, jsonMsg);
}

void iota::esp::ngsi::IotaMqttService::handle_mqtt_message(
    std::string& apikey, std::string& idDevice, std::string& payload,
    std::string& type) {
  try {
    if (MQTT_COMMAND_RESPONSE == type) {
      return processCommandResponse(apikey, idDevice, payload);
    }

    if (MQTT_COMMAND_IGNORE == type) {  // This is for when "cmd" is echoed back
      // to us. This is nasty, but a limitation
      // of MQTT broker.
      return;
    }

    if (payload != "") {
      // when type is not either "cmdget" or "cmdexe", payload is an actual JSON
      publishContextBroker(payload, apikey, idDevice);
    }

  } catch (iota::IotaException& ex) {
    IOTA_LOG_ERROR(m_logger, "Error while processing mqtt message"
                                 << ex.what() << " payload: " << payload
                                 << " apikey: [" << apikey << "] device: ["
                                 << idDevice << "]");
  } catch (...) {
  }
}
