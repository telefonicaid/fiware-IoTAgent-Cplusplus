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
#include "TTModules.h"
#include "CC_Logger.h"

TTModules* TTModules::decodeModule(std::string keyword) {
  if (keyword == "GPS") {
    return new TTGPSModules(keyword);
  }

  if (keyword == "GM") {
    return new TTGM_Modules(keyword);
  }

  if (keyword == "GC") {
    return new TTGC_Modules(keyword);
  }

  return new TTP1_BModules(keyword);
}

TTModules::TTModules(std::string n) {
  name.assign(n);  // This is literally what goes in "name" of the produced JSON
                   // (in most of the cases).
  bValid = false;

  sleepCondition.assign("");
  sleepTime.assign("");
}

TTModules::~TTModules() {
  // dtor
}

bool TTModules::isValid() { return bValid; }

std::string TTModules::toJSON() { return json; }

std::string TTModules::toPlainJSON() { return json; }

void TTModules::setSleepCond(std::string& s) { sleepCondition.assign(s); }

void TTModules::setSleepTime(std::string& time) { sleepTime.assign(time); }

void TTModules::parseSleepCondAndTime(CC_AttributesType* attributes) {
  std::string sleepcondition("");
  std::string sleeptime("");

  CC_AttributesType::iterator itSleepTime = attributes->find("sleeptime");
  CC_AttributesType::iterator itSleepCond = attributes->find("sleepcondition");

  if (itSleepCond != attributes->end()) {
    sleepcondition.assign(itSleepCond->second.getValueAsString());
  }

  if (itSleepTime != attributes->end()) {
    sleeptime.assign(itSleepTime->second.getValueAsString());
  }

  setSleepCond(sleepcondition);
  setSleepTime(sleeptime);
}

std::string TTModules::doMetadatas() {
  std::string outMetadatas("");

  if (sleepCondition != "" || sleepTime != "") {
    outMetadatas.assign(",\"metadatas\":[{");
    outMetadatas.append(appendSleepCondTime());
    outMetadatas.append("}]");
  }

  return outMetadatas;
}

std::string TTModules::appendSleepCondTime() {
  std::string outMetadatas("");

  if (sleepCondition != "") {
    outMetadatas.append("\"name\":\"sleepcondition\",");
    outMetadatas.append("\"type\":\"string\",");
    outMetadatas.append("\"value\":\"");
    outMetadatas.append(sleepCondition);
    outMetadatas.append("\"");
  }

  if (sleepTime != "") {
    if (sleepCondition != "") {
      outMetadatas.append("},{");
    }

    outMetadatas.append("\"name\":\"sleeptime\",");
    outMetadatas.append("\"type\":\"string\",");
    outMetadatas.append("\"value\":\"");
    outMetadatas.append(sleepTime);
    outMetadatas.append("\"");
  }

  return outMetadatas;
}

TTGPSModules::TTGPSModules(std::string name) : TTModules(name) {}

void TTGPSModules::populate_default_internals() {
  refCoordX = "param_1";
  refCoordY = "param_2";
  separator = ",";

  metaName = "location";
  metaType = "string";
  metaValue = "WGS84";
}

void TTGPSModules::populateInternalMappings(TiXmlElement* element) {
  CC_Logger::getSingleton()->logDebug(
      "TTGPSModules: populateInternalMappings ...");
  TiXmlElement* xmlMapping = element->FirstChildElement(GPS_MAPPING);

  if (xmlMapping) {
    CC_Logger::getSingleton()->logDebug("TTGPSModules: %s found", GPS_MAPPING);
    // These two will be used as references when transforming.
    refCoordX = ESP_XmlUtils::queryStringValue(xmlMapping, "refA");
    refCoordY = ESP_XmlUtils::queryStringValue(xmlMapping, "refB");

    separator = ESP_XmlUtils::queryStringValue(xmlMapping, "separator");
  }

  TiXmlElement* xmlMetadata = element->FirstChildElement(GPS_METADATA);
  if (xmlMetadata) {
    // metadata attribute will be composed based on these values
    metaName = ESP_XmlUtils::queryStringValue(xmlMetadata, "name");
    metaType = ESP_XmlUtils::queryStringValue(xmlMetadata, "type");
    metaValue = ESP_XmlUtils::queryStringValue(xmlMetadata, "value");
  }

  if (refCoordX != "" && refCoordY != "" && separator != "" && metaName != "" &&
      metaType != "" && metaValue != "") {
    CC_Logger::getSingleton()->logDebug(
        "TTGPSModules: populateInternalMappings ... ALL DONE");
  } else {
    CC_Logger::getSingleton()->logError(
        "TTGPSModules: populateInternalMappings ... ERROR ,some internal "
        "mappings are empty");
  }
}

