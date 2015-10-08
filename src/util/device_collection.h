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
#ifndef SRC_UTIL_DEVICE_COLLECTION_H_
#define SRC_UTIL_DEVICE_COLLECTION_H_

#include <string>
#include <vector>

#include "collection.h"
#include "device.h"

namespace iota {

class DeviceCollection : public Collection {
 public:
  DeviceCollection();

  DeviceCollection(DeviceCollection&);

  ~DeviceCollection();

  int insertd(const Device& obj);
  int findd(const Device& query);
  Device nextd();
  int removed(const Device& query);
  int countd(const Device& query);

  int updated(const Device& query, const Device& sett, bool upsert = false);

  int createTableAndIndex();

 protected:
  Device BSON2Obj(mongo::BSONObj& obj);

  mongo::BSONObj Obj2BSON(const Device& query, bool withShardKey);

  void BSON2Map(mongo::BSONObj& obj, const std::string& id,
                std::map<std::string, std::string>& datamap);
  void BSON2MapAttribute(mongo::BSONObj& obj, const std::string& id,
                         std::map<std::string, std::string>& datamap,
                         std::string key_map);

  void Map2BSON(mongo::BSONObjBuilder& obj, const std::string& fieldName,
                const std::map<std::string, std::string>& datamap);

  void Map2BSONAttribute(mongo::BSONObjBuilder& obj,
                         const std::string& fieldName,
                         const std::map<std::string, std::string>& datamap,
                         std::string key_map);

  // TODO comprobar con shard void fillSharKey(BSONObjBuilder &obj);

 private:
};  // end class DeviceCollection

}  // end namespace  riot

#endif
