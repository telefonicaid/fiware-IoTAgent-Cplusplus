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
#include "SearchResponse.h"
#include "ngsi/Entity.h"

iota::esp::tt::SearchResponse::SearchResponse()
    : m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {}

iota::esp::tt::SearchResponse::~SearchResponse() {}

std::string iota::esp::tt::SearchResponse::searchTTResponse(
    std::vector<CC_AttributesType>& attributes, std::string entity_name,
    std::string entity_type, QueryContextWrapper* qc) {
  return doSearchTTResponse(attributes, entity_name, entity_type, qc);
}

std::string iota::esp::tt::SearchResponse::doSearchTTResponse(
    std::vector<CC_AttributesType>& attributes, std::string entity_name,
    std::string entity_type, QueryContextWrapper* qcw) {
  GCResponse* gcResponse;
  GMResponse* gmResponse;
  iota::QueryContext q;

  IOTA_LOG_DEBUG(m_logger, "doSearchTTResponse name:["
                               << entity_name << "] type:[" << entity_type
                               << "]");

  iota::Entity entity(entity_name, entity_type, "false");

  q.add_entity(entity);

  //
  std::string module;
  std::string busid;
  std::string finalResponse("");

  ::iota::ContextResponses cResponses = qcw->queryContext(q);  // The Call

  // as of today, 10-12-2014, if response from CB is empty, it means that
  // all attributes need to be published.
  if (cResponses.get_context_responses().size() > 0) {
    ::iota::ContextResponse cResp = cResponses.get_context_responses()[0];

    IOTA_LOG_DEBUG(m_logger, "doSearchTTResponse: response STATUS: ["
                                 << cResp.get_code() << "]");
    if (cResp.get_code() == "200") {
      IOTA_LOG_DEBUG(m_logger, "doSearchTTResponse: Checking Attributes... ");
      ::iota::ContextResponse cResp = cResponses.get_context_responses()[0];

      ::iota::ContextElement& cElement = cResp.get_context_element();

      // OK, some context_responses are available, let's match them with our
      // CC_AttributesType
      // to get responses per module.
      for (int i = 0; i < attributes.size(); i++) {
        CC_AttributesType::iterator itModule = attributes[i].find("module");
        CC_AttributesType::iterator itBus = attributes[i].find("busid");

        if (itModule != attributes[i].end() && itBus != attributes[i].end()) {
          module.assign(itModule->second.getValueAsString());
          busid.assign(itBus->second.getValueAsString());

          IOTA_LOG_DEBUG(m_logger,
                         "doSearchTTResponse: Checking response for Module ["
                             << module << "] and entity [" << entity_name
                             << "] ");

          // at this point, I have no idea about how to deal with this module,
          // it could be GC, GM, P1, K, or B
          // each of them may have different expectations about retrieved
          // Attributes from ContextBroker, so I let
          // TTResponse classes to take care of this task.

          iota::esp::tt::TTResponse* ttResponse =
              iota::esp::tt::TTResponse::decodeStringToTTResponse(module);

          ttResponse->setBusId(busid);
          IOTA_LOG_DEBUG(m_logger,
                         "doSearchTTResponse: TTResponse object created "
                         "obtaining info from CB Response:");
          ttResponse->processContextElement(cElement, attributes[i]);
          IOTA_LOG_DEBUG(m_logger,
                         "doSearchTTResponse: TTResponse object created "
                         "obtaining info from CB Response: DONE");

          if (finalResponse != "") {
            finalResponse.append(",");
          }

          IOTA_LOG_DEBUG(m_logger, "doSearchTTResponse: Response obtained: [ "
                                       << ttResponse->toTTString() << " ]");
          finalResponse.append(ttResponse->toTTString());

          IOTA_LOG_DEBUG(m_logger,
                         "doSearchTTResponse: TTResponse appended response: [ "
                             << finalResponse << "]");

          delete ttResponse;
        }
      }
    }
  }
  return finalResponse;
}

