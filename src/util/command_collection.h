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
#ifndef SRC_UTIL_COMMAND_COLLECTION_H__
#define SRC_UTIL_COMMAND_COLLECTION_H__

#include <string>

#include "collection.h"
#include "command.h"

namespace iota {

class CommandCollection : public Collection {
 public:
  CommandCollection();
  CommandCollection(CommandCollection&);
  ~CommandCollection();

  int insert(const Command& obj);
  int find(const std::string& id);
  int find(const Command& query);
  Command next();
  int remove(const Command& query);
  int count(const Command& query);

  int createTableAndIndex();

 protected:
  Command BSON2Obj(const mongo::BSONObj& obj);

  mongo::BSONObj Obj2BSON(const Command& command, bool withShardKey);

 private:
};  // end class GenericCollection

}  // end namespace  riot

#endif