void TTGPSModules::transform(CC_AttributesType* attributes) {
  parseSleepCondAndTime(attributes);
  bValid = false;

  json.assign("{");

  CC_AttributesType::iterator itCoordX = attributes->find(refCoordX);
  CC_AttributesType::iterator itCoordY = attributes->find(refCoordY);
  if (itCoordX != attributes->end() && itCoordY != attributes->end() &&
      "" != itCoordX->second.getValueAsString() &&
      "" != itCoordY->second.getValueAsString()) {
    json.append("\"name\":\"position\",");
    json.append("\"type\":\"coords\",");
    json.append("\"value\":\"");

    json.append(itCoordX->second.getValueAsString());
    json.append(separator);
    json.append(itCoordY->second.getValueAsString());
    json.append("\",");
    json.append("\"metadatas\":[{");
    json.append("\"name\":\"");
    json.append(metaName);
    json.append("\",\"type\":\"");
    json.append(metaType);
    json.append("\",\"value\":\"");
    json.append(metaValue);
    json.append("\"}");
    // GPS has its own metadatas, so in this case, I need to append sleepCond
    // and SleepTime.
    std::string sleepAndCond = appendSleepCondTime();
    if (sleepAndCond != "") {
      json.append(",{");
      json.append(sleepAndCond);
      json.append("}");
    }
    json.append("]");
    bValid = true;
  } else {
    CC_Logger::getSingleton()->logError(
        "TTGPSModules: ERROR ,[%s] or [%s] not found in incoming "
        "CC_AttributesType",
        refCoordX.c_str(), refCoordY.c_str());

    bValid = false;
  }
  json.append("}");
}

TTP1_BModules::TTP1_BModules(std::string name) : TTModules(name) {}

void TTP1_BModules::addToMap(std::string paramPos, tt::Pair* pair) {
  mappings.insert(std::pair<std::string, tt::Pair*>(paramPos, pair));
}

void TTP1_BModules::populate_default_internals() {
  if (name == "P1") {
    /*
             <mapping ref="param_1" name="mcc"/>
            <mapping ref="param_2" name="mnc"/>
            <mapping ref="param_3" name="cell-id"/>
            <mapping ref="param_4" name="lac"/>
            <mapping ref="param_5" name="dbm" optional="true" />
    */

    tt::Pair* pair_mcc = new tt::Pair("mcc");

    addToMap("param_1", pair_mcc);

    tt::Pair* pair_mnc = new tt::Pair("mnc");
    addToMap("param_2", pair_mnc);

    tt::Pair* pair_cell = new tt::Pair("cell-id");
    addToMap("param_3", pair_cell);

    tt::Pair* pair_lac = new tt::Pair("lac");
    addToMap("param_4", pair_lac);

    tt::Pair* pair_dbm = new tt::Pair("dbm");
    pair_dbm->set_optional();
    addToMap("param_5", pair_dbm);
    CC_Logger::getSingleton()->logDebug("TTP1_BModules: loading Defaults P1");
  } else if (name == "B") {
    /*
           <ttmodule keyword="B" ref="module">
          <mapping ref="param_1" name="voltaje"/>
          <mapping ref="param_2" name="estado_activacion"/>
          <mapping ref="param_3" name="hay_cargador"/>
          <mapping ref="param_4" name="estado_carga"/>
          <mapping ref="param_5" name="modo_carga" />
          <mapping ref="param_6" name="tiempo_desco_stack" />
        </ttmodule>
  */

    tt::Pair* pair_volt = new tt::Pair("voltaje");

    addToMap("param_1", pair_volt);

    tt::Pair* pair_actv = new tt::Pair("estado_activacion");
    addToMap("param_2", pair_actv);

    tt::Pair* pair_charg = new tt::Pair("hay_cargador");
    addToMap("param_3", pair_charg);

    tt::Pair* pair_status = new tt::Pair("estado_carga");
    addToMap("param_4", pair_status);

    tt::Pair* pair_mode = new tt::Pair("modo_carga");
    addToMap("param_5", pair_mode);

    tt::Pair* pair_stack = new tt::Pair("tiempo_desco_stack");
    addToMap("param_6", pair_stack);
    CC_Logger::getSingleton()->logDebug("TTP1_BModules: loading Defaults B");
  }
}

