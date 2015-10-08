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
#ifndef SRC_UTIL_STORE_CONST_H_
#define SRC_UTIL_STORE_CONST_H_

#include <string>
namespace iota {
namespace store {
struct types {
  virtual ~types(){};

  /** unique iotagent identifier to send to iota manager */
  static const std::string IOTAGENT_ID;
  /** name for iotagent for logs */
  static const std::string IOTAGENT_NAME;

  /// DEVICES
  static const std::string DEVICE_TABLE;
  static const std::string DEVICE_SORT;
  static const std::string DEVICES;
  static const std::string DEVICE_ID;
  static const std::string NAME;
  static const std::string ENDPOINT;
  static const std::string ENTITY_TYPE;
  static const std::string ENTITY_NAME;
  static const std::string SERVICE;
  static const std::string SERVICE_PATH;
  static const std::string STATUS;
  static const std::string COMMANDS;
  static const std::string COMMAND;
  static const std::string ATTRIBUTES;
  static const std::string ATTRIBUTE;
  static const std::string STATIC_ATTRIBUTES;
  static const std::string TIMEZONE;
  static const std::string MODEL;
  static const std::string ACTIVATE;
  static const std::string REGISTRATION_ID;
  static const std::string DURATION_CB;
  static const std::string TIMESTAMP_DATA;
  static const std::string ATTRIBUTE_ID;
  static const std::string ATTRIBUTE_VALUE;

  static const std::string TYPE;
  static const std::string ENTITY;

  /// COMMANDS
  static const std::string COMMAND_TABLE;
  static const std::string COMMAND_SORT;
  static const std::string ID;
  static const std::string COMMAND_ID;
  static const std::string TIMEOUT;
  static const std::string URI_RESP;
  static const std::string SEQUENCE;
  static const std::string DEVICE;
  static const std::string EXPIRED;
  static const std::string BODY;

  static const std::string STORAGE;
  static const std::string FILE;
  static const std::string HOST;
  static const std::string POOL_SIZE;
  static const std::string REPLICA_SET;
  static const std::string PORT;
  static const std::string USER;
  static const std::string PWD;
  static const std::string DBNAME;
  // indicate that use mongodb
  static const std::string MONGODB;

  // Types for values
  // Status
  static const std::string INACTIVE;
  static const std::string ACTIVE;
  static const std::string DEFAULT;

  /// SERVICE
  static const std::string SERVICE_TABLE;
  static const std::string SERVICE_SORT;
  static const std::string SERVICES;
  static const std::string SERVICE_ID;
  static const std::string RESOURCE;
  static const std::string APIKEY;
  static const std::string TOKEN;
  static const std::string CBROKER;
  static const std::string OUTGOING_ROUTE;
  static const std::string PUBLIC_IP;

  /// SERVICE MANAGER
  static const std::string PROTOCOL;
  static const std::string MANAGER_SERVICE_TABLE;
  static const std::string IOTAGENT;
  /// PROTOCOL
  static const std::string PROTOCOL_TABLE;
  static const std::string PROTOCOL_DESCRIPTION;
  static const std::string PROTOCOL_ID;
  static const std::string PROTOCOL_NAME;
  static const std::string ENDPOINTS;
  static const std::string ENDPOINT_AG;
  static const std::string ENDPOINT_ID;

  // Types for values
  // detailed
  static const std::string ON;
  static const std::string OFF;

  // Types for query uri
  static const std::string COUNT;
  static const std::string LIMIT;
  static const std::string OFFSET;
  static const std::string DETAILED;
  static const std::string DEVICE_PAR;

  /// LOCATION
  static const std::string COORDS;
};
};
};

#endif
