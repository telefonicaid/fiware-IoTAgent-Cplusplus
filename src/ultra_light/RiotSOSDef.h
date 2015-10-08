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
 * File: RiotSOSDef.h
 * Author: desgw (Telefonica Investigacion y Desarrollo)
 *
 * Created on jue sep 29 09:45:55 CEST 2011
 *
 */

#ifndef RIOTSOSDEF_H
#define RIOTSOSDEF_H

#include <pion/http/types.hpp>
#include <KVP.h>
#include <boost/uuid/uuid.hpp>
#include <vector>

namespace iota {

// Operaciones
#define SOS_REGISTER_SENSOR "RegisterSensor"
#define SOS_INSERT_OBSERVATION "InsertObservation"
#define SOS_ALARM_GATEWAY "alarm"
#define SOS_COMMAND_RESPONSE "commandResponse"

// Parametros
// UniversalIdentifierOfLogicalHub
#define QUERY_PARAM_UC "UC"

// Namespace UUID
const boost::uuids::uuid namespace_uuid = {{0x6b, 0xa7, 0xb8, 0x10, 0x9d, 0xad,
                                            0x11, 0xd1, 0x80, 0xb4, 0x00, 0xc0,
                                            0x4f, 0xd4, 0x30, 0xc8}};

// Limite superior (en numero de mensajes) que puede alcanzar la cola de envio
// hacia
// el destino.
const int NUM_MAX_PENDING_MESSAGES = 20000;

// Estructura con datos HTTP necesarios para la aplicacion y
// que deben ser pasados por SOAP
struct HTTPData {
  std::string method;
  std::string ip;
  std::string res;
  std::vector<KVP> query;
  ////FFFF   pion::net::HTTPTypes::Headers h_http;
  std::string respuesta;
  std::string id_registro;
};

struct RiotServerData {
  std::string address;
  int port;
  int port_sip;
  std::string ssl_pem_file;
  // int ssl_port;
};

struct RiotResponseInformation {
  std::string id_operacion;
  std::string id_message;
  int code;
  std::string txt;
};
};
#endif
