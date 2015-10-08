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
#include "ESP_Plugin_Postprocessor_UL20.h"
#include "ESP_XmlUtils.h"
#include "CC_Logger.h"

ESP_Plugin_Postprocessor_UL20* ESP_Plugin_Postprocessor_UL20::instance = NULL;
std::string ESP_Postprocessor_UL20::TYPE = "ul20";

ESP_Plugin_Postprocessor_UL20::ESP_Plugin_Postprocessor_UL20() {
  // ctor
}

ESP_Postprocessor_Base* ESP_Plugin_Postprocessor_UL20::createPostprocessor(
    TiXmlElement* element) {
  std::string type = ESP_XmlUtils::queryStringValue(element, "type");

  if (type == ESP_Postprocessor_UL20::TYPE) {
    return new ESP_Postprocessor_UL20();
  }
  return NULL;
}

ESP_Plugin_Postprocessor_UL20* ESP_Plugin_Postprocessor_UL20::getSingleton() {
  if (ESP_Plugin_Postprocessor_UL20::instance == NULL) {
    ESP_Plugin_Postprocessor_UL20::instance =
        new ESP_Plugin_Postprocessor_UL20();
  }
  return ESP_Plugin_Postprocessor_UL20::instance;
}

ESP_Postprocessor_UL20::ESP_Postprocessor_UL20() {
  _type = ESP_Postprocessor_UL20::TYPE;
}

bool ESP_Postprocessor_UL20::initialize() {
  result = "";
  resultValid = false;

  return true;
}

bool ESP_Postprocessor_UL20::terminate() { return true; }

bool ESP_Postprocessor_UL20::execute(CC_AttributesType* attributes) {
  CC_AttributesType::iterator it;
  // Foreach element, just concatenate alias|value#alias2|value or just
  // alias|value if there's only one measure
  bool multi = false;

  if (bypass) {
    // Although Bypass might have been configured, we have to make sure
    // it's a multi measure scenario, so it will use topicRef and topicValue
    // to tell if multi-measures are coming into payload.

    it = attributes->find(topicRef);
    if (it != attributes->end()) {
      // typical scenario: value = "multi", so when topicRef = "type", topic
      // will be like
      // /multi/
      if (it->second.getValueAsString() == topicValue) {
        // it's a genuine multi scenario, payload will contain all measures in
        // UL2.0
        multi = true;
      }
    }

    // payload is just used as output
  }

  if (multi) {
    it = attributes->find(measure);

    if (it != attributes->end()) {
      result = it->second.getValueAsString();
      resultValid = true;
      return true;
    }
  }

  // Else:

  it = attributes->find(alias);

  if (it != attributes->end()) {
    result = it->second.getValueAsString() + "|";
    it = attributes->find(measure);

    if (it != attributes->end()) {
      result = result + it->second.getValueAsString();
      resultValid = true;
      return true;
    }
  }

  return false;
}

/**
How it works:
 UL2.0 postprocessor will output UL2.0 formatted message (for measures only).
 It has two operation modes:
 1. Single measure.
    This scenario means one single measure comes in a MQTT message in "raw"
format, that is
    to say, value is the payload, and the "last" topic level is something
unknown to ESP, in
    other words, it can be "any" alias.  Example:
    topic: 1234/id0001/t
    payload: 23

    What "t" means is not known by ESP, but the output of UL2.0 postprocessor
has to be:
        t|23

    CONFIGURATION.
    For single measure mode these are the required tags:
    * <measure ref="payload"/> : this will indicate where to find the actual
measure (value) in the attributes
    coming out of the parser. "payload" is the name of the attribute at parser
level.
    * <alias ref="type" /> : this is for the attribute that will contain the
"alias" (UL2.0 is alias|value)

    This means that for UL2.0 to work properly the "type" attribute has to
contain whatever comes in that topic level
    and the payload has to be stored as is. When parser has to take care of
multiple conditions, a new operation
    "default" might be used to tell the parser to process the payload as
expected.

 2. Multiple measures in UL2.0.
    Although the "multi" topic with UL2.0 measures is currently being processed
as SensorML creating multiple Observations,
    it makes sense to also have multiple measures returned as UL2.0. In this
case, what happens is that once "multi" is
    found in the topic (at "type" level) and if UL20 postprocessor is configured
for "bypass" measures, it will just
    extract the payload and return it as UL2.0. Example:
    topic: 1234/id001/multi
    payload: t|23#p|33#loc|34.33/2.23424

    Output: t|23#p|33#loc|34.33/2.23424

    CONFIGURATION.
    The tag "bypass" must be present. With this tag we tell the postprocessor to
search for two elements, the particular
    attribute that means is a multi measure message and its value (typically is
"type : multi"):
    * <bypass ref="type" value="multi" />

    With that, it will evaluate the attributes, and when "multi" is found as
"type", then it will just return payload
    as is.

    COMPATIBILITY: UL20 postprocessor can be configured to have both modes, in
other words, if multi measures are received,
    then it will output them as they come, but if single measure is received,
with the alias as "type", then it will create
    the UL2.0 message on its own.

    UPDATE: due to how condition tag works, it's not possible to have "default"
condition and "condition equal to multi" at the same
    time and receive an mqtt message with /multi/ topic, this would cause
issues. If UL20 processing is going to be used, then SensorML and
    multi-measure has to be disabled.


*/
void ESP_Postprocessor_UL20::parseCustomElement(TiXmlElement* element) {
  TiXmlElement* xmlMeasure = element->FirstChildElement("measure");
  if (xmlMeasure) {
    measure = ESP_XmlUtils::queryStringValue(xmlMeasure, "ref");
  }
  TiXmlElement* xmlalias = element->FirstChildElement("alias");
  if (xmlalias) {
    alias = ESP_XmlUtils::queryStringValue(xmlalias, "ref");
  }
  TiXmlElement* xmlBypass = element->FirstChildElement("bypass");
  if (xmlBypass) {
    bypass = true;
    topicRef = ESP_XmlUtils::queryStringValue(xmlBypass, "ref");
    topicValue = ESP_XmlUtils::queryStringValue(xmlBypass, "value");
  }
}

const char* ESP_Postprocessor_UL20::getResultData() { return result.c_str(); }

int ESP_Postprocessor_UL20::getResultSize() { return result.length(); }

bool ESP_Postprocessor_UL20::isResultValid() { return resultValid; }
