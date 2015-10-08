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
#include "device_collection.h"

iota::DeviceCollection::DeviceCollection()
    : Collection(iota::store::types::DEVICE_TABLE){};

iota::DeviceCollection::DeviceCollection(DeviceCollection& dc)
    : Collection(dc){};

iota::DeviceCollection::~DeviceCollection(){};

int iota::DeviceCollection::insertd(const Device& obj) {
  return iota::Collection::insert(Obj2BSON(obj, true));
}

int iota::DeviceCollection::updated(const Device& query, const Device& sett,
                                    bool upsert) {
  return iota::Collection::update(Obj2BSON(query, true), Obj2BSON(sett, true),
                                  upsert);
}

int iota::DeviceCollection::findd(const Device& query) {
  return iota::Collection::find(Obj2BSON(query, true));
}

iota::Device iota::DeviceCollection::nextd() {
  mongo::BSONObj data = iota::Collection::next();
  return BSON2Obj(data);
}

int iota::DeviceCollection::removed(const Device& query) {
  return iota::Collection::remove(Obj2BSON(query, true));
}

int iota::DeviceCollection::countd(const Device& query) {
  return iota::Collection::count(Obj2BSON(query, true));
}

/*void iota::DeviceCollection::fillSharKey(mongo::BSONObjBuilder &obj)
{
    // CLAVE DE SHARD  service, name
    if (_m_device._name.empty())
    {
        throw std::runtime_error("DeviceCollection::fillSharKey name is needed
as shardKey");
    }
    obj.append("name", _m_device._name);

    if (_m_device._service.empty())
    {
        throw std::runtime_error("DeviceCollection::fillSharKey service is
needed as shardKey");
    }
    obj.append("service", _m_device._service);

}*/

void iota::DeviceCollection::BSON2Map(
    mongo::BSONObj& obj, const std::string& id,
    std::map<std::string, std::string>& datamap) {
  std::string name, value;

  if (obj.isValid()) {
    mongo::BSONObjIterator array_eltos(obj);
    mongo::BSONElement elto;
    while (array_eltos.more()) {
      elto = array_eltos.next();
      if (elto.isABSONObj()) {
        mongo::BSONObj eltoBSON = elto.Obj();
        // leemos name  y commands o attributes
        name = eltoBSON.getStringField(iota::store::types::NAME);
        value = eltoBSON.getStringField(iota::store::types::ATTRIBUTE_VALUE);
        datamap.insert(std::pair<std::string, std::string>(name, value));
      }

    }  // while
  }
}

void iota::DeviceCollection::BSON2MapAttribute(
    mongo::BSONObj& obj, const std::string& id,
    std::map<std::string, std::string>& datamap, std::string key_map) {
  std::string name, value;

  if (obj.isValid()) {
    mongo::BSONObjIterator array_eltos(obj);
    mongo::BSONElement elto;
    while (array_eltos.more()) {
      elto = array_eltos.next();
      if (elto.isABSONObj()) {
        mongo::BSONObj eltoBSON = elto.Obj();
        // leemos name  y commands o attributes
        name = eltoBSON.getStringField(key_map);
        value = eltoBSON.jsonString();
        datamap.insert(std::pair<std::string, std::string>(name, value));
      }

    }  // while
  }
}

void iota::DeviceCollection::Map2BSON(
    mongo::BSONObjBuilder& obj, const std::string& fieldName,
    const std::map<std::string, std::string>& datamap) {
  mongo::BSONArrayBuilder arrayBSON;

  for (std::map<std::string, std::string>::const_iterator iter =
           datamap.begin();
       iter != datamap.end(); ++iter) {
    mongo::BSONObjBuilder objB;
    objB.append(iota::store::types::NAME, iter->first);
    objB.append(iota::store::types::TYPE, fieldName);
    objB.append(fieldName, iter->second);
    arrayBSON.append(objB.obj());
  }
  obj.append(fieldName, arrayBSON.arr());
}

void iota::DeviceCollection::Map2BSONAttribute(
    mongo::BSONObjBuilder& obj, const std::string& fieldName,
    const std::map<std::string, std::string>& datamap, std::string key_map) {
  mongo::BSONArrayBuilder arrayBSON;

  for (std::map<std::string, std::string>::const_iterator iter =
           datamap.begin();
       iter != datamap.end(); ++iter) {
    std::string body = iter->second;

    try {
      mongo::BSONObjBuilder objB;
      mongo::BSONObj obj = mongo::fromjson(body);
      objB.appendElements(obj);
      objB.append(key_map, iter->first);
      arrayBSON.append(objB.obj());
    } catch (mongo::MsgAssertionException& exc) {
      IOTA_LOG_ERROR(m_logger, "PARSER ERROR " << exc.getCode() << ":"
                                               << exc.what());
      IOTA_LOG_ERROR(m_logger, "PARSER data: " << body);
    }
  }
  obj.append(fieldName, arrayBSON.arr());
}

