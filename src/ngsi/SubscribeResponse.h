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
#ifndef SRC_NGSI_SUBSCRIBERESPONSE_H_
#define SRC_NGSI_SUBSCRIBERESPONSE_H_

#include "SubscribeResponse.h"
#include <rapidjson/document.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
namespace iota {
class SubscribeResponse {
 public:
  SubscribeResponse(const std::istringstream& str_subs_response);
  SubscribeResponse(){};
  SubscribeResponse(const rapidjson::Value& subs_response);
  ~SubscribeResponse(){};
  std::string get_string();
  void add_subscription_id(const std::string& subs_id);
  std::string& get_subscription_id() { return _subscriptionId; };

  template <typename Writer>
  void Serialize(Writer& writer) const {
    writer.StartObject();
    writer.String("subscribeResponse");
    writer.StartObject();
    writer.String("subscriptionId");
    writer.String(_subscriptionId.c_str(),
                  (rapidjson::SizeType)_subscriptionId.length());
    writer.EndObject();
    writer.EndObject();
  };

 private:
  std::string _subscriptionId;
};
}
#endif
