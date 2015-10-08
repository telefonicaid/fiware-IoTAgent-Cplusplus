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
#ifndef ESP_SML_TRANSFORMER_H
#define ESP_SML_TRANSFORMER_H

#include "CC_Logger.h"
#include "tinyxml.h"
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <ESP_TimeUtils.h>

#define XMLNS_SOS "http://www.opengis.net/sos/1.0"
#define XMLNS_GML "http://www.opengis.net/gml"
#define XMLNS_OM "http://www.opengis.net/om/1.0"
#define XMLNS_SML "http://www.opengis.net/sensorML/1.0.1"
#define XMLNS_SWE "http://www.opengis.net/swe/1.0.1"
#define XMLNS_XLINK "http://www.w3.org/1999/xlink"
#define XMLNS_XSI "http://www.w3.org/2001/XMLSchema-instance"

#define XMLNS_PAID_DEFAULT "urn:ogc:def:dictionary:PAID:1.0:paid"
#define XMLNS_PAID_COMMAND \
  "urn:ogc:def:dictionary:PAID:1.0:paid:PaidCommand.xsd"

// Own Keywords used in decoding input ESP postprocessor config XML
#define SML_KEYWORD "keyword"
#define SML_SENSORID "sensorid"
#define SML_PROCEDURE "procedure"
#define SML_SAMPLINGTIME "samplingtime"
#define SML_TIMEPOSITION "timeposition"
#define SML_OBSERVEDPROP "observedproperty"
#define SML_TRANSFORMER "transformer"
#define SML_LOCATION "location"
//#define SML_POSITION "position"
#define SML_ORIENTATION "orientation"
#define SML_RESULT "result"
#define SML_TYPE "type"
#define SML_VALUE "value"
#define SML_REF "ref"
#define SML_MODE "mode"
#define SML_MODE_AUTO "auto"
#define SML_FRAME "frame"
#define SML_UOM "uom"
#define SML_QRANGE "qrange"

#define SML_COMMAND_RESPONSE "commandresponse"
#define SML_REF_NAME "refname"

// Data types supported by SensorML
#define SML_TYPE_TEXT "Text"
#define SML_TYPE_QUANTITY "Quantity"
#define SML_TYPE_BOOLEAN "Boolean"
#define SML_TYPE_DATARECORD "DataRecord"
#define SML_TYPE_COORDINATE "Coordinate"
#define SML_TYPE_POSITION "position"
#define SML_TYPE_FIELD "Field"
#define SML_TYPE_CMD_PARAM "cmdParam"
#define SML_TYPE_CMD_RESULT "resultCommand"
#define SML_TYPE_CMD_ERROR "errorCommand"

// XML tags for SensorML
#define SML_XML_OBSERVATION "om:Observation"
#define SML_XML_INSERTOBSERVATION "sos:InsertObservation"
#define SML_XML_ASSIGNEDSENSORID "sos:AssignedSensorId"
#define SML_XML_SAMPLINGTIME "om:samplingTime"
#define SML_XML_TIMEINSTANT "gml:TimeInstant"
#define SML_XML_TIMEPOSITION "gml:timePosition"
#define SML_XML_PROCEDURE "om:procedure"
#define SML_XML_OBSERVEDPROPERTY "om:observedProperty"
#define SML_XML_FEATUREOFINTEREST "om:featureOfInterest"
#define SML_XML_RESULT "om:result"
#define SML_XML_QUANTITY "swe:Quantity"
#define SML_XML_UOM "swe:uom"
#define SML_XML_VALUE "swe:value"
#define SML_XML_TEXT "swe:Text"
#define SML_XML_BOOLEAN "swe:Boolean"
#define SML_XML_DATARECORD "swe:DataRecord"
#define SML_XML_FIELD "swe:field"
#define SML_XML_NAME "name"
#define SML_XML_XLINK_HREF "xlink:href"
#define SML_XML_UOM "swe:uom"
#define SML_XML_DEFINITION "definition"
#define SML_XML_CODE "code"
#define SML_XML_QRANGE "swe:QuantityRange"
#define SML_XML_POSITION "swe:Position"
#define SML_XML_LOCATION "swe:location"
#define SML_XML_LOCALFRAME "localFrame"
#define SML_XML_REFERENCEFRAME "referenceFrame"
#define SML_XML_COORDINATE "swe:coordinate"
#define SML_XML_VECTOR "swe:Vector"

// tags for commands
#define SML_XML_COMMAND "paid:command"
#define SML_XML_CMD_RESPONSE "paid:commandResponse"
#define SML_XML_CMD_PARAM "paid:cmdParam"
#define SML_XML_CMD_RESULT "paid:resultCommand"
#define SML_XML_CMD_ERROR "paid:errorCommand"

// Keep adding more defines for other SensorML tags...

