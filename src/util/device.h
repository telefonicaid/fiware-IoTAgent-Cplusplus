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
#ifndef SRC_UTIL_DEVICE_H_
#define SRC_UTIL_DEVICE_H_

#include "timer.h"
#include "store_const.h"
#include "rest/riot_conf.h"
#include <boost/functional/hash.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/shared_ptr.hpp>

namespace iota {

struct Device : public virtual Timer {
  explicit Device(std::string name, std::string service)
      : Timer(), _name(name), _service(service) {
    _active = INT_MIN;
    _duration_cb = LONG_MIN;
    _protocol = "";
    _timestamp_data = -1;
  };

  explicit Device(std::string entity_name, std::string entity_type,
                  std::string service)
      : Timer(),
        _entity_name(entity_name),
        _entity_type(entity_type),
        _service(service) {
    _active = INT_MIN;
    _duration_cb = LONG_MIN;
    _protocol = "";
    _timestamp_data = -1;
  };

  Device(const Device& dev) : Timer() {
    if (!dev._name.empty()) {
      _name = dev._name;
    }
    if (!dev._service.empty()) {
      _service = dev._service;
    }
    if (!dev._service_path.empty()) {
      _service_path = dev._service_path;
    }
    if (!dev._entity_type.empty()) {
      _entity_type = dev._entity_type;
    }
    if (!dev._entity_name.empty()) {
      _entity_name = dev._entity_name;
    }
    if (!dev._timezone.empty()) {
      _timezone = dev._timezone;
    }
    if (!dev._endpoint.empty()) {
      _endpoint = dev._endpoint;
    }
    if (!dev._model.empty()) {
      _model = dev._model;
    }
    if (!dev._registration_id.empty()) {
      _registration_id = dev._registration_id;
    }

    if (!dev._protocol.empty()) {
      _protocol = dev._protocol;
    }

    if (dev._duration_cb != LONG_MIN) {
      _duration_cb = dev._duration_cb;
    } else {
      _duration_cb = LONG_MIN;
    }

    // Timestamp for registering last communication
    if (dev._timestamp_data != -1) {
      _timestamp_data = dev._timestamp_data;
    } else {
      _timestamp_data = -1;
    }

    std::map<std::string, std::string>::const_iterator iter;
    for (iter = dev._attributes.begin(); iter != dev._attributes.end();
         ++iter) {
      _attributes.insert(
          std::pair<std::string, std::string>(iter->first, iter->second));
    }
    for (iter = dev._commands.begin(); iter != dev._commands.end(); ++iter) {
      _commands.insert(
          std::pair<std::string, std::string>(iter->first, iter->second));
    }
    for (iter = dev._static_attributes.begin();
         iter != dev._static_attributes.end(); ++iter) {
      _static_attributes.insert(
          std::pair<std::string, std::string>(iter->first, iter->second));
    }

    if (dev._active != INT_MIN) {
      _active = dev._active;
    } else {
      _active = INT_MIN;
    }
  };

  virtual ~Device(){};

  bool operator==(const Device& a) const {
    if ((_name.compare(a._name) == 0) && (_service.compare(a._service) == 0)) {
      return true;
    }
    return false;
  }

  bool operator==(const boost::shared_ptr<Device>& a) const {
    if ((_name.compare(a->_name) == 0) &&
        (_service.compare(a->_service) == 0)) {
      return true;
    }
    return false;
  }

  bool operator()(const Device& one, const Device& two) const {
    std::string one_full(one._name);
    one_full.append(one._service);
    std::string two_full(two._name);
    two_full.append(two._service);
    return (one_full < two_full);
  }

  bool operator()(const boost::shared_ptr<Device>& one,
                  const boost::shared_ptr<Device>& two) const {
    std::string one_full(one->_name);
    one_full.append(one->_service);
    std::string two_full(two->_name);
    two_full.append(two->_service);
    return (one_full < two_full);
  }

  friend std::size_t hash_value(boost::shared_ptr<Device> const& item) {
    std::size_t seed = 0;
    boost::hash_combine(seed, item->_name);
    boost::hash_combine(seed, item->_service);
    boost::hash_combine(seed, item->_service_path);
    return seed;
  }

  friend std::size_t hash_value(Device const& item) {
    std::size_t seed = 0;
    boost::hash_combine(seed, item._name);
    boost::hash_combine(seed, item._service);
    boost::hash_combine(seed, item._service_path);
    return seed;
  }

  std::string unique_entity() const {
    return _service + ":" + _service_path + ":" + _entity_name + ":" +
           _entity_type + ":" + _protocol;
  }

  std::string unique_name() const { return _name; }

  std::string get_real_name(
      const boost::property_tree::ptree& service_ptree) const {
    std::string entity_type("thing");
    std::string entity_id(_name);
    std::string service_entity_type =
        service_ptree.get<std::string>(iota::store::types::ENTITY_TYPE, "");

    if (!_entity_type.empty() &&
        _entity_type.compare(iota::store::types::DEFAULT) != 0) {
      //  used entity_type defined in device
      entity_type.assign(_entity_type);
    } else if (!service_entity_type.empty()) {
      entity_type = service_entity_type;
    }
    // Entity name (default is entity_type:device_id)
    if (!_entity_name.empty() &&
        _entity_name.compare(iota::store::types::DEFAULT) != 0) {
      //  used entity_type defined in device
      entity_id.assign(_entity_name);
    } else {
      entity_id.assign(entity_type + ":" + _name);
    }

    return entity_id;
  }

  std::string get_attribute(std::string object_id) {
    return _attributes[object_id];
  }

  std::string _name;
  std::string _protocol;
  std::string _service;
  std::string _service_path;
  std::string _entity_type;
  std::string _entity_name;
  std::string _timezone;
  std::string _endpoint;
  std::map<std::string, std::string> _attributes;
  std::map<std::string, std::string> _commands;
  std::map<std::string, std::string> _static_attributes;
  std::map<std::string, std::string> _lazy;
  std::string _model;
  int _active;
  std::string _registration_id;
  long long _duration_cb;
  long long _timestamp_data;
};

struct entity_hash {
  std::size_t operator()(Device const& item) const {
    return boost::hash<std::string>()(item.unique_entity());
  }
  std::size_t operator()(std::string item) const {
    return boost::hash<std::string>()(item);
  }
};

struct entity_equal {
  bool operator()(Device const& item, std::string ent) const {
    return (item.unique_entity().compare(ent) == 0);
  }

  bool operator()(std::string ent, Device const& item) const {
    return (item.unique_entity().compare(ent) == 0);
  }
};

struct name_hash {
  std::size_t operator()(Device const& item) const {
    return boost::hash<std::string>()(item.unique_name());
  }
  std::size_t operator()(std::string item) const {
    return boost::hash<std::string>()(item);
  }
};

struct name_equal {
  bool operator()(Device const& item, std::string ent) const {
    return (item.unique_name().compare(ent) == 0);
  }

  bool operator()(std::string ent, Device const& item) const {
    return (item.unique_name().compare(ent) == 0);
  }
};
};

#endif
