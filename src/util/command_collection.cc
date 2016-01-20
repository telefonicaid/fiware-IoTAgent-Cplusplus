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

#include "command_collection.h"
#include "store_const.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "util/iota_logger.h"

iota::CommandCollection::CommandCollection()
    : Collection(iota::store::types::COMMAND_TABLE){};

iota::CommandCollection::CommandCollection(CommandCollection& dc)
    : Collection(dc){};

iota::CommandCollection::~CommandCollection(){};

int iota::CommandCollection::insert(const Command& obj) {
  return iota::Collection::insert(Obj2BSON(obj, true));
}

int iota::CommandCollection::find(const Command& query) {
  return iota::Collection::find(Obj2BSON(query, true));
};

iota::Command iota::CommandCollection::next() {
  mongo::BSONObj data = iota::Collection::next();
  return BSON2Obj(data);
}

int iota::CommandCollection::remove(const iota::Command& query) {
  return iota::Collection::remove(Obj2BSON(query, true));
}

int iota::CommandCollection::count(const iota::Command& query) {
  return iota::Collection::count(Obj2BSON(query, true));
}

iota::Command iota::CommandCollection::BSON2Obj(const mongo::BSONObj& obj) {
  std::string nodo = obj.getStringField(iota::store::types::DEVICE);
  std::string service = obj.getStringField(iota::store::types::SERVICE);
  std::string service_path =
      obj.getStringField(iota::store::types::SERVICE_PATH);

  iota::Command result(nodo, service, service_path);
  // Timeout asociado al comando (segundos)
  result.set_timeout(obj.getIntField(iota::store::types::TIMEOUT));

  result.set_entity_type(obj.getStringField(iota::store::types::ENTITY_TYPE));

  // URI respuesta
  result.set_uri_resp(obj.getStringField(iota::store::types::URI_RESP));

  // Identificador de comando, usamos el de mongo
  result.set_id(obj.getStringField(iota::store::types::COMMAND_ID));

  // Identificador de comando, deberia ser unico
  result.set_name(obj.getStringField(iota::store::types::NAME));

  // Si esta expirado
  // TODO bool _expired;

  // Si el comando se recupera por polling, debe almacenarse
  mongo::BSONObj body = obj.getObjectField(iota::store::types::COMMAND);
  boost::property_tree::ptree pt;
  std::set<std::string> fields;
  body.getFieldNames(fields);
  std::string nname, value;
  int num = 0;
  for (std::set<std::string>::iterator it = fields.begin(); it != fields.end();
       ++it) {
    nname = *it;
    value = body.getStringField(nname);
    pt.put(nname, value);
    num++;
  }
  if (num > 0) {
    result.set_command(pt);
  }

  // estado del comando
  result.set_status (obj.getIntField(iota::store::types::STATUS));

  // estado del comando
  result.set_service(obj.getStringField(iota::store::types::SERVICE));

  // sequence, specific id between CB and IotAgent
  result.set_sequence(obj.getStringField(iota::store::types::SEQUENCE));

  return result;
};

mongo::BSONObj iota::CommandCollection::Obj2BSON(const Command& command,
                                                 bool withShardKey) {
  mongo::BSONObjBuilder obj;
  if (withShardKey) {
    if (!command.get_id().empty()) {
      obj.append(iota::store::types::COMMAND_ID, command.get_id());
    }
  }

  if (!command.get_name().empty()) {
    obj.append(iota::store::types::NAME, command.get_name());
  }

  if (!command.get_service().empty()) {
    obj.append(iota::store::types::SERVICE, command.get_service());
  }

  if (!command.get_service_path().empty()) {
    obj.append(iota::store::types::SERVICE_PATH, command.get_service_path());
  }

  if (!command.get_entity().empty()) {
    obj.append(iota::store::types::DEVICE, command.get_entity());
  }

  if (!command.get_entity_type().empty()) {
    obj.append(iota::store::types::ENTITY_TYPE, command.get_entity_type());
  }

  if (command.get_expired() != INT_MIN) {
    obj.append(iota::store::types::EXPIRED, command.get_expired());
  }
  if (!command.get_sequence().empty()) {
    obj.append(iota::store::types::SEQUENCE, command.get_sequence());
  }
  if (command.get_status() != INT_MIN) {
    obj.append(iota::store::types::STATUS, command.get_status());
  }
  if (command.get_timeout() != INT_MIN) {
    obj.append(iota::store::types::TIMEOUT, command.get_timeout());
  }
  if (!command.get_uri_resp().empty()) {
    obj.append(iota::store::types::URI_RESP, command.get_uri_resp());
  }

  boost::property_tree::ptree pt = command.get_command();
  mongo::BSONObjBuilder objCommand;
  int num = 0;
  for (boost::property_tree::ptree::iterator pos = pt.begin(); pos != pt.end();
       ++pos) {
    objCommand.append(pos->first, pos->second.data());
    num++;
  }
  if (num > 0) {
    obj.append(iota::store::types::COMMAND, objCommand.obj());
  }

  return obj.obj();
};

int iota::CommandCollection::createTableAndIndex() {
  int res = 200;
  // db.COMMAND.ensureIndex( { "id":1 , "service" :1, "service_path":1}, {
  // unique: true } )
  mongo::BSONObj indexUni = BSON(iota::store::types::COMMAND_ID
                                 << 1 << iota::store::types::SERVICE << 1
                                 << iota::store::types::SERVICE_PATH << 1);

  return createIndex(indexUni, true);
  return 0;
}
