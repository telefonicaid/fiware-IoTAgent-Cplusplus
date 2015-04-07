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


iota::ProtocolCollection::ProtocolCollection():Collection(
    iota::store::types::PROTOCOL_TABLE) {
};


iota::ProtocolCollection::ProtocolCollection(ProtocolCollection& dc):Collection(
    dc) {
};

iota::ProtocolCollection::~ProtocolCollection() {
};


int iota::ProtocolCollection::createTableAndIndex() {

  int res = 200;
  ensureIndex("shardKey",
              BSON(iota::store::types::PROTOCOL_DESCRIPTION << 1),
              true);

  return res;
}

int iota::ProtocolCollection::insert(const Protocol& obj) {
  return iota::Collection::insert(Obj2BSON(obj, true));
}

int iota::ProtocolCollection::update(const Protocol& query,
                                   const Protocol& sett) {
  return iota::Collection::update(
                Obj2BSON(query, true),
                Obj2BSON(sett, true));
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
    if (!protocol.get_name().empty()) {
      obj.append(iota::store::types::PROTOCOL_NAME, protocol.get_name());
    }
  }

  if (!protocol.get_description().empty()) {
      obj.append(iota::store::types::PROTOCOL_DESCRIPTION, protocol.get_description());
    }

  return obj.obj();
}

iota::Protocol iota::ProtocolCollection::BSON2Obj(const mongo::BSONObj& obj) {

  std::string name = obj.getStringField(iota::store::types::PROTOCOL_NAME);
  std::string description = obj.getStringField(iota::store::types::PROTOCOL_DESCRIPTION);

  Protocol result(name);
  result.set_description(description);

  return result;
}

std::vector<iota::Protocol> iota::ProtocolCollection::get_all(){
  std::vector<iota::Protocol>  result;

  mongo::BSONObj query;
  mongo::BSONObjBuilder fieldsToReturn;
  fieldsToReturn.append(iota::store::types::PROTOCOL_NAME, 1);
  fieldsToReturn.append(iota::store::types::PROTOCOL_DESCRIPTION, 1);

  iota::Collection::find(a_queryOptions, query,
                           0, 0,
                           iota::store::types::PROTOCOL_NAME,
                           fieldsToReturn, 0);

  while(more()){
    result.push_back(next());
  }

  return result;
}

