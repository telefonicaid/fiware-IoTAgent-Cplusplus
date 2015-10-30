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
#include "common.h"

namespace iota {

class Service : public virtual Timer {
 public:
  Service();

  Service(const std::string& service);

  virtual ~Service();

  boost::shared_ptr<JsonDocument> get_document();

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

    void put_metadata(const std::string& value, const std::string& type,
                      const std::string& name);

    void put_static_attribute(const std::string& value, const std::string& type,
                              const std::string& name);

    void put_attribute(const std::string& value, const std::string& type,
                       const std::string& name);

    std::string get(const std::string& field,
                    const std::string& default_value = std::string());

    int get(const std::string& field, int default_value);

    std::string get(const std::string& field, const std::string& default_value,
                    const iota::JsonValue& obj);

    int get(const std::string& field, int default_value,
            const iota::JsonValue& obj);

    iota::JsonValue& getObject(const std::string& field);

    iota::JsonValue& getObject(const std::string& field, iota::JsonValue& obj);

    void put(const std::string& field, const std::string& value);

    void put(const std::string& field, const std::string& value,
             iota::JsonValue& obj);

    void put(const std::string& field, int value);

    void put(const std::string& field, int value, iota::JsonValue& obj);

    void putObject(const std::string& resource_name);

    void putObject(const std::string& resource_name, iota::JsonValue& obj);

    std::string get_resource();

    void set_resource(const std::string& resource);

    std::string get_token();

    void set_token(const std::string& token);

    std::string get_service_path();

    void set_service_path(const std::string& service_path);

    int get_timeout();

    void set_timeout(int timeout);

    std::string get_service();

    void set_service(const std::string& service);

    std::string get_cbroker();

    void set_cbroker(const std::string& cbroker);

    std::string get_real_name() const;

    void read_xml_file(const std::string& file_path);

    std::string read_json(std::stringstream& _is);

    std::string toString() const;

    std::string toString(const iota::JsonValue& obj) const;

   protected:
   private:
    int _timeout;
    std::string _service;
    std::string _service_path;
    std::string _cbroker;
    std::string _token;
    std::string _resource;

    boost::shared_ptr<JsonDocument> _document;
};
};

#endif
