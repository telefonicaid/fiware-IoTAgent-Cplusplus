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
#include "service_mgmt_collection.h"
#include "service_collection.h"
#include "iota_exception.h"
#include "protocol_collection.h"


iota::ServiceMgmtCollection::ServiceMgmtCollection() {
  setBBDD(iota::store::types::MANAGER_SERVICE_TABLE);
};

iota::ServiceMgmtCollection::ServiceMgmtCollection(ServiceMgmtCollection& dc) {
};

iota::ServiceMgmtCollection::~ServiceMgmtCollection() {
};

/*void iota::ServiceMgmtCollection::fillSharKey(mongo::BSONObjBuilder &obj)
{
    // CLAVE DE SHARD  service, name
    if (_m_device._name.empty())
    {
        throw std::runtime_error("ServiceMgmtCollection::fillSharKey name is needed as shardKey");
    }
    obj.append("name", _m_device._name);

    if (_m_device._service.empty())
    {
        throw std::runtime_error("ServiceMgmtCollection::fillSharKey service is needed as shardKey");
    }
    obj.append("service", _m_device._service);

}*/

const std::string & iota::ServiceMgmtCollection::getPostSchema() const{
  return _POST_SCHEMA;
}

const std::string iota::ServiceMgmtCollection::_POST_SCHEMA(
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
  "\"protocol\": {"
  "\"type\":\"array\","
  "\"id\": \"protocol\","
  "\"items\":{"
  "\"type\":\"string\""
  "},\"minItems\": 1"
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
  ",\"required\": [\"apikey\", \"protocol\", \"cbroker\"]"
  "}"
  "}"
  "}"
  ",\"required\": [\"services\"]"
  "}");



int iota::ServiceMgmtCollection::createTableAndIndex() {

  int res = 200;
  //db.SERVICE_MGMT.ensureIndex({"service":1, service_path:1, iotagent:1, protocol:1},{"unique":1})
  mongo::BSONObj indexUni = BSON(iota::store::types::SERVICE << 1 <<
                                      iota::store::types::SERVICE_PATH << 1
                                   << iota::store::types::IOTAGENT << 1 <<
                                      iota::store::types::PROTOCOL_NAME << 1);

  return createIndex(indexUni, true);
}

std::vector<iota::ServiceType> iota::ServiceMgmtCollection::get_services_by_protocol(
              const std::string &protocol_name,
              int limit, int skip){
  std::vector<iota::ServiceType>  result;

  mongo::BSONObj query;
  mongo::BSONObjBuilder fieldsToReturn;
  fieldsToReturn.append(iota::store::types::SERVICE, 1);
  fieldsToReturn.append(iota::store::types::SERVICE_PATH, 1);

  iota::Collection::find(a_queryOptions, query,
                           limit, skip,
                           BSON(iota::store::types::SERVICE << 1 <<
                           iota::store::types::SERVICE_PATH << 1),
                           fieldsToReturn, 0);

  std::string ser, ser_path;
  mongo::BSONObj elto;
  while(more()){
      //TODO  comprobar que no esta repetido
    elto = next();
    ser = elto.getStringField(iota::store::types::SERVICE);
    ser_path = elto.getStringField(iota::store::types::SERVICE_PATH);
    result.push_back(iota::ServiceType(ser, ser_path));
  }


  return result;
}

std::vector<iota::ServiceType> iota::ServiceMgmtCollection::get_services_group_protocol(
              const std::string &protocol_name,
              int limit, int skip){
  std::vector<iota::ServiceType>  result;

  std::vector<mongo::BSONObj> pipeline;
  // TODO  match y sort
  pipeline.push_back( BSON( "$project" <<
       BSON("service" << 1 << "service_path" << 1 << "protocol" <<1)
       ));
  pipeline.push_back( BSON("$group" <<
       BSON("_id" <<
       BSON("service" << "$service" << "service_path"<< "$service_path") <<
       "protocol" << BSON("$addToSet" << "$protocol" ) ))
  );
  mongo::BSONObj res =iota::Collection::aggregate(pipeline, 0);
  /*std::string ser, ser_path;
  mongo::BSONObj elto;
  while(more()){
      //TODO  comprobar que no esta repetido
    elto = next();
    ser = elto.getStringField(iota::store::types::SERVICE);
    ser_path = elto.getStringField(iota::store::types::SERVICE_PATH);
    result.push_back(iota::ServiceType(ser, ser_path));
  }*/


  return result;
}

std::vector<iota::IotagentType> iota::ServiceMgmtCollection::get_iotagents_by_service(
        const std::string & service, const std::string& service_path,
        const std::string& protocol_id,
        int limit, int skip){
  std::vector<iota::IotagentType>  result;

  mongo::BSONObj query;
  mongo::BSONObjBuilder fieldsToReturn;
  fieldsToReturn.append(iota::store::types::IOTAGENT, 1);

  mongo::BSONObjBuilder bson_query;
  bson_query.append(iota::store::types::SERVICE, service);
  ServiceCollection::addServicePath(service_path, bson_query);
  if (!protocol_id.empty()){
    bson_query.append(iota::store::types::PROTOCOL_NAME, protocol_id);
  }

  mongo::BSONObj res = iota::Collection::distinct("iotagent" , bson_query.obj(), 0);

  mongo::BSONObjIterator fields (res.getObjectField ("values"));
  while(fields.more()) {
    result.push_back(fields.next().str ());
  }

  return result;
}


void iota::ServiceMgmtCollection::getElementsFromBSON(mongo::BSONObj &obj,
                                std::vector<mongo::BSONObj> &result){

    std::vector<mongo::BSONElement> be = obj.getField(
                                           iota::store::types::SERVICES).Array();
    std::map<std::string, std::string>protocols;
    iota::ProtocolCollection protocolCol;
    protocolCol.fillProtocols(protocols);
    std::map<std::string,std::string>::iterator it;
    std::string descriptionSTR;

    for (unsigned int i = 0; i<be.size(); i++) {
      mongo::BSONObj obj = be[i].embeddedObject();
      //cogemos el protocolo
      if (obj.hasField(iota::store::types::PROTOCOL_NAME)){
         mongo::BSONElement protocolsObj = obj.getField(iota::store::types::PROTOCOL_NAME);
         std::vector<mongo::BSONElement> proArr = protocolsObj.Array();
         for (unsigned int j = 0; j<proArr.size(); j++) {
            std::string protocol_id = proArr[j].str();
            it=protocols.find(protocol_id);
            if (it == protocols.end()){
                std::string errorSTR = "No exists protocol " + protocol_id;
                PION_LOG_ERROR(m_logger, errorSTR);
                throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                           errorSTR, iota::types::RESPONSE_CODE_BAD_REQUEST);
            }else{
              descriptionSTR = it->second;
            }
            mongo::BSONObjBuilder newdata;
            newdata.appendElements(obj.removeField(iota::store::types::PROTOCOL_NAME));
            newdata.append(iota::store::types::PROTOCOL_NAME, protocol_id);
            newdata.append(iota::store::types::PROTOCOL_DESCRIPTION, descriptionSTR);
            mongo::BSONObj ddd  = newdata.obj();
            result.push_back(ddd);
         }
      }
   }
}

const std::string & iota::ServiceMgmtCollection::get_resource_name(){
  return iota::store::types::PROTOCOL;
}
