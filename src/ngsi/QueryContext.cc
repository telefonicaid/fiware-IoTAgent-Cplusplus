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
#include <ngsi/QueryContext.h>
#include "ngsi_const.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <stdexcept>

iota::QueryContext::QueryContext(const std::istringstream& str_query) {
  rapidjson::Document document;
  char buffer[str_query.str().length()];
  // memcpy(buffer, str_attribute.c_str(), str_attribute.length());
  strcpy(buffer, str_query.str().c_str());
  if (document.ParseInsitu<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "QueryContext: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());
  }

  if (!document.HasMember(iota::ngsi::NGSI_ENTITIES.c_str()) ||
      !document[iota::ngsi::NGSI_ENTITIES.c_str()].IsArray()) {
    std::ostringstream what;
    what << "QueryContext: ";
    what << "invalid or missing field [";
    what << iota::ngsi::NGSI_ENTITIES;
    what << "]";
    throw std::runtime_error(what.str());
  }
  const rapidjson::Value& entities =
      document[iota::ngsi::NGSI_ENTITIES.c_str()];
  for (rapidjson::SizeType i = 0; i < entities.Size(); i++) {
    iota::Entity entitiy(entities[i]);
    add_entity(entitiy);
  }

  if (document.HasMember(iota::ngsi::NGSI_ATTRIBUTES.c_str()) &&
      !document[iota::ngsi::NGSI_ATTRIBUTES.c_str()].IsArray()) {
    std::ostringstream what;
    what << "QueryContext: ";
    what << "invalid or missing field [";
    what << iota::ngsi::NGSI_ATTRIBUTES;
    what << "]";
    throw std::runtime_error(what.str());
  }
  const rapidjson::Value& attributes = document["attributes"];

  for (rapidjson::SizeType i = 0; i < attributes.Size(); i++) {
    add_attribute(attributes[i].GetString());
  }
};

iota::QueryContext::QueryContext(const rapidjson::Value& query) {
  if (!query.IsObject() ||
      !query.HasMember(iota::ngsi::NGSI_ENTITIES.c_str()) ||
      !query[iota::ngsi::NGSI_ENTITIES.c_str()].IsArray()) {
    std::ostringstream what;
    what << "QueryContext: ";
    what << "invalid or missing field [";
    what << iota::ngsi::NGSI_ENTITIES;
    what << "]";
    throw std::runtime_error(what.str());
  }
  if (!query.HasMember(iota::ngsi::NGSI_ATTRIBUTES.c_str()) ||
      !query[iota::ngsi::NGSI_ATTRIBUTES.c_str()].IsArray()) {
    std::ostringstream what;
    what << "QueryContext: ";
    what << "invalid or missing field [";
    what << iota::ngsi::NGSI_ATTRIBUTES;
    what << "]";
    throw std::runtime_error(what.str());
  }

  const rapidjson::Value& entities = query[iota::ngsi::NGSI_ENTITIES.c_str()];
  for (rapidjson::SizeType i = 0; i < entities.Size(); i++) {
    iota::Entity entity(entities[i]);
    add_entity(entity);
  }

  const rapidjson::Value& attributes =
      query[iota::ngsi::NGSI_ATTRIBUTES.c_str()];

  for (rapidjson::SizeType i = 0; i < attributes.Size(); i++) {
    add_attribute(attributes[i].GetString());
  }
};

std::string iota::QueryContext::get_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  Serialize(writer);
  return buffer.GetString();
};

void iota::QueryContext::add_entity(const iota::Entity& entity) {
  _entities.push_back(entity);
};

void iota::QueryContext::add_attribute(const std::string& attribute) {
  _attributes.push_back(attribute);
};
