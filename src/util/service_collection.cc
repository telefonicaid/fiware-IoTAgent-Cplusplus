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
#include <iostream>

#include <time.h>
#include <algorithm>
#include "store_const.h"
#include "service_collection.h"

iota::ServiceCollection::ServiceCollection()
    : Collection(iota::store::types::SERVICE_TABLE){};

iota::ServiceCollection::ServiceCollection(ServiceCollection& dc)
    : Collection(dc){};

iota::ServiceCollection::~ServiceCollection(){};

void iota::ServiceCollection::addServicePath(const std::string& data,
                                             mongo::BSONObjBuilder& obj) {
  if (data.empty()) {
    obj.append(iota::store::types::SERVICE_PATH,
               iota::types::FIWARE_SERVICEPATH_DEFAULT);
  } else if (data.compare("/*") != 0 && data.compare("/#") != 0) {
    obj.append(iota::store::types::SERVICE_PATH, data);
  }
}

/*void iota::ServiceCollection::fillSharKey(mongo::BSONObjBuilder &obj)
{
    // CLAVE DE SHARD  service, name
    if (_m_device._name.empty())
    {
        throw std::runtime_error("ServiceCollection::fillSharKey name is needed
as shardKey");
    }
    obj.append("name", _m_device._name);

    if (_m_device._service.empty())
    {
        throw std::runtime_error("ServiceCollection::fillSharKey service is
needed as shardKey");
    }
    obj.append("service", _m_device._service);

}*/

const std::string& iota::ServiceCollection::get_resource_name() {
  return iota::store::types::RESOURCE;
}

int iota::ServiceCollection::fill_all_resources(
    const std::string& service, const std::string& service_path,
    std::vector<std::string>& resources) {
  int result = 0;
  mongo::BSONObj query =
      BSON(iota::store::types::SERVICE
           << service << iota::store::types::SERVICE_PATH << service_path);
  mongo::BSONObjBuilder field_return;
  field_return.append(iota::store::types::RESOURCE, 1);

  find(query, field_return);
  mongo::BSONObj obj;
  std::string resource;
  while (more()) {
    obj = next();
    resource = obj.getStringField(iota::store::types::RESOURCE);
    if (!resource.empty()) {
      resources.push_back(resource);
      result++;
    }
  }

  return result;
}

int iota::ServiceCollection::createTableAndIndex() {

  // db.SERVICE.ensureIndex({"apikey":1, resource:1},{"unique":1})
  mongo::BSONObj indexUni1 =
      BSON("apikey" << 1 << "resource" << 1);
  createIndex(indexUni1, true);

  // db.SERVICE.ensureIndex({"service":1, service_path:1,
  // resource:1},{"unique":1})
  mongo::BSONObj indexUni =
      BSON("service" << 1 << "service_path" << 1 << "resource" << 1);

  return createIndex(indexUni, true);
}

void iota::ServiceCollection::getElementsFromBSON(
    mongo::BSONObj& obj, std::vector<mongo::BSONObj>& result) {
  std::vector<mongo::BSONElement> be =
      obj.getField(iota::store::types::SERVICES).Array();
  for (unsigned int i = 0; i < be.size(); i++) {
    result.push_back(be[i].embeddedObject());
  }
}
