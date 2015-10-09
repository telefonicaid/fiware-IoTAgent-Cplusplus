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
#ifndef SRC_UTIL_JSON_UTIL_H_
#define SRC_UTIL_JSON_UTIL_H_

#include "common.h"
#include "iota_exception.h"
#include "rest/types.h"
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <mongo/bson/bson.h>
#include <map>

namespace iota {

template <typename ReturnType>
ReturnType get_value_from_rapidjson(const JsonValue& value) {
  if (value.IsBool() || value.IsTrue() || value.IsFalse()) {
    return boost::lexical_cast<ReturnType>(value.GetBool());
  } else if (value.IsInt() || value.IsUint() || value.IsInt64() ||
             value.IsUint64()) {
    return boost::lexical_cast<ReturnType>(value.GetInt64());
  } else if (value.IsString()) {
    return boost::lexical_cast<ReturnType>(value.GetString());
  } else if (value.IsDouble()) {
    std::ostringstream os;
    os << value.GetDouble();
    return boost::lexical_cast<ReturnType>(os.str());
  } else {
    throw iota::IotaException(
        iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER,
        "Value " + boost::lexical_cast<std::string>(value.GetType()) +
            " unknown for conversion [boolean, number, string] are valid types",
        iota::types::RESPONSE_CODE_BAD_REQUEST);
  }
};

template <typename ReturnType>
ReturnType get_value_from_rapidjson(const rapidjson::Value& value) {
  if (value.IsBool() || value.IsTrue() || value.IsFalse()) {
    return boost::lexical_cast<ReturnType>(value.GetBool());
  } else if (value.IsInt() || value.IsUint() || value.IsInt64() ||
             value.IsUint64()) {
    return boost::lexical_cast<ReturnType>(value.GetInt64());
  } else if (value.IsString()) {
    return boost::lexical_cast<ReturnType>(value.GetString());
  } else if (value.IsDouble()) {
    std::ostringstream os;
    os << value.GetDouble();
    return boost::lexical_cast<ReturnType>(os.str());
  } else {
    throw iota::IotaException(
        iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER,
        "Value " + boost::lexical_cast<std::string>(value.GetType()) +
            " unknown for conversion [boolean, number, string] are valid types",
        iota::types::RESPONSE_CODE_BAD_REQUEST);
  }
};
/**
 * Convert object Json (with simple fields) into std::string map. A field can be
 * get by map["field"].
 * @param value from rapidjson with simple fields (string, boolean, number).
 * @param map<std::string, std::string> where result will be stored.
 */
void get_map_from_rapidjson(const JsonValue& value,
                            std::map<std::string, std::string>& to_map);

/**
 * Convert a mongo BSON to boost::property_tree::ptree
 * @param bson is a mongo::BSONObj.
 * @param p_tree is a boost::property_tree::ptree where result will be stored
 */
void bson_to_ptree(const mongo::BSONObj& bson,
                   boost::property_tree::ptree& p_tree);

/**
 * Convert a rapidjson::Value to string
 * @param attribute is a rapidjson::Value (string, array, null, ).
 * @param p_tree is a boost::property_tree::ptree where result will be stored
 */
std::string get_str_value(const rapidjson::Value& attribute);
};

#endif
