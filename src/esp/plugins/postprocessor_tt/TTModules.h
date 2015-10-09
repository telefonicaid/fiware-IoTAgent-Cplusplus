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
#ifndef TTMODULES_H
#define TTMODULES_H

#include <string>
#include <map>
#include "tinyxml.h"
#include "ESP_XmlUtils.h"
#include "TDA.h"

#define TAG_XML_MAPPING "mapping"
#define CB_OUTGC_PREFIX "_TTcurrent_"
#define GPS_MAPPING "dualCompoundMapping"
#define GPS_METADATA "metadataAttribute"

namespace tt {

class Pair {
 public:
  Pair(std::string nameParam) : name(nameParam) { mandatory = true; };

  void setValue(std::string value) { this->value.assign(value); };

  void set_optional() { this->mandatory = false; }

  bool is_mandatory() { return mandatory; }

  virtual ~Pair(){};

  std::string getValue() { return value; };
  std::string getName() { return name; };

 private:
  std::string value;
  std::string name;
  bool mandatory;
};
}

/**
  @name TTModules
  This class and its derivates are the abstraction of the TT Modules that
  postprocessor can handle to produce
  JSON output. Depeding on the module, some of them will be represented by a
  compound type, with specific sub-attributes.
  The mapping of those sub-attributes is configured in the config XML
  (sensortt-open.xml).
    All objects of these classes are created when postprocessor is initiated and
  stored in an internal map.
  Then, at runtime, when a TT message is decoded, the postprocessor will get
  those objects and invoke their "transform" method
  that will produce the JSON accordingly to its module. Calling "toJSON" will
  return the actual JSON in string format. "toPlainJSON"
  is used when publishing for the first time in CB an attribute that doesn't
  require the prefix (case of GC modules)

  Example of what has to be included into the XML config file in order to
  process all supported modules.
  @code
        <ttmodule keyword="GM" ref="module" />
        <ttmodule keyword="GC" ref="module" />


        <ttmodule keyword="P1" ref="module">
          <mapping ref="param_1" name="mcc"/>
          <mapping ref="param_2" name="mnc"/>
          <mapping ref="param_3" name="cell-id"/>
          <mapping ref="param_4" name="lac"/>
          <mapping ref="param_5" name="dbm" optional="true" />
        </ttmodule>
        <ttmodule keyword="B" ref="module">
          <mapping ref="param_1" name="voltaje"/>
          <mapping ref="param_2" name="estado_activacion"/>
          <mapping ref="param_3" name="hay_cargador"/>
          <mapping ref="param_4" name="estado_carga"/>
          <mapping ref="param_5" name="modo_carga" />
          <mapping ref="param_6" name="tiempo_desco_stack" />
        </ttmodule>
        <ttmodule keyword="GPS" ref="module">
          <dualCompoundMapping refA="param_1" refB="param_2" separator=","/>
          <metadataAttribute name="location" type="string" value="WGS84" />
        </ttmodule>
  @endcode
  Each of the "<ttmodule>" tags will represent an object of the corresponding
  class in memory.
*/
class TTModules {
 public:
  bool isValid();

  /**
  @name decodeModule
  @brief It instantiates the appropriated TTModules derivated class depending on
  the keyword (GPS, B, P1, GM, GC)
  @param [IN] keyword: string identifying the module that came from TT Device
  (GPS, P1, B, GM, GC)
  @param [IN] TiXmlElement* : pointer to the XML object that was parsed from
  config XML representing this module.
  @return
      Any of these objects:
      @see TTGC_Modules
      @see TTGM_Modules
      @see TTGPS_Modules
      @see TTP1_BModules
  */
  static TTModules* decodeModule(std::string keyword);

  TTModules(std::string name);

  virtual ~TTModules();

  /**
  @name transform
  @brief this method will create actual JSON on each TT module that is going to
  be published in CB
  @param [IN] CC_AttributesType: the output of ESP parser, a map of objects
  containing parsed TT messages
  */
  virtual void transform(CC_AttributesType* inputAttributes) = 0;

  /**
  @name toJSON
  @brief it returns the JSON in string format.
  @return
      JSON of the attribute in string format, including metadatas.
  */
  virtual std::string toJSON();

  /**
  @name toPlainJSON
  @brief it returns a JSON but without prefix "_TTcurrent_" if it's a GC, rest
  of modules returns the
  same JSON as toJSON.
  */
  virtual std::string toPlainJSON();

  /**
  @name populateInternalMappings
  @brief taking the XML element from the config file, it will parse all possible
  mappings defined in the file, needed to do the transformation.
  @param [IN] TiXmlElement: the object enclosed by the tag <ttmodule> in the XML
  */
  virtual void populateInternalMappings(TiXmlElement* element) = 0;

  /**
   @name populate_default_internals
   @brief this function will populate internal mapping with a default
   configuration, so instead of using mapping from XML file, it will use
   one by default.
   */
  virtual void populate_default_internals(){};

 protected:
  /**
  @name setSleepCond
  @brief it sets the SleepCondition value from the TT message. It's called
  internally by @see transform
  @param string: read value
  */
  void setSleepCond(std::string&);

