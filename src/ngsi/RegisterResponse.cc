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
#include "RegisterResponse.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <stdexcept>

iota::RegisterResponse::RegisterResponse(
    const std::istringstream& str_reg_response) {
  rapidjson::Document document;
  char buffer[str_reg_response.str().length()];
  // memcpy(buffer, str_attribute.c_str(), str_attribute.length());
  strcpy(buffer, str_reg_response.str().c_str());
  if (document.Parse<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "RegisterResponse: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());
  }

  // ContextBroker, can response with out
  // { "duration" : "PT24H", "registrationId" : "xxxxxxxxxxx" }
  //
  if (document.HasMember("registerResponse")) {
    if ((document["registerResponse"].HasMember("registrationId")) &&
        (document["registerResponse"]["registrationId"].IsString())) {
      _registrationId.assign(
          document["registerResponse"]["registrationId"].GetString());
    }
    if ((document["registerResponse"].HasMember("duration")) &&
        (document["registerResponse"]["duration"].IsString())) {
      _duration.assign(document["registerResponse"]["duration"].GetString());
    }
  } else {
    if ((document.HasMember("registrationId")) &&
        (document["registrationId"].IsString())) {
      _registrationId.assign(document["registrationId"].GetString());
    }
    if ((document.HasMember("duration")) && (document["duration"].IsString())) {
      _duration.assign(document["duration"].GetString());
    }
  }

  if (_registrationId.empty()) {
    std::ostringstream what;
    what << "RegisterResponse: ";
    what << "[registrationId]";
    what << "Missing field";
    throw std::runtime_error(what.str());
  }
  if (_duration.empty()) {
    std::ostringstream what;
    what << "RegisterResponse: ";
    what << "[duration]";
    what << "Missing field";
    throw std::runtime_error(what.str());
  }
}

iota::RegisterResponse::RegisterResponse(const rapidjson::Value& reg_response) {
  if (reg_response["registerResponse"].HasMember("registrationId")) {
    _registrationId.assign(
        reg_response["registerResponse"]["registrationId"].GetString());
  }
  if (reg_response["registerResponse"].HasMember("duration")) {
    _duration.assign(reg_response["registerResponse"]["duration"].GetString());
  }
};

std::string iota::RegisterResponse::get_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  Serialize(writer);
  return buffer.GetString();
};

void iota::RegisterResponse::add_registration_id(const std::string& reg_id) {
  _registrationId = reg_id;
};

void iota::RegisterResponse::add_duration(const std::string& data) {
  _duration = data;
};
