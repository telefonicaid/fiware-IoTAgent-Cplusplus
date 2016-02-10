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
#ifndef SRC_NGSI_ATTRIBUTEREGISTER_H_
#define SRC_NGSI_ATTRIBUTEREGISTER_H_

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <vector>
#include <string>
#include <sstream>
namespace iota {
class AttributeRegister {
 public:
  AttributeRegister(const std::string& name, const std::string& type,
                    const std::string& is_domain)
      : _name(name), _type(type), _is_domain(is_domain){};
  AttributeRegister(const std::istringstream& str_attribute);
  AttributeRegister(const rapidjson::Value& attribute);
  ~AttributeRegister(){};
  std::string get_string();
  std::string& get_name() { return _name; };
  std::string& get_type() { return _type; };
  std::string& get_is_domain() { return _is_domain; };
  template <typename Writer>
  void Serialize(Writer& writer) const {
    writer.StartObject();
    writer.String("name");
    writer.String(_name.c_str(), (rapidjson::SizeType)_name.length());
    writer.String("type");
    writer.String(_type.c_str(), (rapidjson::SizeType)_type.length());
    writer.String("isDomain");
    writer.String(_is_domain.c_str(), (rapidjson::SizeType)_is_domain.length());
    writer.EndObject();
  }

 private:
  std::string _name;
  std::string _type;
  std::string _is_domain;
};
}
#endif
