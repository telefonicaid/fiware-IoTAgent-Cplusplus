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
#include "SubscribeResponse.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <stdexcept>

iota::SubscribeResponse::SubscribeResponse(
    const std::istringstream& str_subs_response) {
  rapidjson::Document document;
  char buffer[str_subs_response.str().length()];
  // memcpy(buffer, str_attribute.c_str(), str_attribute.length());
  strcpy(buffer, str_subs_response.str().c_str());
  if (document.Parse<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "SubscribeResponse: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());
  }

  if (document.HasMember("subscribeResponse")) {
    if ((document["subscribeResponse"].HasMember("subscriptionId")) &&
        (document["subscribeResponse"]["subscriptionId"].IsString())) {
      _subscriptionId.assign(
          document["subscribeResponse"]["subscriptionId"].GetString());
    }
  }
  if (_subscriptionId.empty()) {
    std::ostringstream what;
    what << "SubscribeResponse: ";
    what << "[subscriptionId]";
    what << "Missing field";
    throw std::runtime_error(what.str());
  }
}

iota::SubscribeResponse::SubscribeResponse(
    const rapidjson::Value& subs_response) {
  if (subs_response["subscribeResponse"].HasMember("subscriptionId")) {
    _subscriptionId.assign(
        subs_response["subscribeResponse"]["subscriptionId"].GetString());
  }
};
std::string iota::SubscribeResponse::get_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  Serialize(writer);
  return buffer.GetString();
};

void iota::SubscribeResponse::add_subscription_id(const std::string& subs_id) {
  _subscriptionId = subs_id;
};
