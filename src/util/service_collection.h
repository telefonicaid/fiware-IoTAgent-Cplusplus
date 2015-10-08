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
#ifndef SRC_UTIL_SERVICE_COLLECTION_H_
#define SRC_UTIL_SERVICE_COLLECTION_H_

#include <string>
#include <vector>

#include "collection.h"
#include "device.h"

namespace iota {

class ServiceCollection : public Collection {
 public:
  ServiceCollection();

  ServiceCollection(ServiceCollection&);

  ~ServiceCollection();

  int createTableAndIndex();

  static void addServicePath(const std::string& service_path,
                             mongo::BSONObjBuilder& obj);

  virtual void getElementsFromBSON(mongo::BSONObj& obj,
                                   std::vector<mongo::BSONObj>& result);

  virtual int fill_all_resources(const std::string& service,
                                 const std::string& service_path,
                                 std::vector<std::string>& resources);

  virtual const std::string& get_resource_name();

 protected:
  // TODO comprobar con shard void fillSharKey(BSONObjBuilder &obj);

 private:
};  // end class ServiceCollection

}  // end namespace  riot

#endif
