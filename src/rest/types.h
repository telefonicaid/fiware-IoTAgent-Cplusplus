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
#ifndef SRC_REST_TYPES_H_
#define SRC_REST_TYPES_H_

#include <string>
namespace iota {

const int DEFAULT_TIMEOUT = 5;

struct types {
  virtual ~types(){};

  // Constants for configuration file
  static const std::string CONF_FILE_RESOURCES;
  static const std::string CONF_FILE_RESOURCE;
  static const std::string CONF_FILE_SERVICES;
  static const std::string CONF_FILE_APIKEY;
  static const std::string CONF_FILE_SERVICE;
  static const std::string CONF_FILE_SERVICE_PATH;
  static const std::string CONF_FILE_TOKEN;
  static const std::string CONF_FILE_CBROKER;
  static const std::string CONF_FILE_PROXY;
  static const std::string CONF_FILE_ENTITY_TYPE;
  static const std::string CONF_FILE_STATISTICS;

  static const std::string CONF_FILE_OPTIONS;
  static const std::string CONF_FILE_FILE_NAME;

  static const std::string CONF_FILE_TIMEOUT;
  static const std::string CONF_FILE_DIR_LOG;
  static const std::string CONF_FILE_TIMEZONES;
  static const std::string CONF_FILE_TCP_SERVERS;
  static const std::string CONF_FILE_IOTA_MANAGER;

  // Storage
  static const std::string CONF_FILE_DEVICES_STORE;
  static const std::string CONF_FILE_MONGO;
  static const std::string CONF_FILE_ERROR;

  // API Errors
  static const std::string ERROR_DELIMITER;
  static const std::string ERROR_BAD_REQUEST;

  // OAuth
  static const std::string CONF_FILE_OAUTH;
  static const std::string CONF_FILE_OAUTH_TRUST_TOKEN_URL;
  static const std::string CONF_FILE_OAUTH_VALIDATE_TOKEN_URL;
  static const std::string CONF_FILE_OAUTH_ROLES_URL;
  static const std::string CONF_FILE_OAUTH_PROJECTS_URL;
  static const std::string CONF_FILE_ACCESS_CONTROL;
  static const std::string CONF_FILE_PEP_USER;
  static const std::string CONF_FILE_PEP_PASSWORD;
  static const std::string CONF_FILE_PEP_DOMAIN;
  static const std::string CONF_FILE_ON_BEHALF_USER;
  static const std::string CONF_FILE_ON_BEHALF_PASSWORD;
  static const std::string CONF_FILE_OAUTH_TIMEOUT;
  static const std::string CONF_FILE_PEP_RULES;
  static const std::string CONF_FILE_PEP_RULES_URI;
  static const std::string CONF_FILE_PEP_RULES_ACTION;
  static const std::string CONF_FILE_PEP_RULES_VERB;

  static const std::string CONF_FILE_SCHEMA_PATH;
  // Error response
  static const std::string MULTIPART_ERROR;

  // https://github.com/telefonicaid/fiware-orion/blob/develop/src/lib/rest/HttpStatusCode.h
  // http response for ngsi operations

