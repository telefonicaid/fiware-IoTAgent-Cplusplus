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
#ifndef SRC_NGSI_REGISTERCONTEXT_H_
#define SRC_NGSI_REGISTERCONTEXT_H_

#include "ngsi/ContextRegistration.h"
#include <rapidjson/rapidjson.h>
#include <vector>
#include <string>
namespace iota {
class RegisterContext {
 public:
  RegisterContext(const std::istringstream& str_register);
  RegisterContext(){};
  RegisterContext(const rapidjson::Value& reg);
  ~RegisterContext(){};
  std::string get_string();
  std::string get_duration();
  std::string get_registrationId();
  void add_duration(const std::string& duration);
  void add_registrationId(const std::string& registrationId);
  void add_context_registration(const ContextRegistration& ctx_reg);
  std::vector<ContextRegistration> get_context_registrations() {
    return _context_registrations;
  };

  template <typename Writer>
  void Serialize(Writer& writer) const {
    writer.StartObject();
    writer.String("contextRegistrations");
    writer.StartArray();
    for (std::vector<ContextRegistration>::const_iterator it =
             _context_registrations.begin();
         it != _context_registrations.end(); ++it) {
      it->Serialize(writer);
    }
    writer.EndArray();
    if (_duration.empty() == false) {
      writer.String("duration");
      writer.String(_duration.c_str(), (rapidjson::SizeType)_duration.length());
    }
    if (_registrationId.empty() == false) {
      writer.String("registrationId");
      writer.String(_registrationId.c_str(),
                    (rapidjson::SizeType)_registrationId.length());
    }

    writer.EndObject();
  };

 private:
  std::vector<ContextRegistration> _context_registrations;
  std::string _duration;
  std::string _registrationId;
};
}
#endif