class ESP_SML_Base;

/**
* This map will contain pointers to objects of ESP_SML_Base and its derived
classes so that, when
the transformation is done (within "execute" method at
Plugin_postprocessor_SensorML) it will
just use this map as there will be one per Postprocessor.
*/
class ESP_SML_Map {
 public:
  ESP_SML_Map();
  ~ESP_SML_Map(){};

  void addToMap(std::string key, ESP_SML_Base* element);

  // This method is used to replace "fixed" or default SensorML value with
  // referenced value
  // from parser
  void transformRefToValue(std::string ref, std::string newValue);

  void addTimestampSML(ESP_SML_Base* obj);
  void computeTimestamp();
  void resetValues();  // Set Values to empty
 private:
  std::multimap<std::string, ESP_SML_Base*> mapTrans;

  ESP_SML_Base* smlObjectWithTimestamp;
  // ESP_SML_Base* smlObjectID;
  ESP_TimeUtils timeStamp;
};

class ESP_SML_Base {
 public:
  static ESP_SML_Base* translateObject(TiXmlElement* element, std::string type,
                                       ESP_SML_Map* mapTrans);

  ESP_SML_Base();
  ESP_SML_Base(TiXmlElement* element, ESP_SML_Map* mapT);
  // ESP_SML_Transformer(ESP_SML_Transformer* const&);
  virtual ~ESP_SML_Base();

  virtual void setValue(std::string value){};

  // It can be (and it will) overwritten if needed
  virtual TiXmlElement* getTransformedXML();

 protected:
  ESP_SML_Map* mapTrans;

  void addToMap(std::string key, ESP_SML_Base* element);

  void createXml(const char* xmlTag);

  TiXmlElement* xmlConverted;
  std::string strValue;

  bool isValueDefined();

  void setDefinition(TiXmlElement* element);
  void setName(TiXmlElement* element);
  void setXlink(TiXmlElement* element);

 private:
  static int test;
};

class ESP_SML_List : public ESP_SML_Base {
 public:
  ESP_SML_List(TiXmlElement* elemWithChildren, ESP_SML_Map* mapT);
  ESP_SML_List(TiXmlElement* elemWithChildren, ESP_SML_Map* mapT,
               const char* xmlType);
  ~ESP_SML_List();

  void addSMLElement(ESP_SML_Base* element);

  ESP_SML_Base* nextSMLElement();

  TiXmlElement* getTransformedXML();

  int getNumOfSML() { return vTransformers.size(); };

 protected:
  virtual void parseElement(TiXmlElement* inputElement);

 private:
  std::vector<ESP_SML_Base*> vTransformers;
  unsigned int nPos;
};

class ESP_SML_Keyword {
 public:
  ESP_SML_Keyword();
  void setKeyword(std::string key);
  std::string getKeyword();
  bool canIPrint();
  void testKey(std::string key);
  void setFound(bool found);

  // Needed after using postprocessor output
  void resetFound();

 private:
  bool bFound;
  std::string keyword;
};

class ESP_SML_Unit {
 public:
  ESP_SML_Unit();
  ~ESP_SML_Unit();

  virtual void setValue(std::string value);

  TiXmlText* getText();

 protected:
  TiXmlText* xmlText;
};

class ESP_SML_Quantity : public ESP_SML_Base, public ESP_SML_Unit {
 public:
  ESP_SML_Quantity(TiXmlElement* inputElement, ESP_SML_Map* mapT);
  ~ESP_SML_Quantity();

  // using ESP_SML_Unit::setValue;
  void setValue(std::string value);

 private:
  std::string uom;
  std::string qrange;

  TiXmlElement* xmlValue;
};

class ESP_SML_Coordinate : public ESP_SML_Quantity {
 public:
  ESP_SML_Coordinate(TiXmlElement* inputElement, ESP_SML_Map* mapT);

  ~ESP_SML_Coordinate(){};

  TiXmlElement* getTransformedXML();

 private:
  TiXmlElement* xmlCoordinate;
};

class ESP_SML_Boolean : public ESP_SML_Base, public ESP_SML_Unit {
 public:
  ESP_SML_Boolean(TiXmlElement* inputElement, ESP_SML_Map* mapT);

  ~ESP_SML_Boolean(){};

  // using ESP_SML_Unit::setValue;
  void setValue(std::string value);

 private:
  TiXmlElement* xmlValue;
};

class ESP_SML_Text : public ESP_SML_Base, public ESP_SML_Unit {
 public:
  ESP_SML_Text(TiXmlElement* inputElement, ESP_SML_Map* mapT);
  ~ESP_SML_Text(){};

  // using ESP_SML_Unit::setValue;
  void setValue(std::string value);

 private:
  TiXmlElement* xmlValue;
};

