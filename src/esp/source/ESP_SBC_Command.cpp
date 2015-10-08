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
#include "ESP_SBC_Command.h"
#include "ESP_XmlUtils.h"

#include <iostream>

ESP_SBC_Command::ESP_SBC_Command(std::string input) {
  // ctor
  parseCorrect = false;
  doc.Parse((const char*)input.c_str(), 0, TIXML_ENCODING_UTF8);
  TiXmlElement* element = doc.FirstChildElement(XML_PAID_COMMAND);
  if (element) {
    name = ESP_XmlUtils::queryStringValue(element, "name");
    dest = ESP_XmlUtils::queryStringValue(element, "dest");
    cmd_id = ESP_XmlUtils::queryStringValue(element, "id");

    if (cmd_id != "") {
      parseCorrect = true;
      outputUL20 = "cmdid|" + cmd_id + "#";
    }
    parseParams(element);
  }
}

std::string ESP_SBC_Command::getCommandName() { return name; }

std::string ESP_SBC_Command::getCommandID() { return cmd_id; }

std::string ESP_SBC_Command::getDestination() { return dest; }

void ESP_SBC_Command::parseParams(TiXmlElement* element) {
  TiXmlElement* pElem = element->FirstChildElement("paid:cmdParam");
  // param.
  for (; pElem; pElem = pElem->NextSiblingElement()) {
    std::string key = ESP_XmlUtils::queryStringValue(pElem, "name");
    TiXmlElement* pChild = pElem->FirstChildElement();
    if (pChild) {
      TiXmlElement* pGrandson = pChild->FirstChildElement("swe:value");
      if (pGrandson && pGrandson->GetText() > 0) {
        std::string value =
            std::string(pGrandson->GetText(), strlen(pGrandson->GetText()));
        mapParams.insert(std::pair<std::string, std::string>(key, value));
        outputUL20 = outputUL20 + key + "|" + value + "#";
      }
    }
  }
}

std::map<std::string, std::string> ESP_SBC_Command::getParamMap() {
  return mapParams;
}

ESP_SBC_Command::~ESP_SBC_Command() {
  // dtor
}

std::string ESP_SBC_Command::getUL20output() { return outputUL20; }

bool ESP_SBC_Command::isParseCorrect() { return parseCorrect; }
