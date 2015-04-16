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
#include "rest/types.h"

namespace iota {

// Configuration
const std::string types::CONF_FILE_RESOURCES("resources");
const std::string types::CONF_FILE_RESOURCE("resource");
const std::string types::CONF_FILE_SERVICES("services");
const std::string types::CONF_FILE_APIKEY("apikey");
const std::string types::CONF_FILE_SERVICE("service");
const std::string types::CONF_FILE_SERVICE_PATH("service_path");
const std::string types::CONF_FILE_TOKEN("token");
const std::string types::CONF_FILE_CBROKER("cbroker");
const std::string types::CONF_FILE_PROXY("http_proxy");
const std::string types::CONF_FILE_ENTITY_TYPE("entity_type");
const std::string types::CONF_FILE_STATISTICS("statistics");

const std::string types::CONF_FILE_OPTIONS("options");
const std::string types::CONF_FILE_FILE_NAME("FileName");

const std::string types::CONF_FILE_TIMEOUT("timeout");
const std::string types::CONF_FILE_DIR_LOG("dir_log");
const std::string types::CONF_FILE_TIMEZONES("timezones");

const std::string types::CONF_FILE_IOTA_MANAGER("iota_manager");


const std::string types::CONF_FILE_DEVICES_STORE("devices_store");

const std::string types::CONF_FILE_MONGO("mongodb");

// OAuth
const std::string types::CONF_FILE_OAUTH("oauth");
const std::string types::CONF_FILE_OAUTH_TRUST_TOKEN_URL("on_behalf_trust_url");
const std::string
types::CONF_FILE_OAUTH_VALIDATE_TOKEN_URL("validate_token_url");
const std::string types::CONF_FILE_OAUTH_ROLES_URL("get_roles_url");
const std::string
types::CONF_FILE_OAUTH_PROJECTS_URL("retrieve_subservices_url");
const std::string types::CONF_FILE_ACCESS_CONTROL("access_control_url");
const std::string types::CONF_FILE_PEP_USER("pep_user");
const std::string types::CONF_FILE_PEP_PASSWORD("pep_password");
const std::string types::CONF_FILE_PEP_DOMAIN("pep_domain");
const std::string types::CONF_FILE_ON_BEHALF_USER("on_behalf_user");
const std::string types::CONF_FILE_ON_BEHALF_PASSWORD("on_behalf_password");
const std::string types::CONF_FILE_OAUTH_TIMEOUT("timeout");

// Errors
const std::string types::MULTIPART_ERROR("Multipart content error");

//
const std::string types::CONF_FILE_ERROR("Configuration file with errors");

//https://github.com/telefonicaid/fiware-orion/blob/develop/src/lib/rest/HttpStatusCode.h
const std::string types::RESPONSE_MESSAGE_NONE("Undefined");
const std::string types::RESPONSE_MESSAGE_OK("OK");
const std::string types::RESPONSE_MESSAGE_FOUND("Found");
const std::string
types::RESPONSE_MESSAGE_BAD_REQUEST("The request is not well formed");
const std::string
types::RESPONSE_MESSAGE_FORBIDDEN("The request is not allowed");
const std::string
types::RESPONSE_MESSAGE_METHOD_NOT_ALLOWED("The method is not allowed");

const std::string
types::RESPONSE_MESSAGE_CONTEXT_ELEMENT_NOT_FOUND("No context element found");
const std::string
types::RESPONSE_MESSAGE_BAD_VERB("Request ok but verb/method NOT OK");
const std::string
types::RESPONSE_MESSAGE_BAD_URI("An uri parameter is not valid");
const std::string
types::RESPONSE_MESSAGE_NOT_ACCEPTABLE("The Accept header in the request is not supported");
const std::string
types::RESPONSE_MESSAGE_LENGTH_REQUIRED("Content-Length header missing");
const std::string
types::RESPONSE_MESSAGE_REQUEST_ENTITY_TOO_LARGE("Request Entity Too Large - over 1Mb of payload");
const std::string
types::RESPONSE_MESSAGE_UNSUPPORTED_MEDIA_TYPE("Unsupported Media Type (only support application/xml and application/json)");
const std::string
types::RESPONSE_MESSAGE_SUBSCRIPTION_ID_NOT_FOUND("The subscriptionId does not correspond to an active subscription");
const std::string
types::RESPONSE_MESSAGE_MISSING_PARAMETER("A parameter is missing in the request");
const std::string
types::RESPONSE_MESSAGE_INVALID_PARAMETER("A parameter of the request is invalid/not allowed");
const std::string
types::RESPONSE_MESSAGE_ERROR_IN_METADATA("Generic error in metadata (e.g. 'expires' older than 'timestamp')");
const std::string
types::RESPONSE_MESSAGE_ENTITY_ID_RE_NOT_ALLOWED("Regular Expression for EntityId is not allowed by receiver");
const std::string
types::RESPONSE_MESSAGE_ENTITY_TYPE_REQUIRED("The EntityType is required by the receiver");
const std::string
types::RESPONSE_MESSAGE_ATTRIBUTE_LIST_REQUIRED("The Attribute List is required by the receiver");
const std::string
types::RESPONSE_MESSAGE_RECEIVER_INTERNAL_ERROR("An unknown error at the receiver has occurred");
const std::string
types::RESPONSE_MESSAGE_NOT_IMPLEMENTED("The given operation is not implemented");
const std::string
types::RESPONSE_MESSAGE_NO_REGISTERED_DEVICE("The given device is not registered in IotAgent");
const std::string
types::RESPONSE_MESSAGE_NO_COMMAND("The attribute must be of type command");
const std::string
types::RESPONSE_MESSAGE_NO_DEVICE_NO_COMMAND("the device does not have implemented this command");

const std::string
types::RESPONSE_MESSAGE_COMMAND_BAD("this is not a valid command");

const std::string
types::RESPONSE_MESSAGE_DEVICE_NO_ACTIVE("The device is not active");

const std::string
types::RESPONSE_MESSAGE_BAD_CONFIG("The config file for iotagent is not correct");

const std::string
types::RESPONSE_MESSAGE_NO_SERVICE("The service does not exist");

const std::string
types::RESPONSE_MESSAGE_NO_DEVICE("The device does not exist");

const std::string
types::RESPONSE_MESSAGE_MORE_ONE_SERVICE("There are conflicts in provisioned service");

const std::string
types::RESPONSE_MESSAGE_DUPLICATED_DATA("There are conflicts, object already exists");

const std::string
types::RESPONSE_MESSAGE_DATA_NOT_FOUND("Field or data not founded");


const std::string
types::RESPONSE_MESSAGE_INTERNAL_ERROR("Conctact with your administrator, there was an internal error");
const std::string types::RESPONSE_MESSAGE_DATABASE_ERROR("Database error");

const std::string
types::RESPONSE_MESSAGE_ENTITY_ALREADY_EXISTS("There are conflicts, entity already exists");


const unsigned int types::RESPONSE_CODE_NONE = 0;
const unsigned int types::RESPONSE_CODE_OK = 200;
const unsigned int types::RESPONSE_CODE_FOUND = 302;
const unsigned int types::RESPONSE_CODE_BAD_REQUEST = 400;
const unsigned int types::RESPONSE_CODE_BAD_CONFIG = 400;
const unsigned int types::RESPONSE_CODE_FORBIDDEN = 403;
const unsigned int types::RESPONSE_CODE_CONTEXT_ELEMENT_NOT_FOUND = 404;
const unsigned int types::RESPONSE_CODE_NO_SERVICE=404;
const unsigned int types::RESPONSE_CODE_MORE_ONE_SERVICE=409;
const unsigned int types::RESPONSE_CODE_DEVICE_NO_ACTIVE=409;
const unsigned int types::RESPONSE_CODE_ENTITY_ALREADY_EXISTS=409;
const unsigned int types::RESPONSE_CODE_BAD_VERB = 405;
const unsigned int types::RESPONSE_CODE_METHOD_NOT_ALLOWED = 405;
const unsigned int types::RESPONSE_CODE_NOT_ACCEPTABLE = 406;
const unsigned int types::RESPONSE_CODE_LENGTH_REQUIRED = 411;
const unsigned int types::RESPONSE_CODE_REQUEST_ENTITY_TOO_LARGE = 413;
const unsigned int types::RESPONSE_CODE_UNSUPPORTED_MEDIA_TYPE = 415;
const unsigned int types::RESPONSE_CODE_SUBSCRIPTION_ID_NOT_FOUND = 470;
const unsigned int types::RESPONSE_CODE_MISSING_PARAMETER = 471;
const unsigned int types::RESPONSE_CODE_INVALID_PARAMETER = 472;
const unsigned int types::RESPONSE_CODE_ERROR_IN_METADATA = 473;
const unsigned int types::RESPONSE_CODE_ENTITY_ID_RE_NOT_ALLOWED = 480;
const unsigned int types::RESPONSE_CODE_ENTITY_TYPE_REQUIRED = 481;
const unsigned int types::RESPONSE_CODE_ATTRIBUTE_LIST_REQUIRED = 482;
const unsigned int types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR = 500;
const unsigned int types::RESPONSE_CODE_NOT_IMPLEMENTED = 501;
const unsigned int types::RESPONSE_CODE_NO_REGISTERED_DEVICE = 404;
const unsigned int types::RESPONSE_CODE_NO_COMMAND = 400;
const unsigned int types::RESPONSE_CODE_NO_DEVICE_NO_COMMAND = 400;
const unsigned int types::RESPONSE_CODE_DATA_NOT_FOUND  =404;


///ALARMS
const unsigned int types::ALARM_CODE_NO_MONGO = 100;
const unsigned int types::ALARM_CODE_NO_CB = 200;

const std::string
types::ALARM_MESSAGE_NO_MONGO("couldn't connect to mongo ");
const std::string
types::ALARM_MESSAGE_NO_CB("couldn't connect to context broker ");

const std::string types::ERROR_DELIMITER = "@@ERROR_DELIMITER@@";
const std::string types::ERROR_BAD_REQUEST = "@@ERROR_BAD_REQUEST@@";

const std::string types::HEADER_TRACE_MESSAGES("X-Trace-Message");
const std::string types::HEADER_OUTGOING_ROUTE("X-M2M-Outgoing-Route");
const std::string types::HEADER_INTERNAL_TYPE("X-Async-Internal");

const std::string types::STAT_TRAFFIC("traffic");
const std::string types::STAT_TRAFFIC_IN("in");
const std::string types::STAT_TRAFFIC_OUT("out");
const std::string types::STAT_TRAFFIC_DURATION("duration");

const std::string types::FIWARE_SERVICE("Fiware-Service");
const std::string types::FIWARE_SERVICEPATH("Fiware-ServicePath");
const std::string types::FIWARE_SERVICEPATH_DEFAULT("/");
const std::string types::IOT_HTTP_HEADER_ACCEPT("Accept");
const std::string types::IOT_CONTENT_TYPE_JSON("application/json");
const std::string types::IOT_CONTENT_SUBTYPE_JSON("/json");
const std::string types::IOT_HTTP_HEADER_AUTH("X-Auth-Token");

const int types::LIMIT_DEFAULT = 20;
const int types::LIMIT_MAX = 1000;

//CACHE SIZES
const int types::MAX_SIZE_CACHE = 1000;
const int types::MAX_SIZE_COMMAND_CACHE = 1000;


//COMMANDS
const std::string types::STATUS("status");
const std::string types::STATUS_TYPE("string");
const std::string types::STATUS_OP("APPEND");

const std::string types::INFO("info");
const std::string types::INFO_TYPE("string");
const std::string types::INFO_OP("APPEND");

const std::string types::ERROR("error");
const std::string types::COMMAND_TYPE("command");
const std::string types::PENDING("pending");
const std::string types::RAW("@@RAW@@");



//STATUS COMMANDS
const int types::READY_FOR_READ = 0;
const std::string types::READY_FOR_READ_MESSAGE("pending");
const int types::EXPIRED_READ = 1;
const std::string types::EXPIRED_READ_MESSAGE("expired read");
const int types::DELIVERED = 2;
const std::string types::DELIVERED_MESSAGE("delivered");
const int types::EXPIRED_DELIVERY = 3;
const std::string types::EXPIRED_DELIVERY_MESSAGE("delivered but no respond");
const int types::EXECUTED = 4;
const std::string types::EXECUTED_MESSAGE("OK");
const int types::NOT_DELIVERED = 5;
const std::string types::NOT_DELIVERED_MESSAGE("not delivered");


}


