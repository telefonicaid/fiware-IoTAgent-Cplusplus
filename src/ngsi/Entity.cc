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
#include "ngsi/Entity.h"
#include "ngsi_const.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <stdexcept>

iota::Entity::Entity(const std::istringstream& str_entity) {
  rapidjson::Document document;
  char buffer[str_entity.str().length()];
  // memcpy(buffer, str_attribute.c_str(), str_attribute.length());
  strcpy(buffer, str_entity.str().c_str());
  if (document.ParseInsitu<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "Entity: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());
  }
  if (!document.HasMember(iota::ngsi::NGSI_ID.c_str()) ||
      !document[iota::ngsi::NGSI_ID.c_str()].IsString()) {
    std::ostringstream what;
    what << "Entity: ";
    what << "missing field [";
    what << iota::ngsi::NGSI_ID;
    what << "]";
    throw std::runtime_error(what.str());
  }
  _id.assign(document[iota::ngsi::NGSI_ID.c_str()].GetString());
  if (document.HasMember(iota::ngsi::NGSI_TYPE.c_str()) &&
      document[iota::ngsi::NGSI_TYPE.c_str()].IsString()) {
    _type.assign(document[iota::ngsi::NGSI_TYPE.c_str()].GetString());
  }
  if (document.HasMember(iota::ngsi::NGSI_ISPATTERN.c_str()) &&
      document[iota::ngsi::NGSI_ISPATTERN.c_str()].IsString()) {
    _is_pattern.assign(
        document[iota::ngsi::NGSI_ISPATTERN.c_str()].GetString());
  }
}

iota::Entity::Entity(const rapidjson::Value& entity) {
  if (!entity.IsObject() || !entity.HasMember(iota::ngsi::NGSI_ID.c_str()) ||
      !entity[iota::ngsi::NGSI_ID.c_str()].IsString()) {
    std::ostringstream what;
    what << "Entity: ";
    what << "missing field [";
    what << iota::ngsi::NGSI_ID;
    what << "]";
    throw std::runtime_error(what.str());
  }
  _id.assign(entity[iota::ngsi::NGSI_ID.c_str()].GetString());
  if (entity.HasMember(iota::ngsi::NGSI_TYPE.c_str()) &&
      entity[iota::ngsi::NGSI_TYPE.c_str()].IsString()) {
    _type.assign(entity[iota::ngsi::NGSI_TYPE.c_str()].GetString());
  }
  if (entity.HasMember(iota::ngsi::NGSI_ISPATTERN.c_str()) &&
      entity[iota::ngsi::NGSI_ISPATTERN.c_str()].IsString()) {
    _is_pattern.assign(entity[iota::ngsi::NGSI_ISPATTERN.c_str()].GetString());
  }
};
std::string iota::Entity::get_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  Serialize(writer);
  return buffer.GetString();
};