void TTP1_BModules::populateInternalMappings(TiXmlElement* element) {
  CC_Logger::getSingleton()->logDebug(
      "TTP1_BModules: populateInternalMappings ...");
  TiXmlElement* xmlMapping = element->FirstChildElement(TAG_XML_MAPPING);

  type.assign("compound");

  while (xmlMapping) {
    //<mapping ref="param_1" name="mcc"/>
    std::string refParam = ESP_XmlUtils::queryStringValue(xmlMapping, "ref");
    std::string nameParam = ESP_XmlUtils::queryStringValue(xmlMapping, "name");
    bool optional = ESP_XmlUtils::queryBoolValue(xmlMapping, "optional");

    tt::Pair* pairParam = new tt::Pair(nameParam);
    if (optional) {
      pairParam->set_optional();
    }

    addToMap(refParam, pairParam);
    CC_Logger::getSingleton()->logDebug("Adding mapping: [%s]",
                                        nameParam.c_str());
    xmlMapping = xmlMapping->NextSiblingElement();
    /* if (xmlMapping == NULL)
     {
         break;
     }
     */
  }
}

void TTP1_BModules::transform(CC_AttributesType* inputMap) {
  // inputMap => "param_1", "-3.324" (== value)
  // inputMap => "param_2", "564"
  // inputMap => "param_3", "6CD"
  //(...)

  // mappings ==> "param_1", ( "mcc", <value> )
  // mappings ==> "param_2", ( "mnc", <value> )
  // (...)
  bValid = false;

  parseSleepCondAndTime(inputMap);

  std::string jSonValue;

  jSonValue.assign("{");

  int count = 0;
  bValid = true;
  CC_Logger::getSingleton()->logDebug("TTModules: checking map: size %d",
                                      mappings.size());
  for (std::map<std::string, tt::Pair*>::iterator itMap = mappings.begin();
       itMap != mappings.end(); itMap++) {
    CC_Logger::getSingleton()->logDebug("TTModules: checking map: ...");

    CC_AttributesType::iterator itInput = inputMap->find(itMap->first);
    if (itInput != inputMap->end() &&
        "" != itInput->second.getValueAsString()) {
      if (count > 0) {
        jSonValue.append("},{");
      }
      ESP_Attribute valueESP = itInput->second;

      CC_Logger::getSingleton()->logDebug(
          "TTModules: checking map: one hit found! [%s]", itMap->first.c_str());

      itMap->second->setValue(valueESP.getValueAsString());
      std::string value("");
      value.append("\"name\":\"");
      value.append(itMap->second->getName());
      value.append("\",\"type\":\"string\",\"value\":\"");
      value.append(valueESP.getValueAsString());
      value.append("\"");

      jSonValue.append(value);
      count++;
    } else if (itMap->second->is_mandatory()) {
      // Do something about missing parameter, like deeming the TTAttribute not
      // valid.
      bValid = false;
      CC_Logger::getSingleton()->logError(
          "TTModules: checking map: NOT found! [%s]", itMap->first.c_str());
    }
  }
  jSonValue.append("}");

  json.assign("{");

  json.append("\"name\":\"");
  json.append(name);
  json.append("\",");
  json.append("\"type\":\"compound\",");

  json.append("\"value\":[");
  json.append(jSonValue);

  json.append("]");

  json.append(doMetadatas());
  json.append("}");
}