  static const std::string RESPONSE_MESSAGE_NONE;
  static const std::string RESPONSE_MESSAGE_OK;
  static const std::string RESPONSE_MESSAGE_FOUND;
  static const std::string RESPONSE_MESSAGE_BAD_REQUEST;
  static const std::string RESPONSE_MESSAGE_FORBIDDEN;
  static const std::string RESPONSE_MESSAGE_CONTEXT_ELEMENT_NOT_FOUND;
  static const std::string RESPONSE_MESSAGE_BAD_VERB;
  static const std::string RESPONSE_MESSAGE_METHOD_NOT_ALLOWED;
  static const std::string RESPONSE_MESSAGE_NOT_ACCEPTABLE;
  static const std::string RESPONSE_MESSAGE_NO_SERVICE;
  static const std::string RESPONSE_MESSAGE_NO_DEVICE;
  static const std::string RESPONSE_MESSAGE_BAD_PROTOCOL;
  static const std::string RESPONSE_MESSAGE_DATA_NOT_FOUND;
  static const std::string RESPONSE_MESSAGE_BAD_URI;
  static const std::string RESPONSE_MESSAGE_MORE_ONE_SERVICE;
  static const std::string RESPONSE_MESSAGE_DUPLICATED_DATA;
  static const std::string RESPONSE_MESSAGE_LENGTH_REQUIRED;
  static const std::string RESPONSE_MESSAGE_REQUEST_ENTITY_TOO_LARGE;
  static const std::string RESPONSE_MESSAGE_UNSUPPORTED_MEDIA_TYPE;
  static const std::string RESPONSE_MESSAGE_SUBSCRIPTION_ID_NOT_FOUND;
  static const std::string RESPONSE_MESSAGE_MISSING_PARAMETER;
  static const std::string RESPONSE_MESSAGE_INVALID_PARAMETER;
  static const std::string RESPONSE_MESSAGE_ERROR_IN_METADATA;
  static const std::string RESPONSE_MESSAGE_ENTITY_ID_RE_NOT_ALLOWED;
  static const std::string RESPONSE_MESSAGE_ENTITY_TYPE_REQUIRED;
  static const std::string RESPONSE_MESSAGE_ATTRIBUTE_LIST_REQUIRED;
  static const std::string RESPONSE_MESSAGE_RECEIVER_INTERNAL_ERROR;
  static const std::string RESPONSE_MESSAGE_NOT_IMPLEMENTED;
  static const std::string RESPONSE_MESSAGE_NO_REGISTERED_DEVICE;
  static const std::string RESPONSE_MESSAGE_NO_COMMAND;
  static const std::string RESPONSE_MESSAGE_NO_DEVICE_NO_COMMAND;
  static const std::string RESPONSE_MESSAGE_INTERNAL_ERROR;
  static const std::string RESPONSE_MESSAGE_BAD_GATEWAY;
  static const std::string RESPONSE_MESSAGE_DATABASE_ERROR;
  static const std::string RESPONSE_MESSAGE_BAD_CONFIG;
  static const std::string RESPONSE_MESSAGE_DEVICE_NO_ACTIVE;
  static const std::string RESPONSE_MESSAGE_COMMAND_BAD;
  static const std::string RESPONSE_MESSAGE_ENTITY_ALREADY_EXISTS;
  static const std::string RESPONSE_MESSAGE_ENTITY_COMMANDS_ALREADY_EXISTS;
  static const std::string RESPONSE_MESSAGE_MISSING_IOTAGENTS;

  static const std::string REASON_MALFORMED_HEADER;
  static const std::string REASON_MALFORMED_DATA;

  static const std::string DETAILS_HEADER_FIWARE_SERVICE;
  static const std::string DETAILS_HEADER_FIWARE_SERVICE_PATH;
  static const std::string DETAILS_FORBIDDEN_CHARACTERS;

  static const std::string IOTA_FORBIDDEN_CHARACTERS;

  static const unsigned int RESPONSE_CODE_NONE;
  static const unsigned int RESPONSE_CODE_OK;
  static const unsigned int RESPONSE_CODE_FOUND;
  static const unsigned int RESPONSE_CODE_BAD_REQUEST;
  static const unsigned int RESPONSE_CODE_FORBIDDEN;
  static const unsigned int RESPONSE_CODE_CONTEXT_ELEMENT_NOT_FOUND;
  static const unsigned int RESPONSE_CODE_BAD_VERB;
  static const unsigned int RESPONSE_CODE_METHOD_NOT_ALLOWED;
  static const unsigned int RESPONSE_CODE_NOT_ACCEPTABLE;
  static const unsigned int RESPONSE_CODE_NO_SERVICE;
  static const unsigned int RESPONSE_CODE_MORE_ONE_SERVICE;
  static const unsigned int RESPONSE_CODE_LENGTH_REQUIRED;
  static const unsigned int RESPONSE_CODE_REQUEST_ENTITY_TOO_LARGE;
  static const unsigned int RESPONSE_CODE_UNSUPPORTED_MEDIA_TYPE;
  static const unsigned int RESPONSE_CODE_SUBSCRIPTION_ID_NOT_FOUND;
  static const unsigned int RESPONSE_CODE_MISSING_PARAMETER;
  static const unsigned int RESPONSE_CODE_INVALID_PARAMETER;
  static const unsigned int RESPONSE_CODE_ERROR_IN_METADATA;
  static const unsigned int RESPONSE_CODE_ENTITY_ID_RE_NOT_ALLOWED;
  static const unsigned int RESPONSE_CODE_ENTITY_TYPE_REQUIRED;
  static const unsigned int RESPONSE_CODE_ATTRIBUTE_LIST_REQUIRED;
  static const unsigned int RESPONSE_CODE_RECEIVER_INTERNAL_ERROR;
  static const unsigned int RESPONSE_CODE_NOT_IMPLEMENTED;
  static const unsigned int RESPONSE_CODE_NO_REGISTERED_DEVICE;
  static const unsigned int RESPONSE_CODE_NO_COMMAND;
  static const unsigned int RESPONSE_CODE_NO_DEVICE_NO_COMMAND;
  static const unsigned int RESPONSE_CODE_BAD_CONFIG;
  static const unsigned int RESPONSE_CODE_BAD_GATEWAY;
  static const unsigned int RESPONSE_CODE_DEVICE_NO_ACTIVE;
  static const unsigned int RESPONSE_CODE_ENTITY_ALREADY_EXISTS;
  static const unsigned int RESPONSE_CODE_DATA_NOT_FOUND;

