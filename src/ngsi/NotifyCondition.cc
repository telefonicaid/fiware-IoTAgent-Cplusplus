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
#include "ngsi/NotifyCondition.h"
#include "ngsi/ngsi_const.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <stdexcept>

iota::NotifyCondition::NotifyCondition(
    const std::istringstream& str_condition) {
  rapidjson::Document document;
  char buffer[str_condition.str().length()];
  // memcpy(buffer, str_attribute.c_str(), str_attribute.length());
  strcpy(buffer, str_condition.str().c_str());
  if (document.Parse<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "NotifyCondition: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());
  }
  if (!document.HasMember(iota::ngsi::NGSI_TYPE.c_str()) ||
      !document[iota::ngsi::NGSI_TYPE.c_str()].IsString() ||
      !document.HasMember(iota::ngsi::NGSI_CONDVALUES.c_str()) ||
      !document[iota::ngsi::NGSI_CONDVALUES.c_str()].IsArray()) {
    std::ostringstream what;
    what << "NotifyCondition: ";
    what << "invalid or missing field [" << iota::ngsi::NGSI_TYPE.c_str();
    what << "/" << iota::ngsi::NGSI_CONDVALUES.c_str();
    what << "]";
    throw std::runtime_error(what.str());
  }
  _type.assign(document[iota::ngsi::NGSI_TYPE.c_str()].GetString());

  const rapidjson::Value& values =
      document[iota::ngsi::NGSI_CONDVALUES.c_str()];

  for (rapidjson::SizeType i = 0; i < values.Size(); i++) {
    std::string value(values[i].GetString());
    add_value(value);
  }
}

iota::NotifyCondition::NotifyCondition(const rapidjson::Value& ncond) {
  if (!ncond.IsObject() || !ncond.HasMember(iota::ngsi::NGSI_TYPE.c_str()) ||
      !ncond[iota::ngsi::NGSI_TYPE.c_str()].IsString() ||
      !ncond.HasMember(iota::ngsi::NGSI_CONDVALUES.c_str()) ||
      !ncond[iota::ngsi::NGSI_CONDVALUES.c_str()].IsArray()) {
    std::ostringstream what;
    what << "NotifyCondition: ";
    what << "invalid or missing field [" << iota::ngsi::NGSI_TYPE.c_str();
    what << "/" << iota::ngsi::NGSI_CONDVALUES.c_str();
    what << "]";
    throw std::runtime_error(what.str());
  }

  _type.assign(ncond[iota::ngsi::NGSI_TYPE.c_str()].GetString());

  const rapidjson::Value& values = ncond[iota::ngsi::NGSI_CONDVALUES.c_str()];
  for (rapidjson::SizeType i = 0; i < values.Size(); i++) {
    std::string value(values[i].GetString());
    add_value(value);
  }
};
std::string iota::NotifyCondition::get_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  Serialize(writer);
  return buffer.GetString();
};

void iota::NotifyCondition::add_value(const std::string& value) {
  _values.push_back(value);
}
