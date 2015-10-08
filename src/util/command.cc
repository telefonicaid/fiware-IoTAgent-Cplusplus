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
#include "command.h"
#include "RiotISO8601.h"

iota::Command::Command(const std::string& id_entity, const std::string& service,
                       const std::string& service_path) {
  _service = service;
  _service_path = service_path;
  _entity = id_entity;

  _id = "";
  _expired = INT_MIN;
  _timeout = INT_MIN;
  _status = INT_MIN;
}

iota::Command::Command(const std::string& id_cmd, const std::string& service,
                       const std::string& service_path,
                       const std::string& sequence,
                       const std::string& id_entity,
                       const std::string& responseURI,
                       const boost::property_tree::ptree& cmd) {
  _id = id_cmd;
  _service = service;
  _service_path = service_path;
  _sequence = sequence;
  _entity = id_entity;
  _responseURI = responseURI;
  _command = cmd;

  _expired = INT_MIN;
  _timeout = INT_MIN;
  _status = INT_MIN;
}

// Constructor para la recuperacion de comandos almacenados
iota::Command::Command(const std::string& id_cmd, const std::string& name,
                       const std::string& service,
                       const std::string& service_path,
                       const std::string& sequence,
                       const std::string& id_entity,
                       const std::string& entity_type,
                       const std::string& responseURI, const int timeout,
                       const std::string& timestamp,
                       const boost::property_tree::ptree& cmd) {
  _id = id_cmd;
  _name = name;
  _service = service;
  _service_path = service_path;
  _sequence = sequence;
  _entity = id_entity;
  _entity_type = entity_type;
  _responseURI = responseURI;
  _command = cmd;
  if (timeout < 0) {
    _timeout = INT_MIN;
  } else {
    _timeout = timeout;
  }

  // El timestamp sirva para saber si ha expirado (junto con el timeout)
  if (!timestamp.empty()) {
    iota::RiotISO8601 tstamp(timestamp);
    iota::RiotISO8601 ahora;
    boost::posix_time::time_duration diff =
        ahora.getPosixTime() - tstamp.getPosixTime();

    if (diff.total_milliseconds() >= boost::lexical_cast<double>(_timeout)) {
      _expired = 1;
    } else {
      // Actualizacion timeout
      _timeout = boost::lexical_cast<int>(
          boost::lexical_cast<double>(_timeout) - diff.total_seconds());
      _expired = 0;
    }
  } else {
    _expired = 0;
  }

  _timeout = timeout;
  _status = INT_MIN;
}

iota::Command::~Command(void) { cancel(); }
