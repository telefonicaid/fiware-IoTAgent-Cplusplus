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
#include "ESP_SML_Classes.h"
#include "ESP_XmlUtils.h"

ESP_SML_Map::ESP_SML_Map() { smlObjectWithTimestamp = NULL; }

void ESP_SML_Map::transformRefToValue(std::string ref, std::string value) {
  // Working with the multimap

  std::pair<std::multimap<std::string, ESP_SML_Base*>::iterator,
            std::multimap<std::string, ESP_SML_Base*>::iterator> it;
  it = mapTrans.equal_range(ref);
  std::multimap<std::string, ESP_SML_Base*>::iterator it2;

  for (it2 = it.first; it2 != it.second; ++it2) {
    CC_Logger::getSingleton()->logDebug("Replacing: %s", (*it2).first.c_str());
    (*it2).second->setValue(value);
  }
}

void ESP_SML_Map::resetValues() {
  for (std::multimap<std::string, ESP_SML_Base*>::iterator it =
           mapTrans.begin();
       it != mapTrans.end(); it++) {
    it->second->setValue("");
  }
}

void ESP_SML_Map::computeTimestamp() {
  if (smlObjectWithTimestamp != NULL) {
    int year, month, day, h, m, s, ms;
    timeStamp.getCurrentUTCTimestamp(&year, &month, &day, &h, &m, &s, &ms);

    std::string strValue =
        timeStamp.formatUTCTimestamp(year, month, day, h, m, s);

    smlObjectWithTimestamp->setValue(strValue);
  }
}

void ESP_SML_Map::addToMap(std::string key, ESP_SML_Base* element) {
  mapTrans.insert(std::pair<std::string, ESP_SML_Base*>(key, element));
}

void ESP_SML_Map::addTimestampSML(ESP_SML_Base* obj) {
  smlObjectWithTimestamp = obj;
}

/**
* This method is the one which must be changed in order to add more dataTypes to
SensorML
output as it will translate XML elements into rigth ESP_SML_Transformer derived
classs.
*/
ESP_SML_Base* ESP_SML_Base::translateObject(TiXmlElement* element,
                                            std::string type,
                                            ESP_SML_Map* mapT) {
  if (type.compare(SML_TYPE_QUANTITY) == 0) {
    return new ESP_SML_Quantity(element, mapT);
  }

  if (type.compare(SML_TYPE_BOOLEAN) == 0) {
    return new ESP_SML_Boolean(element, mapT);
  }

  if (type.compare(SML_TYPE_TEXT) == 0) {
    return new ESP_SML_Text(element, mapT);
  }

  if (type.compare(SML_TYPE_DATARECORD) == 0) {
    return new ESP_SML_DataRecord(element, mapT);
  }

  if (type.compare(SML_TYPE_COORDINATE) == 0) {
    return new ESP_SML_Coordinate(element, mapT);
  }

  if (type.compare(SML_TYPE_POSITION) == 0) {
    return new ESP_SML_Position(element, mapT);
  }

  if (type.compare(SML_TYPE_FIELD) == 0) {
    return new ESP_SML_Field(element, mapT);
  }

  if (type.compare(SML_TYPE_CMD_PARAM) == 0) {
    // return new ESP_SML_CommandParam(element,mapT);
    return new ESP_SML_CmdParam(SML_XML_CMD_PARAM, element, mapT);
  }

  if (type.compare(SML_TYPE_CMD_RESULT) == 0) {
    // return new ESP_SML_ResultCommand(element,mapT);
    return new ESP_SML_CmdParam(SML_XML_CMD_RESULT, element, mapT);
  }

  if (type.compare(SML_TYPE_CMD_ERROR) == 0) {
    // return new ESP_SML_ErrorCommand(element,mapT);
    return new ESP_SML_CmdParam(SML_XML_CMD_ERROR, element, mapT);
  }

  return NULL;
}