iota::esp::tt::TTResponse::TTResponse(std::string m)
    : m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  module.assign(m);
  mainResponse.assign("");
  ignore_module = false;
}

void iota::esp::tt::TTResponse::setBusId(std::string bus) { busid.assign(bus); }

iota::esp::tt::TTResponse* iota::esp::tt::TTResponse::decodeStringToTTResponse(
    std::string module) {
  if (module == "GM") {
    return new iota::esp::tt::GMResponse(module);
  }

  if (module == "GC") {
    return new iota::esp::tt::GCResponse(module);
  }

  if (module == "B") {
    return new iota::esp::tt::BResponse(module);
  }

  if (module == "GPS") {
    return new iota::esp::tt::GPSResponse(module);
  }

  return new iota::esp::tt::TTCloseResponse(module);
}

void iota::esp::tt::TTResponse::processContextElement(
    ::iota::ContextElement& cElement, CC_AttributesType& attributes) {
  bool byDefault = true;

  /*
  TODO: sigue por aquí.
  */

  populateInternalFields(attributes);

  // I search input CC_AttributesType in cElement (response from ContextBroker)
  for (int j = 0; j < cElement.get_attributes().size(); j++) {
    ::iota::Attribute& attrib = cElement.get_attributes()[j];
    // some Modules may have their names stored in a different "param" within
    // Cc_AttributesType

    if (attrib.get_name() == nameTTAttribute) {
      IOTA_LOG_DEBUG(m_logger,
                     "TTResponse:processContextElement: Attribute Found: "
                         << nameTTAttribute);
      parseResponse(attrib);
      byDefault = false;
    }
  }

  // in case the module wasn't found in QueryContext response
  // then at least we need to return something
  //(depending on the type of TT module)
  if (byDefault) {
    parseDefault();
  }
}

void iota::esp::tt::TTResponse::populateInternalFields(
    CC_AttributesType& attribute) {
  CC_AttributesType::iterator itCond = attribute.find("sleepcondition");
  CC_AttributesType::iterator itTime = attribute.find("sleeptime");

  if (itCond != attribute.end()) {
    sleepCondition.assign(itCond->second.getValueAsString());
  }

  if (itTime != attribute.end()) {
    sleepTime.assign(itTime->second.getValueAsString());
  }
}

void iota::esp::tt::TTCloseResponse::populateInternalFields(
    CC_AttributesType& attribute) {
  nameTTAttribute.assign(module);
  TTResponse::populateInternalFields(attribute);
}

iota::esp::tt::GCResponse::GCResponse(std::string m) : TTResponse(m) {}

iota::esp::tt::GMResponse::GMResponse(std::string m) : TTResponse(m) {}

iota::esp::tt::TTCloseResponse::TTCloseResponse(std::string m)
    : TTResponse(m) {}

std::string iota::esp::tt::TTResponse::toTTString() {
  if (ignore_module)
    return "";
  else
    return mainResponse;
}

/*
*/
void iota::esp::tt::TTResponse::parseDefault() {
  mainResponse.assign("#");
  mainResponse.append(busid);
  mainResponse.append(",");
  mainResponse.append(module);
  mainResponse.append(",");
  mainResponse.append(sleepTime);
  mainResponse.append("$");
  mainResponse.append(sleepCondition);
}

void iota::esp::tt::TTResponse::parseResponse(
    ::iota::Attribute& foundAttribute) {
  mainResponse.assign(parseModule(foundAttribute));

  mainResponse.append(parseCompoundSimpleValue(foundAttribute));
  mainResponse.append(",");
  mainResponse.append(parseMetadatas(foundAttribute));
}

std::string iota::esp::tt::TTResponse::parseModule(::iota::Attribute& attrib) {
  std::string response("#");
  response.append(busid);
  response.append(",");
  response.append(module);
  return response;
}

