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
#include "ContextResponse.h"
#include "ngsi_const.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <stdexcept>

iota::ContextResponse::ContextResponse() {
  _code.assign("200");
  _reason.assign("OK");
}

iota::ContextResponse::ContextResponse(
    const std::istringstream& str_context_response) {
  rapidjson::Document document;
  char buffer[str_context_response.str().length()];
  strcpy(buffer, str_context_response.str().c_str());
  if (document.ParseInsitu<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "ContextResponse: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());
  }
  if (!document.HasMember(iota::ngsi::NGSI_STATUSCODE.c_str()) ||
      !document[iota::ngsi::NGSI_STATUSCODE.c_str()].IsObject() ||
      !document[iota::ngsi::NGSI_STATUSCODE.c_str()].HasMember(
          iota::ngsi::NGSI_CODE.c_str()) ||
      !document[iota::ngsi::NGSI_STATUSCODE.c_str()][iota::ngsi::NGSI_CODE
                                                         .c_str()].IsString()) {
    std::ostringstream what;
    what << "ContextResponse: ";
    what << "invalid or missing field [" << iota::ngsi::NGSI_STATUSCODE << "]";
    throw std::runtime_error(what.str());
  }
  _code.assign(
      document[iota::ngsi::NGSI_STATUSCODE.c_str()][iota::ngsi::NGSI_CODE
                                                        .c_str()].GetString());

  if (document[iota::ngsi::NGSI_STATUSCODE.c_str()].HasMember(
          iota::ngsi::NGSI_REASONPHRASE.c_str()) &&
      document[iota::ngsi::NGSI_STATUSCODE.c_str()]
              [iota::ngsi::NGSI_REASONPHRASE.c_str()].IsString()) {
    _reason.assign(document[iota::ngsi::NGSI_STATUSCODE.c_str()]
                           [iota::ngsi::NGSI_REASONPHRASE.c_str()].GetString());
  }
  if (document[iota::ngsi::NGSI_STATUSCODE.c_str()].HasMember(
          iota::ngsi::NGSI_DETAILS.c_str()) &&
      document[iota::ngsi::NGSI_STATUSCODE.c_str()][iota::ngsi::NGSI_DETAILS
                                                        .c_str()].IsString()) {
    _details.assign(
        document[iota::ngsi::NGSI_STATUSCODE
                     .c_str()][iota::ngsi::NGSI_DETAILS.c_str()].GetString());
  }

  if (document.HasMember(iota::ngsi::NGSI_CONTEXTELEMENT.c_str()) &&
      document[iota::ngsi::NGSI_CONTEXTELEMENT.c_str()].IsObject()) {
    const rapidjson::Value& ce_value =
        document[iota::ngsi::NGSI_CONTEXTELEMENT.c_str()];
    iota::ContextElement context_element(ce_value);
    _context_element = context_element;
  }
}

iota::ContextResponse::ContextResponse(
    const rapidjson::Value& context_response) {
  if (!context_response.HasMember(iota::ngsi::NGSI_STATUSCODE.c_str()) ||
      !context_response[iota::ngsi::NGSI_STATUSCODE.c_str()].IsObject() ||
      !context_response[iota::ngsi::NGSI_STATUSCODE.c_str()].HasMember(
          iota::ngsi::NGSI_CODE.c_str()) ||
      !context_response[iota::ngsi::NGSI_STATUSCODE.c_str()]
                       [iota::ngsi::NGSI_CODE.c_str()].IsString()) {
    std::ostringstream what;
    what << "ContextResponse: ";
    what << "invalid or missing field [" << iota::ngsi::NGSI_STATUSCODE << "]";
    throw std::runtime_error(what.str());
  }
  _code.assign(context_response[iota::ngsi::NGSI_STATUSCODE.c_str()]
                               [iota::ngsi::NGSI_CODE.c_str()].GetString());

  if (context_response[iota::ngsi::NGSI_STATUSCODE.c_str()].HasMember(
          iota::ngsi::NGSI_REASONPHRASE.c_str()) &&
      context_response[iota::ngsi::NGSI_STATUSCODE.c_str()]
                      [iota::ngsi::NGSI_REASONPHRASE.c_str()].IsString()) {
    _reason.assign(
        context_response[iota::ngsi::NGSI_STATUSCODE.c_str()]
                        [iota::ngsi::NGSI_REASONPHRASE.c_str()].GetString());
  }
  if (context_response[iota::ngsi::NGSI_STATUSCODE.c_str()].HasMember(
          iota::ngsi::NGSI_DETAILS.c_str()) &&
      context_response[iota::ngsi::NGSI_STATUSCODE.c_str()]
                      [iota::ngsi::NGSI_DETAILS.c_str()].IsString()) {
    _details.assign(
        context_response[iota::ngsi::NGSI_STATUSCODE.c_str()]
                        [iota::ngsi::NGSI_DETAILS.c_str()].GetString());
  }

  if (context_response.HasMember(iota::ngsi::NGSI_CONTEXTELEMENT.c_str()) &&
      context_response[iota::ngsi::NGSI_CONTEXTELEMENT.c_str()].IsObject()) {
    const rapidjson::Value& ce_value =
        context_response[iota::ngsi::NGSI_CONTEXTELEMENT.c_str()];
    iota::ContextElement context_element(ce_value);
    _context_element = context_element;
  }
};
std::string iota::ContextResponse::get_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  Serialize(writer);
  return buffer.GetString();
};

void iota::ContextResponse::add_context_element(
    const iota::ContextElement& context_element) {
  iota::ContextElement ce(context_element);
  _context_element = ce;
};