ESP_SML_Base::ESP_SML_Base(TiXmlElement* element, ESP_SML_Map* mapT) {
  xmlConverted = NULL;
  mapTrans = mapT;
  if (element) {
    std::string refer = ESP_XmlUtils::queryStringValue(element, SML_REF);
    if (refer.length() > 0) {
      addToMap(refer, this);
    } else {
      // Refer takes priority over value, so in case of both, refer will always
      // be processed.
      strValue = ESP_XmlUtils::queryStringValue(element, SML_VALUE);
    }
  }
}

bool ESP_SML_Base::isValueDefined() {
  return strValue.length() > 0 ? true : false;
}

void ESP_SML_Base::createXml(const char* strTag) {
  if (strTag) {
    xmlConverted = new TiXmlElement(strTag);
  }
}

ESP_SML_Base::ESP_SML_Base() {
  // empty constructor
}

ESP_SML_Base::~ESP_SML_Base() {
  // dtor
}

void ESP_SML_Base::setDefinition(TiXmlElement* element) {
  std::string definition =
      ESP_XmlUtils::queryStringValue(element, SML_XML_DEFINITION);
  if (definition.length() > 0) {
    xmlConverted->SetAttribute(SML_XML_DEFINITION, definition.c_str());
  }
}

TiXmlElement* ESP_SML_Base::getTransformedXML() { return xmlConverted; }

void ESP_SML_Base::setName(TiXmlElement* element) {
  std::string name = ESP_XmlUtils::queryStringValue(element, SML_XML_NAME);
  if (name.length() > 0) {
    xmlConverted->SetAttribute(SML_XML_NAME, name.c_str());
  }
}

void ESP_SML_Base::setXlink(TiXmlElement* element) {
  std::string xlkink =
      ESP_XmlUtils::queryStringValue(element, SML_XML_XLINK_HREF);
  if (xlkink.length() > 0) {
    xmlConverted->SetAttribute(SML_XML_XLINK_HREF, xlkink.c_str());
  }
}

ESP_SML_List::ESP_SML_List(TiXmlElement* elemWithChildren, ESP_SML_Map* mapT)
    : ESP_SML_Base(elemWithChildren, mapT) {
  nPos = 0;
}

ESP_SML_List::ESP_SML_List(TiXmlElement* elemWithChildren, ESP_SML_Map* mapT,
                           const char* xmlType)
    : ESP_SML_Base(elemWithChildren, mapT) {
  nPos = 0;
  createXml(xmlType);
  parseElement(elemWithChildren);
}

void ESP_SML_List::addSMLElement(ESP_SML_Base* objTrans) {
  if (objTrans) {
    vTransformers.push_back(objTrans);
  }
}

ESP_SML_Base* ESP_SML_List::nextSMLElement() {
  if (nPos < vTransformers.size()) {
    return vTransformers[nPos++];
  } else {
    nPos = 0;  // Resetting index
  }

  return NULL;
}

ESP_SML_List::~ESP_SML_List() {
  for (nPos = 0; nPos < vTransformers.size(); nPos++) {
    delete (vTransformers[nPos]);
  }
}

TiXmlElement* ESP_SML_List::getTransformedXML() {
  for (ESP_SML_Base* b = nextSMLElement(); b != NULL; b = nextSMLElement()) {
    xmlConverted->LinkEndChild(b->getTransformedXML());
  }
  return xmlConverted;
}

void ESP_SML_Base::addToMap(std::string key, ESP_SML_Base* element) {
  // this->mapTrans.insert(std::pair<std::string,ESP_SML_Base* >(key,element));
  mapTrans->addToMap(key, element);
}

void ESP_SML_List::parseElement(TiXmlElement* element) {
  if (element == NULL) {
    return;
  }

  // Parsing transformer objects
  for (TiXmlElement* e = element->FirstChildElement(SML_TRANSFORMER); e != NULL;
       e = e->NextSiblingElement(SML_TRANSFORMER)) {
    std::string typeofObj = ESP_XmlUtils::queryStringValue(e, SML_TYPE);
    // CC_Logger::getSingleton()->logDebug("Type: %s",typeofObj.c_str());

    ESP_SML_Base* tfObj = ESP_SML_Base::translateObject(e, typeofObj, mapTrans);

    addSMLElement(tfObj);
  }
}

