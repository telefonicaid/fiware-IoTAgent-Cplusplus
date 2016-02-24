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
#ifndef SRC_NGSI_ATTRIBUTE_H_
#define SRC_NGSI_ATTRIBUTE_H_

#include "util/FuncUtil.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <vector>
#include <string>
#include <sstream>
namespace iota {
class Attribute {
 public:
  Attribute(const std::string& name, const std::string& type,
            const std::string& value);
  // Constructor without value for compound attributes
  Attribute(const std::string& name, const std::string& type);
  Attribute(const std::istringstream& str_attribute);
  Attribute(const rapidjson::Value& attribute);
  Attribute();
  ~Attribute(){};
  std::string get_string();
  std::string& get_name() { return _name; };
  std::string& get_type() { return _type; };
  std::string& get_value() { return _value; };
  bool compound_object() { return _compound_as_object;};
  void set_value(std::string& value) { _value = value; };
  void set_name(std::string& name) { _name = iota::render_identifier(name); };
  void set_type(std::string& type) { _type = iota::render_identifier(type); };
  std::vector<Attribute>& get_metadatas() { return _metadata; };
  void add_metadata(const Attribute& metadata);
  std::vector<Attribute>& get_value_compound() { return _value_compound; };
  void add_value_compound(Attribute& val, bool compound_object = false);

  std::vector<Attribute>& get_compound_value() { return _value_compound; };

  template <typename Writer>
  void Serialize(Writer& writer) const {
    writer.StartObject();
    writer.String("name");
    writer.String(_name.c_str(), (rapidjson::SizeType)_name.length());
    writer.String("type");
    writer.String(_type.c_str(), (rapidjson::SizeType)_type.length());
    writer.String("value");
    if (_type != "compound") {
      writer.String(_value.c_str(), (rapidjson::SizeType)_value.length());
    } else {
      if (_value_compound.size() > 0) {
        writer.StartArray();
        for (std::vector<Attribute>::const_iterator it =
                 _value_compound.begin();
             it != _value_compound.end(); ++it) {
          it->Serialize(writer);
        }
        writer.EndArray();
      } else {
        std::string v;
        if (_value.empty()) {
          v.assign("{}");
        } else {
          v.assign(_value);
        }
        rapidjson::Document doc_value;
        char buffer[v.length()];
        strcpy(buffer, v.c_str());
        if (!doc_value.Parse<0>(buffer).HasParseError()) {
          doc_value.Accept(writer);
        }
      }
    }
    if (_metadata.size() != 0) {
      writer.String("metadatas");
      writer.StartArray();
      for (std::vector<Attribute>::const_iterator it = _metadata.begin();
           it != _metadata.end(); ++it) {
        it->Serialize(writer);
      }
      writer.EndArray();
    }
    writer.EndObject();
  }

 private:
  std::string _name;
  std::string _type;
  std::string _value;
  std::vector<Attribute> _metadata;
  std::vector<Attribute> _value_compound;
  bool _compound_as_object;
};
}
#endif
