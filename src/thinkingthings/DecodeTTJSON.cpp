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
#include "DecodeTTJSON.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <stdexcept>
#include <sstream>

iota::esp::tt::DecodeTTJSON::DecodeTTJSON() {}

void iota::esp::tt::DecodeTTJSON::parse(std::string& strJSON) {
  plainJSON.assign("");
  processedJSON.assign("");

  rapidjson::Document document;
  char buffer[strJSON.length()];
  strcpy(buffer, strJSON.c_str());
  if (document.ParseInsitu<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "DecodeTTJSON: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());
  }

  if (document.HasMember("processed") == false) {
    std::ostringstream what;
    what << "DecodeTTJSON: ";
    what << "missing field [";
    what << "processed]";
    throw std::runtime_error(what.str());
  } else {
    if (document["processed"].IsObject()) {
      rapidjson::StringBuffer sb;
      rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
      document["processed"].Accept(writer);
      processedJSON.assign(sb.GetString());
    }
  }
  if (document.HasMember("plain") == false) {
    std::ostringstream what;
    what << "DecodeTTJSON: ";
    what << "missing field [";
    what << "plain]";
    throw std::runtime_error(what.str());
  } else {
    if (document["plain"].IsObject()) {
      rapidjson::StringBuffer sb;
      rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
      document["plain"].Accept(writer);
      plainJSON.assign(sb.GetString());
    }
  }
}

iota::esp::tt::DecodeTTJSON::~DecodeTTJSON() {
  // dtor
}

std::string iota::esp::tt::DecodeTTJSON::getProcessedJSON() {
  return processedJSON;
}

std::string iota::esp::tt::DecodeTTJSON::getPlainJSON() { return plainJSON; }
