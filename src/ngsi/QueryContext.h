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
#ifndef SRC_NGSI_QUERYCONTEXT_H_
#define SRC_NGSI_QUERYCONTEXT_H_

#include <ngsi/Entity.h>
#include <rapidjson/rapidjson.h>
#include <vector>
#include <string>
namespace iota {
class QueryContext {
 public:
  QueryContext(const std::istringstream& str_query);
  QueryContext(){};
  QueryContext(const rapidjson::Value& query);
  ~QueryContext(){};
  std::string get_string();
  void add_entity(const Entity& entity);
  void add_attribute(const std::string& attribute);
  std::vector<Entity>& get_entities() { return _entities; };

  std::vector<std::string>& get_attributes() { return _attributes; };

  template <typename Writer>
  void Serialize(Writer& writer) const {
    writer.StartObject();
    writer.String("entities");
    writer.StartArray();
    for (std::vector<Entity>::const_iterator it = _entities.begin();
         it != _entities.end(); ++it) {
      it->Serialize(writer);
    }
    writer.EndArray();
    if (_attributes.size() > 0) {
      writer.String("attributes");
      writer.StartArray();
      for (std::vector<std::string>::const_iterator it = _attributes.begin();
           it != _attributes.end(); ++it) {
        writer.String((*it).c_str(), (rapidjson::SizeType)(*it).length());
      }
      writer.EndArray();
    }
    writer.EndObject();
  };

 private:
  std::vector<Entity> _entities;
  std::vector<std::string> _attributes;
};
}
#endif
