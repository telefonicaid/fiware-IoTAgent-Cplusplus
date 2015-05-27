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
#include "json_util.h"

void iota::get_map_from_rapidjson(const JsonValue& value,
                            std::map<std::string, std::string>& to_map) {
  if (!value.IsObject()) {
    return;
  }
  for (JsonValue::ConstMemberIterator itr = value.MemberBegin();
       itr != value.MemberEnd(); ++itr) {
    to_map.insert(std::pair<std::string, std::string>(itr->name.GetString(),
                  get_value_from_rapidjson<std::string>(itr->value)));
  }
}

void iota::bson_to_ptree(const mongo::BSONObj& bson, boost::property_tree::ptree& p_tree) {
  std::istringstream ss(bson.jsonString(mongo::Strict));
  boost::property_tree::read_json(ss, p_tree);
}
