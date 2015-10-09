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
#include "ESP_Plugin_Postprocessor_TT.h"
#include "ESP_XmlUtils.h"
#include "CC_Logger.h"

ESP_Plugin_Postprocessor_TT* ESP_Plugin_Postprocessor_TT::instance = NULL;
std::string ESP_Postprocessor_TT::TYPE = "TTOpen";

ESP_Plugin_Postprocessor_TT::ESP_Plugin_Postprocessor_TT() {
  // ctor
}

ESP_Postprocessor_Base* ESP_Plugin_Postprocessor_TT::createPostprocessor(
    TiXmlElement* element) {
  std::string type = ESP_XmlUtils::queryStringValue(element, "type");

  if (type == ESP_Postprocessor_TT::TYPE) {
    return new ESP_Postprocessor_TT();
  }
  return NULL;
}

ESP_Plugin_Postprocessor_TT* ESP_Plugin_Postprocessor_TT::getSingleton() {
  if (ESP_Plugin_Postprocessor_TT::instance == NULL) {
    ESP_Plugin_Postprocessor_TT::instance = new ESP_Plugin_Postprocessor_TT();
  }
  return ESP_Plugin_Postprocessor_TT::instance;
}

ESP_Postprocessor_TT::ESP_Postprocessor_TT() {
  _type = ESP_Postprocessor_TT::TYPE;
}

bool ESP_Postprocessor_TT::initialize() {
  result.erase();
  resultValid = false;
  return true;
}

bool ESP_Postprocessor_TT::terminate() { return true; }

bool ESP_Postprocessor_TT::execute(CC_AttributesType* attributes) {
  CC_Logger::getSingleton()->logDebug("TT Postprocessor: EXECUTING");
  resultValid = false;

  CC_AttributesType::iterator itModule = attributes->find("module");

  if (itModule == attributes->end()) {
    CC_Logger::getSingleton()->logError(
        "PostProcessor TT: Missing keyword: [module]");
    return false;
  }

  // Now it's time to check if the keyword is in the map.
  std::string keyword(itModule->second.getValueAsString());
  CC_Logger::getSingleton()->logDebug("Searching module [%s]", keyword.c_str());

  // processing begins here:
  std::map<std::string, TTModules*>::iterator itParam =
      modulesMap.find(keyword);

  if (itParam != modulesMap.end()) {
    TTModules* ttObj = (TTModules*)itParam->second;

    CC_Logger::getSingleton()->logDebug("Module found: transforming...");

    ttObj->transform(attributes);

    if (ttObj->isValid()) {
      result.assign("{ \"");
      result.append(TT_PROCESSED);
      result.append("\" : ");
      result.append(ttObj->toJSON());
      result.append(",\"");
      result.append(TT_PLAIN);
      result.append("\" : ");
      result.append(ttObj->toPlainJSON());
      result.append("}");
    }

    if (ttObj->toJSON() != "" && ttObj->toPlainJSON() != "" &&
        ttObj->isValid()) {
      resultValid = true;
    }

    CC_Logger::getSingleton()->logDebug("Module found: transforming... DONE");
  }
  return true;
}

void ESP_Postprocessor_TT::parseCustomElement(TiXmlElement* element) {
  CC_Logger::getSingleton()->logDebug("TT Postprocessor: parsing XML");

  TiXmlElement* xmlTTModule = element->FirstChildElement(TAG_XML_TTMODULE);
  while (xmlTTModule) {
    std::string keyword;

    keyword = ESP_XmlUtils::queryStringValue(xmlTTModule, ATTRB_KEYWORD);

    TTModules* ttModule = TTModules::decodeModule(keyword);
    ttModule->populateInternalMappings(xmlTTModule);
    modulesMap.insert(std::pair<std::string, TTModules*>(keyword, ttModule));

    CC_Logger::getSingleton()->logDebug(
        "PostprocessorTT: Adding [%s] to the map", keyword.c_str());
    xmlTTModule = xmlTTModule->NextSiblingElement();
  }
  checkParams();
}

void ESP_Postprocessor_TT::load_default_TTModules() {
  TTGC_Modules* tt_gc_ptr = new TTGC_Modules("GC");
  tt_gc_ptr->populate_default_internals();

  TTGM_Modules* tt_gm_ptr = new TTGM_Modules("GM");
  tt_gm_ptr->populate_default_internals();

  TTGPSModules* tt_gps_ptr = new TTGPSModules("GPS");
  tt_gps_ptr->populate_default_internals();

  TTP1_BModules* tt_p1_ptr = new TTP1_BModules("P1");
  tt_p1_ptr->populate_default_internals();

  TTP1_BModules* tt_bat_ptr = new TTP1_BModules("B");
  tt_bat_ptr->populate_default_internals();

  modulesMap.insert(std::pair<std::string, TTModules*>("GC", tt_gc_ptr));
  modulesMap.insert(std::pair<std::string, TTModules*>("GM", tt_gm_ptr));
  modulesMap.insert(std::pair<std::string, TTModules*>("GPS", tt_gps_ptr));
  modulesMap.insert(std::pair<std::string, TTModules*>("P1", tt_p1_ptr));
  modulesMap.insert(std::pair<std::string, TTModules*>("B", tt_bat_ptr));
}

bool ESP_Postprocessor_TT::checkParams() {
  bool res = false;

  if (modulesMap.size() == 0) {
    CC_Logger::getSingleton()->logError(
        "No TTModules were found, TTOpen messages might not be processed "
        "properly");
  } else {
    res = true;
  }

  return res;
}

const char* ESP_Postprocessor_TT::getResultData() { return result.c_str(); }

int ESP_Postprocessor_TT::getResultSize() { return result.length(); }

bool ESP_Postprocessor_TT::isResultValid() { return resultValid; }

ESP_Postprocessor_TT::~ESP_Postprocessor_TT() {
  std::map<std::string, TTModules*>::iterator itMap;
  for (itMap = modulesMap.begin(); itMap != modulesMap.end(); itMap++) {
    delete itMap->second;
  }
  modulesMap.clear();
}
