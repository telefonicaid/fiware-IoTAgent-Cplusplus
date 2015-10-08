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
#ifndef SRC_NGSI_ENTITY_H_
#define SRC_NGSI_ENTITY_H_

#include <ngsi/AttributeRegister.h>
#include <rapidjson/rapidjson.h>
#include <vector>
#include <string>
namespace iota {
class Entity {
 public:
  Entity(const std::string& id, const std::string& type,
         const std::string& is_pattern)
      : _id(id), _type(type), _is_pattern(is_pattern){};
  Entity(const std::istringstream& str_entity);
  Entity(const rapidjson::Value& entity);
  Entity(){};
  ~Entity(){};
  std::string get_string();
  std::string& get_id() { return _id; };
  std::string& get_type() { return _type; };
  std::string& get_is_pattern() { return _is_pattern; };

  void set_id(const std::string& id) { _id = id; };
  void set_type(const std::string& type) { _type = type; };

  template <typename Writer>
  void Serialize(Writer& writer) const {
    writer.StartObject();
    writer.String("id");
    writer.String(_id.c_str(), (rapidjson::SizeType)_id.length());
    writer.String("type");
    writer.String(_type.c_str(), (rapidjson::SizeType)_type.length());
    writer.String("isPattern");
    writer.String(_is_pattern.c_str(),
                  (rapidjson::SizeType)_is_pattern.length());
    writer.EndObject();
  };

 private:
  std::string _id;
  std::string _type;
  std::string _is_pattern;
};
}
#endif
