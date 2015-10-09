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
#include <ngsi/RegisterContext.h>
#include "ngsi_const.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <stdexcept>
#include <iostream>

iota::RegisterContext::RegisterContext(const std::istringstream& reg_ctx) {
  rapidjson::Document document;
  char buffer[reg_ctx.str().length()];
  // memcpy(buffer, str_attribute.c_str(), str_attribute.length());
  strcpy(buffer, reg_ctx.str().c_str());
  if (document.ParseInsitu<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "RegisterContext: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());
  }

  if (!document.HasMember(iota::ngsi::NGSI_CONTEXTREGISTRATIONS.c_str()) ||
      !document[iota::ngsi::NGSI_CONTEXTREGISTRATIONS.c_str()].IsArray()) {
    std::ostringstream what;
    what << "RegisterContext: ";
    what << "invalid or missing  field [";
    what << iota::ngsi::NGSI_CONTEXTREGISTRATIONS;
    what << "]";
    throw std::runtime_error(what.str());
  }
  const rapidjson::Value& context_registrations =
      document[iota::ngsi::NGSI_CONTEXTREGISTRATIONS.c_str()];

  for (rapidjson::SizeType i = 0; i < context_registrations.Size(); i++) {
    iota::ContextRegistration context_registration(context_registrations[i]);
    add_context_registration(context_registration);
  }

  if (document.HasMember(iota::ngsi::NGSI_DURATION.c_str()) &&
      document[iota::ngsi::NGSI_DURATION.c_str()].IsString()) {
    add_duration(document[iota::ngsi::NGSI_DURATION.c_str()].GetString());
  }

  if (document.HasMember(iota::ngsi::NGSI_REGISTRATIONID.c_str()) &&
      document[iota::ngsi::NGSI_REGISTRATIONID.c_str()].IsString()) {
    add_registrationId(
        document[iota::ngsi::NGSI_REGISTRATIONID.c_str()].GetString());
  }
};

iota::RegisterContext::RegisterContext(const rapidjson::Value& reg_ctx) {
  if (!reg_ctx.IsObject() ||
      !reg_ctx.HasMember(iota::ngsi::NGSI_CONTEXTREGISTRATIONS.c_str()) ||
      !reg_ctx[iota::ngsi::NGSI_CONTEXTREGISTRATIONS.c_str()].IsArray()) {
    std::ostringstream what;
    what << "RegisterContext: ";
    what << "invalid or missing field [";
    what << iota::ngsi::NGSI_CONTEXTREGISTRATIONS;
    what << "]";
    throw std::runtime_error(what.str());
  }
  const rapidjson::Value& context_registrations =
      reg_ctx[iota::ngsi::NGSI_CONTEXTREGISTRATIONS.c_str()];

  for (rapidjson::SizeType i = 0; i < context_registrations.Size(); i++) {
    iota::ContextRegistration context_registration(context_registrations[i]);
    add_context_registration(context_registration);
  }

  if (reg_ctx.HasMember(iota::ngsi::NGSI_DURATION.c_str()) &&
      reg_ctx[iota::ngsi::NGSI_DURATION.c_str()].IsString()) {
    add_duration(reg_ctx[iota::ngsi::NGSI_DURATION.c_str()].GetString());
  }

  if (reg_ctx.HasMember(iota::ngsi::NGSI_REGISTRATIONID.c_str()) &&
      reg_ctx[iota::ngsi::NGSI_REGISTRATIONID.c_str()].IsString()) {
    add_registrationId(
        reg_ctx[iota::ngsi::NGSI_REGISTRATIONID.c_str()].GetString());
  }
};

std::string iota::RegisterContext::get_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  Serialize(writer);
  return buffer.GetString();
};

void iota::RegisterContext::add_context_registration(
    const iota::ContextRegistration& context_registration) {
  _context_registrations.push_back(context_registration);
};

void iota::RegisterContext::add_duration(const std::string& duration) {
  _duration = duration;
}

std::string iota::RegisterContext::get_duration() { return _duration; }

void iota::RegisterContext::add_registrationId(
    const std::string& registrationId) {
  _registrationId = registrationId;
}

std::string iota::RegisterContext::get_registrationId() {
  return _registrationId;
}
