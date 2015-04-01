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


iota::ServiceCollection::ServiceCollection():Collection(
    iota::store::types::SERVICE_TABLE) {
};


iota::ServiceCollection::ServiceCollection(ServiceCollection& dc):Collection(
    dc) {
};

iota::ServiceCollection::~ServiceCollection() {
};


void iota::ServiceCollection::addServicePath(const std::string& data,
    mongo::BSONObjBuilder& obj) {

  if (data.empty()) {
    obj.append(iota::store::types::SERVICE_PATH, iota::types::FIWARE_SERVICEPATH_DEFAULT);
  }
  else if (data.compare("/*")!= 0 && data.compare("/#")!= 0) {
    obj.append(iota::store::types::SERVICE_PATH, data);
  }
}

/*void iota::ServiceCollection::fillSharKey(mongo::BSONObjBuilder &obj)
{
    // CLAVE DE SHARD  service, name
    if (_m_device._name.empty())
    {
        throw std::runtime_error("ServiceCollection::fillSharKey name is needed as shardKey");
    }
    obj.append("name", _m_device._name);

    if (_m_device._service.empty())
    {
        throw std::runtime_error("ServiceCollection::fillSharKey service is needed as shardKey");
    }
    obj.append("service", _m_device._service);

}*/

std::string iota::ServiceCollection::getSchema(const std::string& method) {
  std::ostringstream schema;

  if (method.compare("POST") == 0) {
    return   POST_SCHEMA;
  }
  else {
    return   PUT_SCHEMA;
  }

}


const std::string iota::ServiceCollection::POST_SCHEMA(
  "{\"$schema\": \"http://json-schema.org/draft-04/schema#\","
  "\"title\": \"Service\","
  "\"description\": \"A service\","
  "\"additionalProperties\":false,"
  "\"type\": \"object\","
  "\"properties\": {"
  "\"services\": {"
  "\"type\":\"array\","
  "\"id\": \"services\","
  "\"items\":{"
  "\"type\":\"object\","
  "\"additionalProperties\":false,"
  "\"id\": \"0\","
  "\"properties\":{"
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
  ",\"required\": [\"apikey\", \"resource\", \"cbroker\"]"
  "}"
  "}"
  "}"
  ",\"required\": [\"services\"]"
  "}");

const std::string iota::ServiceCollection::PUT_SCHEMA(
  "{\"$schema\": \"http://json-schema.org/draft-04/schema#\","
  "\"title\": \"Device\","
  "\"description\": \"A device\","
  "\"additionalProperties\":false,"
  "\"type\": \"object\","
  "\"properties\": {"
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
  "}");


int iota::ServiceCollection::createTableAndIndex() {

  int res = 200;
  ensureIndex("shardKey",
              BSON("service" << 1 << "service_path" << 1 << "resource" <<1),
              true);
  return res;
}
