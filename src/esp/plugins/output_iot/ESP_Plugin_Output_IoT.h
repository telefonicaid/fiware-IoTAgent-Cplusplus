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
#ifndef __ESP_PLUGIN_OUTPUT_IoT_H__
#define __ESP_PLUGIN_OUTPUT_IoT_H__

#include <TDA.h>
#include "mqtt/IotaMqttService.h"

#define IOT_APIKEY_REF "apikeyref"  // xml attribute
#define IOT_DEVICE_REF "sensoridref"
#define IOT_TYPE_REF "typeref"  // from the config file,
// ESP attribute that will contain the type of request, used for commands.

/* //////////////////  */
/* PLUGIN OUTPUT FILE   */
/* //////////////////  */
class ESP_Plugin_Output_IoT : public ESP_Plugin_Output_Base {
 private:
  static ESP_Plugin_Output_IoT* instance;
  int id;
  std::map<int, FILE*> files;
  ESP_Plugin_Output_IoT();

 public:
  static ESP_Plugin_Output_Base* getSingleton();
  static ESP_Plugin_Output_IoT* getInstance();
  ESP_Output_Base* createOutput(TiXmlElement* element);
};

class ESP_Output_IoT : public ESP_Output_Base {
 public:
  std::string _name;
  // where to find apikey, iddevice, etc... in output ESP Attributes.
  std::string apikeyAttribute;
  std::string idDeviceAttribute;
  std::string typeAttribute;
#ifdef USE_MQTT
  bool execute(CC_AttributesType* attributes,
               ESP_Postprocessor_Base* postprocessor,
               std::map<std::string, void*> userData);
#endif
  void parseCustomElement(TiXmlElement* element);
};

#endif