ESP_SML_Keyword::ESP_SML_Keyword() { bFound = false; }

bool ESP_SML_Keyword::canIPrint() { return bFound; }

void ESP_SML_Keyword::setKeyword(std::string key) { keyword = key; }

std::string ESP_SML_Keyword::getKeyword() { return keyword; }

void ESP_SML_Keyword::setFound(bool found) { bFound = found; }

void ESP_SML_Keyword::testKey(std::string key) {
  if (key.compare(keyword) == 0 && keyword.length() > 0) {
    bFound = true;
  }
}

// Needed after using postprocessor output
void ESP_SML_Keyword::resetFound() { bFound = false; }

ESP_SML_Unit::ESP_SML_Unit() { xmlText = new TiXmlText(""); }

ESP_SML_Unit::~ESP_SML_Unit() {}

void ESP_SML_Unit::setValue(std::string value) {
  xmlText->SetValue(value.c_str());
}

TiXmlText* ESP_SML_Unit::getText() { return xmlText; }

ESP_SML_Quantity::~ESP_SML_Quantity() {}

void ESP_SML_Quantity::setValue(std::string value) {
  ESP_SML_Unit::setValue(value);
}

TiXmlElement* ESP_SML_Coordinate::getTransformedXML() {
  if (xmlConverted) {
    xmlCoordinate->LinkEndChild(xmlConverted);
  }
  return xmlCoordinate;
}

// using ESP_SML_Unit::setValue;
void ESP_SML_Boolean::setValue(std::string value) {
  ESP_SML_Unit::setValue(value);
}

// using ESP_SML_Unit::setValue;
void ESP_SML_Text::setValue(std::string value) {
  ESP_SML_Unit::setValue(value);
}

ESP_SML_DataRecord::ESP_SML_DataRecord(TiXmlElement* inputElement,
                                       ESP_SML_Map* mapT)
    : ESP_SML_List(inputElement, mapT, SML_XML_DATARECORD) {}

ESP_SML_Field::ESP_SML_Field(TiXmlElement* element, ESP_SML_Map* mapT)
    : ESP_SML_List(element, mapT, SML_XML_FIELD) {
  setName(element);
  setXlink(element);
}

ESP_SML_SamplingTime::ESP_SML_SamplingTime(TiXmlElement* inputElem,
                                           ESP_SML_Map* mapT)
    : ESP_SML_Base(inputElem, mapT) {
  createXml(SML_XML_SAMPLINGTIME);

  if (inputElem == NULL) {
    return;
  }

  mode = ESP_XmlUtils::queryStringValue(inputElem, SML_MODE);
  frame = ESP_XmlUtils::queryStringValue(inputElem, SML_FRAME);
  // for future improvements: implement format for timestamp retrieved as value
  // if (!isValueDefined() && mode.compare(SML_MODE_AUTO)==0)
  // {

  // }

  TiXmlElement* timeInst;
  TiXmlElement* timePos;

  timePos = new TiXmlElement(SML_XML_TIMEPOSITION);
  timePos->SetAttribute(SML_FRAME, frame.c_str());

  timePos->LinkEndChild(xmlText);

  timeInst = new TiXmlElement(SML_XML_TIMEINSTANT);
  timeInst->SetAttribute(SML_FRAME, frame.c_str());
  timeInst->LinkEndChild(timePos);

  xmlConverted->LinkEndChild(timeInst);

  if (mode.compare(SML_MODE_AUTO) == 0) {
    // This is needed for creating timestamp on every iteration
    mapT->addTimestampSML(this);
  } else {
    xmlText->SetValue(strValue.c_str());
  }
}