std::string iota::esp::tt::TTResponse::parseMetadatas(
    ::iota::Attribute& attrib) {
  std::string sleepcond;
  std::string sleeptime;

  if (attrib.get_metadatas().size() > 0) {
    for (int i = 0; i < attrib.get_metadatas().size(); i++) {
      if (attrib.get_metadatas()[i].get_name() == SLEEPCONDITION) {
        sleepcond.assign(attrib.get_metadatas()[i].get_value());
      }
      if (attrib.get_metadatas()[i].get_name() == SLEEPTIME) {
        sleeptime.assign(attrib.get_metadatas()[i].get_value());
      }
    }
  }

  return sleeptime + "$" + sleepcond;
}

std::string iota::esp::tt::GCResponse::parseCompoundSimpleValue(
    ::iota::Attribute& attribute) {
  std::string output("");

  output.append(",");
  output.append(attribute.get_name());
  output.append(",");
  output.append(attribute.get_value());
  return output;
}

void iota::esp::tt::GCResponse::populateInternalFields(
    CC_AttributesType& attributes) {
  TTResponse::populateInternalFields(attributes);

  CC_AttributesType::iterator itName = attributes.find("param_1");
  if (itName != attributes.end()) {
    nameTTAttribute.assign(itName->second.getValueAsString());
  }

  CC_AttributesType::iterator itValue = attributes.find("param_2");
  if (itValue != attributes.end()) {
    value = itValue->second.getValueAsString();
  } else {
    IOTA_LOG_ERROR(m_logger,
                   "GCResponse:populateInternalFields: Value [param_2] was not "
                   "found, response won't be complete ");
  }
}

void iota::esp::tt::GCResponse::parseDefault() {
  mainResponse.assign("#");
  mainResponse.append(busid);
  mainResponse.append(",");
  mainResponse.append(module);

  mainResponse.append(",");
  mainResponse.append(nameTTAttribute);
  mainResponse.append(",");
  mainResponse.append(value);
  mainResponse.append(",");
  mainResponse.append(sleepTime);
  mainResponse.append("$");
  mainResponse.append(sleepCondition);
}

void iota::esp::tt::GMResponse::parseDefault() {
  mainResponse.assign("#");
  mainResponse.append(busid);
  mainResponse.append(",");
  mainResponse.append(module);

  mainResponse.append(",");
  mainResponse.append(nameTTAttribute);
  mainResponse.append(",");
  mainResponse.append(sleepTime);
  mainResponse.append("$");
  mainResponse.append(sleepCondition);
}

std::string iota::esp::tt::GMResponse::parseCompoundSimpleValue(
    ::iota::Attribute& attribute) {
  std::string output("");

  output.append(",");
  output.append(attribute.get_name());

  return output;
}

void iota::esp::tt::GMResponse::populateInternalFields(
    CC_AttributesType& attributes) {
  CC_AttributesType::iterator itName = attributes.find("param_1");
  if (itName != attributes.end()) {
    nameTTAttribute.assign(itName->second.getValueAsString());
  }

  TTResponse::populateInternalFields(attributes);
}

std::string iota::esp::tt::TTCloseResponse::parseCompoundSimpleValue(
    ::iota::Attribute& attribute) {
  std::string output("");

  // This will be for P1 module only.
  output.append(",");
  output.append(attribute.get_value());

  return output;
}

void iota::esp::tt::TTCloseResponse::processContextElement(
    ::iota::ContextElement& context, CC_AttributesType& attributes) {
  populateInternalFields(attributes);

  parseDefault();
}

iota::esp::tt::BResponse::BResponse(std::string module)
    : TTCloseResponse(module) {}

