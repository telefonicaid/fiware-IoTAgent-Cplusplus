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
#ifndef SRC_THINKINGTHINGS_SEARCHRESPONSE_H_
#define SRC_THINKINGTHINGS_SEARCHRESPONSE_H_
#include <string>

#include "thinkingthings/QueryContextWrapper.h"
#include "util/iota_logger.h"
#include "TDA.h"

#include "ngsi/Attribute.h"
#include "ngsi/ContextElement.h"
#include "ngsi/UpdateContext.h"
#include "ngsi/ContextResponses.h"
#include "ngsi/ContextResponse.h"
#include "ngsi/RegisterContext.h"
#include "ngsi/QueryContext.h"
#include "ngsi/Entity.h"
#include "TDA.h"

#define COMPOUND "compound"
#define SLEEPTIME "sleeptime"
#define SLEEPCONDITION "sleepcondition"

#define TT_B_STATUS_CHARGE_ACTIVATION "estado_activacion_carga"
#define TT_B_CHARGING_MODE "modo_carga"
#define TT_B_DISCONN_STACK_TIME "tiempo_desconexion_stack"

namespace iota {
namespace esp {
namespace tt {

/**
  @name TTResponse
  @brief This class and its derivates are used to create the response to be sent
  to TT Devices. Each module will have a different way to create that
  as some are compound attributes while others aren't. GPS module is another
  special case as CB uses a specific attribute "position"
*/
class TTResponse {
 public:
  std::string mainResponse; /**< string that will contain the response to be
                               sent to TT Device */

  TTResponse(std::string module);
  virtual ~TTResponse(){};

  /**
  @name toTTString
  @brief It will return the final response to be sent to the TT Device. It must
  be called ONLY AFTER calling @see processContextElement.
  @return
    The string (mainResponse).
  */
  virtual std::string toTTString();

  /**
    @name decodeStringToTTResponse
    @brief This method will decode the input string and instantiate one of the
    TTResponse extenders according to the type of TT module
    @param string: the identifier of the module (GPS, P1, B, GM, GC, K)
    @return
       An instance of the corresponding object to the input module.
      @see TTCloseResponse:
      @see BResponse:
      @see GPSResponse:
      @see GMResponse:
      @see GCResponse:
  */
  static TTResponse* decodeStringToTTResponse(std::string);

  /**
  @name processContextElement
  This method will compare the result of the queryContext (contextElement
  object) with the output of ESP processing pipeline (CC_AttributesType) that
  represent
  the modules received from the TT Device. It will internally call @see
  parseResponse or @see parseDefault accordingly. After this method has
  finished, @see toTTString method
  will return the final response to be sent to the device.
  @param [IN] ::iota::ContextElement : this object is the result of the
  QueryContext to the Context Broker.
  @param [IN] CC_AttributesType : this object is the output of ESP (which
  represents what comes from the TT Device).

  */
  virtual void processContextElement(::iota::ContextElement& contextElement,
                                     CC_AttributesType& attribute);

  /**
  @name setBusId
  @brief it just sets the value of the bus
  @param [IN] string: the bus number as obtained from the TT message
  */
  void setBusId(std::string bus);

 protected:
  bool ignore_module;
  std::string module;
  std::string busid;
  std::string nameTTAttribute; /**< this is the name used for comparing the
            result from the Context Broker with the output of ESP parser,
            some modules will have just "P1" or "B" but others will depend on
            the name of the GM or GC */
  std::string sleepCondition;
  std::string sleepTime;
  pion::logger m_logger;

  /**
    @name parseMetadatas
    @brief This method is called to process the metadatas of the queryContext
    response that must be included in the response. Metadatas
    contain SleepCondition and SleepTime that must be returned to the device.
    @param [IN] ::iota::Attribute : the attribute parsed from the queryContext
    response
    @return
      the values are represented in this way:
      <sleep_time>$<sleep_cond>
  */
  std::string parseMetadatas(::iota::Attribute& attrb);

  /**
     @name parseResponse

       parseResponse will transform the Attribute coming from ContextBroker into
     a string with actual values to be returned
       to the device and depending on the type of module (and therefore the
     class that extends this one) will look like this:
       "name,value,etc..." or similar.
       There are three calls whithin this one:
        - @see parseModule
        - @see parseCompoundSimpleValue
        - @see parseMetadatas.

     @param [IN] ::iota::Attribute : the attribute from Context Broker.

   */
  virtual void parseResponse(::iota::Attribute& foundAttribute);

  /**
      @name parseDefault
      @brief parseDefault is used when no Attribute is found on CB, but as
     device still needs an answer this method will
      produce the default response.
      It's the alternative call to @see parseResponse.
  */
  virtual void parseDefault();

