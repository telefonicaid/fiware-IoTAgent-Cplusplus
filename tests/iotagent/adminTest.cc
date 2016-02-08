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
#include "adminTest.h"

#include <Variant/Schema.h>
#include <Variant/SchemaLoader.h>

#include <iostream>
#include <string>
#include <sstream>

#if defined LIBVARIANT

#include <Variant/Schema.h>
#include <Variant/SchemaLoader.h>

#endif  // defined

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "util/iota_exception.h"

#include "util/iota_logger.h"
#include <pion/process.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include "util/device_collection.h"
#include "util/service_collection.h"
#include "util/service_mgmt_collection.h"
#include "services/admin_service.h"

#define PATH_CONFIG "../../tests/iotagent/config_mongo.json"
#define PATH_CONFIG_WITH_AUTH "../../tests/iotagent/config_auth.json"

#define IOTASSERT_MESSAGE(x, y)                          \
  std::cout << "@" << __LINE__ << "@" << x << std::endl; \
  CPPUNIT_ASSERT_MESSAGE(x, y)

#define IOTASSERT(y)                                \
  std::cout << "@" << __LINE__ << "@" << std::endl; \
  CPPUNIT_ASSERT(y)

#define ASYNC_TIME_WAIT \
  boost::this_thread::sleep(boost::posix_time::milliseconds(8000));

CPPUNIT_TEST_SUITE_REGISTRATION(AdminTest);
iota::AdminService* AdminService_ptr;
namespace iota {
std::string URL_BASE = "/iot";
std::string logger("main");
}

const std::string AdminTest::HOST("127.0.0.1");
const std::string AdminTest::CONTENT_JSON("application/json");

////////////
//// DEVICES
const std::string AdminTest::URI_DEVICE("/devices");
// POST
const std::string AdminTest::POST_DEVICE(
    "{\"devices\": "
    "[{\"device_id\": \"device_id\",\"protocol\": "
    "\"PDI-IoTA-UltraLight\",\"entity_name\": \"entity_name\",\"entity_type\": "
    "\"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": "
    "\"America/Santiago\","
    "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
    "\"device_id@ping|%s\" }],"
    "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
    "\"temperature\",\"type\": \"int\" }],"
    "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
    "\"value\": \"50\"  }]"
    "}]}");

const std::string AdminTest::BAD_PROTOCOL_POST_DEVICE(
    "{\"devices\": "
    "[{\"device_id\": \"device_id\",\"protocol\": \"kk\",\"entity_name\": "
    "\"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": "
    "\"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
    "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
    "\"device_id@ping|%s\" }],"
    "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
    "\"temperature\",\"type\": \"int\" }],"
    "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
    "\"value\": \"50\"  }]"
    "}]}");

const std::string AdminTest::BAD_POST_DEVICE(
    "{\"devices\": "
    "[{\"device_id\": \"device_id\",\"protocol\": "
    "\"PDI-IoTA-UltraLight\",\"entity_namee\": "
    "\"entity_namee\",\"entity_type\": \"entity_type\",\"endpoint\": "
    "\"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
    "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
    "\"device_id@ping|%s\" }],"
    "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
    "\"temperature\",\"type\": \"int\" }],"
    "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
    "\"value\": \"50\"  }]"
    "}]}");
const std::string BAD_PROTOCOL_POST_DEVICE(
    "{\"devices\": "
    "[{\"device_id\": \"device_id\",\"protocol\": \"kk\",\"entity_name\": "
    "\"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": "
    "\"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
    "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
    "\"device_id@ping|%s\" }],"
    "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
    "\"temperature\",\"type\": \"int\" }],"
    "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
    "\"value\": \"50\"  }]"
    "}]}");

const std::string AdminTest::BAD_POST_DEVICE2(
    "{\"devices\": "
    "[{\"device_id\": \"device_id\",\"protocol\": "
    "\"PDI-IoTA-UltraLight\",\"entity_name\": \"entity_name\",\"entity_type\": "
    "\"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": "
    "\"America/Santiago\","
    "\"commands\": [{\"namee\": \"ping\",\"type\": \"command\",\"value\": "
    "\"device_id@ping|%s\" }],"
    "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
    "\"temperature\",\"type\": \"int\" }],"
    "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
    "\"value\": \"50\"  }]"
    "}]}");
const std::string AdminTest::BAD_POST_DEVICE3(
    "{\"devices\": "
    "[{\"device_id\": \"device_id\",\"protocol\": "
    "\"PDI-IoTA-UltraLight\",\"entity_name\": \"entity_name\",\"entity_type\": "
    "\"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": "
    "\"America/Santiago\","
    "\"commands\": [{\"bomba\":\"bomba\",\"name\": \"ping\",\"type\": "
    "\"command\",\"value\": \"device_id@ping|%s\" }],"
    "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
    "\"temperature\",\"type\": \"int\" }],"
    "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
    "\"value\": \"50\"  }]"
    "}]}");

const std::string AdminTest::BAD_POST_DEVICE_NO_DEVICE_ID(
    "{\"devices\": "
    "[{\"protocol\": \"PDI-IoTA-UltraLight\",\"entity_name\": "
    "\"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": "
    "\"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
    "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
    "\"device_id@ping|%s\" }],"
    "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
    "\"temperature\",\"type\": \"int\" }],"
    "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
    "\"value\": \"50\"  }]"
    "}]}");

const int AdminTest::POST_RESPONSE_CODE = 201;
const std::string AdminTest::POST_RESPONSE("");
const std::string AdminTest::POST_DEVICE_NO_DEVICE_ID(
    "{\"devices\": [{\"device_idd\": \"device_id\",\"protocol\": "
    "\"PDI-IoTA-UltraLight\",\"entity_name\": \"entity_name\",\"entity_type\": "
    "\"entity_type\",\"timezone\": \"America/Santiago\"}]}");
const int AdminTest::POST_RESPONSE_CODE_NO_DEVICE_ID = 400;
const std::string AdminTest::POST_RESPONSE_NO_DEVICE_ID("");

// GET ALL empty
const std::string AdminTest::GET_EMPTY_RESPONSE_DEVICES(
    "{ \"count\": 0,\"devices\": []}");
const int AdminTest::GET_RESPONSE_CODE = 200;
const int AdminTest::GET_RESPONSE_CODE_NOT_FOUND = 404;
// PUT
const std::string AdminTest::PUT_DEVICE("{\"entity_name\": \"entity_name2\"}");
const int AdminTest::PUT_RESPONSE_CODE = 204;
const std::string AdminTest::PUT_RESPONSE("");
// DELETE
const int AdminTest::DELETE_RESPONSE_CODE = 204;
const std::string AdminTest::DELETE_RESPONSE("");
// GET elto
const std::string AdminTest::GET_DEVICE_RESPONSE(
    "{ \"count\": 0,{ \"devices\": []}");

////////////////////
////  SERVICES
const std::string AdminTest::URI_SERVICE("/TestAdmin/services");
const std::string AdminTest::URI_SERVICE2("/TestAdmin/agents/d/services");
// POST
const std::string AdminTest::POST_SERVICE(
    "{\"services\": [{"
    "\"apikey\": \"apikey\",\"token\": \"token\","
    "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\",\"resource\": "
    "\"/TestAdmin/d\"}]}");
const std::string AdminTest::POST_SERVICE2(
    "{\"services\": [{"
    "\"apikey\": \"apikey\",\"token\": \"token\","
    "\"outgoing_route\": \"gretunnel\","
    "\"entity_type\": \"thing\",\"resource\": \"/TestAdmin/d\"}]}");
const std::string AdminTest::BAD_POST_SERVICE1(
    "{\"services\": [{"
    "\"apikey\": \"apikey\",\"token\": \"token\","
    "\"cbroker\": \"cbroker\",\"entity_type\": \"thing\",\"resource\": "
    "\"/TestAdmin/d\"}]}");
const std::string AdminTest::BAD_POST_SERVICE2(
    "{\"services\": [{"
    "\"apikey\": \"apikey\",\"token\": \"token\","
    "\"cbroker\": \"cbroker\",\"entity_type\": \"thing\",\"resource\": "
    "\"TestAdmin/d\"}]}");

const std::string AdminTest::POST_SERVICE_WITH_ATTRIBUTES(
    "{\"services\": [{"
    "\"apikey\": \"apikey\",\"token\": \"token\","
    "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
    "\"temperature\",\"type\": \"int\" }],"
    "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
    "\"value\": \"50\"  }],"
    "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\",\"resource\": "
    "\"/TestAdmin/d\"}]}");
const std::string AdminTest::POST_SERVICE_TO_DELETE_FIELDS(
    "{\"services\": [{"
    "\"apikey\": \"apikey\",\"token\": \"token\","
    "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
    "\"temperature\",\"type\": \"int\" }],"
    "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
    "\"value\": \"50\"  }],"
    "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\",\"resource\": "
    "\"/TestAdmin/d\"}]}");
const std::string AdminTest::PUT_SERVICE_TO_DELETE_FIELDS(
    "{"
    "\"apikey\": \"apikey\",\"token\": \"\","
    "\"attributes\": [],"
    "\"static_attributes\": [],"
    "\"cbroker\": \"\",\"entity_type\": \"thing\",\"resource\": "
    "\"/TestAdmin/d\"}");
// GET ALL empty
const std::string AdminTest::GET_EMPTY_RESPONSE_SERVICES(
    "{ \"count\": 0,\"services\": []}");
// PUT
const std::string AdminTest::PUT_SERVICE("{\"entity_type\": \"entity_type2\"}");
const std::string AdminTest::PUT_SERVICE_WITH_ATTRIBUTES(
    "{\"entity_type\": \"entity_type2\","
    "\"static_attributes\": [{\"name\": \"other\",\"type\": \"string\", "
    "\"value\": \"other\"  }]}");
// GET elto
const std::string AdminTest::GET_SERVICE_RESPONSE(
    "{ \"count\": 0,\"devices\": []}");

////////////////////
////  PROTOCOLS
const std::string AdminTest::URI_PROTOCOLS("/TestAdmin/protocols");
// POST
const std::string AdminTest::POST_PROTOCOLS1(
    "{\"endpoint\": \"host1\","
    "\"resource\": \"/TestAdmin/d\","
    "\"protocol\": \"UL20\","
    "\"description\": \"Ultralight 2.0\""
    "}");
const std::string AdminTest::POST_PROTOCOLS2(
    "{\"endpoint\": \"host2\","
    "\"resource\": \"/TestAdmin/mqtt\","
    "\"protocol\": \"MQTT\","
    "\"description\": \"mqtt example\","
    "\"services\": [{"
    "\"apikey\": \"apikey3\","
    "\"service\": \"service2\","
    "\"service_path\": \"/ssrv2\","
    "\"token\": \"token2\","
    "\"cbroker\": \"http://127.0.0.1:1026\","
    "\"resource\": \"/TestAdmin/mqtt\","
    "\"entity_type\": \"thing\""
    "}]}");
const std::string AdminTest::POST_PROTOCOLS3(
    "{\"endpoint\": \"host3\","
    "\"resource\": \"/TestAdmin/d\","
    "\"protocol\": \"UL20\","
    "\"description\": \"Ultralight 2.0\","
    "\"services\": [{"
    "\"apikey\": \"apikey3\","
    "\"service\": \"service2\","
    "\"service_path\": \"/ssrv2\","
    "\"token\": \"token2\","
    "\"cbroker\": \"http://127.0.0.1:1026\","
    "\"resource\": \"/TestAdmin/d\","
    "\"entity_type\": \"thing\""
    "}]}");
const std::string AdminTest::POST_PROTOCOLS4("TODO");
const std::string AdminTest::GET_PROTOCOLS_RESPONSE(
    "{ \"count\": 0,\"devices\": []}");

////////////////////
////  SERVICE_MANAGEMENT
const std::string AdminTest::URI_SERVICES_MANAGEMET("/TestAdmin/services");
// POST
const std::string AdminTest::POST_SERVICE_MANAGEMENT1(
    "{\"services\": [{"
    "\"protocol\": [\"55261958d31fc2151cc44c70\", "
    "\"55261958d31fc2151cc44c73\"],"
    "\"apikey\": \"apikey\",\"token\": \"token\","
    "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\""
    "}]}");
