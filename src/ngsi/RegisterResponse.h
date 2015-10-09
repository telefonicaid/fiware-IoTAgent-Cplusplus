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
#ifndef SRC_NGSI_REGISTERRESPONSE_H_
#define SRC_NGSI_REGISTERRESPONSE_H_

#include "RegisterResponse.h"
#include <rapidjson/document.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
namespace iota {
class RegisterResponse {
 public:
  RegisterResponse(const std::istringstream& str_reg_response);
  RegisterResponse(){};
  RegisterResponse(const rapidjson::Value& reg_response);
  ~RegisterResponse(){};
  std::string get_string();
  void add_registration_id(const std::string& reg_id);
  std::string& get_registration_id() { return _registrationId; };

  void add_duration(const std::string& data);
  std::string& get_duration() { return _duration; };

  template <typename Writer>
  void Serialize(Writer& writer) const {
    writer.StartObject();
    writer.String("registerResponse");
    writer.StartObject();
    writer.String("registrationId");
    writer.String(_registrationId.c_str(),
                  (rapidjson::SizeType)_registrationId.length());
    writer.EndObject();
    writer.StartObject();
    writer.String("duration");
    writer.String(_duration.c_str(), (rapidjson::SizeType)_duration.length());
    writer.EndObject();
    writer.EndObject();
  };

 private:
  std::string _registrationId;
  std::string _duration;
};
}
#endif
