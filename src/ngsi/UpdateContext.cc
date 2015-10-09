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
#include <ngsi/UpdateContext.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <stdexcept>

iota::UpdateContext::UpdateContext(const std::istringstream& str_operation) {
  rapidjson::Document document;
  char buffer[str_operation.str().length()];
  // memcpy(buffer, str_attribute.c_str(), str_attribute.length());
  strcpy(buffer, str_operation.str().c_str());
  if (document.ParseInsitu<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "UpdateContext: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());
  }

  if (document.HasMember("updateAction") == false) {
    std::ostringstream what;
    what << "UpdateContext: ";
    what << "missing field [";
    what << "updateAction]";
    throw std::runtime_error(what.str());
  }
  _action.assign(document["updateAction"].GetString());
  if (document.HasMember("contextElements")) {
    const rapidjson::Value& context_elements = document["contextElements"];
    if (context_elements.IsArray()) {
      for (rapidjson::SizeType i = 0; i < context_elements.Size(); i++) {
        iota::ContextElement context_element(context_elements[i]);
        add_context_element(context_element);
      }
    } else {
      std::ostringstream what;
      what << "UpdateContext: ";
      what << "invalid type field [";
      what << "contextElements]";
      throw std::runtime_error(what.str());
    }

  } else {
    std::ostringstream what;
    what << "UpdateContext: ";
    what << "missing field [";
    what << "contextElements]";
    throw std::runtime_error(what.str());
  }
};

iota::UpdateContext::UpdateContext(const rapidjson::Value& operation) {
  if (!operation.IsObject()) {
    throw std::runtime_error("Invalid object");
  }

  if (operation.HasMember("updateAction") == false) {
    std::ostringstream what;
    what << "UpdateContext: ";
    what << "missing field [";
    what << "updateAction]";
    throw std::runtime_error(what.str());
  }
  _action.assign(operation["updateAction"].GetString());
  if (operation.HasMember("contextElements")) {
    const rapidjson::Value& context_elements = operation["contextElements"];
    if (context_elements.IsArray()) {
      for (rapidjson::SizeType i = 0; i < context_elements.Size(); i++) {
        iota::ContextElement context_element(context_elements[i]);
        add_context_element(context_element);
      }
    } else {
      std::ostringstream what;
      what << "UpdateContext: ";
      what << "invalid type field [";
      what << "contextElements]";
      throw std::runtime_error(what.str());
    }
  } else {
    std::ostringstream what;
    what << "UpdateContext: ";
    what << "missing field [";
    what << "contextElements]";
    throw std::runtime_error(what.str());
  }
};

std::string iota::UpdateContext::get_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  Serialize(writer);
  return buffer.GetString();
};

void iota::UpdateContext::add_context_element(
    const iota::ContextElement& context_element) {
  _context_elements.push_back(context_element);
};
