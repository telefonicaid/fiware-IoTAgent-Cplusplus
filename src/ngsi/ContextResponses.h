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
#ifndef SRC_NGSI_CONTEXTRESPONSES_H_
#define SRC_NGSI_CONTEXTRESPONSES_H_

#include "ContextResponse.h"
#include <rapidjson/rapidjson.h>
#include <vector>
#include <string>
namespace iota {
class ContextResponses {
 public:
  ContextResponses(const std::istringstream& str_context_responses);
  ContextResponses(){};
  ContextResponses(const rapidjson::Value& context_responses);
  ~ContextResponses(){};

  std::string get_string();
  std::string get_message_response();

  void add_context_response(const ContextResponse& context_response);
  std::vector<ContextResponse> get_context_responses() {
    return _context_responses;
  };

  template <typename Writer>
  void Serialize(Writer& writer) const {
    writer.StartObject();
    writer.String("contextResponses");
    writer.StartArray();
    for (std::vector<ContextResponse>::const_iterator it =
             _context_responses.begin();
         it != _context_responses.end(); ++it) {
      it->Serialize(writer);
    }
    writer.EndArray();
    writer.EndObject();
  };

 private:
  std::vector<ContextResponse> _context_responses;
};
}
#endif
