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
#include "output_iot/ESP_Plugin_Output_IoT.h"
#include "ESP_XmlUtils.h"
#include "CC_Logger.h"

ESP_Plugin_Output_IoT* ESP_Plugin_Output_IoT::instance = NULL;

/* ---------------------- */
/* PLUGIN Output FILE      */
/* ---------------------- */
ESP_Plugin_Output_IoT::ESP_Plugin_Output_IoT() { id = 0; }

ESP_Plugin_Output_Base* ESP_Plugin_Output_IoT::getSingleton() {
  if (ESP_Plugin_Output_IoT::instance == NULL) {
    ESP_Plugin_Output_IoT::instance = new ESP_Plugin_Output_IoT();
  }
  return ESP_Plugin_Output_IoT::instance;
}

ESP_Plugin_Output_IoT* ESP_Plugin_Output_IoT::getInstance() {
  return (ESP_Plugin_Output_IoT*)ESP_Plugin_Output_IoT::getSingleton();
}

ESP_Output_Base* ESP_Plugin_Output_IoT::createOutput(TiXmlElement* element) {
  std::string type = ESP_XmlUtils::queryStringValue(element, "type");
  std::string name = ESP_XmlUtils::queryStringValue(element, "name");

  ESP_Output_Base* result = NULL;
  if (type == "iot") {
    CC_Logger::getSingleton()->logDebug("IOT output parser found");
    result = new ESP_Output_IoT();
  }

  // Assigns
  if (result != NULL) {
    result->_type = type;
    result->_name = name;
  }

  return result;
}

// --------------- //
// OUTPUT FILE
// --------------- //
void ESP_Output_IoT::parseCustomElement(TiXmlElement* element) {
  this->_name = ESP_XmlUtils::queryStringValue(element, "name");

  this->apikeyAttribute =
      ESP_XmlUtils::queryStringValue(element, IOT_APIKEY_REF);
  this->idDeviceAttribute =
      ESP_XmlUtils::queryStringValue(element, IOT_DEVICE_REF);

  this->typeAttribute = ESP_XmlUtils::queryStringValue(element, IOT_TYPE_REF);

  CC_Logger::getSingleton()->logDebug(
      "IoT Output Plugin, parsingCustomElement");
}

#ifdef USE_MQTT
bool ESP_Output_IoT::execute(CC_AttributesType* attributes,
                             ESP_Postprocessor_Base* postprocessor,
                             std::map<std::string, void*> userData) {
  CC_Logger::getSingleton()->logDebug("IoT Output Plugin: Execute");

  std::string apikey;
  std::string payload;
  std::string idDevice;
  std::string type;
  ESP_Attribute* apikeyattr =
      ESP_Attribute::searchAttributeRefByName(attributes, apikeyAttribute);
  ESP_Attribute* idDeviceAttr =
      ESP_Attribute::searchAttributeRefByName(attributes, idDeviceAttribute);

  ESP_Attribute* type_attr =
      ESP_Attribute::searchAttributeRefByName(attributes, typeAttribute);

  if (apikeyattr != NULL && !apikeyattr->getValueAsString().empty()) {
    apikey = apikeyattr->getValueAsString();
  } else {
    CC_Logger::getSingleton()->logError(
        "FATAL: apikey not found in received message.NO VALID OUTPUT WILL BE "
        "GENERATED. (Is the config file correct?) ");
  }
  if (idDeviceAttr != NULL && !idDeviceAttr->getValueAsString().empty()) {
    idDevice = idDeviceAttr->getValueAsString();
  } else {
    CC_Logger::getSingleton()->logError(
        "FATAL: device id not found in received message.NO VALID OUTPUT WILL "
        "BE "
        "GENERATED. (Is the config file correct?) ");
  }

  if (type_attr != NULL) {
    type = type_attr->getValueAsString();
  }

  // Check if it's a command request from the device.

  std::map<std::string, void*>::iterator it =
      userData.find("contextBrokerPublisher");

  if (it != userData.end()) {
    CC_Logger::getSingleton()->logDebug("Output IoT: IotMqttService found");

    // Now we can send data to it.
    iota::esp::ngsi::IotaMqttService* iota_publisher =
        (iota::esp::ngsi::IotaMqttService*)it->second;

    if (postprocessor->isResultValid()) {
      payload = std::string(postprocessor->getResultData());
      CC_Logger::getSingleton()->logDebug(
          "Output IoT: calling IotMqttService [%s] [%s] [%s] [%s]",
          apikey.c_str(), idDevice.c_str(), type.c_str(), payload.c_str());
      iota_publisher->handle_mqtt_message(apikey, idDevice, payload, type);
      return true;
    }
  }
  return false;
}
#endif