  /**
    @name parseModule
    @brief Called by parseRespone, this method will produce the beginning of the
    TT response string, that includes the module and bus-id.
    @param [IN] ::iota::Attribute : attribute from Context Broker response.
    @return
      Typically it will produce somethin like this:
      @code
        #0,P1,
      @endcode
      Where 0 is the BUS-id and P1 is the module in the example.
  */
  virtual std::string parseModule(::iota::Attribute& attribute);

  /**
    @name populateInternalFields
    @brief This method will obtain some fields needed to create the response
  from ESP output. These fields are typically the name of the
  module, sleepCondition and sleepTime and depending on the module, the values
  needed for @see parseDefault when it applies.
    @param [IN] ::iota::Attribute : attribute from Context Broker response.
  */
  virtual void populateInternalFields(CC_AttributesType& attribute);

  /**
    @name parseCompoundSimpleValue
    @brief This method will extract the value from the ContextBroker response,
  taking care of simple or compound values depending on the
  Module.
    @param [IN] ::iota::Attribute : attribute from Context Broker response.
  */
  virtual std::string parseCompoundSimpleValue(::iota::Attribute& attrib) = 0;

 private:
};

class TTCloseResponse : public TTResponse {
 public:
  TTCloseResponse(std::string module);
  virtual ~TTCloseResponse(){};
  /**
  @name processContextElement
  @brief due to a new requierement, this method will always do the @see
  parseDefault

  */
  void processContextElement(::iota::ContextElement& contextElement,
                             CC_AttributesType& attribute);

 protected:
  virtual void populateInternalFields(CC_AttributesType& attribute);

  virtual std::string parseCompoundSimpleValue(::iota::Attribute& attrib);
};

class GPSResponse : public TTResponse {
 public:
  GPSResponse(std::string module);
  virtual ~GPSResponse(){};

 protected:
  virtual void populateInternalFields(CC_AttributesType& attribute);
  virtual std::string parseCompoundSimpleValue(::iota::Attribute& attrib);
};

class BResponse : public TTCloseResponse {
 public:
  BResponse(std::string module);
  virtual ~BResponse(){};
  /**
  @name processContextElement
  @brief due to a new requierement, this method will always do the @see
  parseDefault
  */
  void processContextElement(::iota::ContextElement& contextElement,
                             CC_AttributesType& attribute);
  void parseDefault();

 protected:
  virtual void populateInternalFields(CC_AttributesType& attribute);
  virtual std::string parseCompoundSimpleValue(::iota::Attribute& attrib);

 private:
  std::string estado_activacion_carga;  /**< charge activation status */
  std::string modo_carga;               /**< charging mode */
  std::string tiempo_desconexion_stack; /**< stack disconnection time */
};

class GMResponse : public TTResponse {
 public:
  GMResponse(std::string module);

  virtual ~GMResponse(){};
  void parseDefault();

 protected:
  virtual void populateInternalFields(CC_AttributesType& attribute);
  virtual std::string parseCompoundSimpleValue(::iota::Attribute& attrib);

 private:
};

class GCResponse : public TTResponse {
 public:
  GCResponse(std::string module);

  virtual ~GCResponse(){};
  void parseDefault();

 protected:
  virtual void populateInternalFields(CC_AttributesType& attribute);
  virtual std::string parseCompoundSimpleValue(::iota::Attribute& attrib);

 private:
  std::string value; /**< this is the value of the GC coming from the device,
                        used when creating default response */
};

/**
  @name SearchResponse
  @brief this class encapsulates the call to the QueryContext service in CB and
  the later processing of the response to create the http response
  that has to be sent to the TT device.
*/
class SearchResponse {
 public:
  SearchResponse();

  std::string searchTTResponse(std::vector<CC_AttributesType>& attributes,
                               std::string entity_name, std::string entity_type,
                               QueryContextWrapper* qc);

  virtual ~SearchResponse();

 protected:
 private:
  pion::logger m_logger;

  /**
  @name doSearchTTResponse
  @brief This is the method that will actually implement the algorithm of
  checking the queryContext, compare it to what came from the device
  and produce a valid response that can be sent to the TT Device.
  @param [IN] std::vector<CC_AttributesType>: the ESP output, is what has come
  from the device.
  @param [IN] std::string: entity_name, the name of the device in the
  ContextBroker domain.
  @param [IN] std::string : entity_type.
  @param [IN] QueryContextWrapper: this object will make just the call to the
  QueryContext.
  */
  virtual std::string doSearchTTResponse(
      std::vector<CC_AttributesType>& attribute, std::string entity_name,
      std::string entity_type, QueryContextWrapper* qc);
};
}
}
}
#endif  // SEARCHRESPONSE_H
