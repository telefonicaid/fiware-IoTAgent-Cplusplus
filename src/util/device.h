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
#include <boost/shared_ptr.hpp>

namespace iota {

struct Device : public virtual Timer {

  explicit Device(std::string name, std::string service): Timer(),
    _name(name), _service(service) {_active = INT_MIN;_duration_cb = LONG_MIN;};

  explicit Device(std::string entity_name, std::string entity_type,
                  std::string service):
    Timer(), _entity_name(entity_name), _entity_type(entity_type),
    _service(service) {_active = INT_MIN;_duration_cb = LONG_MIN;};

  Device(const Device& dev):
    Timer() {
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

    if (dev._duration_cb != LONG_MIN) {
      _duration_cb = dev._duration_cb;
    }else{
      _duration_cb = LONG_MIN;
    }

    std::map<std::string, std::string>::const_iterator iter;
    for (iter = dev._attributes.begin(); iter != dev._attributes.end(); ++iter) {
      _attributes.insert(std::pair<std::string, std::string>(iter->first,
                         iter->second));
    }
    for (iter = dev._commands.begin(); iter != dev._commands.end(); ++iter) {
      _commands.insert(std::pair<std::string, std::string>(iter->first,
                       iter->second));
    }
    for (iter = dev._static_attributes.begin();
         iter != dev._static_attributes.end(); ++iter) {
      _static_attributes.insert(std::pair<std::string, std::string>(iter->first,
                                iter->second));
    }

    if (dev._active != INT_MIN) {
      _active = dev._active;
    }else{
       _active = INT_MIN;
    }
  };


  virtual ~Device() {};

  bool operator==(const Device& a) const {
    if ((_name.compare(a._name) == 0) &&
        (_service.compare(a._service) == 0)) {
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
    return _service+":"+_service_path + ":"+
           _entity_name+":"+_entity_type;
  }

  std::string unique_name() const {
    return _name;
  }

  std::string get_real_name() const {
    if (_entity_name.empty()) {
      return _name;
    }
    else {
      return _entity_name;
    }
  }

  std::string get_attribute(std::string object_id) {
    return _attributes[object_id];
  }

  std::string _name;
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