const std::string AdminTest::POST_SERVICE_MANAGEMENT2(
    "{\"services\": [{"
    "\"protocol\": [\"55261958d31fc2151cc44c70\", "
    "\"55261958d31fc2151cc44c73\"],"
    "\"apikey\": \"apikey\",\"token\": \"token\","
    "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\""
    "}]}");

const std::string AdminTest::GET_SERVICE_MANAGEMENT_RESPONSE(
    "{ \"count\": 0,\"devices\": []}");

const std::string AdminTest::POST_SERVICE_WRONG_RESOURCE(
    "{\"services\": [{"
    "\"apikey\": \"apikey\",\"token\": \"token\","
    "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\",\"resource\": "
    "\"/TestAdmin/dummy\"}]}");

const std::string AdminTest::POST_SERVICE_INVALID_RESOURCE(
    "{\"services\": [{"
    "\"apikey\": \"apikey\",\"token\": \"token\","
    "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\",\"resource\": "
    "\"<script>alert(ok)</script>\"}]}");

const std::string AdminTest::POST_SERVICE_INVALID_APIKEY(
    "{\"services\": [{"
    "\"apikey\": \"<script>alert(ok)</script>\",\"token\": \"token\","
    "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\",\"resource\": "
    "\"/TestAdmin/d\"}]}");

////////////////////
////  DEVICE _MANAGEMENT
const std::string AdminTest::URI_DEVICES_MANAGEMEMT("/TestAdmin/devices");
// POST
const std::string AdminTest::POST_DEVICE_MANAGEMENT1(
    "{\"devices\": "
    "[{\"device_id\": \"device_id\",\"entity_name\": "
    "\"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": "
    "\"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
    "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
    "\"device_id@ping|%s\" }],"
    "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
    "\"temperature\",\"type\": \"int\" }],"
    "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
    "\"value\": \"50\"  }]"
    "}]}");

const std::string AdminTest::GET_DEVICE_MANAGEMENT_RESPONSE(
    "{ \"count\": 0,\"devices\": []}");

AdminTest::AdminTest() {}

AdminTest::~AdminTest() {}

void AdminTest::setUp() {}

void AdminTest::tearDown() {}

void AdminTest::testGetConf() {
  std::cout << "START testGetConf" << std::endl;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  pion::tcp::connection tcp_conn(iota::Process::get_process().get_io_service());
  tcp_conn.set_lifecycle(pion::tcp::connection::LIFECYCLE_CLOSE);
  boost::system::error_code error_code;
  error_code = tcp_conn.connect(boost::asio::ip::address::from_string(HOST),
                                iota::Process::get_process().get_http_port());
  pion::http::request http_request("/TestAdmin/agents?conf");
  http_request.set_method("GET");
  http_request.add_header("Fiware-Service", "ss");
  http_request.add_header("Fiware-ServicePath", "/ss");
  http_request.send(tcp_conn, error_code);
  pion::http::response http_response(http_request);
  http_response.receive(tcp_conn, error_code);
  std::cout << http_response.get_status_code() << std::endl;
  IOTASSERT(http_response.get_status_code() == 200);
  std::string configuration = http_response.get_content();
  std::cout << http_response.get_content() << std::endl;
  IOTASSERT(configuration.size() > 0);
  tcp_conn.close();
  tcp_conn.connect(boost::asio::ip::address::from_string(HOST),
                   iota::Process::get_process().get_http_port());
  pion::http::request http_request1("/TestAdmin/agents?conf");
  http_request1.set_method("DELETE");
  http_request1.add_header("Fiware-Service", "ss");
  http_request1.add_header("Fiware-ServicePath", "/ss");
  http_request1.send(tcp_conn, error_code);
  pion::http::response http_response1(http_request1);
  http_response1.receive(tcp_conn, error_code);
  IOTASSERT_MESSAGE("Receive " + boost::lexical_cast<std::string>(
                                     http_response1.get_status_code()),
                    http_response1.get_status_code() == 501);

  conf->release();

  std::cout << "END testGetConf" << std::endl;
}

void AdminTest::testReload() {
  std::cout << "START testReload" << std::endl;

  // be sure that there is not a created instance
  iota::Configurator* confRE = iota::Configurator::instance();
  confRE->release();

  iota::Configurator* conf = iota::Configurator::instance();

  if (conf->hasError()) {
    std::string errSTR = conf->getError();
    std::cout << "Fail error message:" << errSTR << std::endl;
    IOTASSERT_MESSAGE(
        "Fail error message",
        errSTR.compare("error, no config file was initializated") == 0);
  } else {
    std::cout << "CPPUNIT_FAIL no Error but no file config " << std::endl;
    CPPUNIT_FAIL("no Error but no file config");
  }

  conf->release();

  std::cout << "END   testReload" << std::endl;
}

void AdminTest::testPostConf() {
  std::cout << "START testPostConf" << std::endl;

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);
  pion::tcp::connection tcp_conn(iota::Process::get_process().get_io_service());
  tcp_conn.set_lifecycle(pion::tcp::connection::LIFECYCLE_CLOSE);
  boost::system::error_code error_code;
  error_code =
      tcp_conn.connect(boost::asio::ip::address::from_string("127.0.0.1"),
                       iota::Process::get_process().get_http_port());
  pion::http::request http_request("/TestAdmin/agents");
  http_request.set_method("POST");
  http_request.set_content(conf->getAll());
  http_request.add_header("Fiware-Service", "ss");
  http_request.add_header("Fiware-ServicePath", "/ss");
  http_request.send(tcp_conn, error_code);
  pion::http::response http_response(http_request);
  http_response.receive(tcp_conn, error_code);
  std::cout << "POST 1" << http_response.get_status_code() << " "
            << http_response.get_content() << std::endl;
  IOTASSERT_MESSAGE("POST configuration 200",
                    http_response.get_status_code() == 200);
  tcp_conn.close();

  // Reg fake agent

  error_code =
      tcp_conn.connect(boost::asio::ip::address::from_string("127.0.0.1"),
                       iota::Process::get_process().get_http_port());
  std::cout << "POST agent configuration" << std::endl;
  pion::http::request http_request_1("/TestAdmin/agents/res");
  http_request_1.set_method("POST");
  http_request_1.add_header("Fiware-Service", "ss");
  http_request_1.add_header("Fiware-ServicePath", "/ss");
  std::string mod_res(
      "{\"statistics\": \"true\",\"services\": [{\"service\": \"change\"}]}");
  http_request_1.set_content(mod_res);

  http_request_1.send(tcp_conn, error_code);
  pion::http::response http_response_1(http_request_1);
  http_response_1.receive(tcp_conn, error_code);
  std::cout << "POST NEW" << http_response_1.get_status_code() << std::endl;
  std::cout << "POST " << http_response_1.get_content() << std::endl;

  IOTASSERT_MESSAGE("POST configuration 400",
                    http_response_1.get_status_code() == 400);
  tcp_conn.close();
  error_code =
      tcp_conn.connect(boost::asio::ip::address::from_string("127.0.0.1"),
                       iota::Process::get_process().get_http_port());
  std::cout << "PUT agent configuration 2" << std::endl;
  pion::http::request http_request_2("/TestAdmin/agents/res");
  http_request_2.set_method("PUT");
  std::string mod_res_nok("{\[\"services\": [{\"service\": \"change\"}]}");
  http_request_2.set_content(mod_res_nok);
  http_request_2.add_header("Fiware-Service", "ss");
  http_request_2.add_header("Fiware-ServicePath", "/ss");
  http_request_2.send(tcp_conn, error_code);
  pion::http::response http_response_2(http_request_2);
  http_response_2.receive(tcp_conn, error_code);
  tcp_conn.close();

  // Modify
  error_code =
      tcp_conn.connect(boost::asio::ip::address::from_string("127.0.0.1"),
                       iota::Process::get_process().get_http_port());
  std::cout << "PUT agent configuration 2" << std::endl;
  pion::http::request http_request_3("/TestAdmin/agents/evadts");
  http_request_3.set_method("PUT");
  http_request_3.add_header("Fiware-Service", "ss");
  http_request_3.add_header("Fiware-ServicePath", "/ss");
  std::string ss(
      "{\"services\": [{\"apikey\": \"apikey3\",\"cbroker\": "
      "\"http:://0.0.0.0:1026\", \"service\": \"6664\",\"entity_type\": "
      "\"thing\",\"service_path\": \"servicepath3\",\"token\": \"token3\"}]}");
  http_request_3.set_content(ss);

  http_request_3.send(tcp_conn, error_code);
  pion::http::response http_response_3(http_request_3);
  http_response_3.receive(tcp_conn, error_code);
  std::cout << http_response_3.get_status_code() << std::endl;
  std::cout << http_response_3.get_content() << std::endl;
  IOTASSERT_MESSAGE("PUT configuration 200",
                    http_response_3.get_status_code() == 200);

  conf->release();
  std::cout << "END testPostConf" << std::endl;
}

void AdminTest::testGetAgents() {
  std::cout << "START testGetAgents" << std::endl;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  pion::tcp::connection tcp_conn(iota::Process::get_process().get_io_service());
  // tcp_conn.set_lifecycle(pion::tcp::connection::LIFECYCLE_KEEPALIVE);
  boost::system::error_code error_code;
  error_code =
      tcp_conn.connect(boost::asio::ip::address::from_string("127.0.0.1"),
                       iota::Process::get_process().get_http_port());
  pion::http::request http_request("/TestAdmin/agents");
  http_request.set_method("GET");
  http_request.add_header("Fiware-Service", "ss");
  http_request.add_header("Fiware-ServicePath", "/ss");
  http_request.send(tcp_conn, error_code);
  pion::http::response http_response(http_request);
  http_response.receive(tcp_conn, error_code);
  std::cout << "GET Agents " << http_response.get_status_code() << std::endl;
  IOTASSERT(http_response.get_status_code() == 200);
  tcp_conn.close();
  error_code =
      tcp_conn.connect(boost::asio::ip::address::from_string("127.0.0.1"),
                       iota::Process::get_process().get_http_port());
  pion::http::request http_request_1("/TestAdmin/agents/noexist");
  http_request_1.set_method("GET");
  http_request_1.add_header("Fiware-Service", "ss");
  http_request_1.add_header("Fiware-ServicePath", "/ss");
  http_request_1.send(tcp_conn, error_code);
  pion::http::response http_response1(http_request_1);
  http_response1.receive(tcp_conn, error_code);
  std::cout << "/TestAdmin/agents/noexist " << http_response1.get_status_code()
            << std::endl;
  IOTASSERT_MESSAGE("404 agent no exists",
                    http_response1.get_status_code() == 404);
  tcp_conn.close();
  error_code =
      tcp_conn.connect(boost::asio::ip::address::from_string("127.0.0.1"),
                       iota::Process::get_process().get_http_port());
  pion::http::request http_request_2("/TestAdmin/agents");
  http_request_2.set_method("DELETE");
  http_request_2.add_header("Fiware-Service", "ss");
  http_request_2.add_header("Fiware-ServicePath", "/ss");
  http_request_2.send(tcp_conn, error_code);
  pion::http::response http_response2(http_request_2);
  http_response2.receive(tcp_conn, error_code);
  std::cout << "/TestAdmin/agents " << http_response2.get_status_code()
            << std::endl;
  IOTASSERT(http_response2.get_status_code() == 501);
  tcp_conn.close();

  conf->release();
  std::cout << "END testGetAgents" << std::endl;
}

void AdminTest::testTimezones() {
  std::cout << "start Timezones" << std::endl;
  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);
  iota::Process& process = iota::Process::get_process();
  process.get_admin_service()->set_timezone_database(
      "../../tests/iotagent/date_time_zonespec.csv");
  boost::posix_time::ptime t =
      process.get_admin_service()->get_local_time_from_timezone(
          "Europe/Madrid");

  std::cout << "End Timezone! " << std::endl;
  conf->release();
}

