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
#ifndef SRC_UTIL_PROTOCOL_COLLECTION_H_
#define SRC_UTIL_PROTOCOL_COLLECTION_H_

#include <string>
#include <vector>

#include "collection.h"
#include "protocol.h"

namespace iota {

class ProtocolCollection : public Collection {
 public:
  ProtocolCollection();

  ProtocolCollection(ProtocolCollection&);

  ~ProtocolCollection();

  int createTableAndIndex();

  int insert(const Protocol& obj);
  int find(const Protocol& query);
  Protocol next();
  int remove(const Protocol& query);
  int count(const Protocol& query);

  int update(const Protocol& query, const Protocol& sett);

  std::vector<iota::Protocol> get_all();

  /**
   *  in protocols fills with protocol name and protocol description
   **/
  void fillProtocols(std::map<std::string, std::string>& protocols);

  std::vector<iota::Protocol> get_endpoint_by_protocol(std::string protocol);

 protected:
  iota::Protocol BSON2Obj(const mongo::BSONObj& obj);

  mongo::BSONObj Obj2BSON(const Protocol& protocol, bool withShardKey);

 private:
};  // end class ProtocolCollection

}  // end namespace  iota

#endif
