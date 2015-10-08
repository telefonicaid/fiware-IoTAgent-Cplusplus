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
#ifndef IOTAMQTTSERVICEIMPL_H
#define IOTAMQTTSERVICEIMPL_H

#include "IotaMqttService.h"
#include "util/iota_logger.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include "ngsi/UpdateContext.h"
#include "rest/riot_conf.h"
#include "rest/iot_cb_comm.h"
#include <pion/http/response_writer.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "MqttService.h"

#include "util/TTCBPublisher.h"

#define CB_DEFAULT_TYPE "string"

/**
This is the class that will actually implement the sending of entities to the
ContextBroker
*/

namespace iota {
namespace esp {
namespace ngsi {
class IotaMqttServiceImpl : public IotaMqttService {
 public:
  IotaMqttServiceImpl(std::string iotServiceName);

  virtual ~IotaMqttServiceImpl();
  void set_resthandle(iota::RestHandle* service_ptr);
  void set_command_service(iota::esp::ngsi::IotaMqttCommands* command_ptr);

  /**
  @name extract_command_id
  @brief it will return the payload without the command id, and the command id
  in a separate string.
  This is used for processing command responses in mqtt, where the command id is
  mandatory to be included
  in the payload using the format "cmdid|<command id>#". If not present, an
  exception will be thrown.

  @param [IN] in_payload: the mqtt response payload including "cmdid|<comand
  id>#" among other parameters
  @param [OUT] out_payload: the payload without the cmdid
  @param [OUT] out_id: the desired command id.
  @throw iota::IotaException when cmdid is missing.
  */
  void extract_command_id(std::string in_payload, std::string& out_payload,
                          std::string& out_id);

 protected:
 private:
  pion::logger m_logger;
  std::string iotService;

  iota::tt::TTCBPublisher* publisher_ptr;

  iota::RestHandle* resthandle_ptr_;

  iota::esp::ngsi::IotaMqttCommands* mqtt_command_ptr_;

  std::string doPublishCB(std::string& apikey, std::string& idDevice,
                          std::string& json);

  void processCommandResponse(std::string& apikey, std::string& idDevice,
                              std::string& payload);

  void add_info(boost::property_tree::ptree& pt, const std::string& apiKey);
};
}
}
}
#endif  // IOTAMQTTSERVICEIMPL_H