void AdminTest::testCsvProvision() {
  std::cout << "START testCsvProvision" << std::endl;
  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  pion::tcp::connection tcp_conn(iota::Process::get_process().get_io_service());
  boost::system::error_code error_code;
  error_code =
      tcp_conn.connect(boost::asio::ip::address::from_string("127.0.0.1"),
                       iota::Process::get_process().get_http_port());

  // Multipart no content
  pion::http::request http_request("/TestAdmin/devices");
  http_request.set_method("POST");
  http_request.set_content_type(
      pion::http::types::CONTENT_TYPE_MULTIPART_FORM_DATA);

  http_request.add_header("Fiware-Service", "ss");
  http_request.add_header("Fiware-ServicePath", "/ss");
  http_request.send(tcp_conn, error_code);
  pion::http::response http_response(http_request);
  http_response.receive(tcp_conn, error_code);
  tcp_conn.close();
  IOTASSERT(http_response.get_status_code() == 400);

  error_code =
      tcp_conn.connect(boost::asio::ip::address::from_string("127.0.0.1"),
                       iota::Process::get_process().get_http_port());

  // Multipart csv
  pion::http::request http_request_csv("/TestAdmin/devices");
  http_request_csv.set_method("POST");
  std::string ct(pion::http::types::CONTENT_TYPE_MULTIPART_FORM_DATA);
  ct.append("; boundary=----WebKitFormBoundarynqrI4c1BfROrEpu7");
  http_request_csv.set_content_type(ct);
  std::string csv_ok(
      "------WebKitFormBoundarynqrI4c1BfROrEpu7\r\n"
      "Content-Disposition: form-data; name=\"field1\"\r\n"
      "\r\n"
      "item1,item2, item3\nvalue1, value2, value3\r\n");
  http_request_csv.set_content(csv_ok);
  http_request_csv.add_header("Fiware-Service", "ss");
  http_request_csv.add_header("Fiware-ServicePath", "/ss");
  http_request_csv.send(tcp_conn, error_code);
  pion::http::response http_response_csv(http_request_csv);
  http_response_csv.receive(tcp_conn, error_code);
  tcp_conn.close();
  IOTASSERT(http_response_csv.get_status_code() == 200);
  conf->release();
  std::cout << "END testCsvProvision" << std::endl;
}

int AdminTest::http_test(const std::string& uri, const std::string& method,
                         const std::string& service,
                         const std::string& service_path,
                         const std::string& content_type,
                         const std::string& body,
                         const std::map<std::string, std::string>& headers,
                         const std::string& query_string,
                         std::string& response) {
  pion::tcp::connection tcp_conn(iota::Process::get_process().get_io_service());
  boost::system::error_code error_code;
  error_code = tcp_conn.connect(boost::asio::ip::address::from_string(HOST),
                                iota::Process::get_process().get_http_port());

  pion::http::request http_request(uri);
  http_request.set_method(method);
  http_request.set_content_type(content_type);
  if (!service.empty()) {
    http_request.add_header(iota::types::FIWARE_SERVICE, service);
  }
  if (!service_path.empty()) {
    http_request.add_header(iota::types::FIWARE_SERVICEPATH, service_path);
  }

  // http_request.add_header("Accept", "application/json");

  if (!query_string.empty()) {
    http_request.set_query_string(query_string);
  }

  std::map<std::string, std::string>::const_iterator iter;
  for (iter = headers.begin(); iter != headers.end(); ++iter) {
    std::cout << "header: " << iter->first << iter->second << std::endl;
    http_request.add_header(iter->first, iter->second);
  }
  if (!body.empty()) {
    http_request.set_content(body);
  }
  std::cout << "send" << std::endl;
  http_request.send(tcp_conn, error_code);
  pion::http::response http_response(http_request);
  http_response.receive(tcp_conn, error_code);
  tcp_conn.close();
  int code_res = http_response.get_status_code();
  response.assign(http_response.get_content());

  return code_res;
}

void AdminTest::testPostBadContentType() {
  std::cout << "START @UT@START testPostBadContentType" << std::endl;
  srand(time(NULL));
  std::map<std::string, std::string> headers;
  std::string query_string;

  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  std::string response;
  int code_res;
  std::string service = "service";
  service.append(boost::lexical_cast<std::string>(rand()));
  std::cout << "@UT@service " << service << std::endl;

  std::cout << "@UT@POST Service" << std::endl;
  code_res =
      http_test("/TestAdmin/services", "POST", service, "", "application/xml",
                "<value>XML not allowed</value>", headers, "", response);
  IOTASSERT(code_res == 415);

  std::cout << "END@UT@ testPostBadContentType" << std::endl;
}

