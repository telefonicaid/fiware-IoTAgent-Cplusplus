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
#ifndef SRC_UTIL_SERVICE_H_
#define SRC_UTIL_SERVICE_H_

#include "timer.h"
#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>

namespace iota {

struct Service : public virtual Timer {
  explicit Service(std::string service) : Timer(), _service(service), _timeout(0){};

  virtual ~Service(){};

  bool operator==(const Service& a) const {
    if (_service.compare(a._service) == 0) {
      return true;
    }
    return false;
  }

  bool operator==(const boost::shared_ptr<Service>& a) const {
    if (_service.compare(a->_service) == 0) {
      return true;
    }
    return false;
  }

  bool operator()(const Service& one, const Service& two) const {
    std::string one_full(one._service);
    std::string two_full(two._service);
    return (one_full < two_full);
  }

  bool operator()(const boost::shared_ptr<Service>& one,
                  const boost::shared_ptr<Service>& two) const {
    std::string one_full(one->_service);
    std::string two_full(two->_service);
    return (one_full < two_full);
  }

  friend std::size_t hash_value(boost::shared_ptr<Service> const& item) {
    std::size_t seed = 0;
    boost::hash_combine(seed, item->_service);
    return seed;
  }

  friend std::size_t hash_value(Service const& item) {
    std::size_t seed = 0;
    boost::hash_combine(seed, item._service);
    return seed;
  }

  std::string get_resource() { return _resource; };

  void set_resource(const std::string& resource) { _resource = resource; }

  std::string get_token() { return _token; };

  void set_token(const std::string& token) { _token = token; }

  std::string get_service_path() { return _service_path; };

  void set_service_path(const std::string& service_path) {
    _service_path = service_path;
  }

  int get_timeout() { return _timeout; };

  void set_timeout(int timeout) { _timeout = timeout; }

  std::string get_service() { return _service; };

  void set_service(const std::string& service) { _service = service; }

  std::string get_cbroker() { return _cbroker; };

  void set_cbroker(const std::string& cbroker) { _cbroker = cbroker; }

  std::string get_real_name() const { return _service; }

 protected:
 private:
  int _timeout;
  std::string _service;
  std::string _service_path;
  std::string _cbroker;
  std::string _token;
  std::string _resource;
};
};

#endif
