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
#ifndef SRC_NGSI_UPDATECONTEXT_H_
#define SRC_NGSI_UPDATECONTEXT_H_

#include <ngsi/ContextElement.h>
#include <rapidjson/rapidjson.h>
#include <vector>
#include <string>
namespace iota {
class UpdateContext {
 public:
  UpdateContext(const std::istringstream& str_UpdateContext);
  UpdateContext(const std::string& action) : _action(action){};
  UpdateContext(const rapidjson::Value& UpdateContext);
  ~UpdateContext(){};
  std::string get_string();
  void add_context_element(const ContextElement& attribute);
  std::string& get_action() { return _action; };
  std::vector<ContextElement> get_context_elements() {
    return _context_elements;
  };

  template <typename Writer>
  void Serialize(Writer& writer) const {
    writer.StartObject();
    writer.String("updateAction");
    writer.String(_action.c_str(), (rapidjson::SizeType)_action.length());
    writer.String("contextElements");
    writer.StartArray();
    for (std::vector<ContextElement>::const_iterator it =
             _context_elements.begin();
         it != _context_elements.end(); ++it) {
      it->Serialize(writer);
    }
    writer.EndArray();
    writer.EndObject();
  };

 private:
  std::string _action;
  std::vector<ContextElement> _context_elements;
};
}
#endif