TTP1_BModules::~TTP1_BModules() {
  std::map<std::string, tt::Pair*>::iterator itMap;
  for (itMap = mappings.begin(); itMap != mappings.end(); itMap++) {
    delete itMap->second;
  }
}

TTGM_Modules::TTGM_Modules(std::string name) : TTModules(name) {}

void TTGM_Modules::transform(CC_AttributesType* attributes) {
  bValid = false;
  CC_AttributesType::iterator itName = attributes->find("param_1");

  CC_AttributesType::iterator itValue = attributes->find("param_2");

  std::string nameGM("");
  std::string valueGM("");

  parseSleepCondAndTime(attributes);

  if (itValue == attributes->end()) {
    CC_Logger::getSingleton()->logError(
        "WARNING: missing [param_2] from incoming message.");
    bValid = false;
  } else {
    valueGM.assign(itValue->second.getValueAsString());
  }

  if (itName == attributes->end()) {
    CC_Logger::getSingleton()->logError(
        "WARNING: missing [param_1] from incoming message.");
    bValid = false;
  } else {
    nameGM.assign(itName->second.getValueAsString());
  }

  json.assign("{");

  if (nameGM != "" && valueGM != "") {
    json.append("\"name\":\"");
    json.append(nameGM);
    json.append("\",");
    json.append("\"type\":\"string\",");

    json.append("\"value\":\"");
    json.append(valueGM);
    json.append("\"");
    json.append(doMetadatas());
    bValid = true;
  }

  json.append("}");
}

void TTGM_Modules::populateInternalMappings(TiXmlElement* element) {}

TTGC_Modules::TTGC_Modules(std::string name) : TTModules(name) {}

void TTGC_Modules::transform(CC_AttributesType* attributes) {
  CC_AttributesType::iterator itName = attributes->find("param_1");

  CC_AttributesType::iterator itValue = attributes->find("param_2");

  std::string nameGC("");
  std::string valueGC("");

  parseSleepCondAndTime(attributes);

  bValid = false;

  if (itValue == attributes->end()) {
    CC_Logger::getSingleton()->logError(
        "WARNING: missing [param_2] from incoming message.");
    bValid = false;
  } else {
    valueGC.assign(itValue->second.getValueAsString());
  }

  if (itName == attributes->end()) {
    CC_Logger::getSingleton()->logError(
        "WARNING: missing [param_1] from incoming message.");
    bValid = false;
  } else {
    nameGC.assign(itName->second.getValueAsString());
  }

  json.assign("{");
  plainJson.assign("{");

  if (nameGC != "" && valueGC != "") {
    json.append("\"name\":\"");
    json.append(CB_OUTGC_PREFIX);
    json.append(nameGC);
    json.append("\",");
    json.append("\"type\":\"string\",");

    json.append("\"value\":\"");
    json.append(valueGC);

    json.append("\"");

    json.append(doMetadatas());

    plainJson.append("\"name\":\"");
    plainJson.append(nameGC);
    plainJson.append("\",");
    plainJson.append("\"type\":\"string\",");

    plainJson.append("\"value\":\"");
    plainJson.append(valueGC);

    plainJson.append("\"");
    plainJson.append(doMetadatas());

    bValid = true;
  }

  json.append("}");
  plainJson.append("}");
}

void TTGC_Modules::populateInternalMappings(TiXmlElement* element) {}

std::string TTGC_Modules::toPlainJSON() { return plainJson; }
