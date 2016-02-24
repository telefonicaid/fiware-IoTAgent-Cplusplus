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
#include "util/json_util.h"

#define EMPTY_VALUE " "

iota::Attribute::Attribute(const std::string& name, const std::string& type,
                           const std::string& value): _compound_as_object(false) {
  if (name.empty()) {
    _name = EMPTY_VALUE;
  } else {
    _name = iota::render_identifier(name);
  }

  _type = iota::render_identifier(type);

  if (value.empty()) {
    _value = EMPTY_VALUE;
  } else {
    _value = value;
  }
}

iota::Attribute::Attribute(const std::string& name, const std::string& type):_compound_as_object(false) {
  if (name.empty()) {
    _name = EMPTY_VALUE;
  } else {
    _name = iota::render_identifier(name);
  }

  _type = iota::render_identifier(type);
}

iota::Attribute::Attribute(const std::istringstream& str_attribute):_compound_as_object(false) {
  rapidjson::Document document;
  char buffer[str_attribute.str().length()];
  strcpy(buffer, str_attribute.str().c_str());
  if (document.Parse<0>(buffer).HasParseError()) {
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
  _name.assign(iota::render_identifier(document["name"].GetString()));
  if (_name.empty()) {
    _name = EMPTY_VALUE;
  }

  if (document.HasMember("type")) {
    _type.assign(iota::render_identifier(document["type"].GetString()));
  }
  if (document.HasMember("value")) {
    if (_type == "compound") {
      const rapidjson::Value& data = document["value"];
      if (data.IsArray()) {
        for (rapidjson::SizeType i = 0; i < data.Size(); i++) {
          iota::Attribute val(data[i]["name"].GetString(),
                              data[i]["type"].GetString(),
                              data[i]["value"].GetString());
          add_value_compound(val);
        }
      } else {
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        data.Accept(writer);
        _value = sb.GetString();
        _compound_as_object = true;
      }

    } else if (_type == "command" && document["value"].IsObject()) {
      rapidjson::StringBuffer sb;
      rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
      document["value"].Accept(writer);
      _value = sb.GetString();

    } else {
      _value.assign(document["value"].GetString());
      if (_value.empty()) {
        // CB does not allow empty fields, so we add a space
        _value = EMPTY_VALUE;
      }
    }
  } else {
    _value = EMPTY_VALUE;
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
    } else {
      std::ostringstream what;
      what << "Attribute: ";
      what << "invalid type field [";
      what << "metadatas]";
      throw std::runtime_error(what.str());
    }
  }
};

iota::Attribute::Attribute(const rapidjson::Value& attribute):_compound_as_object(false) {
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
  _name.assign(iota::render_identifier(attribute["name"].GetString()));
  if (attribute.HasMember("type")) {
    _type.assign(iota::render_identifier(attribute["type"].GetString()));
  }
  if (attribute.HasMember("value")) {
    if (_type == "compound") {
      const rapidjson::Value& data = attribute["value"];
      if (data.IsArray()) {
        for (rapidjson::SizeType i = 0; i < data.Size(); i++) {
          iota::Attribute val(data[i]["name"].GetString(),
                              data[i]["type"].GetString(),
                              data[i]["value"].GetString());
          add_value_compound(val);
        }
      } else {
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        data.Accept(writer);
        _value = sb.GetString();
        _compound_as_object = true;
      }
    } else if (_type == "command" && attribute["value"].IsObject()) {
      rapidjson::StringBuffer sb;
      rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
      attribute["value"].Accept(writer);
      _value = sb.GetString();

    } else {
      const rapidjson::Value& data = attribute["value"];
      _value.assign(iota::get_str_value(data));
      if (_value.empty()) {
        _value = EMPTY_VALUE;
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
    } else {
      std::ostringstream what;
      what << "Attribute: ";
      what << "invalid type field [";
      what << "metadatas]";
      throw std::runtime_error(what.str());
    }
  }
};

iota::Attribute::Attribute(): _compound_as_object(false) {}
std::string iota::Attribute::get_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  Serialize(writer);
  return buffer.GetString();
};

void iota::Attribute::add_metadata(const iota::Attribute& metadata) {
  _metadata.push_back(metadata);
};

void iota::Attribute::add_value_compound(iota::Attribute& val,
                                         bool compound_object) {
  if (compound_object == false) {
    _value_compound.push_back(val);
  } else {
    _compound_as_object = true;
    std::string v(val.get_value());
    _value.assign(v);
  }
};
