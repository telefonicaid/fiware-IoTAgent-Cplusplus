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
#include "ngsi/ContextRegistration.h"
#include "ngsi/AttributeRegister.h"
#include "ngsi/ngsi_const.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <stdexcept>
#include <iostream>

iota::ContextRegistration::ContextRegistration(
    const std::istringstream& str_register) {
  rapidjson::Document document;
  char buffer[str_register.str().length()];
  strcpy(buffer, str_register.str().c_str());
  if (document.ParseInsitu<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "ContextRegistration: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());
  }
  if (!document.HasMember(iota::ngsi::NGSI_ENTITIES.c_str()) ||
      !document[iota::ngsi::NGSI_ENTITIES.c_str()].IsArray()) {
    std::ostringstream what;
    what << "ContextRegistration: ";
    what << "invalid or missing field [" << iota::ngsi::NGSI_ENTITIES << "]";
    throw std::runtime_error(what.str());
  }

  const rapidjson::Value& entities =
      document[iota::ngsi::NGSI_ENTITIES.c_str()];

  for (rapidjson::SizeType i = 0; i < entities.Size(); i++) {
    iota::Entity entity(entities[i]);
    add_entity(entity);
  }

  if (!document.HasMember(iota::ngsi::NGSI_ATTRIBUTES.c_str()) ||
      !document[iota::ngsi::NGSI_ATTRIBUTES.c_str()].IsArray()) {
    std::ostringstream what;
    what << "ContextRegistration: ";
    what << "invalid or missing field [" << iota::ngsi::NGSI_ATTRIBUTES << "]";
    throw std::runtime_error(what.str());
  }

  const rapidjson::Value& attributes =
      document[iota::ngsi::NGSI_ATTRIBUTES.c_str()];

  for (rapidjson::SizeType i = 0; i < attributes.Size(); i++) {
    iota::AttributeRegister attribute(attributes[i]);
    add_attribute(attribute);
  }

  if (document.HasMember(iota::ngsi::NGSI_PROVIDINGAPPLICATION.c_str()) &&
      document[iota::ngsi::NGSI_PROVIDINGAPPLICATION.c_str()].IsString()) {
    add_provider(
        document[iota::ngsi::NGSI_PROVIDINGAPPLICATION.c_str()].GetString());
  }
};

iota::ContextRegistration::ContextRegistration(const rapidjson::Value& reg) {
  if (!reg.IsObject() || !reg.HasMember(iota::ngsi::NGSI_ENTITIES.c_str()) ||
      !reg[iota::ngsi::NGSI_ENTITIES.c_str()].IsArray()) {
    std::ostringstream what;
    what << "ContextRegistration: ";
    what << "invalid or missing field [" << iota::ngsi::NGSI_ENTITIES << "]";
    throw std::runtime_error(what.str());
  }
  const rapidjson::Value& entities = reg[iota::ngsi::NGSI_ENTITIES.c_str()];

  for (rapidjson::SizeType i = 0; i < entities.Size(); i++) {
    iota::Entity entity(entities[i]);
    add_entity(entity);
  }

  if (!reg.HasMember(iota::ngsi::NGSI_ATTRIBUTES.c_str()) ||
      !reg[iota::ngsi::NGSI_ATTRIBUTES.c_str()].IsArray()) {
    std::ostringstream what;
    what << "ContextRegistration: ";
    what << "invalid or missing field [" << iota::ngsi::NGSI_ATTRIBUTES << "]";
    throw std::runtime_error(what.str());
  }

  const rapidjson::Value& attributes = reg[iota::ngsi::NGSI_ATTRIBUTES.c_str()];

  for (rapidjson::SizeType i = 0; i < attributes.Size(); i++) {
    iota::AttributeRegister attribute(attributes[i]);
    add_attribute(attribute);
  }

  if (reg.HasMember(iota::ngsi::NGSI_PROVIDINGAPPLICATION.c_str()) &&
      reg[iota::ngsi::NGSI_PROVIDINGAPPLICATION.c_str()].IsString()) {
    add_provider(
        reg[iota::ngsi::NGSI_PROVIDINGAPPLICATION.c_str()].GetString());
  }
};

std::string iota::ContextRegistration::get_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  Serialize(writer);
  return buffer.GetString();
};

void iota::ContextRegistration::add_entity(const iota::Entity& entity) {
  _entities.push_back(entity);
};

void iota::ContextRegistration::add_attribute(
    const iota::AttributeRegister& attribute) {
  _attributes.push_back(attribute);
};

void iota::ContextRegistration::add_provider(const std::string& provider) {
  _provider.assign(provider);
};

void iota::ContextRegistration::set_env_info(
    boost::property_tree::ptree service_info,
    boost::shared_ptr<Device> device) {
  _service_info = service_info;
  _device_info = device;

  if (_device_info.get() == NULL) {
    return;
  }

  iota::Entity ent(_device_info.get()->_entity_name,
                   _device_info.get()->_entity_type, "false");

  // Check service info
  // If device info, this information is taken.
  // If device info has entity_type, this info is taken.
  // If device info has not entity_type, service entity_type is taken.
  // If _type is defined in constructor, does not follow default.

  std::string service_entity_type = _service_info.get<std::string>(
      iota::store::types::ENTITY + "_" + iota::store::types::TYPE, "");

  std::string entity_type("thing");
  std::string entity_id(ent.get_id());
  if (_device_info.get() != NULL) {
    if (!_device_info->_entity_type.empty() &&
        _device_info->_entity_type.compare(iota::store::types::DEFAULT) != 0) {
      entity_type.assign(_device_info->_entity_type);
    } else {
      // Device has not entity type, default is used
      if (!service_entity_type.empty()) {
        entity_type = service_entity_type;
      } else if (!ent.get_type().empty()) {
        entity_type = ent.get_type();
      }
    }
    // Entity name (default is entity_type:device_id)
    if (!_device_info->_entity_name.empty() &&
        _device_info->_entity_name.compare(iota::store::types::DEFAULT) != 0) {
      entity_id.assign(_device_info->_entity_name);
    } else {
      // Device has not entity_name, default is used if _type is not defined
      // in constructor

      if (ent.get_type().empty()) {
        entity_id.assign(entity_type + ":" + _device_info->_name);
      }
    }

  } else {
    // No device info
    if (!service_entity_type.empty()) {
      entity_type = service_entity_type;
    } else if (!ent.get_type().empty()) {
      entity_type = ent.get_type();
    }
    if (ent.get_type().empty()) {
      entity_id.assign(entity_type + ":" + ent.get_id());
    }
  }
  ent.set_type(entity_type);
  ent.set_id(entity_id);

  // entity_type and entity_name also set in device of cache
  device->_entity_type = entity_type;
  device->_entity_name = entity_id;

  add_entity(ent);
}
