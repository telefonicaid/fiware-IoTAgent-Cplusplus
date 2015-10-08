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
#include "postprocessor_sensorml/ESP_Plugin_Postprocessor_Sensorml.h"
#include "ESP_XmlUtils.h"
#include "CC_Logger.h"
#include "stdio.h"
#include "ESP_StringUtils.h"
#include <sstream>

ESP_Plugin_Postprocessor_Sensorml* ESP_Plugin_Postprocessor_Sensorml::instance =
    NULL;

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< PLUGIN POSTPROCESSOR BASIC >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
ESP_Postprocessor_Base* ESP_Plugin_Postprocessor_Sensorml::createPostprocessor(
    TiXmlElement* element) {
  std::string type = ESP_XmlUtils::queryStringValue(element, "type");

  if (type == "sensorml") {
    return new ESP_Postprocessor_Sensorml(
        ESP_XmlUtils::queryStringValue(element, "name"), SML_MEASURE);
  }
  if (type == "sensorml_cmd") {
    return new ESP_Postprocessor_Sensorml(
        ESP_XmlUtils::queryStringValue(element, "name"), SML_COMMAND);
  }
  return NULL;
}

ESP_Plugin_Postprocessor_Sensorml*
ESP_Plugin_Postprocessor_Sensorml::getSingleton() {
  if (ESP_Plugin_Postprocessor_Sensorml::instance == NULL) {
    ESP_Plugin_Postprocessor_Sensorml::instance =
        new ESP_Plugin_Postprocessor_Sensorml();
  }
  return ESP_Plugin_Postprocessor_Sensorml::instance;
}

ESP_Postprocessor_Sensorml::ESP_Postprocessor_Sensorml(std::string name,
                                                       int sType) {
  this->_name = name;
  decl = new TiXmlDeclaration("1.0", "UTF-8", "no");
  doc.LinkEndChild(decl);
  pthread_mutex_init(&mutexPost, NULL);
  subType = sType;
}

bool ESP_Postprocessor_Sensorml::initialize() {
  pthread_mutex_lock(&mutexPost);
  printer = new TiXmlPrinter();
  printer->SetIndent("    ");
  smlKeyword.resetFound();
  result.clear();
  mapTrans.resetValues();  // Set Values to empty to start again (TODO: Check
                           // with david)
  return true;
}

void ESP_Postprocessor_Sensorml::preProcessAttributes(
    CC_AttributesType* attributes) {
  CC_AttributesType::iterator it;

  // Location
  it = attributes->find("location");
  if (it != attributes->end()) {
    std::string value = it->second.getValueAsString();

    // Decompose and Add Attributes
    value = ESP_StringUtils::trim(value, "#");
    CC_StringTokenizer ST(value, "/");
    for (int i = 0; i < ST.countElements(); i++) {
      ESP_Attribute attribute;
      std::string name;
      if (i == 0) {
        name = "latitude";
      }
      if (i == 1) {
        name = "longitude";
      }
      if (i == 2) {
        name = "altitude";
      }
      attribute.setValue(
          name, ST.elementAt(i).c_str(), ST.elementAt(i).length(),
          ESP_DataType::ESP_DataType_STRING, ESP_DataCode::ESP_DataCode_STRING);
      attributes->insert(CC_AttributesPair(name, attribute));
    }
  }
}

bool ESP_Postprocessor_Sensorml::execute(CC_AttributesType* attributes) {
  // Transform Attributes
  preProcessAttributes(attributes);

  if (attributes->find(smlKeyword.getKeyword()) != attributes->end()) {
    smlKeyword.setFound(true);

    // for (unsigned int i=0; i<context->result.size(); i++)
    for (CC_AttributesType::iterator it = attributes->begin();
         it != attributes->end(); it++) {
      mapTrans.transformRefToValue(it->second.getName(),
                                   it->second.getValueAsString());
      smlKeyword.testKey(
          it->second
              .getName());  // this will set the flag in case keyword is found
    }

    mapTrans.computeTimestamp();

    doc.Accept(printer);
    result = printer->CStr();
  }

  return true;
}

bool ESP_Postprocessor_Sensorml::terminate() {
  delete (printer);

  pthread_mutex_unlock(&mutexPost);
  return true;
}

void ESP_Postprocessor_Sensorml::parseCustomElement(TiXmlElement* element) {
  TiXmlElement* xmlKeyword = element->FirstChildElement(SML_KEYWORD);
  if (xmlKeyword) {
    smlKeyword.setKeyword(
        ESP_XmlUtils::queryStringValue(xmlKeyword, SML_VALUE));
    // CC_Logger::getSingleton()->logDebug("Found keyword
    // %s",ESP_XmlUtils::queryStringValue(xmlKeyword,SML_VALUE).c_str());
  }

  if (subType == SML_MEASURE) {
    smlObject = new ESP_SML_InsertObservation(element, &mapTrans);
  } else {
    TiXmlElement* xmlCmdResponse =
        element->FirstChildElement(SML_COMMAND_RESPONSE);
    smlObject = new ESP_SML_CommandResponse(xmlCmdResponse, &mapTrans);
  }

  xmlRes = smlObject->getTransformedXML();
  doc.LinkEndChild(xmlRes);
}

const char* ESP_Postprocessor_Sensorml::getResultData() {
  // CC_Logger::getSingleton()->logDebug("SensorML Postprocessor: returning
  // data");
  if (smlKeyword.canIPrint()) {
    CC_Logger::getSingleton()->logDebug(
        "SensorML PostProcessor %s: Returning output", _name.c_str());

  } else {
    // CC_Logger::getSingleton()->logDebug("SensorML PostProcessor %s: ignoring
    // output",_name.c_str());
    return NULL;
  }

  return result.c_str();
}

int ESP_Postprocessor_Sensorml::getResultSize() { return result.length(); }

bool ESP_Postprocessor_Sensorml::isResultValid() {
  return smlKeyword.canIPrint();
}

ESP_Postprocessor_Sensorml::~ESP_Postprocessor_Sensorml() {
  delete smlObject;
  delete decl;
  delete xmlRes;  // this will delete all XML objects linked to this element, so
                  // there is no need for deleting
  // every single xmlConverted object within ESP_SML_Base derived ones.
}