/**
 Message to be constructed has to look like this:
 #<id_i2c>,B,<estado_activacion_carga>,<modo_carga>,<tiempo_desconexion_stack>,<sleep_user>&<condition>,
*/
std::string iota::esp::tt::BResponse::parseCompoundSimpleValue(
    ::iota::Attribute& attribute) {
  std::string status("");
  std::string mode("");
  std::string disconnection("");

  std::string output(",");
  if (attribute.get_value_compound().size() > 0) {
    for (int i = 0; i < attribute.get_value_compound().size(); i++) {
      if (attribute.get_value_compound()[i].get_name() == TT_B_CHARGING_MODE) {
        mode.assign(attribute.get_value_compound()[i].get_value());
      }

      if (attribute.get_value_compound()[i].get_name() ==
          TT_B_DISCONN_STACK_TIME) {
        disconnection.assign(attribute.get_value_compound()[i].get_value());
      }

      if (attribute.get_value_compound()[i].get_name() ==
          TT_B_STATUS_CHARGE_ACTIVATION) {
        status.assign(attribute.get_value_compound()[i].get_value());
      }
    }

    output.append(status);
    output.append(",");
    output.append(mode);
    output.append(",");
    output.append(disconnection);
  }

  return output;
}

void iota::esp::tt::BResponse::populateInternalFields(
    CC_AttributesType& attributes) {
  TTResponse::populateInternalFields(attributes);
  nameTTAttribute.assign(module);

  // Now we retrieve the values for the fields that must be included in the
  // response
  // in case of parseDefault.
  // NOTE that the position is imposed by the TT protocol:
  // B,<voltaje>,<estado_activacion>,<hay_cargador>,<estado_carga>,<modo_carga>,<tiempo_desco_stack>
  // and I need:
  // estado_activacion, modo_carga, tiempo_desco_stack

  CC_AttributesType::iterator itEstado = attributes.find("param_2");
  CC_AttributesType::iterator itModo = attributes.find("param_5");
  CC_AttributesType::iterator itTiempo = attributes.find("param_6");

  if (itEstado != attributes.end()) {
    estado_activacion_carga.assign(itEstado->second.getValueAsString());
  } else {
    IOTA_LOG_ERROR(m_logger,
                   "BResponse::populateInternalFields: [param_2] not found, "
                   "missing [estado_activacion]");
  }

  if (itModo != attributes.end()) {
    modo_carga.assign(itModo->second.getValueAsString());
  } else {
    IOTA_LOG_ERROR(m_logger,
                   "BResponse::populateInternalFields: [param_5] not found, "
                   "missing [modo_carga]");
  }

  if (itTiempo != attributes.end()) {
    tiempo_desconexion_stack.assign(itTiempo->second.getValueAsString());
  } else {
    IOTA_LOG_ERROR(m_logger,
                   "BResponse::populateInternalFields: [param_6] not found, "
                   "missing [tiempo_desco_stack]");
  }
}

void iota::esp::tt::BResponse::parseDefault() {
  mainResponse.assign("#");
  mainResponse.append(busid);
  mainResponse.append(",");
  mainResponse.append(module);
  mainResponse.append(",");
  mainResponse.append(estado_activacion_carga);
  mainResponse.append(",");
  mainResponse.append(modo_carga);
  mainResponse.append(",");
  mainResponse.append(tiempo_desconexion_stack);
  mainResponse.append(",");
  mainResponse.append(sleepTime);
  mainResponse.append("$");
  mainResponse.append(sleepCondition);
}

void iota::esp::tt::BResponse::processContextElement(
    ::iota::ContextElement& context, CC_AttributesType& attributes) {
  populateInternalFields(attributes);

  parseDefault();
}

iota::esp::tt::GPSResponse::GPSResponse(std::string module)
    : TTResponse(module) {}

void iota::esp::tt::GPSResponse::populateInternalFields(
    CC_AttributesType& attribute) {
  nameTTAttribute.assign(
      "position");  // specific for this module, and needed for matching.

  TTResponse::populateInternalFields(attribute);
}

std::string iota::esp::tt::GPSResponse::parseCompoundSimpleValue(
    ::iota::Attribute& attribute) {
  return "";  // Easy!
}