void AdminTest::testPostDevice() {
  std::cout << "START @UT@START testPostDevice" << std::endl;
  srand(time(NULL));
  std::map<std::string, std::string> headers;
  std::string query_string;
  std::string device_post_device(
      "{\"devices\": "
      "[{\"device_id\": \"testpostdevice\",\"protocol\": "
      "\"PDI-IoTA-UltraLight\",\"entity_name\": "
      "\"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": "
      "\"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
      "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
      "\"device_id@ping|%s\" }],"
      "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
      "\"temperature\",\"type\": \"int\" }],"
      "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
      "\"value\": \"50\"  }]"
      "}]}");

  std::string response;
  int code_res;
  std::string service = "testpostdevice";
  std::cout << "@UT@service " << service << std::endl;
  code_res = http_test("/TestAdmin/services", "DELETE", service, "",
                       "application/json", "", headers, "", response);
  code_res = http_test("/TestAdmin/devices/testpostdevice", "DELETE", service,
                       "", "application/json", "", headers, "", response);
  // POST the service
  std::cout << "@UT@POST Service" << std::endl;
  code_res = http_test("/TestAdmin/services", "POST", service, "",
                       "application/json", POST_SERVICE, headers, "", response);
  IOTASSERT(code_res == 201);

  std::cout << "@UT@GET get empty" << std::endl;
  code_res = http_test("/TestAdmin/devices", "GET", service, "",
                       "application/json", "", headers, query_string, response);
  boost::algorithm::trim(response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == GET_RESPONSE_CODE);
  IOTASSERT(GET_EMPTY_RESPONSE_DEVICES.compare(response) == 0);

  std::cout << "@UT@GET The device does not exist" << std::endl;
  code_res = http_test("/TestAdmin/devices/noexists", "GET", service, "",
                       "application/json", "", headers, query_string, response);
  boost::algorithm::trim(response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 404);
  IOTASSERT(response.compare("{\"reason\":\"The device does not "
                             "exist\",\"details\":\"noexists\"}") == 0);

  std::cout << "@UT@POST" << std::endl;
  code_res =
      http_test("/TestAdmin/devices", "POST", service, "", "application/json",
                device_post_device, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@GET" << std::endl;
  code_res =
      http_test("/TestAdmin/devices", "GET", service, "", "application/json",
                "", headers, "entity=entity_name", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE("Search by entity name ", code_res = 200);
  IOTASSERT(response.find("testpostdevice") != std::string::npos);

  std::cout << "@UT@GET limit = -1" << std::endl;
  code_res = http_test("/TestAdmin/devices", "GET", service, "",
                       "application/json", "", headers, "limit=-1", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE("Search by entity name ", code_res = 200);
  IOTASSERT(response.find("testpostdevice") == std::string::npos);

  std::cout << "@UT@POST_BAD" << std::endl;
  code_res =
      http_test("/TestAdmin/devices", "POST", service, "", "application/json",
                POST_DEVICE_NO_DEVICE_ID, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE_NO_DEVICE_ID);

  std::cout << "@UT@PUT" << std::endl;
  code_res = http_test("/TestAdmin/devices/testpostdevice", "PUT", service, "",
                       "application/json", PUT_DEVICE, headers, "", response);
  std::cout << "@UT@RESPONSEPUT: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == PUT_RESPONSE_CODE);

  std::cout << "@UT@GET" << std::endl;
  code_res = http_test("/TestAdmin/devices/testpostdevice", "GET", service, "",
                       "application/json", "", headers, query_string, response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == GET_RESPONSE_CODE);
  IOTASSERT(
      response.find(
          "\"commands\" : [ { \"name\" : \"ping\", \"type\" : \"command\"") !=
      std::string::npos);
  IOTASSERT(response.find("\"attributes\" : [ { \"object_id\" : \"temp\", "
                          "\"name\" : \"temperature\"") != std::string::npos);
  IOTASSERT(
      response.find("\"static_attributes\" : [ { \"name\" : \"humidity\"") !=
      std::string::npos);

  std::cout << "@UT@DELETE" << std::endl;
  code_res = http_test("/TestAdmin/devices/testpostdevice", "DELETE", service,
                       "", "application/json", "", headers, "", response);
  std::cout << "@UT@RESPONSEDELETE: " << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == DELETE_RESPONSE_CODE);

  std::cout << "@UT@GET" << std::endl;
  code_res = http_test("/TestAdmin/devices/testpostdevice", "GET", service, "",
                       "application/json", "", headers, query_string, response);
  boost::algorithm::trim(response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == GET_RESPONSE_CODE_NOT_FOUND);

  std::cout << "@UT@DELETE Service" << std::endl;
  code_res = http_test("/TestAdmin/services/" + service, "DELETE", service, "",
                       "application/json", "", headers, "", response);
  IOTASSERT(code_res == 400);
  IOTASSERT(response.find("resource parameter is mandatory") !=
            std::string::npos);

  std::cout << "END@UT@ testPostDevice" << std::endl;
}

void AdminTest::testAttributeService() {
  srand(time(NULL));
  std::cout << "START @UT@START testAttributeService" << std::endl;
  std::map<std::string, std::string> headers;
  std::string query_string("apikey=apikey&resource=/TestAdmin/d");

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  std::string response;
  std::string service = "service";
  service.append(boost::lexical_cast<std::string>(rand()));
  std::cout << "@UT@service " << service << std::endl;
  int code_res;
  // Removing
  code_res = http_test("/TestAdmin/services/" + service, "DELETE", service, "",
                       "application/json", "", headers, query_string, response);
  IOTASSERT_MESSAGE(service + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == 204);

  std::cout << "@UT@POST" << std::endl;
  code_res = http_test("/TestAdmin/services", "POST", service, "",
                       "application/json", POST_SERVICE, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(service + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@medida" << std::endl;
  code_res = http_test("/TestAdmin/d", "POST", service, "", "application/json",
                       "2014-02-18T16:41:20Z|t|23", headers,
                       "i=unitTest_dev1_endpoint&k=apikey3", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  // TODO  IOTASSERT_MESSAGE(service + "|" +
  // boost::lexical_cast<std::string>(code_res), code_res ==
  // POST_RESPONSE_CODE);

  std::cout << "@UT@DELETE: " << service << std::endl;
  code_res = http_test("/TestAdmin/services/" + service, "DELETE", service, "",
                       "application/json", "", headers,
                       "apikey=apikey&resource=/TestAdmin/d", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
}

void AdminTest::testPostService() {
  srand(time(NULL));
  std::cout << "START @UT@START testPostService" << std::endl;
  std::map<std::string, std::string> headers;
  std::string query_string("apikey=apikey&resource=/TestAdmin/d");

  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  std::string response;
  std::string service = "testpostservice";
  std::cout << "@UT@service " << service << std::endl;
  int code_res;
  // Removing
  code_res = http_test("/TestAdmin/services/" + service, "DELETE", service, "",
                       "application/json", "", headers, query_string, response);
  IOTASSERT_MESSAGE(service + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == 204);

  std::cout << "@UT@GET get empty this service" << std::endl;
  code_res = http_test("/TestAdmin/services/" + service, "GET", service, "",
                       "application/json", "", headers, query_string, response);
  boost::algorithm::trim(response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(service + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == 200);
  IOTASSERT_MESSAGE("",
                    response.compare("{ \"count\": 0,\"services\": []}") == 0);

  std::cout << "@UT@POST" << std::endl;
  code_res = http_test("/TestAdmin/services", "POST", service, "",
                       "application/json", POST_SERVICE, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(service + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@GET" << std::endl;
  code_res = http_test("/TestAdmin/services", "GET", service, "",
                       "application/json", "", headers, query_string, response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;

  std::cout << "@UT@PUT" << std::endl;
  // PUT no apikey, no resource 400
  code_res =
      http_test("/TestAdmin/services", "PUT", service, "", "application/json",
                PUT_SERVICE, headers, query_string, response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(service + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == PUT_RESPONSE_CODE);

  std::cout << "@UT@GET" << std::endl;
  code_res = http_test("/TestAdmin/services", "GET", service, "",
                       "application/json", "", headers, query_string, response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(service + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == GET_RESPONSE_CODE);
  IOTASSERT_MESSAGE(service + "|entity_type2",
                    response.find("entity_type2") != std::string::npos);

  std::cout << "@UT@GET with bad resource" << std::endl;
  code_res =
      http_test("/TestAdmin/services", "GET", service, "", "application/json",
                "", headers, "dd=d&resource=/io/kk", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE("query parameter resource does not work",
                    response.compare("{ \"count\": 0,\"services\": []}") == 0);

  std::string service_2("testpostservice_2");
  std::cout << "@UT@POST" << std::endl;
  code_res =
      http_test("/TestAdmin/services", "POST", service_2, "",
                "application/json", POST_SERVICE2, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(
      service_2 + "|" + boost::lexical_cast<std::string>(code_res),
      code_res == POST_RESPONSE_CODE);
  std::cout << "@UT@GET" << std::endl;
  code_res = http_test("/TestAdmin/services", "GET", service_2, "",
                       "application/json", "", headers, "", response);
  IOTASSERT_MESSAGE(service_2 + "|outgoing_route" + response,
                    response.find("outgoing_route") != std::string::npos);

  /*std::cout << "@UT@PUTBAD" << std::endl;
  // PUT no apikey, no resource 400
  code_res = http_test("/TestAdmin/services/" + service_2, "PUT", service_2, "",
                       "application/json", "{\"resource\":\"/TestAdmin/d2\"}",
                       headers, query_string, response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE("bad put with an busy resource", code_res == 409);
  IOTASSERT_MESSAGE("bad put with an busy resource2",
                    response.find("\"reason\":\"There are conflicts, object "
                                  "already exists\",\"details\":\"duplicate "
                                  "key: iotest.SERVICE") != std::string::npos);
*/

  std::cout << "@UT@DELETE " << service << std::endl;
  code_res = http_test("/TestAdmin/services", "DELETE", service, "/*",
                       "application/json", "", headers, query_string, response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(service + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == DELETE_RESPONSE_CODE);

  std::cout << "@UT@DELETE " << service_2 << std::endl;
  code_res = http_test("/TestAdmin/services", "DELETE", service_2, "/*",
                       "application/json", "", headers, query_string, response);

  std::cout << "@UT@GET" << std::endl;
  code_res = http_test("/TestAdmin/services", "GET", service, "/*",
                       "application/json", "", headers, query_string, response);
  boost::algorithm::trim(response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(service + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == 200);
  IOTASSERT_MESSAGE(
      response,
      response.find("{ \"count\": 0,\"services\": []}") != std::string::npos);

  std::cout
      << "@UT@GET Invalid Parameter (fiware-Service different from service-id)"
      << std::endl;
  code_res = http_test("/TestAdmin/services/OtherService", "GET", service, "",
                       "application/json", "", headers, query_string, response);
  boost::algorithm::trim(response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(service + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == 400);

  // Error cases
  std::cout << "@UT@POST" << std::endl;
  code_res = http_test("/TestAdmin/services", "POST", service, "",
                       "application/json", POST_SERVICE, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(service + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@DELETE " << service << std::endl;
  // DELETE no apikey, resource: 400
  code_res = http_test("/TestAdmin/services/" + service, "DELETE", service, "",
                       "application/json", "", headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(service + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == 400);

  std::cout << "@UT@PUT" << std::endl;
  // PUT no apikey, no resource 400
  code_res = http_test("/TestAdmin/services/" + service, "PUT", service, "",
                       "application/json", PUT_SERVICE, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(service + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == 400);

  std::cout << "@UT@GET" << std::endl;
  code_res = http_test("/TestAdmin/services", "GET", service, "",
                       "application/json", "", headers, query_string, response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(service + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == GET_RESPONSE_CODE);
  IOTASSERT_MESSAGE(service + "|entity_type2",
                    response.find("entity_type2") == std::string::npos);

  std::cout << "@UT@DELETE@785 " << service << std::endl;
  code_res = http_test("/TestAdmin/services/" + service, "DELETE", service, "",
                       "application/json", "", headers, query_string, response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(service + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == DELETE_RESPONSE_CODE);

  std::cout << "@UT@DELETE " << service << std::endl;
  std::string delete_query_error(query_string);
  delete_query_error.append("&device=true");
  code_res =
      http_test("/TestAdmin/services/" + service, "DELETE", "ss", "/*",
                "application/json", "", headers, delete_query_error, response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(service + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == 400);

  std::cout << "@UT@DELETE: " << service << std::endl;
  code_res = http_test("/TestAdmin/services/" + service, "DELETE", service, "",
                       "application/json", "", headers,
                       "apikey=apikey&resource=/TestAdmin/d2", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;

  // With attributes
  std::string srv_att(service);
  srv_att.append("_att");
  code_res = http_test("/TestAdmin/services", "DELETE", srv_att, "",
                       "application/json", "", headers, query_string, response);
  IOTASSERT_MESSAGE(srv_att + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == 204);

  std::cout << "@UT@GET get empty this service" << std::endl;
  code_res = http_test("/TestAdmin/services", "GET", srv_att, "",
                       "application/json", "", headers, query_string, response);
  boost::algorithm::trim(response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(srv_att + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == 200);
  IOTASSERT_MESSAGE("",
                    response.compare("{ \"count\": 0,\"services\": []}") == 0);

  std::cout << "@UT@POST WITH ATTR" << std::endl;
  code_res =
      http_test("/TestAdmin/services", "POST", srv_att, "", "application/json",
                POST_SERVICE_WITH_ATTRIBUTES, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(srv_att + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@PUT" << std::endl;
  code_res =
      http_test("/TestAdmin/services", "PUT", srv_att, "", "application/json",
                PUT_SERVICE_WITH_ATTRIBUTES, headers, query_string, response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(srv_att + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == PUT_RESPONSE_CODE);

  std::cout << "@UT@GET" << std::endl;
  code_res = http_test("/TestAdmin/services", "GET", srv_att, "",
                       "application/json", "", headers, query_string, response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(srv_att + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == GET_RESPONSE_CODE);
  IOTASSERT_MESSAGE(srv_att + "|entity_type2",
                    response.find("entity_type2") != std::string::npos);
  IOTASSERT_MESSAGE(
      srv_att + "|attribute static=other",
      response.find("\"static_attributes\" : [ { \"name\" : \"other\"") !=
          std::string::npos);
  IOTASSERT_MESSAGE(
      srv_att + "|attribute static=humidity not found",
      response.find("\"static_attributes\" : [ { \"name\" : \"humidity\"") ==
          std::string::npos);
  IOTASSERT_MESSAGE(
      srv_att + "|attributes " + response,
      response.find("\"attributes\" : [ { \"object_id\" : \"temp\", \"name\" : "
                    "\"temperature\", \"type\" : \"int\" } ]") !=
          std::string::npos);

  // Final delete
  std::cout << "@UT@DELETE " << srv_att << std::endl;
  code_res = http_test("/TestAdmin/services", "DELETE", srv_att, "/*",
                       "application/json", "", headers, query_string, response);
  IOTASSERT_MESSAGE(srv_att + "|" + boost::lexical_cast<std::string>(code_res),
                    code_res == 204);

  std::cout << "@UT@POST TO DELETE FIELDS" << std::endl;

  std::string srv_fields(service);
  srv_fields.append("_fields");
  code_res = http_test("/TestAdmin/services", "POST", srv_fields, "",
                       "application/json", POST_SERVICE_TO_DELETE_FIELDS,
                       headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(
      srv_fields + "|" + boost::lexical_cast<std::string>(code_res),
      code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@PUT" << std::endl;
  code_res = http_test("/TestAdmin/services", "PUT", srv_fields, "",
                       "application/json", PUT_SERVICE_TO_DELETE_FIELDS,
                       headers, query_string, response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE(
      srv_fields + "|" + boost::lexical_cast<std::string>(code_res),
      code_res == PUT_RESPONSE_CODE);

  std::cout << "@UT@GET" << std::endl;
  code_res = http_test("/TestAdmin/services", "GET", srv_fields, "",
                       "application/json", "", headers, query_string, response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  CPPUNIT_ASSERT_MESSAGE(
      "No context broker ",
      response.find("\"cbroker\" : \"\"") != std::string::npos);
  CPPUNIT_ASSERT_MESSAGE(
      "No attributes ",
      response.find("\"attributes\" : []") != std::string::npos);

  // Final delete
  std::cout << "@UT@DELETE " << srv_fields << std::endl;
  code_res = http_test("/TestAdmin/services", "DELETE", srv_fields, "/*",
                       "application/json", "", headers, query_string, response);
  IOTASSERT_MESSAGE(
      srv_fields + "|" + boost::lexical_cast<std::string>(code_res),
      code_res == 204);

  std::cout << "END@UT@ testPostService" << std::endl;
}

void AdminTest::testPostService2() {
  srand(time(NULL));
  std::cout << "START @UT@START testPostService2" << std::endl;
  std::map<std::string, std::string> headers;
  std::string query_string("apikey=apikey&resource=/TestAdmin/d");

  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  std::string response;
  std::string service = "service";
  service.append(boost::lexical_cast<std::string>(rand()));
  std::cout << "@UT@service " << service << std::endl;

  std::cout << "@UT@GET get empty" << std::endl;
  int code_res =
      http_test(URI_SERVICE2 + "/" + service, "GET", service, "",
                "application/json", "", headers, query_string, response);
  boost::algorithm::trim(response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\": 0,\"services\": []}") == 0);

  std::cout << "@UT@POST" << std::endl;
  code_res = http_test(URI_SERVICE2, "POST", service, "", "application/json",
                       POST_SERVICE, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@PUT" << std::endl;
  code_res = http_test(URI_SERVICE2 + "/" + service, "PUT", service, "",
                       "application/json", PUT_SERVICE, headers, query_string,
                       response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == PUT_RESPONSE_CODE);

  std::cout << "@UT@GET" << std::endl;
  code_res = http_test(URI_SERVICE2 + "/" + service, "GET", service, "",
                       "application/json", "", headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == GET_RESPONSE_CODE);

  std::cout << "@UT@GET only return one" << std::endl;
  code_res = http_test(URI_SERVICE2, "GET", service, "/*", "application/json",
                       "", headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == GET_RESPONSE_CODE);
  IOTASSERT_MESSAGE(
      "only return one",
      response.find("\"count\": 1,\"services\"") != std::string::npos);

  std::cout << "@UT@POST" << std::endl;
  code_res = http_test(URI_SERVICE2, "POST", service, "/ssrv2",
                       "application/json", POST_SERVICE, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@GET only return two" << std::endl;
  code_res = http_test(URI_SERVICE2, "GET", service, "/*", "application/json",
                       "", headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == GET_RESPONSE_CODE);
  IOTASSERT_MESSAGE(
      "only return one",
      response.find("\"count\": 2,\"services\"") != std::string::npos);

  std::cout << "@UT@DELETE " << service << std::endl;
  code_res = http_test(URI_SERVICE2 + "/" + service, "DELETE", service, "/*",
                       "application/json", "", headers, query_string, response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == DELETE_RESPONSE_CODE);

  std::cout << "@UT@GET" << std::endl;
  code_res = http_test(URI_SERVICE2 + "/" + service, "GET", service, "/*",
                       "application/json", "", headers, "", response);
  boost::algorithm::trim(response);
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\": 0,\"services\": []}") == 0);

  std::cout << "END@UT@ testPostService2" << std::endl;
}

void AdminTest::testPostDeviceSameEntity() {
  srand(time(NULL));
  std::cout << "START @UT@START testPostDeviceSameEntity" << std::endl;
  std::string service = "testpostdevicesameentity";
  service.append(boost::lexical_cast<std::string>(rand()));
  std::string servicepath = "/t";
  std::map<std::string, std::string> headers;
  std::string response;
  const std::string POST_DEVICEping(
      "{\"devices\": "
      "[{\"device_id\": \"devicese1\",\"protocol\": "
      "\"PDI-IoTA-UltraLight\",\"entity_name\": "
      "\"entity_name\",\"entity_type\": "
      "\"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": "
      "\"America/Santiago\","
      "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
      "\"device_id@ping|%s\" }]"
      "}]}");

  const std::string POST_DEVICEping2(
      "{\"devices\": "
      "[{\"device_id\": \"devicese2\",\"protocol\": "
      "\"PDI-IoTA-UltraLight\",\"entity_name\": "
      "\"entity_name\",\"entity_type\": "
      "\"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": "
      "\"America/Santiago\","
      "\"commands\": [{\"name\": \"ping2\",\"type\": \"command\",\"value\": "
      "\"device_id@ping|%s\" }]"
      "}]}");

  const std::string POST_DEVICEping3(
      "{\"devices\": "
      "[{\"device_id\": \"devicese3\",\"protocol\": "
      "\"PDI-IoTA-UltraLight\",\"entity_name\": "
      "\"entity_name\",\"entity_type\": "
      "\"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": "
      "\"America/Santiago\","
      "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
      "\"device_id@ping|%s\" }]"
      "}]}");

  std::cout << "@UT@POST SERVICE" << std::endl;
  int code_res =
      http_test(URI_SERVICE2, "POST", service, servicepath, "application/json",
                POST_SERVICE, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  // POST device
  std::cout << "@UT@POST DEVICE ping" << std::endl;
  code_res =
      http_test("/TestAdmin/devices", "POST", service, servicepath,
                "application/json", POST_DEVICEping, headers, "", response);
  std::cout << "@UT@5RESPONSEPOST: " << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == 201);

  // POST device
  std::cout << "@UT@POST DEVICE ping2" << std::endl;
  code_res =
      http_test("/TestAdmin/devices", "POST", service, servicepath,
                "application/json", POST_DEVICEping2, headers, "", response);
  std::cout << "@UT@5RESPONSEPOST: " << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == 201);

  // POST device
  std::cout << "@UT@POST DEVICE ping same entity and command" << std::endl;
  code_res =
      http_test("/TestAdmin/devices", "POST", service, servicepath,
                "application/json", POST_DEVICEping3, headers, "", response);
  std::cout << "@UT@5RESPONSEPOST: " << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == 409);
  IOTASSERT(response.find("\"reason\":\"There are conflicts, entity with this "
                          "command, already exists\"") != std::string::npos);

  std::cout << "@UT@DELETE " << service << std::endl;
  std::string queryparam = "resource=/iot/d&device=true";
  code_res =
      http_test(URI_SERVICE2 + "/" + service, "DELETE", service, servicepath,
                "application/json", "", headers, queryparam, response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == DELETE_RESPONSE_CODE);

  std::cout << "END @UT@START testPostDeviceSameEntity" << std::endl;
}

void AdminTest::testBADPostDevice() {
  srand(static_cast<unsigned int>(time(0)));
  std::cout << "START @UT@START testBADPostDevice" << std::endl;
  std::map<std::string, std::string> headers;
  std::string query_string;

  int code_res;
  std::string response;
  std::string service = "service";
  service.append(boost::lexical_cast<std::string>(rand()));
  std::cout << "@UT@service " << service << std::endl;

  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  // no existe servicio al hacer POST de device
  std::cout << "@UT@1POST" << std::endl;
  code_res = http_test("/TestAdmin/devices", "POST", service, "",
                       "application/json", POST_DEVICE, headers, "", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 404);
  IOTASSERT(response.find("\"reason\":\"The service does not exist\"") !=
            std::string::npos);

  // operaCION NO PERMITIDA
  std::cout << "@UT@1PUT" << std::endl;
  code_res = http_test("/TestAdmin/devices", "PUT", service, "",
                       "application/json", PUT_DEVICE, headers, "", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 405);
  // IOTASSERT(response.find("\"reason\":\"The method is not allowed\"") !=
  // std::string::npos);

  // no existe servicio al hacer PUT de device
  std::cout << "@UT@1PUT" << std::endl;
  code_res = http_test("/TestAdmin/devices/noexists", "PUT", service, "",
                       "application/json", PUT_DEVICE, headers, "", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 404);
  IOTASSERT(response.find("\"reason\":\"The device does not exist\"") !=
            std::string::npos);

  // creamos le servicio
  std::cout << "@UT@1POSTService" << std::endl;
  code_res = http_test(URI_SERVICE, "POST", service, "", "application/json",
                       POST_SERVICE, headers, "", response);
  std::cout << "@UT@2RESPONSEService: " << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  // no exists device al hacer PUT
  std::cout << "@UT@1PUT" << std::endl;
  code_res = http_test("/TestAdmin/devices/noDevice", "PUT", service, "",
                       "application/json", PUT_DEVICE, headers, "", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 404);
  IOTASSERT(response.find("\"reason\":\"The device does not exist\"") !=
            std::string::npos);

  // hacemos POST de un servicio ya existente
  std::cout << "@UT@1POSTService duplicated" << std::endl;
  code_res = http_test(URI_SERVICE, "POST", service, "", "application/json",
                       POST_SERVICE, headers, "", response);
  std::cout << "@UT@1RESPONSEService: " << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == 409);
  IOTASSERT(response.find("{\"reason\":\"There are conflicts, object already "
                          "exists\",\"details\":\"duplicate key: "
                          "iotest.SERVICE") != std::string::npos);
  // no information with database error 11000  duplicated key
  IOTASSERT(response.find("E1100") == std::string::npos);

  // POST de service con campos que no están en el schema
  std::cout << "@UT@2POST" << std::endl;
  code_res =
      http_test("/TestAdmin/devices", "POST", service, "", "application/json",
                BAD_POST_DEVICE, headers, "", response);
  std::cout << "@UT@2RESPONSEPOST: " << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.find("{\"reason\":\"The request is not well "
                          "formed\",\"details\":\"Additional properties not "
                          "allowed") != std::string::npos);
  IOTASSERT(response.find("[/devices[0]/entity_namee]") != std::string::npos);

  // POST de service con campos bad protocol
  std::cout << "@UT@25POST" << std::endl;
  code_res =
      http_test("/TestAdmin/devices", "POST", service, "", "application/json",
                BAD_PROTOCOL_POST_DEVICE, headers, "", response);
  std::cout << "@UT@25RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(
      response.compare("{\"reason\":\"There are conflicts, protocol is not "
                       "correct\",\"details\":\" [ protocol: kk]\"}") == 0);

  // POST de service cuando faltan campos
  std::cout << "@UT@3POST" << std::endl;
  code_res =
      http_test("/TestAdmin/devices", "POST", service, "", "application/json",
                BAD_POST_DEVICE2, headers, "", response);
  std::cout << "@UT@3RESPONSEPOST: " << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.find("{\"reason\":\"The request is not well "
                          "formed\",\"details\":\"Missing required property: "
                          "name") != std::string::npos);

  // POST de device cuando sobran campos
  std::cout << "@UT@4POST" << std::endl;
  code_res =
      http_test("/TestAdmin/devices", "POST", service, "", "application/json",
                BAD_POST_DEVICE3, headers, "", response);
  std::cout << "@UT@4RESPONSEPOST: " << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.find("{\"reason\":\"The request is not well "
                          "formed\",\"details\":\"Additional properties not "
                          "allowed") != std::string::npos);
  IOTASSERT_MESSAGE(
      response,
      response.find("[/devices[0]/commands[0]/bomba]") != std::string::npos);

  // POST de device cuando no hay device_id
  std::cout << "@UT@45POST" << std::endl;
  code_res =
      http_test("/TestAdmin/devices", "POST", service, "", "application/json",
                BAD_POST_DEVICE_NO_DEVICE_ID, headers, "", response);
  std::cout << "@UT@4RESPONSEPOST: " << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.find("{\"reason\":\"The request is not well "
                          "formed\",\"details\":\"Missing required property: "
                          "device_id [/devices[0]]\"}") != std::string::npos);

  // POST device
  std::cout << "@UT@5POST" << std::endl;
  code_res = http_test("/TestAdmin/devices", "POST", service, "",
                       "application/json", POST_DEVICE, headers, "", response);
  std::cout << "@UT@5RESPONSEPOST: " << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == 201);

  // duplicated device
  std::cout << "@UT@6POST" << std::endl;
  code_res = http_test("/TestAdmin/devices", "POST", service, "",
                       "application/json", POST_DEVICE, headers, "", response);
  std::cout << "@UT@6RESPONSEPOST: " << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == 409);
  IOTASSERT(response.find("{\"reason\":\"There are conflicts, entity with this "
                          "command, already exists") != std::string::npos);

  // GET bad parameter limit
  std::cout << "@UT@6BAD Limit" << std::endl;
  code_res =
      http_test("/TestAdmin/services", "GET", service, "/*", "application/json",
                "", headers, "limit=nonumber", response);
  std::cout << "@UT@6RESPONSEBAD: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.find("\"reason\":\"A parameter of the request is "
                          "invalid/not allowed\",\"details\":\"limit must be a "
                          "number but it is nonumber\"}") != std::string::npos);

  // cbroker no es uri
  std::cout << "@UT@7POST  bad cbroker" << std::endl;
  code_res =
      http_test("/TestAdmin/services", "POST", "badservice", "/badservice",
                "application/json", BAD_POST_SERVICE1, headers, "", response);
  std::cout << "@UT@7RESPONSEPOST: " << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.find("{\"reason\":\"A parameter of the request is "
                          "invalid/not allowed[cbroker]") != std::string::npos);

  // cbroker no es uri
  std::cout << "@UT@8POST  bad resource" << std::endl;
  code_res =
      http_test("/TestAdmin/services", "POST", "badservice", "/badservice",
                "application/json", BAD_POST_SERVICE2, headers, "", response);
  std::cout << "@UT@8RESPONSEPOST: " << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.find("{\"reason\":\"The request is not well formed\"") !=
            std::string::npos);

  std::cout << "@UT@9GET  no device" << std::endl;
  code_res =
      http_test("/TestAdmin/devices/nodev", "GET", "badservice", "/badservice",
                "application/json", "", headers, "", response);
  std::cout << "@UT@9RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 404);
  IOTASSERT(response.find("The device does not exist") != std::string::npos);

  std::cout << "@UT@10PUT  no device" << std::endl;
  code_res =
      http_test("/TestAdmin/devices/nodev", "PUT", "badservice", "/badservice",
                "application/json", PUT_SERVICE, headers, "", response);
  std::cout << "@UT@10RESPONSEPUT: " << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == 404);
  IOTASSERT(response.find("The device does not exist") != std::string::npos);

  std::cout << "@UT@10PUT  body empty" << std::endl;
  code_res =
      http_test("/TestAdmin/devices/nodev", "PUT", "badservice", "/badservice",
                "application/json", "", headers, "", response);
  std::cout << "@UT@response " << response << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.compare("{\"reason\":\"The request is not well "
                             "formed\",\"details\":\"empty body\"}") == 0);

  std::cout << "@UT@10POST  body empty" << std::endl;
  code_res =
      http_test("/TestAdmin/devices", "POST", "badservice", "/badservice",
                "application/json", "", headers, "", response);
  std::cout << "@UT@response " << response << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.compare("{\"reason\":\"The request is not well "
                             "formed\",\"details\":\"empty body\"}") == 0);

  std::cout << "@UT@10PUT  body empty" << std::endl;
  code_res =
      http_test("/TestAdmin/services?resource=noresource", "PUT", "badservice",
                "/badservice", "application/json", "", headers, "", response);
  std::cout << "@UT@response " << response << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.compare(
                "{\"reason\":\"The request is not well "
                "formed\",\"details\":\"Resource does not exist\"}") == 0);

  std::cout << "@UT@10POST  body empty" << std::endl;
  code_res =
      http_test("/TestAdmin/services", "POST", "badservice", "/badservice",
                "application/json", "", headers, "", response);
  std::cout << "@UT@response " << response << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.compare("{\"reason\":\"The request is not well "
                             "formed\",\"details\":\"empty body\"}") == 0);

  std::cout << "@UT@10PUT  body {}" << std::endl;
  code_res =
      http_test("/TestAdmin/devices/nodev", "PUT", "badservice", "/badservice",
                "application/json", "{}", headers, "", response);
  std::cout << "@UT@response " << response << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.compare("{\"reason\":\"The request is not well "
                             "formed\",\"details\":\"empty body\"}") == 0);

  std::cout << "@UT@10PUT  body {}" << std::endl;
  code_res =
      http_test("/TestAdmin/services?resource=noresource", "PUT", "badservice",
                "/badservice", "application/json", "{}", headers, "", response);
  std::cout << "@UT@response " << response << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.compare(
                "{\"reason\":\"The request is not well "
                "formed\",\"details\":\"Resource does not exist\"}") == 0);

  std::cout << "@UT@11GET  negative offset " << std::endl;
  code_res = http_test("/TestAdmin/services", "GET", "service2", "/ssrv2",
                       "application/json", "", headers, "offset=-22", response);
  std::cout << "@UT@response " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\": 0,\"services\": []}") == 0);

  std::cout << "END@UT@ testBADPostDevice" << std::endl;
}

void AdminTest::testNoRestApiService() {
  srand(time(NULL));
  std::cout << "START @UT@START testNoRestApiService" << std::endl;
  std::map<std::string, std::string> headers;
  std::string query_string("apikey=apikey&resource=/TestAdmin/d&device=true");

  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  std::string response;
  std::string service = "service";
  service.append(boost::lexical_cast<std::string>(rand()));
  std::cout << "@UT@service " << service << std::endl;

  std::cout << "@UT@1GET get empty" << std::endl;
  int code_res = http_test(URI_SERVICE2, "GET", service, "", "application/json",
                           "", headers, "", response);
  boost::algorithm::trim(response);
  std::cout << "@UT@2RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT_MESSAGE("",
                    response.compare("{ \"count\": 0,\"services\": []}") == 0);

  std::cout << "@UT@3POST1" << std::endl;
  code_res = http_test(URI_SERVICE2, "POST", service, "", "application/json",
                       POST_SERVICE, headers, "", response);
  std::cout << "@UT@4RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@5POST2" << std::endl;
  code_res = http_test(URI_SERVICE2, "POST", service, "/ssrv1",
                       "application/json", POST_SERVICE, headers, "", response);
  std::cout << "@UT@6RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@7POST2" << std::endl;
  code_res = http_test(URI_SERVICE2, "POST", service, "/ssrv2",
                       "application/json", POST_SERVICE, headers, "", response);
  std::cout << "@UT@8RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@9PUT" << std::endl;
  code_res = http_test(URI_SERVICE2, "PUT", service, "", "application/json",
                       PUT_SERVICE, headers, query_string, response);
  std::cout << "@UT@10RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == PUT_RESPONSE_CODE);

  std::cout << "@UT@11GET" << std::endl;
  code_res = http_test(URI_SERVICE2, "GET", service, "/*", "application/json",
                       "", headers, "", response);
  std::cout << "@UT@12RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == GET_RESPONSE_CODE);

  // añladimos un device para ver si lo borra
  std::cout << "@UT@13POST" << std::endl;
  code_res = http_test("/TestAdmin/devices", "POST", service, "",
                       "application/json", POST_DEVICE, headers, "", response);
  std::cout << "@UT@14RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@15DELETE" << std::endl;
  code_res = http_test(URI_SERVICE2, "DELETE", service, "", "application/json",
                       "", headers, query_string, response);
  std::cout << "@UT@16RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == DELETE_RESPONSE_CODE);

  std::cout << "@UT@17DELETE" << std::endl;
  code_res = http_test(URI_SERVICE2, "DELETE", service, "/ssrv1",
                       "application/json", "", headers, query_string, response);
  std::cout << "@UT@18RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == DELETE_RESPONSE_CODE);

  std::cout << "@UT@19DELETE" << std::endl;
  code_res = http_test(URI_SERVICE2, "DELETE", service, "/ssrv2",
                       "application/json", "", headers, query_string, response);
  std::cout << "@UT@20RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == DELETE_RESPONSE_CODE);

  std::cout << "@UT@21GET" << std::endl;
  code_res = http_test("/TestAdmin/devices", "GET", service, "",
                       "application/json", "", headers, "", response);
  boost::algorithm::trim(response);
  std::cout << "@UT@22RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.find("\"count\": 0,\"devices\": []") != std::string::npos);

  std::cout << "END@UT@ testNoRestApiService" << std::endl;
}

template <typename T>

struct my_id_translator {
  typedef T internal_type;
  typedef T external_type;

  boost::optional<T> get_value(const T& v) { return v.substr(1, v.size() - 2); }
  boost::optional<T> put_value(const T& v) { return '"' + v + '"'; }
};

void AdminTest::testPtreeWrite() {
  using namespace std;
  using boost::property_tree::ptree;
  using boost::property_tree::basic_ptree;
  std::cout << "START@UT@ testPtreeWrite" << std::endl;
  try {
    ptree root, arr, elem2;
    basic_ptree<std::string, std::string> elem1;
    elem1.put<int>("int", 10);
    elem1.put<bool>("bool", true);
    elem2.put<double>("double", 2.2);
    elem2.put<std::string>("string", "some htpp://wwww.google.com string",
                           my_id_translator<std::string>());

    arr.push_back(std::make_pair("", elem1));
    arr.push_back(std::make_pair("", elem2));
    root.put_child("path1.path2", arr);

    std::stringstream ss;
    write_json(ss, root);
    std::string my_string_to_send_somewhere_else = ss.str();

    cout << my_string_to_send_somewhere_else << endl;

  } catch (std::exception& e) {
    cout << e.what();
  }
  std::cout << "END@UT@ testPtreeWrite" << std::endl;
}

void AdminTest::testAbout() {
  std::cout << "START@UT@START testAbout" << std::endl;
  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  pion::tcp::connection tcp_conn(iota::Process::get_process().get_io_service());
  boost::system::error_code error_code;
  error_code =
      tcp_conn.connect(boost::asio::ip::address::from_string("127.0.0.1"),
                       iota::Process::get_process().get_http_port());

  {
    pion::http::request http_request("/TestAdmin/about");
    http_request.set_method("GET");
    std::string ct("application/json");
    http_request.set_content_type(ct);
    std::cout << "@UT@ABOUT" << std::endl;
    http_request.send(tcp_conn, error_code);
    pion::http::response http_response(http_request);
    http_response.receive(tcp_conn, error_code);
    tcp_conn.close();
    int code_res = http_response.get_status_code();
    std::string response = http_response.get_content();
    std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
    IOTASSERT(code_res == 200);
  }

  std::cout << "END@UT@START testAbout" << std::endl;
}

void AdminTest::testcheck_device_protocol() {
  std::cout << "START@UT@START testcheck_device_protocol" << std::endl;
  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  // adm->check_device_protocol("UL20", );

  std::cout << "END@UT@START testAbout" << std::endl;
}

void AdminTest::testValidationSchema() {
  std::cout << "START@UT@START testValidationSchema" << std::endl;
  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  std::string errorSTR;
  errorSTR = validationSchema("{\"miregex\":\"cat\"}", "^c.t$");
  std::cout << "@UT1@" << errorSTR << std::endl;
  CPPUNIT_ASSERT(errorSTR.compare("validation ok") == 0);

  errorSTR = validationSchema("{\"miregex\":\"cat_\"}", "^c.t$");
  std::cout << "@UT2@" << errorSTR << std::endl;
  CPPUNIT_ASSERT(
      errorSTR.compare("String does not match pattern \"^c.t$\" [/miregex]") ==
      0);

  errorSTR = validationSchema("{\"miregex\":\"cat\"}", "^[0-9a-zA-Z_]+$");
  std::cout << "@UT3@" << errorSTR << std::endl;
  CPPUNIT_ASSERT(errorSTR.compare("validation ok") == 0);

  errorSTR = validationSchema("{\"miregex\":\"cat'\"}", "^[0-9a-zA-Z_]+$");
  std::cout << "@UT4@" << errorSTR << std::endl;
  CPPUNIT_ASSERT(
      errorSTR.compare(
          "String does not match pattern \"^[0-9a-zA-Z_]+$\" [/miregex]") == 0);

  std::string patter_no_comi = "[^';:";
  // patter_no_comi += "\"";
  patter_no_comi += "]+$";
  errorSTR = validationSchema("{\"miregex\":\"cat\"}", patter_no_comi);
  std::cout << "@UT5@" << errorSTR << std::endl;
  CPPUNIT_ASSERT(errorSTR.compare("validation ok") == 0);

  std::string json = "{\"miregex\":\"cat";
  json += "'";
  json.append("\"}");
  errorSTR = validationSchema(json, patter_no_comi);
  std::cout << "@UT6@" << errorSTR << std::endl;
  CPPUNIT_ASSERT(errorSTR.find("String does not match pattern") !=
                 std::string::npos);

  errorSTR = validationSchema("{\"miregex\":\"/p\"}", "^/");
  std::cout << "@UT7@" << errorSTR << std::endl;
  CPPUNIT_ASSERT(errorSTR.compare("validation ok") == 0);

  errorSTR = validationSchema("{\"miregex\":\"p\"}", "^/");
  std::cout << "@UT8@" << errorSTR << std::endl;
  CPPUNIT_ASSERT(
      errorSTR.compare("String does not match pattern \"^/\" [/miregex]") == 0);

  std::cout << "END@UT@START testValidationSchema" << std::endl;
}

std::string AdminTest::validationSchema(const std::string& json_str,
                                        const std::string& pattern) {
  std::string errorSTR;

  std::string json_schema(
      "{\"$schema\": \"http://json-schema.org/draft-04/schema\","
      "\"type\":\"object\","
      "\"additionalProperties\":false,"
      "\"properties\":{"
      "\"miregex\":{"
      "\"type\":\"string\","
      "\"pattern\":\"");

  json_schema.append(pattern);
  json_schema.append("\"}}}");

  std::cout << "data:" << json_str << std::endl;
  std::cout << "schema:" << json_schema << std::endl;

  libvariant::Variant data = libvariant::DeserializeGuess(json_str);
  libvariant::Variant schema_data = libvariant::DeserializeGuess(json_schema);
  libvariant::AdvSchemaLoader loader5;
  libvariant::SchemaResult result =
      libvariant::SchemaValidate(schema_data, data, &loader5);
  if (result.Error()) {
    std::string validation_error("Malformed data");
    std::string data_path;
    try {
      validation_error = result.GetErrors().at(0).GetMessage();
      data_path = result.GetErrors().at(0).GetDataPathStr();
    } catch (std::exception& e) {
    }
    errorSTR.append(validation_error);
    errorSTR.append(" [").append(data_path).append("]");
  } else {
    errorSTR.append("validation ok");
  }

  return errorSTR;
}

void AdminTest::testBADConfigurator() {
  std::cout << "START apiKeyTest testBADConfigurator" << std::endl;

  iota::Configurator* conf = iota::Configurator::instance();

  std::string configuration("");

  std::stringstream ss;
  ss << configuration;
  conf->read_file(ss);
  std::string all = conf->getAll();

  std::string error = conf->getError();
  std::cout << "data2:" << error << std::endl;
  CPPUNIT_ASSERT(
      error.compare("Configurator: Text only contains white space(s)[0]") == 0);

  conf->release();
  std::cout << "delete:" << std::endl;
  // delete conf;
  std::cout << "END  apiKeyTest testBADConfigurator" << std::endl;
}

void AdminTest::testConfigurator() {
  std::cout << "START apiKeyTest testConfigurator" << std::endl;
  iota::Configurator* conf = iota::Configurator::instance();

  std::string configuration(
      "{\"timeout\": 5, \"resources\": [{\"resource\": "
      "\"/TestAdmin/res\",\"services\": [{\"apikey\": "
      "\"apikey-1\",\"cbroker\": \"http:://0.0.0.0:1026\", \"service\": "
      "\"service-1\",\"entity_type\": \"thing\",\"service_path\": "
      "\"servicepath\",\"token\": \"token1\"},");
  configuration.append(
      "{\"apikey\": \"apikeyduplicada\",\"cbroker\": \"http:://0.0.0.0:1026\", "
      "\"service\": \"serviceduplicado\",\"entity_type\": "
      "\"thing\",\"service_path\": \"servicepath1\",\"token\": \"token111\"},");
  configuration.append(
      "{\"apikey\": \"apikeyduplicada\",\"cbroker\": \"http:://0.0.0.0:1026\", "
      "\"service\": \"serviceduplicado\",\"entity_type\": "
      "\"thing\",\"service_path\": \"servicepath2\",\"token\": \"token222\"},");
  configuration.append(
      "{\"apikey\": \"\",\"service\": \"service-2\",\"service_path\": "
      "\"servicepath-2\",\"token\": \"token2\", \"entity_type\": \"thing\", "
      "\"cbroker\": \"http:://0.0.0.0:1026\"}]},");
  configuration.append(
      "{\"resource\": \"/TestAdmin/repsol\",\"services\": [{\"apikey\": "
      "\"\",\"service\": \"service-3\",\"service_path\": "
      "\"servicepath-3\",\"cbroker\": \"http:://0.0.0.0:1026\", "
      "\"entity_type\": \"thing\",\"token\": \"token3\"}]}], \"ngsi_url\": "
      "{\"updateContext\": \"/NGSI10/updateContext\"}}");

  std::stringstream ss;
  ss << configuration;
  // std::cout << "read_file" << std::endl;
  conf->read_file(ss);
  // std::cout << "read_file end" << std::endl;

  std::string all = conf->getAll();
  // std::cout << "data:" << all << std::endl;

  std::size_t found = all.find("apikey-1");
  CPPUNIT_ASSERT(found != std::string::npos);

  try {
    std::cout << " getServicebyApiKey -> noexists " << std::endl;
    const iota::JsonValue& pt2 =
        conf->getServicebyApiKey("/TestAdmin/res", "/noexists");
    CPPUNIT_ASSERT(true);
  } catch (std::exception& exc) {
    std::cout << "noexists excepcion" << std::endl;
    std::cout << exc.what() << std::endl;
  }

  std::cout << " getServicebyApiKey -> /TestAdmin/res apikeyduplicada"
            << std::endl;
  try {
    const iota::JsonValue& pt2 =
        conf->getServicebyApiKey("/TestAdmin/res", "apikeyduplicada");
    CPPUNIT_ASSERT(true);
  } catch (std::exception& exc) {
    std::cout << "apikeduplicada excepcion" << std::endl;
    std::cout << exc.what() << std::endl;
  }

  std::cout << " getService -> /TestAdmin/res serviceduplicado" << std::endl;
  try {
    const iota::JsonValue& pt2 =
        conf->getService("/TestAdmin/res", "serviceduplicado", "servicepath1");
    std::string s(pt2["token"].GetString());
    std::cout << "token " << s << std::endl;
    CPPUNIT_ASSERT(s.compare("token111") == 0);
  } catch (std::exception& exc) {
    std::cout << "serviceduplicado excepcion" << std::endl;
    std::cout << exc.what() << std::endl;
    CPPUNIT_ASSERT(true);
  }

  std::cout << " getService -> /TestAdmin/res serviceduplicado" << std::endl;
  try {
    const iota::JsonValue& pt2 =
        conf->getService("/TestAdmin/res", "serviceduplicado", "");
    CPPUNIT_ASSERT(true);
  } catch (std::exception& exc) {
    std::cout << "serviceduplicado excepcion" << std::endl;
    std::cout << exc.what() << std::endl;
  }

  std::cout << " getServicebyApiKey -> /TestAdmin/res apikey-1" << std::endl;
  const iota::JsonValue& pt =
      conf->getServicebyApiKey("/TestAdmin/res", "apikey-1");
  std::string s(pt["service"].GetString());
  CPPUNIT_ASSERT(s.compare("service-1") == 0);
  CPPUNIT_ASSERT_THROW(conf->getServicebyApiKey("/TestAdmin/res", "noapikey"),
                       std::runtime_error);
  CPPUNIT_ASSERT_THROW(conf->getService("/TestAdmin/res", "noservice", "/"),
                       std::runtime_error);

  std::cout << " getService -> /TestAdmin/res service-2" << std::endl;
  const iota::JsonValue& pt_cb =
      conf->getService("/TestAdmin/res", "service-2", "");
  std::string sp(pt_cb["service_path"].GetString());
  std::cout << "servic_path: " << sp << std::endl;
  CPPUNIT_ASSERT(sp.compare("servicepath-2") == 0);

  std::cout << " getServicebyApiKey -> /TestAdmin/res apikey-2  /kk"
            << std::endl;
  CPPUNIT_ASSERT_THROW(conf->getService("/TestAdmin/res", "service-2", "/kk"),
                       std::runtime_error);

  std::cout << " getServicebyApiKey -> /TestAdmin/repsol " << std::endl;
  const iota::JsonValue& pt_empty =
      conf->getServicebyApiKey("/TestAdmin/repsol", "");
  std::string sp1(pt_empty["service"].GetString());
  CPPUNIT_ASSERT(sp1.compare("service-3") == 0);
  std::string url(pt_cb["cbroker"].GetString());
  CPPUNIT_ASSERT(url.compare("http:://0.0.0.0:1026") == 0);

  // First fileds in JSON

  const iota::JsonValue& timeout = conf->get("timeout");
  if (timeout.IsNumber()) {
    CPPUNIT_ASSERT(timeout.GetInt64() == 5);
  }

  // No existe
  CPPUNIT_ASSERT_THROW(conf->get("FieldNotExists"), std::runtime_error);

  // std::cout << conf->getAll() << std::endl;
  // liberamos la memoria del configurador
  conf->release();

  std::cout << "END  apiKeyTest testConfigurator" << std::endl;
}

void AdminTest::testNgsiOperation() {
  std::cout << "START testNgsiOperationn" << std::endl;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);
  const iota::JsonValue& v = conf->get("ngsi_url");
  // std::cout << conf->getAll() << std::endl;
  std::string uc = v["updateContext"].GetString();
  std::cout << "ngsi_url.updateContext " << uc << std::endl;
  CPPUNIT_ASSERT(uc.compare("/NGSI10/updateContext") == 0);

  conf->release();
  std::cout << "END testNgsiOperation" << std::endl;
}

void AdminTest::testHttpMessages() {
  std::cout << "START testHttpMessages" << std::endl;
  iota::Configurator* conf = iota::Configurator::instance();
  std::string men = iota::Configurator::instance()->getHttpMessage(
      pion::http::types::RESPONSE_CODE_NOT_FOUND);
  CPPUNIT_ASSERT(men.compare(pion::http::types::RESPONSE_MESSAGE_NOT_FOUND) ==
                 0);
  std::cout << iota::Configurator::instance()->getHttpMessage(404) << std::endl;

  conf->release();
  std::cout << "END testHttpMessages" << std::endl;
}

void AdminTest::testConversionMap() {
  std::cout << "Start testConversionMap" << std::endl;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);
  std::map<std::string, std::string> to_map;
  conf->get("ngsi_url", to_map);

  CPPUNIT_ASSERT_MESSAGE("Checking json object to map", to_map.size() > 0);
  CPPUNIT_ASSERT_MESSAGE(
      "Checking map value",
      to_map["updateContext"].compare("/NGSI10/updateContext") == 0);
  std::cout << "End testConversionMap" << std::endl;
}

void AdminTest::testAuthInfo() {
  std::cout << "Start testAuthInfo" << std::endl;
  iota::Configurator* conf =
      iota::Configurator::initialize(PATH_CONFIG_WITH_AUTH);
  std::map<std::string, std::string> to_map;
  conf->get("oauth", to_map);

  CPPUNIT_ASSERT_MESSAGE("Checking json object to map", to_map.size() > 0);
  std::cout << to_map["pep_user"] << std::endl;
  CPPUNIT_ASSERT_MESSAGE("Checking map value",
                         to_map["pep_user"].compare("pep") == 0);

  // Pep rules
  std::multimap<std::string, iota::PepRule> rules;
  rules = conf->get_pep_rules();
  CPPUNIT_ASSERT(rules.size() == 2);
  std::multimap<std::string, iota::PepRule>::iterator i_r = rules.begin();
  i_r = rules.find("create");
  CPPUNIT_ASSERT(i_r != rules.end());
  CPPUNIT_ASSERT(i_r->second.verb.compare("POST") == 0);
  CPPUNIT_ASSERT(i_r->second.uri.compare("/ngsi/<protocol>/updateContext") ==
                 0);
  std::cout << "End testAuthInfo" << std::endl;
}

void AdminTest::testForbiddenCharacters() {
  std::cout << "Start TestForbiddenCharacters" << std::endl;
  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  srand(time(NULL));
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);
  std::string device_forbidden("dev_forb");
  std::string device_with_forbidden("{\"devices\": [{\"device_id\": \"");
  std::string cont("\",\"protocol\": \"PDI-IoTA-UltraLight\"}]}");
  std::map<std::string, std::string> headers;
  std::string query_string;
  std::string response;
  int code_res;
  std::string service = "service";
  service.append(boost::lexical_cast<std::string>(rand()));
  std::cout << "@UT@POST Service" << std::endl;
  code_res = http_test("/TestAdmin/services", "POST", service, "",
                       "application/json", POST_SERVICE, headers, "", response);

  std::vector<std::string> forbidden_characters;
  forbidden_characters.push_back("<");
  forbidden_characters.push_back(">");
  forbidden_characters.push_back("(");
  forbidden_characters.push_back(")");
  forbidden_characters.push_back(";");
  forbidden_characters.push_back("=");
  forbidden_characters.push_back("'");
  forbidden_characters.push_back("\\\"");
  forbidden_characters.push_back("/");
  forbidden_characters.push_back("?");
  forbidden_characters.push_back("#");
  forbidden_characters.push_back("&");
  forbidden_characters.push_back(" ");

  for (int i = 0; i < forbidden_characters.size(); i++) {
    std::string post_str(device_with_forbidden);
    post_str.append(device_forbidden);
    post_str.append(forbidden_characters[i]);
    post_str.append(cont);
    code_res = http_test("/TestAdmin/devices", "POST", service, "",
                         "application/json", post_str, headers, "", response);
    std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
    IOTASSERT(code_res == 400);
  }

  // length (256)
  std::string post_str_length(device_with_forbidden);
  post_str_length.append(device_forbidden);
  for (int i = 0; i < 256; i++) {
   post_str_length.append("a");
  }
  post_str_length.append(cont);
  code_res = http_test("/TestAdmin/devices", "POST", service, "",
                       "application/json", post_str_length, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 400);

  std::string post_str(device_with_forbidden);
  post_str.append(device_forbidden);
  post_str.append(cont);
  code_res = http_test("/TestAdmin/devices", "POST", service, "",
                       "application/json", post_str, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 201);
  code_res = http_test("/TestAdmin/services/" + service, "DELETE", service, "",
                       "application/json", "", headers, "", response);
  std::cout << "End TestForbiddenCharacters" << std::endl;
}

void AdminTest::testConfiguratorMongo() {
  std::cout << "START apiKeyTest testConfiguratorMongo" << std::endl;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  std::cout << "before inserts" << std::endl;
  iota::Collection table1("SERVICE");

  mongo::BSONObj p1 = BSON("apikey"
                           << "apikey-1"
                           << "cbroker"
                           << "http:://0.0.0.0:1026"
                           << "service"
                           << "service-1"
                           << "entity_type"
                           << "thing"
                           << "service_path"
                           << "servicepath"
                           << "token"
                           << "token1");
  table1.insert(p1);

  mongo::BSONObj p2 = BSON("apikey"
                           << "apikeyduplicada"
                           << "cbroker"
                           << "http:://0.0.0.0:1026"
                           << "service"
                           << "serviceduplicado"
                           << "entity_type"
                           << "thing"
                           << "service_path"
                           << "servicepath1"
                           << "token"
                           << "token111");
  table1.insert(p2);

  mongo::BSONObj p3 = BSON("apikey"
                           << "apikeyduplicada"
                           << "cbroker"
                           << "http:://0.0.0.0:1026"
                           << "service"
                           << "serviceduplicado"
                           << "entity_type"
                           << "thing"
                           << "service_path"
                           << "servicepath2"
                           << "token"
                           << "token222");
  table1.insert(p3);

  mongo::BSONObj p4 = BSON("apikey"
                           << "service-2"
                           << "cbroker"
                           << "http:://0.0.0.0:1026"
                           << "service"
                           << "serviceduplicado"
                           << "entity_type"
                           << "thing"
                           << "service_path"
                           << "servicepath-2"
                           << "token"
                           << "token111");
  table1.insert(p4);

  mongo::BSONObj p5 = BSON("apikey"
                           << ""
                           << "cbroker"
                           << "http:://0.0.0.0:1026"
                           << "service"
                           << "repsol"
                           << "entity_type"
                           << "thing"
                           << "service_path"
                           << "servicepath-repsol"
                           << "token"
                           << "token111");
  table1.insert(p5);

  mongo::BSONObj p6 = BSON("apikey"
                           << "apikey-6"
                           << "cbroker"
                           << "http:://0.0.0.0:1026"
                           << "service"
                           << "service-6"
                           << "entity_type"
                           << "thing"
                           << "service_path"
                           << "servicepath"
                           << "token"
                           << "token1");
  mongo::BSONObj p_att = BSON("name"
                              << "GPS"
                              << "type"
                              << "string"
                              << "value"
                              << "10");
  mongo::BSONObjBuilder o_p6;
  o_p6.appendElements(p6);
  mongo::BSONArrayBuilder a_obj;
  a_obj.append(p_att);
  o_p6.append("static_attributes", a_obj.arr());
  table1.insert(o_p6.obj());
  iota::RestHandle rh;
  boost::property_tree::ptree pt_cb;
  std::string apiKey;

  try {
    apiKey.assign("noexists");

    boost::property_tree::ptree pt_cb;
    std::cout << " getServicebyApiKey -> noexists " << std::endl;
    rh.get_service_by_apiKey(pt_cb, apiKey);
    std::cout << " getServicebyApiKey -> noexists " << std::endl;
    CPPUNIT_ASSERT(true);
  } catch (std::exception& exc) {
    std::cout << "noexists excepcion" << std::endl;
    std::cout << exc.what() << std::endl;
  }

  std::cout << " getServicebyApiKey -> /TestAdmin/res apikeyduplicada"
            << std::endl;
  try {
    apiKey.assign("apikeyduplicada");
    rh.get_service_by_apiKey(pt_cb, apiKey);
    CPPUNIT_ASSERT(true);
  } catch (std::exception& exc) {
    std::cout << "apikeduplicada excepcion" << std::endl;
    std::cout << exc.what() << std::endl;
  }

  std::cout << " getService -> /TestAdmin/res serviceduplicado" << std::endl;
  try {
    rh.get_service_by_name(pt_cb, "serviceduplicado", "servicepath1");
    std::string s(pt_cb.get<std::string>("service", ""));
    std::cout << "token " << s << std::endl;
    CPPUNIT_ASSERT(s.compare("token111") == 0);
  } catch (std::exception& exc) {
    std::cout << "serviceduplicado excepcion" << std::endl;
    std::cout << exc.what() << std::endl;
    CPPUNIT_ASSERT(true);
  }

  std::cout << " getService -> /TestAdmin/res serviceduplicado" << std::endl;
  try {
    rh.get_service_by_name(pt_cb, "serviceduplicado", "");
    CPPUNIT_ASSERT(true);
  } catch (std::exception& exc) {
    std::cout << "serviceduplicado excepcion" << std::endl;
    std::cout << exc.what() << std::endl;
  }

  try {
    std::cout << " getServicebyApiKey -> /TestAdmin/res apikey-1" << std::endl;
    rh.get_service_by_apiKey(pt_cb, "apikey-1");
    std::string s(pt_cb.get<std::string>("service", ""));
    CPPUNIT_ASSERT(s.compare("service-1") == 0);
    CPPUNIT_ASSERT_THROW(rh.get_service_by_apiKey(pt_cb, "noapikey"),
                         std::runtime_error);
    CPPUNIT_ASSERT_THROW(rh.get_service_by_name(pt_cb, "noservice", "/"),
                         std::runtime_error);

    std::cout << " getService -> /TestAdmin/res service-2" << std::endl;
    rh.get_service_by_name(pt_cb, "service-2", "");
    std::string sp(pt_cb.get<std::string>("service_path", ""));
    std::cout << "servic_path: " << sp << std::endl;
    CPPUNIT_ASSERT(sp.compare("servicepath-2") == 0);

    std::cout << " getServicebyApiKey -> /TestAdmin/res apikey-2  /kk"
              << std::endl;
    CPPUNIT_ASSERT_THROW(rh.get_service_by_name(pt_cb, "service-2", "/kk"),
                         std::runtime_error);

  } catch (std::exception& exc) {
    std::cout << "catch a borrar" << std::endl;
    std::cout << exc.what() << std::endl;
  }

  try {
    std::cout << " getServicebyApiKey -> /TestAdmin/res apikey-6" << std::endl;
    rh.get_service_by_apiKey(pt_cb, "apikey-6");
    std::string s(pt_cb.get<std::string>("service", ""));
    CPPUNIT_ASSERT(s.compare("service-6") == 0);
    boost::property_tree::ptree s_a = pt_cb.get_child("static_attributes");
    CPPUNIT_ASSERT_MESSAGE(
        "Checking static attribute GPS",
        s_a.get<std::string>("name", "").compare("GPS") == 0);
    CPPUNIT_ASSERT_MESSAGE(
        "Checking static attribute GPS",
        s_a.get<std::string>("value", "").compare("10") == 0);
    CPPUNIT_ASSERT_MESSAGE("Checking cbroker",
                           pt_cb.get<std::string>("cbroker", "")
                                   .compare("http:://0.0.0.0:1026") == 0);
  } catch (std::exception& exc) {
  }

  // std::cout << conf->getAll() << std::endl;
  // liberamos la memoria del configurador
  conf->release();

  mongo::BSONObjBuilder pobj;
  table1.remove(pobj.obj());
  std::cout << "END  apiKeyTest testConfiguratorMongo" << std::endl;
}

/**
 * Feature:  encode and decode uri and query params in uri
 *     create a device with an space in the name  'dev 1'  and entity 'ent 1'
 *     check you can get device by entity
 *     check that you can delete the device and query
 **/
void AdminTest::testSpaceURI() {
  std::cout << "START @UT@START testSpaceURI" << std::endl;
  std::map<std::string, std::string> headers;
  std::string query_string;

  const std::string POST_DEVICE_SPACE(
      "{\"devices\": "
      "[{\"device_id\": \"dev 1\",\"protocol\": "
      "\"PDI-IoTA-UltraLight\",\"entity_name\": \"ent 1\",\"entity_type\": "
      "\"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": "
      "\"America/Santiago\""
      "}]}");

  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  std::string response;
  int code_res;
  std::string service = "srvspace";
  std::string device = "dev%201";
  std::cout << "@UT@service " << service << std::endl;

  // POST the service
  std::cout << "@UT@POST Service" << std::endl;
  code_res = http_test("/TestAdmin/services", "POST", service, "",
                       "application/json", POST_SERVICE, headers, "", response);
  IOTASSERT(code_res == 201);

  std::cout << "@UT@GET The device does not exist" << std::endl;
  code_res = http_test("/TestAdmin/devices/noexists", "GET", service, "",
                       "application/json", "", headers, query_string, response);
  boost::algorithm::trim(response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 404);
  IOTASSERT(response.compare("{\"reason\":\"The device does not "
                             "exist\",\"details\":\"noexists\"}") == 0);

  std::cout << "@UT@POST device" << std::endl;
  code_res =
      http_test("/TestAdmin/devices", "POST", service, "", "application/json",
                POST_DEVICE_SPACE, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@GET" << std::endl;
  code_res =
      http_test("/TestAdmin/devices", "GET", service, "", "application/json",
                "", headers, "entity=ent%201", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE("Search by entity name ", code_res = 200);
  IOTASSERT(response.find("device_id") != std::string::npos);

  std::cout << "@UT@DELETE" << std::endl;
  code_res = http_test("/TestAdmin/devices/dev%201", "DELETE", service, "",
                       "application/json", "", headers, "", response);
  std::cout << "@UT@RESPONSEDELETE: " << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == DELETE_RESPONSE_CODE);

  std::cout << "@UT@GET" << std::endl;
  code_res = http_test("/TestAdmin/devices/dev%201", "GET", service, "",
                       "application/json", "", headers, query_string, response);
  boost::algorithm::trim(response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == GET_RESPONSE_CODE_NOT_FOUND);

  std::cout << "@UT@DELETE Service" << std::endl;
  code_res = http_test("/TestAdmin/services/" + service, "DELETE", service, "",
                       "application/json", "", headers, "", response);
  IOTASSERT(code_res == 400);
  IOTASSERT(response.find("resource parameter is mandatory") !=
            std::string::npos);

  std::cout << "END@UT@ testSpaceURI" << std::endl;
}

void AdminTest::testRetriesRegisterManager() {
  std::cout << "START@UT@ testRetriesRegisterManager" << std::endl;

  TestSetup test_setup(get_service_name(__FUNCTION__), "/TestAdmin/d");

  iota::AdminService* adm;
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");

  std::string mock_port = boost::lexical_cast<std::string>(port);

  adm = iota::Process::get_process().get_admin_service();

  std::string manager("http://127.0.0.1:");
  iota::RestHandle* spserv =
      (iota::RestHandle*)iota::Process::get_process().get_service(
          "/TestAdmin/d");

  std::cout << "Timer SET" << std::endl;
  // 1. setup timer to be launched

  adm->set_register_retries(true);
  // 2. setup a mock as IoT Manager
  spserv->set_iota_manager_endpoint(manager + mock_port + "/mock");

  // 3. wait some secs...
  ASYNC_TIME_WAIT
  // 4. check register has been done to iot manager.

  std::string result = cb_mock->get_last("/mock");
  std::cout << "@UT@register:" << result << std::endl;
  CPPUNIT_ASSERT(
      result.find(" \"protocol\" : \"PDI-IoTA-UltraLight\","
                  " \"description\" : \"Ultra Light Propietary Protocol\","
                  " \"iotagent\" : \"http://127.0.0.1/TestAdmin\"") !=
      std::string::npos);

  std::cout << "END@UT@ testRetriesRegisterManager" << std::endl;
}

void AdminTest::testPostServiceWrongResource() {
  std::cout << "START@UT@ testPostServiceWrongResource" << std::endl;

  std::map<std::string, std::string> headers;

  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);
  std::string query_string("apikey=apikey&resource=/TestAdmin/d");
  std::string response;
  std::string service = "service";
  service.append(boost::lexical_cast<std::string>(rand()));
  std::cout << "@UT@service " << service << std::endl;

  std::cout << "@UT@POST" << std::endl;
  int code_res = http_test(URI_SERVICE, "POST", service, "", "application/json",
                           POST_SERVICE_WRONG_RESOURCE, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 400);

  std::cout << "@UT@POST INVALID RESOURCE" << std::endl;
  code_res = http_test(URI_SERVICE, "POST", service, "", "application/json",
                       POST_SERVICE_INVALID_RESOURCE, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 400);

  std::cout << "@UT@POST INVALID APIKEY" << std::endl;
  code_res = http_test(URI_SERVICE, "POST", service, "", "application/json",
                       POST_SERVICE_INVALID_APIKEY, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 400);

  code_res = http_test(URI_SERVICE, "POST", service, "", "application/json",
                       POST_SERVICE, headers, "", response);

  std::cout << "@UT@PUT UNKNOWN RESOURCE" << std::endl;
  code_res = http_test("/TestAdmin/services/" + service, "PUT", service, "",
                       "application/json", "{\"resource\":\"/nanan\"}", headers,
                       query_string, response);
  std::cout << "@UT@PUT RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE("PUT with invalid resource 400 error expected",
                    code_res == 400);

  std::cout << "@UT@PUT INVALID RESOURCE" << std::endl;
  code_res = http_test("/TestAdmin/services/" + service, "PUT", service, "",
                       "application/json",
                       "{\"resource\":\"<script>alert('ok')</script>\"}",
                       headers, query_string, response);
  std::cout << "@UT@PUT RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT_MESSAGE("PUT with invalid resource 400 error expected",
                    code_res == 400);
  IOTASSERT_MESSAGE("Pattern must be checked",
                    response.find("{\"reason\":\"The request is not well "
                                  "formed\",\"details\":\"String does not "
                                  "match pattern ") != std::string::npos);

  std::cout << "END@UT@ testPostServiceWrongResource" << std::endl;
}