  // Special codes
  static const unsigned int RESPONSE_CODE_FIWARE_SERVICE_ERROR;
  static const unsigned int RESPONSE_CODE_FIWARE_SERVICE_PATH_ERROR;
  static const unsigned int RESPONSE_CODE_FORBIDDEN_CHARACTERS;

  /// ALARMS
  static const unsigned int ALARM_CODE_BAD_CONFIGURATION;
  static const unsigned int ALARM_CODE_NO_MONGO;
  static const unsigned int ALARM_CODE_NO_CB;
  static const unsigned int ALARM_CODE_NO_IOTA;
  static const unsigned int ALARM_CODE_NO_IOTA_MANAGER;

  static const unsigned int ALARM_CODE_LOST_MEASURE;
  static const unsigned int ALARM_CODE_LOST_API;

  static const std::string ALARM_MESSAGE_NO_MONGO;
  static const std::string ALARM_MESSAGE_NO_CB;
  static const std::string ALARM_MESSAGE_NO_IOTA;
  static const std::string ALARM_MESSAGE_NO_IOTA_MANAGER;

  // Header to trace messages
  static const std::string HEADER_TRACE_MESSAGES;
  static const std::string HEADER_OUTGOING_ROUTE;
  static const std::string HEADER_INTERNAL_TYPE;

  // Counters
  static const std::string STAT_TRAFFIC;
  static const std::string STAT_TRAFFIC_IN;
  static const std::string STAT_TRAFFIC_OUT;
  static const std::string STAT_TRAFFIC_DURATION;

  // header Fiware
  static const std::string FIWARE_SERVICE;
  static const std::string FIWARE_SERVICEPATH;
  static const std::string FIWARE_SERVICEPATH_DEFAULT;
  static const std::string IOT_HTTP_HEADER_ACCEPT;
  static const std::string IOT_CONTENT_TYPE_JSON;
  static const std::string IOT_CONTENT_SUBTYPE_JSON;
  static const std::string IOT_HTTP_HEADER_AUTH;

  // Manager errors array
  static const std::string IOT_MANAGER_ERRORS;
  static const std::string IOT_MANAGER_ERRORS_CODE;
  static const std::string IOT_MANAGER_ERRORS_DETAILS;
  static const std::string IOT_MANAGER_ERRORS_ENDPOINT;

  static const int LIMIT_DEFAULT;
  static const int LIMIT_MAX;

  // CACHE SIZES
  static const int MAX_SIZE_CACHE;
  static const int MAX_SIZE_COMMAND_CACHE;

  // COMMANDS
  static const std::string STATUS;
  static const std::string STATUS_TYPE;
  static const std::string STATUS_OP;

  static const std::string INFO;
  static const std::string INFO_TYPE;
  static const std::string INFO_OP;

  static const std::string COMMAND_TYPE;
  static const std::string PENDING;
  static const std::string ERROR;
  static const std::string RAW;

  // STATUS COMMANDS

  static const int READY_FOR_READ;
  static const std::string READY_FOR_READ_MESSAGE;
  static const int EXPIRED_READ;
  static const std::string EXPIRED_READ_MESSAGE;
  static const int DELIVERED;
  static const std::string DELIVERED_MESSAGE;
  static const int EXPIRED_DELIVERY;
  static const std::string EXPIRED_DELIVERY_MESSAGE;
  static const int EXECUTED;
  static const std::string EXECUTED_MESSAGE;
  static const int NOT_DELIVERED;
  static const std::string NOT_DELIVERED_MESSAGE;

  // ERROR MESSAGE
  static const std::string FORMATERROR_DELIMITER;
  static const std::string FORMATERROR_INTERNAL_ERROR;
  static const std::string FORMATERROR_ATTRIBUTE_NO_COMMAND;
  static const std::string FORMATERROR_PARAM_NO_INTEGER;
  static const std::string FORMATERROR_NO_COMMAND;
  static const std::string FORMATERROR_NO_HEADER_SERVICE;
  static const std::string FORMATERROR_NO_SERVICE;
  static const std::string FORMATERROR_SERVICE_NO_ACTIVE;

  static const std::string HELP_MESSAGE_OPS;
  static const std::string HELP_MESSAGE;
  static const std::string HELP_MESSAGE_ERR_CONFIG;
  static const std::string HELP_MESSAGE_ERR_PARAM;
  static const std::string HELP_MESSAGE_ERR_BAD_PARAM;
  static const std::string HELP_MESSAGE_ERR_PARAMO;

  static const char* ERROR_MESSAGE[];
};

typedef struct ProtocolData {
  std::string protocol;
  std::string description;
} ProtocolData;

typedef struct PepRule {
  std::string verb;
  std::string uri;
} PepRule;
}
#endif
