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
#ifndef IOTAMQTTSERVICE_H
#define IOTAMQTTSERVICE_H

#include <string>

#include "util/iota_logger.h"

#define MQTT_COMMAND_REQUEST "cmdget"
#define MQTT_COMMAND_RESPONSE "cmdexe"
#define MQTT_COMMAND_IGNORE "cmd"
#define MQTT_MULTIATTRIBUTE "mul20"

#define UL20_SEPARATOR "|"
#define UL20_MEASURE_SEPARATOR "#"

// This is the interface known to Output_IoT

namespace iota {
namespace esp {
namespace ngsi {

/**
 @name IotaMqttService
 @brief This interface abstracts the service for interacting with the
 ContextBroker in the MQTT realm.
 In other words, this is all what MQTT protocol needs when interacting with CB,
 like publishing MQTT measures
 or receiving requests  or responses to commands. Some of those functions aren't
 offered by this service directly
 but by others referenced here.
 This class can do some basic operations with the information coming from the
 caller @see handle_mqtt_message,
 like checking if the mqtt message is a measure, a command request,or a response
 to a command.
 Based on that, it will call one of its virtual methods.

*/
class IotaMqttService {
 public:
  IotaMqttService();

  std::string publishContextBroker(std::string& jsonMsg, std::string& apikey,
                                   std::string& idDevice);

  /**
  @brief This method is the entrypoint for MQTT inner layer calls (caller is
  ESP)
  whenever an MQTT message arrives. There are four pieces of relevant
  information: apikey,
  device id and type are extracted from "topic" and payload from the message
  itself. There
  are three scenarios that are dealt with depending on the type:
  - cmdexe: This is the response for a MQTT command.
  - mul20: this is the multi-measure scenario.
  - any other type (distinct to "cmd"): will be considered as single mesaure
  where "type" is
  the name of the attribute and payload is the value.

  "cmd" type is also received as an echo from the MQTT broker, but this message
  comes from us
  (it represents an MQTT command to the device), so in this case is just
  ignored.
  @param apikey:
  @param idDevice:
  @param payload:
  @param type:

  @return the result of the call of this method should be the publication on
  ContextBroker of the
  Entity (in one or several UpdateContext).

  */
  void handle_mqtt_message(std::string& apikey, std::string& idDevice,
                           std::string& payload, std::string& type);

  virtual ~IotaMqttService();

 protected:
  void publishMultiAttribute(std::string& multi_payload, std::string& apikey,
                             std::string& idDevice);

  virtual std::string doPublishCB(std::string& apikey, std::string& idDevice,
                                  std::string& jsonMsg) = 0;

  virtual std::string doPublishMultiCB(std::string& apikey,
                                       std::string& idDevice,
                                       std::vector<std::string>& v_json) = 0;

  virtual void processCommandResponse(std::string& apikey,
                                      std::string& idDevice,
                                      std::string& payload) = 0;

 private:
  pion::logger m_logger;
};
}
}
}
#endif  // IOTAMQTTSERVICE_H