  /**
  @name setSleepTime
  @brief it sets the SleepTime value obtained from TT message. Called internally
  by @see transform
  @param string: read value.
  */
  void setSleepTime(std::string&);

  /**
  @name doMetadatas
  @brief it adds the metadatas section (that contains sleepcondition and
  sleeptime) to the JSON.
  @return
    JSON with the metadatas field including the "comma". It might be empty if no
  sleepcondition and sleeptime came into the message.
    metadatas section is added with at least one of them.
    @code
      ,"metadatas" :[{...}]
    @endcode
  */
  std::string doMetadatas();

  /**
  @name appendSleepCondTime
  @brief used internally by @see doMetadatas, it has the logic to append
  sleepcond or sleeptime to the metadatas if they were present
  or not.
  @return
    The JSON extract corresponding to SleepCondition and/or sleepTime. It might
  be empty if they didn't come in the TT Message for the module.
    */
  std::string appendSleepCondTime();

  /**
  @name parseSleepCondAndTime
  @brief it will obtain sleepTime and sleepCondition from the TT message, that
  is represented by the ESP parser output: CC_AttributesType
  @param [IN] CC_AttributesType: result of the ESP Parser.
  */
  void parseSleepCondAndTime(CC_AttributesType* attributes);

  std::string
      name; /**< name field of the JSON, depending on the module, it may be
               different to the "module" identifier (for GM and GC) */
  std::string type;
  std::string sleepCondition;
  std::string sleepTime;

  bool bValid; /**< if a mandatory field is missing, the transform operation is
                  not valid */
  std::string json;
};

/**
  @name TTGPSModules
  @brief This class encapsulates the module GPS of the TT protocol.

  Example of configuration in XML
  @code
        <ttmodule keyword="GPS" ref="module">
          <dualCompoundMapping refA="param_1" refB="param_2" separator=","/>
          <metadataAttribute name="location" type="string" value="WGS84" />
        </ttmodule>
  @endcode
*/
class TTGPSModules : public TTModules {
 public:
  TTGPSModules(std::string name);
  virtual ~TTGPSModules(){};

  virtual void transform(CC_AttributesType* inputMap);

  virtual void populateInternalMappings(TiXmlElement* element);
  void populate_default_internals();

 private:
  std::string refCoordX;
  std::string refCoordY;
  std::string separator;
  std::string metaName;
  std::string metaType;
  std::string metaValue;
};

/**
  @name TTP1_BModules
  @brief This class encapsulates P1 and B modules (as they both have a similar
  representation). Obviously, Postprocessor will use two separate
  instances.

  Example of config in the XML file. Note the mapping of the sub-attributes as
  the JSON produced by this class will be a compound type.
  @code
        <ttmodule keyword="P1" ref="module">
          <mapping ref="param_1" name="mcc"/>
          <mapping ref="param_2" name="mnc"/>
          <mapping ref="param_3" name="cell-id"/>
          <mapping ref="param_4" name="lac"/>
          <mapping ref="param_5" name="dbm" optional="true" />
        </ttmodule>
        <ttmodule keyword="B" ref="module">
          <mapping ref="param_1" name="voltaje"/>
          <mapping ref="param_2" name="estado_activacion"/>
          <mapping ref="param_3" name="hay_cargador"/>
          <mapping ref="param_4" name="estado_carga"/>
          <mapping ref="param_5" name="modo_carga" />
          <mapping ref="param_6" name="tiempo_desco_stack" />
        </ttmodule>

  @endcode
*/
class TTP1_BModules : public TTModules {
 public:
  TTP1_BModules(std::string name);
  virtual ~TTP1_BModules();

  virtual void transform(CC_AttributesType* inputMap);
  virtual void populateInternalMappings(TiXmlElement* element);
  void populate_default_internals();

 protected:
 private:
  std::map<std::string, tt::Pair*> mappings;
  void addToMap(std::string paramPos, tt::Pair* pair);
};

/**
 @name TTGM_Modules
 @brief This class encapsulates the GM (Generic Measures) modules of the TT
 protocol.As a side note, the "param_1" field of the
 ESP output will contain the name of the measure, and "param_2" the value. But
 this convention is always fixed and not
 changeable, hence there is no need to have that in the file.

 Example of configuration. There is no specific mapping for this module.
 @code
    <ttmodule keyword="GM" ref="module" />

 @endcode
*/
class TTGM_Modules : public TTModules {
 public:
  TTGM_Modules(std::string name);
  virtual ~TTGM_Modules(){};

  virtual void transform(CC_AttributesType* inputMap);

  virtual void populateInternalMappings(TiXmlElement* element);
};

/**
  @name TTGC_Modules
  @brief This class encapsulates the GC (Generic Configuration) modules. In the
  same way as with GM, "param_1" of the parser's output contains
  the name of the GC, and "param_2" the value.

  Example of configuration
  @code
    <ttmodule keyword="GC" ref="module" />
  @endcode
*/
class TTGC_Modules : public TTModules {
 public:
  TTGC_Modules(std::string name);
  virtual ~TTGC_Modules(){};

  std::string toPlainJSON();

  virtual void transform(CC_AttributesType* inputMap);

  virtual void populateInternalMappings(TiXmlElement* element);

 protected:
  std::string plainJson;
};

#endif  // TTModules_H