// using ESP_SML_Unit::setValue;
void ESP_SML_SamplingTime::setValue(std::string value) {
  ESP_SML_Unit::setValue(value);
}

ESP_SML_Procedure::ESP_SML_Procedure(TiXmlElement* elem, ESP_SML_Map* mapT)
    : ESP_SML_Base(elem, mapT) {
  createXml(SML_XML_PROCEDURE);
}

void ESP_SML_Procedure::setValue(std::string value) {
  xmlConverted->SetAttribute(SML_XML_XLINK_HREF, value.c_str());
}

ESP_SML_SensorID::ESP_SML_SensorID(TiXmlElement* elem, ESP_SML_Map* mapT)
    : ESP_SML_Base(elem, mapT) {
  createXml(SML_XML_ASSIGNEDSENSORID);

  xmlConverted->LinkEndChild(xmlText);
}

// using ESP_SML_Unit::setValue;
void ESP_SML_SensorID::setValue(std::string value) {
  ESP_SML_Unit::setValue(value);
}

ESP_SML_InsertObservation::ESP_SML_InsertObservation(
    TiXmlElement* elemWithChildren, ESP_SML_Map* mapT)
    : ESP_SML_List(elemWithChildren, mapT) {
  createXml(SML_XML_INSERTOBSERVATION);
  xmlConverted->SetAttribute("service", "SOS");
  xmlConverted->SetAttribute("version", "1.0.0");

  // Namespaces:
  xmlConverted->SetAttribute("xmlns:sos", XMLNS_SOS);
  xmlConverted->SetAttribute("xmlns:gml", XMLNS_GML);
  xmlConverted->SetAttribute("xmlns:om", XMLNS_OM);

  xmlConverted->SetAttribute("xmlns:paid", XMLNS_PAID_DEFAULT);
  xmlConverted->SetAttribute("xmlns:sml", XMLNS_SML);
  xmlConverted->SetAttribute("xmlns:swe", XMLNS_SWE);
  xmlConverted->SetAttribute("xmlns:xsi", XMLNS_XSI);
  xmlConverted->SetAttribute("xmlns:xlink", XMLNS_XLINK);

  parseElement(elemWithChildren);
}

ESP_SML_InsertObservation::~ESP_SML_InsertObservation() {}

/**
* Specific logic for creating SensorML object.
*/
void ESP_SML_InsertObservation::parseElement(TiXmlElement* element) {
  if (element == NULL) {
    return;
  }

  ESP_SML_SensorID* smlSensor =
      new ESP_SML_SensorID(element->FirstChildElement(SML_SENSORID), mapTrans);
  addSMLElement(smlSensor);

  ESP_SML_Observation* smlObs = new ESP_SML_Observation(element, mapTrans);

  // XML elements needed to create om:Observation element within SensorML are at
  // the
  // same level in input config xml.

  addSMLElement(smlObs);
}

ESP_SML_Result::ESP_SML_Result(TiXmlElement* elemWithChildren,
                               ESP_SML_Map* mapT)
    : ESP_SML_List(elemWithChildren, mapT, SML_XML_RESULT) {}

ESP_SML_Observation::ESP_SML_Observation(TiXmlElement* elem, ESP_SML_Map* mapT)
    : ESP_SML_List(elem, mapT) {
  createXml(SML_XML_OBSERVATION);
  TiXmlElement* tiFeature = new TiXmlElement(SML_XML_FEATUREOFINTEREST);
  xmlConverted->LinkEndChild(tiFeature);  // quick fix
  parseElement(elem);
}

