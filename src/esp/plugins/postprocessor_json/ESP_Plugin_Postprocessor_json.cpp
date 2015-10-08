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
#include "ESP_Plugin_Postprocessor_json.h"
#include "ESP_XmlUtils.h"
#include "CC_Logger.h"

ESP_Plugin_Postprocessor_json* ESP_Plugin_Postprocessor_json::instance = NULL;
std::string ESP_Postprocessor_json::TYPE = "json";

ESP_Plugin_Postprocessor_json::ESP_Plugin_Postprocessor_json() {
  // ctor
}

ESP_Postprocessor_Base* ESP_Plugin_Postprocessor_json::createPostprocessor(
    TiXmlElement* element) {
  std::string type = ESP_XmlUtils::queryStringValue(element, "type");

  if (type == ESP_Postprocessor_json::TYPE) {
    return new ESP_Postprocessor_json();
  }
  return NULL;
}

ESP_Plugin_Postprocessor_json* ESP_Plugin_Postprocessor_json::getSingleton() {
  if (ESP_Plugin_Postprocessor_json::instance == NULL) {
    ESP_Plugin_Postprocessor_json::instance =
        new ESP_Plugin_Postprocessor_json();
  }
  return ESP_Plugin_Postprocessor_json::instance;
}

ESP_Postprocessor_json::ESP_Postprocessor_json() {
  _type = ESP_Postprocessor_json::TYPE;
}

bool ESP_Postprocessor_json::initialize() {
  result.erase();
  resultValid = false;
  return true;
}

bool ESP_Postprocessor_json::terminate() { return true; }

/**
True is returned whenever the output must be used, so there are three cases:
- cmdget (empty result)
- cmdexe (response to a command with SOME result)
- measure (result will contain a JSON)
*/
bool ESP_Postprocessor_json::execute(CC_AttributesType* attributes) {
  CC_Logger::getSingleton()->logDebug("JSON Postprocessor: EXECUTING");

  CC_AttributesType::iterator it, it2, it_response;

  it_response = attributes->find(bypassRef);

  if (it_response != attributes->end()) {
    // This scenario means, response has to be passed through as it was.
    // useful for commands
    result.assign(it_response->second.getValueAsString());

    CC_Logger::getSingleton()->logDebug(
        "JSON Postprocessor: Bypassing CMD RESPONSE result [%s]",
        result.c_str());

    resultValid = true;
    return true;
  }

  it_response = attributes->find(bypassCmdGet);

  if (it_response != attributes->end()) {
    result.assign("");
    CC_Logger::getSingleton()->logDebug(
        "JSON Postprocessor: Bypassing CMD GET result [%s] <- should be empty",
        result.c_str());
    resultValid = true;
    return true;
  }

  CC_Logger::getSingleton()->logDebug(
      "JSON Postprocessor: Searching alias keyword: [%s]", aliasRef.c_str());

  it = attributes->find(aliasRef);

  if (it != attributes->end()) {
    CC_Logger::getSingleton()->logDebug(
        "JSON Postprocessor: Searching VALUE  keyword: [%s]", valueRef.c_str());

    it2 = attributes->find(valueRef);
    if (it2 != attributes->end()) {
      result.append(std::string("{\"name\" : \""));
      result.append(it->second.getValueAsString());
      result.append("\",\"type\":\"string\",");
      result.append(std::string("\"value\" : \""));
      result.append(it2->second.getValueAsString());
      result.append(std::string("\"}"));

      resultValid = true;
      CC_Logger::getSingleton()->logDebug(
          "JSON Postprocessor: result generated");
    }
  }

  return resultValid;
}

void ESP_Postprocessor_json::parseCustomElement(TiXmlElement* element) {
  // Parsing of mappings for CB entities.
  CC_Logger::getSingleton()->logDebug("JSON Postprocessor: parsing XML");
  TiXmlElement* xmlMeasure = element->FirstChildElement(TAG_XML_VALUEREF);
  if (xmlMeasure) {
    valueRef = ESP_XmlUtils::queryStringValue(xmlMeasure, ATTRB_REF);
  }
  TiXmlElement* xmlalias = element->FirstChildElement(TAG_XML_ALIASREF);
  if (xmlalias) {
    aliasRef = ESP_XmlUtils::queryStringValue(xmlalias, ATTRB_REF);
  }

  TiXmlElement* xmlBypass = element->FirstChildElement(TAG_XML_BYPASS_RESPONSE);
  if (xmlBypass) {
    bypassRef = ESP_XmlUtils::queryStringValue(xmlBypass, ATTRB_REF);
  }

  TiXmlElement* xmlBypassGet =
      element->FirstChildElement(TAG_XML_BYPASS_CMDGET);
  if (xmlBypassGet) {
    bypassCmdGet = ESP_XmlUtils::queryStringValue(xmlBypassGet, ATTRB_REF);
  }
}

const char* ESP_Postprocessor_json::getResultData() { return result.c_str(); }

int ESP_Postprocessor_json::getResultSize() { return result.length(); }

bool ESP_Postprocessor_json::isResultValid() { return resultValid; }
