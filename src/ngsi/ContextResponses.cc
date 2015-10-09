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
#include <ngsi/ContextResponses.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <stdexcept>

iota::ContextResponses::ContextResponses(
    const std::istringstream& str_context_responses) {
  rapidjson::Document document;
  char buffer[str_context_responses.str().length()];
  strcpy(buffer, str_context_responses.str().c_str());
  if (document.ParseInsitu<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "Operation: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());
  }
  if (document.HasMember("contextResponses")) {
    const rapidjson::Value& context_responses = document["contextResponses"];
    if (context_responses.IsArray()) {
      for (rapidjson::SizeType i = 0; i < context_responses.Size(); i++) {
        iota::ContextResponse context_response(context_responses[i]);
        add_context_response(context_response);
      }
    } else {
      // TODO
    }

  } else {
    // TODO
  }
};

iota::ContextResponses::ContextResponses(
    const rapidjson::Value& context_responses) {
  if (context_responses.HasMember("contextResponses")) {
    const rapidjson::Value& context_responses =
        context_responses["contextResponses"];
    if (context_responses.IsArray()) {
      for (rapidjson::SizeType i = 0; i < context_responses.Size(); i++) {
        iota::ContextResponse context_response(context_responses[i]);
        add_context_response(context_response);
      }
    }
  }
};

std::string iota::ContextResponses::get_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  Serialize(writer);
  return buffer.GetString();
};

void iota::ContextResponses::add_context_response(
    const iota::ContextResponse& context_response) {
  _context_responses.push_back(context_response);
};

std::string iota::ContextResponses::get_message_response() {
  std::string res;

  if (_context_responses.size() == 1) {
    res.append(_context_responses.at(0).get_reason());
  } else {
    for (int i = 0; i < _context_responses.size(); i++) {
      if (i > 0) {
        // remove | because of splac
        res.append(" ");
      }
      res.append(_context_responses.at(i).get_reason());
    }
  }

  return res;
};
