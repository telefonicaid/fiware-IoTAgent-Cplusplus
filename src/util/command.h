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
/*
 *
 *
 */

#ifndef SRC_UTIL_COMMAND_H_
#define SRC_UTIL_COMMAND_H_

#include <string>
#include "timer.h"
#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>

namespace iota {

class Command : public virtual Timer {
 public:
  Command(const std::string& id_entity, const std::string& service,
          const std::string& service_path);

  Command(const std::string& id_cmd, const std::string& service,
          const std::string& service_path, const std::string& sequence,
          const std::string& id_entity, const std::string& responseURI,
          const boost::property_tree::ptree& cmd);

  // Constructor para la recuperacion de comandos almacenados
  Command(const std::string& id_cmd, const std::string& name,
          const std::string& service, const std::string& service_path,
          const std::string& sequence, const std::string& id_entity,
          const std::string& entity_type, const std::string& responseURI,
          const int timeout, const std::string& timestamp,
          const boost::property_tree::ptree& cmd);

  ~Command(void);

  bool operator==(const Command& a) const {
    if ((_sequence.compare(a._sequence) == 0) &&
        (_service.compare(a._service) == 0)) {
      return true;
    }
    return false;
  }

  bool operator==(const boost::shared_ptr<Command>& a) const {
    if ((_sequence.compare(a->_sequence) == 0) &&
        (_service.compare(a->_service) == 0)) {
      return true;
    }
    return false;
  }

  bool operator()(const Command& one, const Command& two) const {
    std::string one_full(one._sequence);
    one_full.append(one._service);
    std::string two_full(two._sequence);
    two_full.append(two._service);
    return (one_full < two_full);
  }

  bool operator()(const boost::shared_ptr<Command>& one,
                  const boost::shared_ptr<Command>& two) const {
    std::string one_full(one->_sequence);
    one_full.append(one->_service);
    std::string two_full(two->_sequence);
    two_full.append(two->_service);
    return (one_full < two_full);
  }

  friend std::size_t hash_value(boost::shared_ptr<Command> const& item) {
    std::size_t seed = 0;
    boost::hash_combine(seed, item->_sequence);
    boost::hash_combine(seed, item->_service);
    boost::hash_combine(seed, item->_service_path);
    return seed;
  }

  friend std::size_t hash_value(Command const& item) {
    std::size_t seed = 0;
    boost::hash_combine(seed, item._sequence);
    boost::hash_combine(seed, item._service);
    boost::hash_combine(seed, item._service_path);
    return seed;
  }

  std::string unique_entity() const {
    return _service + ":" + _service_path + ":" + _entity + ":" +
           boost::lexical_cast<std::string>(_status);
  }

  std::string unique_id() const {
    return _service + ":" + _service_path + ":" + _id;
  }

  int get_timeout(void) const { return _timeout; };

 void set_timeout(int t_sec) { _timeout = t_sec; };

  std::string get_sequence(void) const { return _sequence; };

  void set_sequence(const std::string& sequence) { _sequence = sequence; };

  std::string get_entity(void) const { return _entity; };

  void set_entity(const std::string& nodo) { _entity = nodo; };

  void set_id(const std::string& id) { _id = id; };

  std::string get_id() const { return _id; };

  std::string get_uri_resp(void) const { return (_responseURI); };
  void set_uri_resp(const std::string& uri) { _responseURI = uri; };

  int get_expired(void) const { return (_expired); }

  void set_expired(int data) { _expired = data; }

  boost::property_tree::ptree get_command() const { return _command; };

  void set_command(const boost::property_tree::ptree& cmd) { _command = cmd; }

  int get_status() const { return _status; };

  void set_status(int status) { _status = status; }

  std::string get_service() const { return _service; };

  void set_service(const std::string& service) { _service = service; }

  std::string get_service_path() const { return _service_path; };

  void set_service_path(const std::string& service_path) {
    _service_path = service_path;
  }

  std::string get_entity_type() const { return _entity_type; };

  void set_entity_type(const std::string& entity_type) {
    _entity_type = entity_type;
  }

  std::string get_name() const { return _name; };

  void set_name(const std::string& name) { _name = name; }

 protected:
 private:
  // Timeout asociado al comando (segundos)
  int _timeout;

  // Nodo destino al comando
  std::string _entity;

  std::string _entity_type;

  // URI respuesta
  std::string _responseURI;

  // Identificador de comando, deberia ser unico
  std::string _id;

  // Identificador de comando, deberia ser unico
  std::string _name;

  // Si esta expirado
  int _expired;

  // Si el comando se recupera por polling, debe almacenarse
  boost::property_tree::ptree _command;

  // estado del comando
  int _status;

  // estado del comando
  std::string _service;
  std::string _service_path;

  // sequence, specific id between CB and IotAgent
  std::string _sequence;
};

struct entity_command_hash {
  std::size_t operator()(Command const& item) const {
    return boost::hash<std::string>()(item.unique_entity());
  }
  std::size_t operator()(std::string item) const {
    return boost::hash<std::string>()(item);
  }
};

struct entity_command_equal {
  bool operator()(Command const& item, std::string ent) const {
    return (item.unique_entity().compare(ent) == 0);
  }

  bool operator()(std::string ent, Command const& item) const {
    return (item.unique_entity().compare(ent) == 0);
  }
};

struct id_command_hash {
  std::size_t operator()(Command const& item) const {
    return boost::hash<std::string>()(item.unique_id());
  }
  std::size_t operator()(std::string item) const {
    return boost::hash<std::string>()(item);
  }
};

struct id_command_equal {
  bool operator()(Command const& item, std::string id) const {
    return (item.unique_id().compare(id) == 0);
  }

  bool operator()(std::string id, Command const& item) const {
    return (item.unique_id().compare(id) == 0);
  }
};
};

#endif
