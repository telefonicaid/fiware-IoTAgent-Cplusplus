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
#include "postprocessor_text/ESP_Plugin_Postprocessor_Text.h"
#include "ESP_XmlUtils.h"
#include "CC_Logger.h"
#include "stdio.h"
#include "ESP_StringUtils.h"
#include <sstream>

ESP_Plugin_Postprocessor_Text* ESP_Plugin_Postprocessor_Text::instance = NULL;

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< PLUGIN POSTPROCESSOR BASIC >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
ESP_Postprocessor_Base* ESP_Plugin_Postprocessor_Text::createPostprocessor(
    TiXmlElement* element) {
  std::string type = ESP_XmlUtils::queryStringValue(element, "type");

  if (type == "text") {
    return new ESP_Postprocessor_Text();
  }
  return NULL;
}

ESP_Plugin_Postprocessor_Text* ESP_Plugin_Postprocessor_Text::getSingleton() {
  if (ESP_Plugin_Postprocessor_Text::instance == NULL) {
    ESP_Plugin_Postprocessor_Text::instance =
        new ESP_Plugin_Postprocessor_Text();
  }
  return ESP_Plugin_Postprocessor_Text::instance;
}

ESP_Postprocessor_Text::ESP_Postprocessor_Text() {}

bool ESP_Postprocessor_Text::initialize() { return true; }

bool ESP_Postprocessor_Text::execute(CC_AttributesType* attributes) {
  result.clear();
  for (CC_AttributesType::iterator it = attributes->begin();
       it != attributes->end(); it++) {
    result.append(it->second.getName());
    result.append(":");
    result.append(it->second.getValueAsString());
    result.append("\n");
  }
  return true;
}

bool ESP_Postprocessor_Text::terminate() { return true; }

void ESP_Postprocessor_Text::parseCustomElement(TiXmlElement* element) {}

const char* ESP_Postprocessor_Text::getResultData() { return result.c_str(); }

int ESP_Postprocessor_Text::getResultSize() { return result.length(); }

bool ESP_Postprocessor_Text::isResultValid() { return result.length() > 0; }

ESP_Postprocessor_Text::~ESP_Postprocessor_Text() {}
