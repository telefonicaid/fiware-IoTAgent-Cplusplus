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
#include "store_const.h"

namespace iota {

namespace store {

const std::string types::IOTAGENT_ID("identifier");
const std::string types::IOTAGENT_NAME("name");

/// DEVICE_TABLE
const std::string types::DEVICE_TABLE("DEVICE");
const std::string types::DEVICE_SORT("device_id");
const std::string types::DEVICES("devices");
const std::string types::DEVICE_ID("device_id");
const std::string types::ENDPOINT("endpoint");
const std::string types::ENTITY_TYPE("entity_type");
const std::string types::ENTITY_NAME("entity_name");
const std::string types::SERVICE("service");
const std::string types::SERVICE_PATH("service_path");
const std::string types::ATTRIBUTES("attributes");
const std::string types::ATTRIBUTE("attribute");
const std::string types::STATIC_ATTRIBUTES("static_attributes");
const std::string types::STATUS("status");
const std::string types::COMMANDS("commands");
const std::string types::COMMAND("value");
const std::string types::NAME("name");
const std::string types::TIMEZONE("timezone");
const std::string types::MODEL("model");
const std::string types::ACTIVATE("activate");
const std::string types::REGISTRATION_ID("registration_id");
const std::string types::DURATION_CB("duration");
const std::string types::TIMESTAMP_DATA("timestamp_data");
const std::string types::ATTRIBUTE_ID("object_id");
const std::string types::ATTRIBUTE_VALUE("value");
const std::string types::TYPE("type");
const std::string types::ENTITY("entity");

/// COMMAND_TABLE
const std::string types::COMMAND_TABLE("COMMAND");
const std::string types::COMMAND_SORT("node");
const std::string types::COMMAND_ID("id");
const std::string types::ID("id");
const std::string types::TIMEOUT("timeout");
const std::string types::DEVICE("node");
const std::string types::URI_RESP("uri");
const std::string types::SEQUENCE("sequence");
const std::string types::EXPIRED("expired");
const std::string types::BODY("body");

/// SERVICE_TABLE
const std::string types::SERVICE_TABLE("SERVICE");
const std::string types::SERVICE_SORT("service");
const std::string types::SERVICE_ID("service");
const std::string types::SERVICES("services");
const std::string types::RESOURCE("resource");
const std::string types::APIKEY("apikey");
const std::string types::TOKEN("token");
const std::string types::CBROKER("cbroker");
const std::string types::OUTGOING_ROUTE("outgoing_route");
const std::string types::PUBLIC_IP("public_ip");

/// SERVICE_TABLE MANAGER
const std::string types::MANAGER_SERVICE_TABLE("SERVICE_MGMT");
const std::string types::PROTOCOL("protocol");
const std::string types::IOTAGENT("iotagent");
/// PROTOCOL_TABLE
const std::string types::PROTOCOL_TABLE("PROTOCOL");
const std::string types::PROTOCOL_DESCRIPTION("description");
const std::string types::PROTOCOL_NAME("protocol");
const std::string types::PROTOCOL_ID("_id");
const std::string types::ENDPOINTS("endpoints");
const std::string types::ENDPOINT_AG("endpoint");
const std::string types::ENDPOINT_ID("identifier");

// Also has resource from service table.

/// STORAGE
const std::string types::STORAGE("storage");
const std::string types::FILE("file");
const std::string types::HOST("host");
const std::string types::POOL_SIZE("pool_size");
const std::string types::REPLICA_SET("replicaset");
const std::string types::PORT("port");
const std::string types::USER("user");
const std::string types::PWD("pwd");
const std::string types::DBNAME("dbname");
const std::string types::MONGODB("mongodb");
// TODO añadir propiedades configurables de mongo

const std::string types::INACTIVE("inactive");
const std::string types::ACTIVE("active");
const std::string types::DEFAULT("default");

const std::string types::ON("on");
const std::string types::OFF("off");

// Types for query uri
const std::string types::COUNT("count");
const std::string types::LIMIT("limit");
const std::string types::OFFSET("offset");
const std::string types::DETAILED("detailed");
const std::string types::DEVICE_PAR("device");

const std::string types::COORDS("coords");
}
}
