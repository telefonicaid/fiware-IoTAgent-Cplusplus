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
#ifndef SRC_NGSI_CONTEXTRESPONSE_H_
#define SRC_NGSI_CONTEXTRESPONSE_H_

#include "ContextElement.h"
#include <rapidjson/rapidjson.h>
#include <vector>
#include <string>
#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace iota {
class ContextResponse {
 public:
  ContextResponse();
  ContextResponse(const std::istringstream& str_operation);
  ContextResponse(const std::string& status_code,
                  const std::string& reason_phrase)
      : _code(status_code), _reason(reason_phrase){};
  ContextResponse(const rapidjson::Value& context_response);
  ~ContextResponse(){};
  std::string get_string();
  void add_context_element(const ContextElement& context_response);
  std::string& get_code() { return _code; };
  void set_code(const std::string& code) { _code = code; };
  void set_code(const int icode) {
    _code = boost::lexical_cast<std::string>(icode);
  };

  std::string& get_reason() { return _reason; };
  void set_reason(const std::string& reason) {
    _reason = boost::erase_all_copy(reason, "\"");
  };

  std::string& get__details() { return _details; };
  void set_details(const std::string& details) {
    _details = boost::erase_all_copy(details, "\"");
  };

  ContextElement& get_context_element() { return _context_element; };

  template <typename Writer>
  void Serialize(Writer& writer) const {
    writer.StartObject();
    writer.String("statusCode");
    writer.StartObject();
    writer.String("code");
    writer.String(_code.c_str(), (rapidjson::SizeType)_code.length());
    writer.String("reasonPhrase");
    writer.String(_reason.c_str(), (rapidjson::SizeType)_reason.length());
    writer.String("details");
    writer.String(_details.c_str(), (rapidjson::SizeType)_details.length());
    writer.EndObject();

    writer.String("contextElement");
    _context_element.Serialize(writer);
    writer.EndObject();
  };

 private:
  std::string _code;
  std::string _reason;
  std::string _details;
  ContextElement _context_element;
};
}
#endif
