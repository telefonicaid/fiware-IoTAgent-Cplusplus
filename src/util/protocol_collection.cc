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
#include "protocol_collection.h"

iota::ProtocolCollection::ProtocolCollection()
    : Collection(iota::store::types::PROTOCOL_TABLE){};

iota::ProtocolCollection::ProtocolCollection(ProtocolCollection& dc)
    : Collection(dc){};

iota::ProtocolCollection::~ProtocolCollection(){};

int iota::ProtocolCollection::createTableAndIndex() {

  // db.PROTOCOL.ensureIndex({protocol:1},{"unique":1})
  mongo::BSONObj indexUni = BSON(iota::store::types::PROTOCOL_NAME << 1);

  return createIndex(indexUni, true);
}

int iota::ProtocolCollection::insert(const Protocol& obj) {
  return iota::Collection::insert(Obj2BSON(obj, true));
}

int iota::ProtocolCollection::update(const Protocol& query,
                                     const Protocol& sett) {
  return iota::Collection::update(Obj2BSON(query, true), Obj2BSON(sett, true));
}

int iota::ProtocolCollection::find(const Protocol& query) {
  return iota::Collection::find(Obj2BSON(query, true));
}

iota::Protocol iota::ProtocolCollection::next() {
  mongo::BSONObj data = iota::Collection::next();
  return BSON2Obj(data);
}

int iota::ProtocolCollection::remove(const Protocol& query) {
  return iota::Collection::remove(Obj2BSON(query, true));
}

int iota::ProtocolCollection::count(const Protocol& query) {
  return iota::Collection::count(Obj2BSON(query, true));
}

mongo::BSONObj iota::ProtocolCollection::Obj2BSON(const Protocol& protocol,
                                                  bool withShardKey) {
  mongo::BSONObjBuilder obj;
  if (withShardKey) {
    if (!protocol.get_id().empty()) {
      obj.append(iota::store::types::PROTOCOL_ID, protocol.get_id());
    }
  }

  if (!protocol.get_name().empty()) {
    obj.append(iota::store::types::PROTOCOL_NAME, protocol.get_name());
  }

  if (!protocol.get_description().empty()) {
    obj.append(iota::store::types::PROTOCOL_DESCRIPTION,
               protocol.get_description());
  }

  const iota::Protocol::resource_endpoint_vector& endpoints =
      protocol.get_endpoints();
  mongo::BSONArrayBuilder arr;
  int count_endpoints = 0;
  for (std::vector<iota::Protocol::resource_endpoint>::const_iterator it =
           endpoints.begin();
       it != endpoints.end(); ++it) {
    arr.append(BSON(iota::store::types::ENDPOINT << it->endpoint
                                                 << iota::store::types::RESOURCE
                                                 << it->resource));
    count_endpoints++;
  }

  if (count_endpoints > 0) {
    obj.append(iota::store::types::ENDPOINTS, arr.arr());
  }

  return obj.obj();
}

iota::Protocol iota::ProtocolCollection::BSON2Obj(const mongo::BSONObj& obj) {
  mongo::BSONElement oi;
  obj.getObjectID(oi);
  mongo::OID o = oi.__oid();

  std::string name = obj.getStringField(iota::store::types::PROTOCOL_NAME);
  std::string description =
      obj.getStringField(iota::store::types::PROTOCOL_DESCRIPTION);

  Protocol result(name);
  result.set_description(description);

  mongo::BSONObj endpoints = obj.getObjectField(iota::store::types::ENDPOINTS);

  mongo::BSONObjIterator it(endpoints);
  while (it.more()) {
    mongo::BSONObj e = it.next().Obj();
    iota::Protocol::resource_endpoint endp;

    endp.endpoint = e.getStringField(iota::store::types::ENDPOINT);
    endp.resource = e.getStringField(iota::store::types::RESOURCE);
    result.add(endp);
  }
  return result;
}

std::vector<iota::Protocol> iota::ProtocolCollection::get_all() {
  std::vector<iota::Protocol> result;

  mongo::BSONObj query;
  mongo::BSONObjBuilder fieldsToReturn;
  fieldsToReturn.append(iota::store::types::PROTOCOL_NAME, 1);
  fieldsToReturn.append(iota::store::types::PROTOCOL_DESCRIPTION, 1);

  mongo::BSONObj fieldsSort = BSON(iota::store::types::PROTOCOL_NAME << 1);

  iota::Collection::find(a_queryOptions, query, 0, 0, fieldsSort,
                         fieldsToReturn, 0);

  while (more()) {
    result.push_back(next());
  }

  return result;
}

std::vector<iota::Protocol> iota::ProtocolCollection::get_endpoint_by_protocol(
    std::string protocol) {
  std::vector<iota::Protocol> result;

  mongo::BSONObj query = BSON(iota::store::types::PROTOCOL_NAME << protocol);
  iota::Collection::find(query);

  while (more()) {
    result.push_back(next());
  }

  return result;
}

void iota::ProtocolCollection::fillProtocols(
    std::map<std::string, std::string>& protocols) {
  mongo::BSONObj query;
  mongo::BSONObjBuilder fieldsToReturn;
  fieldsToReturn.append(iota::store::types::PROTOCOL_NAME, 1);
  fieldsToReturn.append(iota::store::types::PROTOCOL_DESCRIPTION, 1);

  mongo::BSONObj fieldsSort = BSON(iota::store::types::PROTOCOL_NAME << 1);

  iota::Collection::find(a_queryOptions, query, 0, 0, fieldsSort,
                         fieldsToReturn, 0);

  while (more()) {
    Protocol p = next();
    protocols.insert(
        std::pair<std::string, std::string>(p.get_name(), p.get_description()));
  }
}
