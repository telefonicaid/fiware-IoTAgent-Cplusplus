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

mongo::BSONObj iota::ProtocolCollection::Obj2BSON( const Protocol& protocol,
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
      obj.append(iota::store::types::PROTOCOL_DESCRIPTION, protocol.get_description());
    }

  const iota::Protocol::resource_endpoint_vector &endpoints =  protocol.get_endpoints();
  mongo::BSONArrayBuilder arr;
  int count_endpoints=0;
  for(std::vector<iota::Protocol::resource_endpoint>::const_iterator it = endpoints.begin(); it != endpoints.end(); ++it) {
    arr.append(BSON(iota::store::types::ENDPOINT << it->endpoint <<
                    iota::store::types::RESOURCE << it->resource));
    count_endpoints++;
  }

  if (count_endpoints > 0){
    obj.append(iota::store::types::ENDPOINTS, arr.arr());
  }

  return obj.obj();
}

iota::Protocol iota::ProtocolCollection::BSON2Obj(const mongo::BSONObj& obj) {

  mongo::BSONElement oi;
  obj.getObjectID(oi);
  mongo::OID o = oi.__oid();
  std::string id = o.toString();
  std::string name = obj.getStringField(iota::store::types::PROTOCOL_NAME);
  std::string description = obj.getStringField(iota::store::types::PROTOCOL_DESCRIPTION);

  Protocol result(name);
  result.set_description(description);

  mongo::BSONObj endpoints = obj.getObjectField (iota::store::types::ENDPOINTS);

  mongo::BSONObjIterator it(endpoints);
  while ( it.more() ) {
    mongo::BSONObj e = it.next().Obj();
    iota::Protocol::resource_endpoint endp;

    endp.endpoint = e.getStringField(iota::store::types::ENDPOINT);
    endp.resource = e.getStringField(iota::store::types::RESOURCE);
    result.add(endp);
  }
  return result;
}

std::vector<iota::Protocol> iota::ProtocolCollection::get_all(){
  std::vector<iota::Protocol>  result;

  mongo::BSONObj query;
  mongo::BSONObjBuilder fieldsToReturn;
  fieldsToReturn.append(iota::store::types::PROTOCOL_NAME, 1);
  fieldsToReturn.append(iota::store::types::PROTOCOL_DESCRIPTION, 1);

  mongo::BSONObj fieldsSort = BSON(iota::store::types::PROTOCOL_NAME << 1);

  iota::Collection::find(a_queryOptions, query,
                           0, 0,
                           fieldsSort,
                           fieldsToReturn, 0);

  while(more()){
    result.push_back(next());
  }

  return result;
}

std::string iota::ProtocolCollection::getSchema(const std::string& method) {
  std::ostringstream schema;


  if (method.compare("POST") == 0) {
    return POST_SCHEMA;
  }
  else {
    return PUT_SCHEMA;
  }

}

const std::string iota::ProtocolCollection::POST_SCHEMA(
  "{\"$schema\": \"http://json-schema.org/draft-04/schema#\","
  "\"title\": \"Iotagent_Registration\","
  "\"description\": \"Protocol registration from Iotafgent and registratio of all services\","
  "\"additionalProperties\":false,"
  "\"type\": \"object\","
  "\"properties\": {"
  "\"protocol\": {"
  "\"description\": \"protocol identifier\","
  "\"type\": \"string\""
  "},"
  "\"description\": {"
  "\"description\": \"protocol description\","
  "\"type\": \"string\""
  "},"
  "\"endpoint\": {"
  "\"description\": \"public ip of iotagent\","
  "\"type\": \"string\""
  "},"
  "\"resource\": {"
  "\"description\": \"protocol resource, uri path for the iotagent\","
  "\"type\": \"string\""
  "},"
  "\"services\": {"
  "\"type\":\"array\","
  "\"id\": \"services\","
  "\"items\":{"
  "\"type\":\"object\","
  "\"additionalProperties\":false,"
  "\"id\": \"0\","
  "\"properties\":{"
  "\"service\": {"
  "\"description\": \"service name\","
  "\"type\": \"string\""
  "},"
  "\"service_path\": {"
  "\"description\": \"service path, must start with /\","
  "\"type\": \"string\""
  "},"
  "\"entity_type\": {"
  "\"description\": \"default entity_type, if a device has not got entity_type uses this\","
  "\"type\": \"string\""
  "},"
  "\"apikey\": {"
  "\"description\": \"apikey\","
  "\"type\": \"string\""
  "},"
  "\"token\": {"
  "\"description\": \"token\","
  "\"type\": \"string\""
  "},"
  "\"cbroker\": {"
  "\"description\": \"uri for the context broker\","
  "\"type\": \"string\","
  "\"format\": \"uri\","
  "\"minLength\":1"
  "},"
  "\"outgoing_route\": {"
  "\"description\": \"VPN/GRE tunnel identifier\","
  "\"type\": \"string\""
  "},"
  "\"resource\": {"
  "\"description\": \"uri for the iotagent\","
  "\"type\": \"string\","
  "\"format\":\"regex\","
  "\"pattern\":\"^/\""
  "},"
  "\"attributes\": {"
  "\"type\":\"array\","
  "\"id\": \"attributes\","
  "\"items\":{"
  "\"type\":\"object\","
  "\"additionalProperties\":false,"
  "\"id\": \"0\","
  "\"properties\":{"
  "\"object_id\": {"
  "\"description\": \"The unique identifier by service for a device\","
  "\"type\": \"string\""
  "},"
  "\"name\": {"
  "\"description\": \"Name of the entity, if it does not exits use device_id\","
  "\"type\": \"string\""
  "},"
  "\"type\": {"
  "\"description\": \"type of the entity\","
  "\"type\": \"string\""
  "}"
  "}"
  "}"
  "},"
  "\"static_attributes\": {"
  "\"type\":\"array\","
  "\"id\": \"static_attributes\","
  "\"items\":{"
  "\"type\":\"object\","
  "\"additionalProperties\":false,"
  "\"id\": \"0\","
  "\"properties\":{"
  "\"value\": {"
  "\"description\": \"The unique identifier by service for a device\","
  "\"type\": \"string\""
  "},"
  "\"name\": {"
  "\"description\": \"Name of the entity, if it does not exits use device_id\","
  "\"type\": \"string\""
  "},"
  "\"type\": {"
  "\"description\": \"type of the entity\","
  "\"type\": \"string\""
  "}"
  "}"
  "}"
  "}"
  "}"
  ",\"required\": [\"service\", \"service_path\", \"apikey\", \"resource\", \"cbroker\"]"
  "}"
  "}"
  "}"
  ",\"required\": [\"protocol\",\"endpoint\",\"resource\"]"
  "}");

const std::string iota::ProtocolCollection::PUT_SCHEMA(
  "{\"$schema\": \"http://json-schema.org/draft-04/schema#\","
  "\"title\": \"Protocol\","
  "\"description\": \"only protocol update\","
  "\"additionalProperties\":false,"
  "\"type\": \"object\","
  "\"properties\": {"
  "\"protocol\": {"
  "\"description\": \"protocol identifier\","
  "\"type\": \"string\""
  "},"
  "\"description\": {"
  "\"description\": \"protocol description\","
  "\"type\": \"string\""
  "}"
  "}"
  "}");
