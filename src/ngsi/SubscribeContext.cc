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
#include "ngsi/SubscribeContext.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <stdexcept>

iota::SubscribeContext::SubscribeContext(
    const std::istringstream& str_subscribe) {
  rapidjson::Document document;
  char buffer[str_subscribe.str().length()];
  strcpy(buffer, str_subscribe.str().c_str());
  if (document.Parse<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "SubscribeContext: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());
  }
  if (document.HasMember("entities")) {
    const rapidjson::Value& entities = document["entities"];
    if (entities.IsArray()) {
      for (rapidjson::SizeType i = 0; i < entities.Size(); i++) {
        iota::Entity entity(entities[i]);
        add_entity(entity);
      }
    } else {
      std::ostringstream what;
      what << "SubscribeContext: ";
      what << "[entities] ";
      what << "Malformed field";
      throw std::runtime_error(what.str());
    }

  } else {
    std::ostringstream what;
    what << "SubscribeContext: ";
    what << "[entities] ";
    what << "Missing field";
    throw std::runtime_error(what.str());
  }
  if (document.HasMember("attributes")) {
    const rapidjson::Value& attributes = document["attributes"];
    if (attributes.IsArray()) {
      for (rapidjson::SizeType i = 0; i < attributes.Size(); i++) {
        std::string attribute(attributes[i].GetString());
        add_attribute(attribute);
      }
    }
  }

  if (document.HasMember("throttling")) {
    _throttling.assign(document["throttling"].GetString());
  }
  if (document.HasMember("duration")) {
    _duration.assign(document["duration"].GetString());
  }
  if (document.HasMember("reference")) {
    _reference.assign(document["reference"].GetString());
  }
  if (document.HasMember("notifyConditions")) {
    const rapidjson::Value& conditions = document["notifyConditions"];
    if (conditions.IsArray()) {
      for (rapidjson::SizeType i = 0; i < conditions.Size(); i++) {
        iota::NotifyCondition condition(conditions[i]);
        add_condition(condition);
      }
    }
  }
};

iota::SubscribeContext::SubscribeContext(const rapidjson::Value& subs) {
  if (!subs.IsObject()) {
    std::ostringstream what;
    what << "SubscribeContext: ";
    what << "[entities] ";
    what << "Missing field";
    throw std::runtime_error(what.str());
  }
  if (subs.HasMember("entities")) {
    const rapidjson::Value& entities = subs["entities"];
    if (entities.IsArray()) {
      for (rapidjson::SizeType i = 0; i < entities.Size(); i++) {
        iota::Entity entity(entities[i]);
        add_entity(entity);
      }
    }
  }

  if (subs.HasMember("attributes")) {
    const rapidjson::Value& attributes = subs["attributes"];
    if (attributes.IsArray()) {
      for (rapidjson::SizeType i = 0; i < attributes.Size(); i++) {
        std::string attribute(attributes[i].GetString());
        add_attribute(attribute);
      }
    }
  }

  if (subs.HasMember("reference")) {
    _reference.assign(subs["reference"].GetString());
  }

  if (subs.HasMember("duration")) {
    _duration.assign(subs["reference"].GetString());
  }

  if (subs.HasMember("throttling")) {
    _throttling.assign(subs["throttling"].GetString());
  }

  if (subs.HasMember("notifyConditions")) {
    const rapidjson::Value& conditions = subs["notifyConditions"];
    if (conditions.IsArray()) {
      for (rapidjson::SizeType i = 0; i < conditions.Size(); i++) {
        iota::NotifyCondition condition(conditions[i]);
        add_condition(condition);
      }
    }
  }
};

std::string iota::SubscribeContext::get_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  Serialize(writer);
  return buffer.GetString();
};

void iota::SubscribeContext::add_entity(const iota::Entity& entity) {
  _entities.push_back(entity);
};

void iota::SubscribeContext::add_attribute(const std::string& attribute) {
  _attributes.push_back(attribute);
};

void iota::SubscribeContext::add_reference(const std::string& reference) {
  _reference.assign(reference);
};

void iota::SubscribeContext::add_duration(const std::string& duration) {
  _duration.assign(duration);
};

void iota::SubscribeContext::add_throttling(const std::string& throttling) {
  _throttling.assign(throttling);
};

void iota::SubscribeContext::add_condition(
    const iota::NotifyCondition& condition) {
  _conditions.push_back(condition);
};
