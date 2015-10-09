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
#ifndef SRC_NGSI_CONTEXTREGISTRATION_H_
#define SRC_NGSI_CONTEXTREGISTRATION_H_

#include "ngsi/Entity.h"
#include "ngsi/AttributeRegister.h"
#include <rapidjson/rapidjson.h>
#include <vector>
#include <string>
#include "util/device.h"
#include <boost/property_tree/ptree.hpp>

namespace iota {

class ContextRegistration {
 public:
  ContextRegistration(const std::istringstream& str_register);
  ContextRegistration(){};
  ContextRegistration(const rapidjson::Value& reg);
  ~ContextRegistration(){};
  void add_entity(const Entity& entity);
  void add_attribute(const AttributeRegister& attribute);
  void add_provider(const std::string& provider);
  std::string get_string();
  std::vector<Entity>& get_entities() { return _entities; };

  std::vector<AttributeRegister>& get_attributes() { return _attributes; };

  std::string& get_provider() { return _provider; };

  void set_env_info(boost::property_tree::ptree service_info,
                    boost::shared_ptr<Device> device);

  template <typename Writer>
  void Serialize(Writer& writer) const {
    writer.StartObject();
    writer.String("entities");
    writer.StartArray();
    for (std::vector<Entity>::const_iterator it = _entities.begin();
         it != _entities.end(); ++it) {
      it->Serialize(writer);
    }
    writer.EndArray();
    writer.String("attributes");
    writer.StartArray();
    for (std::vector<AttributeRegister>::const_iterator it =
             _attributes.begin();
         it != _attributes.end(); ++it) {
      it->Serialize(writer);
    }
    writer.EndArray();
    writer.String("providingApplication");
    writer.String(_provider.c_str(), (rapidjson::SizeType)_provider.length());
    writer.EndObject();
  };

 private:
  std::vector<Entity> _entities;
  std::vector<AttributeRegister> _attributes;
  std::string _provider;
  boost::property_tree::ptree _service_info;
  boost::shared_ptr<Device> _device_info;
};
}

#endif