iota::Device iota::DeviceCollection::BSON2Obj(mongo::BSONObj& obj) {
  std::string name = obj.getStringField(iota::store::types::DEVICE_ID);
  std::string service = obj.getStringField(iota::store::types::SERVICE);

  Device result(name, service);
  result._service_path = obj.getStringField(iota::store::types::SERVICE_PATH);
  result._entity_type = obj.getStringField(iota::store::types::ENTITY_TYPE);
  result._entity_name = obj.getStringField(iota::store::types::ENTITY_NAME);
  result._protocol = obj.getStringField(iota::store::types::PROTOCOL);
  result._timezone = obj.getStringField(iota::store::types::TIMEZONE);
  result._endpoint = obj.getStringField(iota::store::types::ENDPOINT);

  result._registration_id =
      obj.getStringField(iota::store::types::REGISTRATION_ID);
  result._duration_cb = obj.getIntField(iota::store::types::DURATION_CB);
  result._timestamp_data =
      obj.getField(iota::store::types::TIMESTAMP_DATA).numberLong();

  mongo::BSONObj objAttr = obj.getObjectField(iota::store::types::ATTRIBUTES);
  BSON2MapAttribute(objAttr, iota::store::types::ATTRIBUTES, result._attributes,
                    iota::store::types::ATTRIBUTE_ID);
  mongo::BSONObj objCommands = obj.getObjectField(iota::store::types::COMMANDS);
  BSON2Map(objCommands, iota::store::types::COMMANDS, result._commands);
  mongo::BSONObj objAttrStatic =
      obj.getObjectField(iota::store::types::STATIC_ATTRIBUTES);
  BSON2MapAttribute(objAttrStatic, iota::store::types::STATIC_ATTRIBUTES,
                    result._static_attributes, iota::store::types::NAME);
  result._model = obj.getStringField(iota::store::types::MODEL);

  if (obj.hasField(iota::store::types::ACTIVATE)) {
    if (obj.getBoolField(iota::store::types::ACTIVATE)) {
      result._active = 1;
    } else {
      result._active = 0;
    }
  } else {
    result._active = 1;
  }

  return result;
}

mongo::BSONObj iota::DeviceCollection::Obj2BSON(const Device& device,
                                                bool withShardKey) {
  mongo::BSONObjBuilder obj;
  if (withShardKey) {
    if (!device._name.empty()) {
      obj.append(iota::store::types::DEVICE_ID, device._name);
    }

    if (!device._service.empty()) {
      obj.append(iota::store::types::SERVICE, device._service);
    }

    if (!device._service_path.empty()) {
      obj.append(iota::store::types::SERVICE_PATH, device._service_path);
    }
  };

  if (device._active != INT_MIN) {
    obj.append(iota::store::types::ACTIVATE, device._active);
  }

  if (!device._endpoint.empty()) {
    obj.append(iota::store::types::ENDPOINT, device._endpoint);
  }

  if (!device._protocol.empty()) {
    obj.append(iota::store::types::PROTOCOL, device._protocol);
  }
  if (!device._entity_name.empty()) {
    obj.append(iota::store::types::ENTITY_NAME, device._entity_name);
  }
  if (!device._entity_type.empty()) {
    obj.append(iota::store::types::ENTITY_TYPE, device._entity_type);
  }
  if (!device._model.empty()) {
    obj.append(iota::store::types::MODEL, device._model);
  }

  if (!device._registration_id.empty()) {
    obj.append(iota::store::types::REGISTRATION_ID, device._registration_id);
  }

  if (device._duration_cb != LONG_MIN) {
    obj.appendNumber(iota::store::types::DURATION_CB, device._duration_cb);
  }

  if (device._timestamp_data != -1) {
    obj.appendIntOrLL(iota::store::types::TIMESTAMP_DATA,
                      device._timestamp_data);
  }

  if (!device._timezone.empty()) {
    obj.append(iota::store::types::TIMEZONE, device._timezone);
  }

  if (device._commands.size() > 0) {
    Map2BSON(obj, iota::store::types::COMMANDS, device._commands);
  }

  if (device._attributes.size() > 0) {
    Map2BSONAttribute(obj, iota::store::types::ATTRIBUTES, device._attributes,
                      iota::store::types::ATTRIBUTE_ID);
  }

  if (device._static_attributes.size() > 0) {
    Map2BSONAttribute(obj, iota::store::types::STATIC_ATTRIBUTES,
                      device._static_attributes, iota::store::types::NAME);
  }

  return obj.obj();
}

int iota::DeviceCollection::createTableAndIndex() {
  int res = 200;
  /// db.DEVICE.ensureIndex( { "device_id": 1, "service":1, "service_path":1 },
  /// { unique: true } )
  mongo::BSONObj indexUni =
      BSON("device_id" << 1 << "service" << 1 << "service_path" << 1);
  int res1 = createIndex(indexUni, true);
  // entity_name and entity_type is unique in service service_path
  /// db.DEVICE.ensureIndex( { "entity_name": 1, "entity_type": 1, "service":1,
  /// "service_path":1 }, { unique: true } )
  mongo::BSONObj indexUni2 =
      BSON("entity_name" << 1 << "entity_type" << 1 << "service" << 1
                         << "service_path" << 1);
  return createIndex(indexUni2, true);
}