void ESP_SML_Observation::parseElement(TiXmlElement* element) {
  if (element == NULL) {
    return;
  }

  /**
  *    As per SensorML specs:
  Observation
  samplingTime
  TimeInstant
  TimePosition

  procedure
  observerdProperty
  featureOfInterest
  result
  Quantity
  OR
  Text
  OR
  ...

  */

  ESP_SML_SamplingTime* smlSamplingTime = new ESP_SML_SamplingTime(
      element->FirstChildElement(SML_SAMPLINGTIME), mapTrans);

  addSMLElement(smlSamplingTime);

  ESP_SML_Procedure* smlProcedure = new ESP_SML_Procedure(
      element->FirstChildElement(SML_PROCEDURE), mapTrans);

  addSMLElement(smlProcedure);

  ESP_SML_ObservedProperty* smlObservedProp = new ESP_SML_ObservedProperty(
      element->FirstChildElement(SML_OBSERVEDPROP), mapTrans);
  addSMLElement(smlObservedProp);

  ESP_SML_Result* smlResult =
      new ESP_SML_Result(element->FirstChildElement(SML_RESULT), mapTrans);

  addSMLElement(smlResult);
}

ESP_SML_Quantity::ESP_SML_Quantity(TiXmlElement* inputElement,
                                   ESP_SML_Map* mapT)
    : ESP_SML_Base(inputElement, mapT) {
  // Example: <transformer type="Quantity" ref="valor" definition="" uom="unit"
  // qrange="0 99999" />

  createXml(SML_XML_QUANTITY);

  uom = ESP_XmlUtils::queryStringValue(inputElement, SML_UOM);

  qrange = ESP_XmlUtils::queryStringValue(inputElement, SML_QRANGE);

  xmlValue = new TiXmlElement(SML_XML_VALUE);

  TiXmlElement* xmlUnit = new TiXmlElement(SML_XML_UOM);
  xmlUnit->SetAttribute(SML_XML_CODE, uom.c_str());

  setDefinition(inputElement);
  xmlConverted->LinkEndChild(xmlValue);
  xmlConverted->LinkEndChild(xmlUnit);

  xmlValue->LinkEndChild(xmlText);
}

ESP_SML_Boolean::ESP_SML_Boolean(TiXmlElement* inputElement, ESP_SML_Map* mapT)
    : ESP_SML_Base(inputElement, mapT) {
  createXml(SML_XML_BOOLEAN);
  xmlValue = new TiXmlElement(SML_XML_VALUE);
  xmlConverted->LinkEndChild(xmlValue);
  xmlValue->LinkEndChild(xmlText);
}

ESP_SML_Text::ESP_SML_Text(TiXmlElement* inputElem, ESP_SML_Map* mapT)
    : ESP_SML_Base(inputElem, mapT) {
  createXml(SML_XML_TEXT);

  xmlValue = new TiXmlElement(SML_XML_VALUE);
  setDefinition(inputElem);

  xmlConverted->LinkEndChild(xmlValue);
  xmlValue->LinkEndChild(xmlText);
}

ESP_SML_ObservedProperty::ESP_SML_ObservedProperty(TiXmlElement* inputElement,
                                                   ESP_SML_Map* mapT)
    : ESP_SML_Base(inputElement, mapT) {
  createXml(SML_XML_OBSERVEDPROPERTY);
  if (isValueDefined()) {
    xmlConverted->SetAttribute(SML_XML_XLINK_HREF, strValue.c_str());
  }
}

ESP_SML_Vector::ESP_SML_Vector(TiXmlElement* elem, ESP_SML_Map* map)
    : ESP_SML_List(elem, map, SML_XML_VECTOR) {
  setDefinition(elem);
}

