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
#include <ngsi/AttributeRegister.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/reader.h>
#include <stdexcept>
#include <iostream>

iota::AttributeRegister::AttributeRegister(
    const std::istringstream& str_attribute) {
  rapidjson::Document document;
  char buffer[str_attribute.str().length()];
  strcpy(buffer, str_attribute.str().c_str());
  if (document.ParseInsitu<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "AttributeRegister: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());
  }

  if (document.HasMember("name") == false) {
    std::ostringstream what;
    what << "AttributeRegister: ";
    what << " missing field [";
    what << "name]";
    throw std::runtime_error(what.str());
  }
  _name.assign(document["name"].GetString());

  if ((document.HasMember("type")) && (document["type"].IsString())) {
    _type.assign(document["type"].GetString());
  }
  if ((document.HasMember("isDomain")) && (document["isDomain"].IsString())) {
    _is_domain.assign(document["isDomain"].GetString());
  }
};

iota::AttributeRegister::AttributeRegister(const rapidjson::Value& attribute) {
  if (!attribute.IsObject()) {
    throw std::runtime_error("Invalid Object");
  }
  if (attribute.HasMember("name") == false) {
    std::ostringstream what;
    what << "AttributeRegister: ";
    what << " missing field [";
    what << "name]";
    throw std::runtime_error(what.str());
  }
  _name.assign(attribute["name"].GetString());
  if (attribute.HasMember("type")) {
    _type.assign(attribute["type"].GetString());
  }
  if (attribute.HasMember("isDomain")) {
    _is_domain.assign(attribute["isDomain"].GetString());
  }
};

std::string iota::AttributeRegister::get_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  Serialize(writer);
  return buffer.GetString();
};
