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
#include "../util/FuncUtil.h"
#include "../rest/types.h"


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

void iota::esp::ngsi::IotaMqttService::publishMultiAttribute(
    std::string& multi_payload, std::string& apikey, std::string& idDevice) {
  std::string token = UL20_MEASURE_SEPARATOR;

  std::vector<std::string> tokens_measures =
      riot_tokenizer(multi_payload, token);

  std::vector<std::string> v_jsons;

  if (tokens_measures.size() == 0) {
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER,
                              "Protocol error, no multiple measures",
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  for (int i = 0; i < tokens_measures.size(); i++) {
    std::string separator = UL20_SEPARATOR;
    std::string measure = tokens_measures[i];

    std::vector<std::string> tokens_io = riot_tokenizer(measure, separator);
    IOTA_LOG_DEBUG(m_logger, "MQTT MultiAttribute: [" << i << "]:[" << measure
                                                      << "]");

    v_jsons.clear();
    int j = 0;

    while (j < tokens_io.size()) {
      std::string attr_name = tokens_io.at(j);
      std::string value_str = tokens_io.at(j + 1);

      if (value_str.empty()) {
        std::ostringstream what;
        what << "Malformed frame :" << multi_payload;

        IOTA_LOG_ERROR(m_logger, what);

        throw iota::IotaException(
            iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER, what.str(),
            iota::types::RESPONSE_CODE_BAD_REQUEST);
      }

      mongo::BSONObj att_json = BSON("name" << attr_name << "type"
                                            << "string"
                                            << "value" << value_str);
      IOTA_LOG_INFO(m_logger, "Temporary JSON: " << att_json.jsonString());
      v_jsons.push_back(att_json.jsonString());
      j = j + 2;
    }

    doPublishMultiCB(apikey, idDevice, v_jsons);
  }
}

void iota::esp::ngsi::IotaMqttService::handle_mqtt_message(
    std::string& apikey, std::string& idDevice, std::string& payload,
    std::string& type) {
  try {
    if (MQTT_COMMAND_RESPONSE == type) {
      processCommandResponse(apikey, idDevice, payload);

    } else if (MQTT_MULTIATTRIBUTE == type) {
      publishMultiAttribute(payload, apikey, idDevice);

    } else if (payload != "") {
      // when type is not either "cmdget" or "cmdexe", payload is an actual JSON
      publishContextBroker(payload, apikey, idDevice);

    } else if (MQTT_COMMAND_IGNORE ==
               type) {  // This is for when "cmd" is echoed back
      // to us. This is nasty, but due to a side effect
      // on MQTT broker.
      return;
    }

  } catch (iota::IotaException& ex) {
    IOTA_LOG_ERROR(m_logger, "Error while processing mqtt message"
                                 << ex.what() << " payload: " << payload
                                 << " apikey: [" << apikey << "] device: ["
                                 << idDevice << "]");
  } catch (...) {
  }
}