class ESP_SML_DataRecord : public ESP_SML_List {
 public:
  ESP_SML_DataRecord(TiXmlElement* inputElement, ESP_SML_Map* mapT);
  ~ESP_SML_DataRecord(){};

  using ESP_SML_List::getTransformedXML;

 private:
};

class ESP_SML_Field : public ESP_SML_List {
 public:
  ESP_SML_Field(TiXmlElement* element, ESP_SML_Map* mapT);
  ~ESP_SML_Field(){};

  using ESP_SML_List::getTransformedXML;
};

class ESP_SML_SamplingTime : public ESP_SML_Base, public ESP_SML_Unit {
 public:
  ESP_SML_SamplingTime(TiXmlElement* inputElement, ESP_SML_Map* mapT);

  ~ESP_SML_SamplingTime(){};

  // using ESP_SML_Unit::setValue;
  void setValue(std::string value);

 private:
  std::string mode;
  std::string frame;
};

class ESP_SML_Procedure : public ESP_SML_Base, public ESP_SML_Unit {
 public:
  ESP_SML_Procedure(TiXmlElement* element, ESP_SML_Map* mapT);

  ~ESP_SML_Procedure(){};

  // using ESP_SML_Unit::setValue;
  void setValue(std::string value);
};

class ESP_SML_Observation : public ESP_SML_List {
 public:
  ESP_SML_Observation(TiXmlElement* elemWithChildren, ESP_SML_Map* mapT);

  ~ESP_SML_Observation(){};

 private:
  void parseElement(TiXmlElement* element);
};

class ESP_SML_ObservedProperty : public ESP_SML_Base {
 public:
  ESP_SML_ObservedProperty(TiXmlElement* inputElement, ESP_SML_Map* mapT);
  ~ESP_SML_ObservedProperty(){};

  TiXmlElement* getTransformedXML() { return xmlConverted; };
};

class ESP_SML_SensorID : public ESP_SML_Base, public ESP_SML_Unit {
 public:
  ESP_SML_SensorID(TiXmlElement* inputElement, ESP_SML_Map* mapT);

  ~ESP_SML_SensorID(){};

  // using ESP_SML_Unit::setValue;
  void setValue(std::string value);
};

class ESP_SML_InsertObservation : public ESP_SML_List {
 public:
  ESP_SML_InsertObservation(TiXmlElement* elemWithChildren, ESP_SML_Map* mapT);
  ~ESP_SML_InsertObservation();

  using ESP_SML_List::getTransformedXML;

 private:
  void parseElement(TiXmlElement* element);
};

class ESP_SML_Result : public ESP_SML_List {
 public:
  ESP_SML_Result(TiXmlElement* elemWithChildren, ESP_SML_Map* mapT);
  ~ESP_SML_Result(){};

  using ESP_SML_List::getTransformedXML;
};

class ESP_SML_Vector : public ESP_SML_List {
 public:
  ESP_SML_Vector(TiXmlElement* elem, ESP_SML_Map* map);
  ~ESP_SML_Vector(){};

  using ESP_SML_List::getTransformedXML;
};

class ESP_SML_Location : public ESP_SML_Base {
 public:
  ESP_SML_Location(TiXmlElement* inputElem, ESP_SML_Map* map);
  ~ESP_SML_Location();

  TiXmlElement* getTransformedXML();

 private:
  ESP_SML_Vector* smlVector;
};

class ESP_SML_Position : public ESP_SML_List {
 public:
  ESP_SML_Position(TiXmlElement* inputElem, ESP_SML_Map* map);
  ~ESP_SML_Position(){};

 private:
  void parseElement(TiXmlElement* elem);
};

class ESP_SML_CmdName : public ESP_SML_Base {
 public:
  ESP_SML_CmdName(TiXmlElement* commandResponseXml, std::string refName,
                  ESP_SML_Map* mapT);
  ~ESP_SML_CmdName(){};  // TinyXML objects are destroyed when Tiny XML Document
                         // is freed up.
  void setValue(std::string value);

 private:
  TiXmlElement* commandResponseXml;
};

class ESP_SML_CommandResponse : public ESP_SML_List {
 public:
  ESP_SML_CommandResponse(TiXmlElement* inputElem, ESP_SML_Map* map);
  ~ESP_SML_CommandResponse();

  void setValue(std::string value);

 private:
  ESP_SML_CmdName* cmdName;
};

/*
This class will mode cmdParam, errorCommand and resultCommand type of responses.
*/
class ESP_SML_CmdParam : public ESP_SML_List {
 public:
  ESP_SML_CmdParam(std::string type, TiXmlElement* inputElem,
                   ESP_SML_Map* mapT);
  ~ESP_SML_CmdParam(){};
};

#endif  // ESP_SML_TRANSFORMER_H