ESP_SML_Coordinate::ESP_SML_Coordinate(TiXmlElement* elem, ESP_SML_Map* map)
    : ESP_SML_Quantity(elem, map) {
  xmlCoordinate = new TiXmlElement(SML_XML_COORDINATE);

  std::string name = ESP_XmlUtils::queryStringValue(elem, SML_XML_NAME);
  if (name.length() > 0) {
    xmlCoordinate->SetAttribute(SML_XML_NAME, name.c_str());
  }
}
ESP_SML_Position::ESP_SML_Position(TiXmlElement* inputElem, ESP_SML_Map* map)
    : ESP_SML_List(inputElem, map) {
  createXml(SML_XML_POSITION);
  std::string localFrame, referenceFrame;
  localFrame = ESP_XmlUtils::queryStringValue(inputElem, SML_XML_LOCALFRAME);
  referenceFrame =
      ESP_XmlUtils::queryStringValue(inputElem, SML_XML_REFERENCEFRAME);

  if (localFrame.length() > 0) {
    xmlConverted->SetAttribute(SML_XML_LOCALFRAME, localFrame.c_str());
  }
  if (referenceFrame.length() > 0) {
    xmlConverted->SetAttribute(SML_XML_REFERENCEFRAME, referenceFrame.c_str());
  }

  parseElement(inputElem);
}

void ESP_SML_Position::parseElement(TiXmlElement* inputElem) {
  TiXmlElement* xmlLocation = inputElem->FirstChildElement(SML_LOCATION);

  ESP_SML_Location* smlLocation = new ESP_SML_Location(xmlLocation, mapTrans);

  addSMLElement(smlLocation);

  // TiXmlElement* xmlOrientation =
  // inputElem->FirstChildElement(SML_ORIENTATION);
}

ESP_SML_Location::ESP_SML_Location(TiXmlElement* inputElem, ESP_SML_Map* map)
    : ESP_SML_Base(inputElem, map) {
  createXml(SML_XML_LOCATION);
  smlVector = new ESP_SML_Vector(inputElem, map);
}

ESP_SML_Location::~ESP_SML_Location() { delete smlVector; }

TiXmlElement* ESP_SML_Location::getTransformedXML() {
  xmlConverted->LinkEndChild(smlVector->getTransformedXML());
  return xmlConverted;
}

ESP_SML_CommandResponse::ESP_SML_CommandResponse(TiXmlElement* inputElem,
                                                 ESP_SML_Map* mapT)
    : ESP_SML_List(inputElem, mapT, SML_XML_CMD_RESPONSE) {
  xmlConverted->SetAttribute("xsi:schemaLocation", XMLNS_PAID_COMMAND);
  xmlConverted->SetAttribute("xmlns:paid", XMLNS_PAID_DEFAULT);
  xmlConverted->SetAttribute("xmlns:sml", XMLNS_SML);
  xmlConverted->SetAttribute("xmlns:swe", XMLNS_SWE);
  xmlConverted->SetAttribute("xmlns:xsi", XMLNS_XSI);
  // setName(inputElem);

  std::string refName = ESP_XmlUtils::queryStringValue(inputElem, SML_REF_NAME);
  if (refName.length() > 0) {
    cmdName = new ESP_SML_CmdName(xmlConverted, refName, mapT);
  } else {
    cmdName = NULL;
  }
}

void ESP_SML_CommandResponse::setValue(std::string value) {
  xmlConverted->SetAttribute("id", value.c_str());
}

ESP_SML_CommandResponse::~ESP_SML_CommandResponse() {
  if (NULL != cmdName) {
    delete cmdName;
  }
}

ESP_SML_CmdParam::ESP_SML_CmdParam(std::string type, TiXmlElement* inputElem,
                                   ESP_SML_Map* mapT)
    : ESP_SML_List(inputElem, mapT, type.c_str()) {
  // createXml(type.c_str());

  setName(inputElem);

  // parseElement(inputElem);
}

/**
This class needs a reference to XmlConverted so that it can add the attribute
when setValue is invoked
*/
ESP_SML_CmdName::ESP_SML_CmdName(TiXmlElement* commandResponseParent,
                                 std::string refName, ESP_SML_Map* mapTrans) {
  this->mapTrans = mapTrans;
  addToMap(refName, this);
  commandResponseXml = commandResponseParent;
}

void ESP_SML_CmdName::setValue(std::string value) {
  commandResponseXml->SetAttribute(SML_XML_NAME, value.c_str());
}
