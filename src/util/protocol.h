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
#ifndef SRC_UTIL_PROTOCOL_H_
#define SRC_UTIL_PROTOCOL_H_

#include "timer.h"
#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>

namespace iota {

class Protocol {
 public:
  typedef struct {
    std::string endpoint;
    std::string resource;
  } resource_endpoint;

  typedef std::vector<resource_endpoint> resource_endpoint_vector;

  Protocol();
  Protocol(const std::string& name);

  virtual ~Protocol();

  void add(resource_endpoint data);

  const resource_endpoint_vector& get_endpoints() const { return _endpoints; };

  std::string get_description() const { return _description; };

  void set_description(const std::string& description) {
    _description = description;
  }

  std::string get_name() const { return _name; };

  void set_name(const std::string& name) { _name = name; }

  std::string get_id() const { return _id; };

  void set_id(const std::string& id) { _id = id; }

 protected:
 private:
  std::string _id;
  std::string _description;
  std::string _name;
  resource_endpoint_vector _endpoints;
};
};

#endif
