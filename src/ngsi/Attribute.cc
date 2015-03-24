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
#include <ngsi/Attribute.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/reader.h>
#include <stdexcept>
#include <iostream>

iota::Attribute::Attribute(const std::istringstream& str_attribute) {
  rapidjson::Document document;
  char buffer[str_attribute.str().length()];
  //memcpy(buffer, str_attribute.c_str(), str_attribute.length());
  strcpy(buffer, str_attribute.str().c_str());
  if (document.ParseInsitu<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "Attribute: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());

  }
  if (document.HasMember("name") == false) {
    std::ostringstream what;
    what << "Attribute: ";
    what << "missing field [";
    what << "name]";
    throw std::runtime_error(what.str());
  }
  _name.assign(document["name"].GetString());

  if (document.HasMember("type")) {
    _type.assign(document["type"].GetString());
  }
  if (document.HasMember("value")) {
    if (_type != "compound") {
      _value.assign(document["value"].GetString());
    }
    else {
      const rapidjson::Value& data = document["value"];
      if (data.IsArray()) {
        for (rapidjson::SizeType i = 0; i < data.Size(); i++) {
          iota::Attribute val(data[i]["name"].GetString(),
                              data[i]["type"].GetString(),
                              data[i]["value"].GetString());
          add_value_compound(val);
        }
      }
    }
  }

  // Metadatas
  if (document.HasMember("metadatas")) {
    const rapidjson::Value& metadata = document["metadatas"];
    if (metadata.IsArray()) {
      for (rapidjson::SizeType i = 0; i < metadata.Size(); i++) {
        iota::Attribute meta(metadata[i]["name"].GetString(),
                             metadata[i]["type"].GetString(),
                             metadata[i]["value"].GetString());
        add_metadata(meta);
      }
    }
    else {
      std::ostringstream what;
      what << "Attribute: ";
      what << "invalid type field [";
      what << "metadatas]";
      throw std::runtime_error(what.str());
    }
  }
};

iota::Attribute::Attribute(const rapidjson::Value& attribute) {

  if (!attribute.IsObject()) {
    throw std::runtime_error("Invalid Object");
  }
  if (attribute.HasMember("name") == false) {
    std::ostringstream what;
    what << "Attribute: ";
    what << "missing field [";
    what << "name]";
    throw std::runtime_error(what.str());
  }
  _name.assign(attribute["name"].GetString());
  if (attribute.HasMember("type")) {
    _type.assign(attribute["type"].GetString());
  }
  if (attribute.HasMember("value")) {
    if (_type != "compound") {
      _value.assign(attribute["value"].GetString());
    }
    else {
      const rapidjson::Value& data = attribute["value"];
      if (data.IsArray()) {
        for (rapidjson::SizeType i = 0; i < data.Size(); i++) {
          iota::Attribute val(data[i]["name"].GetString(),
                              data[i]["type"].GetString(),
                              data[i]["value"].GetString());
          add_value_compound(val);
        }
      }
    }
  }
  if (attribute.HasMember("metadatas")) {
    const rapidjson::Value& metadata = attribute["metadatas"];
    if (metadata.IsArray()) {
      for (rapidjson::SizeType i = 0; i < metadata.Size(); i++) {
        iota::Attribute meta(metadata[i]["name"].GetString(),
                             metadata[i]["type"].GetString(),
                             metadata[i]["value"].GetString());
        add_metadata(meta);
      }
    }
    else {
      std::ostringstream what;
      what << "Attribute: ";
      what << "invalid type field [";
      what << "metadatas]";
      throw std::runtime_error(what.str());
    }
  }

};
std::string iota::Attribute::get_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  Serialize(writer);
  return buffer.GetString();
};

void iota::Attribute::add_metadata(const iota::Attribute& metadata) {
  _metadata.push_back(metadata);
};

void iota::Attribute::add_value_compound(const iota::Attribute& val) {
  _value_compound.push_back(val);
};

