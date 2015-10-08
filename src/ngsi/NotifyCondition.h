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
#ifndef SRC_NGSI_NOTIFYCONDITION_H_
#define SRC_NGSI_NOTIFYCONDITION_H_

#include <rapidjson/document.h>
#include <vector>
#include <string>
#include <sstream>
namespace iota {
class NotifyCondition {
 public:
  NotifyCondition(const std::string& type) : _type(type){};
  NotifyCondition(const std::istringstream& str_cond);
  NotifyCondition(const rapidjson::Value& cond);
  NotifyCondition(){};
  ~NotifyCondition(){};
  std::string get_string();
  std::string& get_type() { return _type; };
  std::vector<std::string>& get_values() { return _values; };
  void add_value(const std::string& value);
  template <typename Writer>
  void Serialize(Writer& writer) const {
    writer.StartObject();
    writer.String("type");
    writer.String(_type.c_str(), (rapidjson::SizeType)_type.length());
    writer.String("condValues");
    writer.StartArray();
    for (std::vector<std::string>::const_iterator it = _values.begin();
         it != _values.end(); ++it) {
      writer.String((*it).c_str(), (rapidjson::SizeType)(*it).length());
    }
    writer.EndArray();
    writer.EndObject();
  };

 private:
  std::vector<std::string> _values;
  std::string _type;
};
}
#endif
