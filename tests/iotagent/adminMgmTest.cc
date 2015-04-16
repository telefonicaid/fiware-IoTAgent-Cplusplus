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
#include "adminMgmTest.h"

#include <Variant/Schema.h>
#include <Variant/SchemaLoader.h>

#include <iostream>
#include <string>
#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "util/iota_exception.h"

#include <pion/logger.hpp>
#include <pion/process.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include "util/device_collection.h"
#include "util/service_collection.h"
#include "util/service_mgmt_collection.h"
#include "services/admin_service.h"

#define PATH_CONFIG_MONGO "../../tests/iotagent/config_mongo.json"

#define  IOTASSERT_MESSAGE(x,y) \
         std::cout << "@" << __LINE__ << "@" << x << std::endl; \
         CPPUNIT_ASSERT_MESSAGE(x,y)

#define  IOTASSERT(y) \
         std::cout << "@" << __LINE__ << "@" << std::endl; \
         CPPUNIT_ASSERT(y)

CPPUNIT_TEST_SUITE_REGISTRATION(AdminMgmTest);
iota::AdminService* AdminService_ptr;
namespace iota {
std::string URL_BASE = "/iot";
std::string logger("main");
}

const std::string AdminMgmTest::HOST("127.0.0.1");
const std::string AdminMgmTest::CONTENT_JSON("application/json");

const int AdminMgmTest::POST_RESPONSE_CODE = 201;
//GET ALL empty
const int AdminMgmTest::GET_RESPONSE_CODE = 200;
const int AdminMgmTest::GET_RESPONSE_CODE_NOT_FOUND = 404;

////////////////////
////  PROTOCOLS
const std::string AdminMgmTest::URI_PROTOCOLS("/iot/protocols");
//POST
const std::string
AdminMgmTest::POST_PROTOCOLS1("{\"endpoint\": \"host1\","
                          "\"resource\": \"/iot/d\","
                          "\"protocol\": \"UL20\","
                          "\"description\": \"Ultralight 2.0\""
                          "}");
const std::string
AdminMgmTest::POST_PROTOCOLS2("{\"endpoint\": \"host2\","
                          "\"resource\": \"/iot/mqtt\","
                          "\"protocol\": \"MQTT\","
                          "\"description\": \"mqtt example\","
                          "\"services\": [{"
                                "\"apikey\": \"apikey3\","
                                "\"service\": \"service2\","
                                "\"service_path\": \"/ssrv2\","
                                "\"token\": \"token2\","
                                "\"cbroker\": \"http://127.0.0.1:1026\","
                                "\"resource\": \"/iot/mqtt\","
                                "\"entity_type\": \"thing\""
                          "}]}");
const std::string
AdminMgmTest::POST_PROTOCOLS3("{\"endpoint\": \"host3\","
                          "\"resource\": \"/iot/d\","
                          "\"protocol\": \"UL20\","
                          "\"description\": \"Ultralight 2.0\","
                          "\"services\": [{"
                                "\"apikey\": \"apikey3\","
                                "\"service\": \"service2\","
                                "\"service_path\": \"/ssrv2\","
                                "\"token\": \"token2\","
                                "\"cbroker\": \"http://127.0.0.1:1026\","
                                "\"resource\": \"/iot/d\","
                                "\"entity_type\": \"thing\""
                          "}]}");
const std::string
AdminMgmTest::POST_PROTOCOLS4("TODO");
const std::string
AdminMgmTest::GET_PROTOCOLS_RESPONSE("{ \"count\": 0,\"devices\": []}");


////////////////////
////  SERVICE_MANAGEMENT
const std::string AdminMgmTest::URI_SERVICES_MANAGEMET("/iot/services");
//POST
const std::string
AdminMgmTest::POST_SERVICE_MANAGEMENT1("{\"services\": [{"
       "\"protocol\": [\"55261958d31fc2151cc44c70\", \"55261958d31fc2151cc44c73\"],"
       "\"apikey\": \"apikey\",\"token\": \"token\","
       "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\""
     "}]}");
const std::string
AdminMgmTest::POST_SERVICE_MANAGEMENT2("{\"services\": [{"
       "\"protocol\": [\"55261958d31fc2151cc44c70\", \"55261958d31fc2151cc44c73\"],"
       "\"apikey\": \"apikey\",\"token\": \"token\","
       "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\""
     "}]}");

const std::string
AdminMgmTest::GET_SERVICE_MANAGEMENT_RESPONSE("{ \"count\": 0,\"devices\": []}");

////////////////////
////  DEVICE _MANAGEMENT
const std::string AdminMgmTest::URI_DEVICES_MANAGEMEMT("/iot/devices");
//POST
const std::string
AdminMgmTest::POST_DEVICE_MANAGEMENT1("{\"devices\": "
                           "[{\"device_id\": \"device_id\",\"entity_name\": \"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
                           "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": \"device_id@ping|%s\" }],"
                           "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }],"
                           "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
                           "}]}");

const std::string
AdminMgmTest::GET_DEVICE_MANAGEMENT_RESPONSE("{ \"count\": 0,\"devices\": []}");

AdminMgmTest::AdminMgmTest() {
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG_MONGO);
  pion::process::initialize();

  adm = new iota::AdminService();
  AdminService_ptr = adm;
  adm->set_manager();
  AdminService_ptr->add_service("/iot/res", AdminService_ptr);
  wserver.reset(new pion::http::plugin_server(scheduler));
  wserver->add_service("/iot", adm);
  wserver->start();

}

AdminMgmTest::~AdminMgmTest() {
  wserver->stop();
  scheduler.shutdown();
}

void AdminMgmTest::setUp() {
}

void AdminMgmTest::tearDown() {
}


int AdminMgmTest::http_test(const std::string& uri,
                         const std::string& method,
                         const std::string& service,
                         const std::string& service_path,
                         const std::string& content_type,
                         const std::string& body,
                         const std::map<std::string,std::string>& headers,
                         const std::string& query_string,
                         std::string& response) {
  pion::tcp::connection tcp_conn(scheduler.get_io_service());
  boost::system::error_code error_code;
  error_code = tcp_conn.connect(
                 boost::asio::ip::address::from_string(HOST), wserver->get_port());

  pion::http::request http_request(uri);
  http_request.set_method(method);
  http_request.set_content_type(content_type);
  if (!service.empty()) {
    http_request.add_header(iota::types::FIWARE_SERVICE, service);
  }
  if (!service_path.empty()) {
    http_request.add_header(iota::types::FIWARE_SERVICEPATH, service_path);
  }

  //http_request.add_header("Accept", "application/json");

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


void AdminMgmTest::testProtocol(){
  std::cout << "START @UT@START testProtocol" << std::endl;
  adm->set_manager();
  std::map<std::string, std::string> headers;
  std::string query_string;
  int code_res;
  std::string response;

  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG_MONGO);

  std::cout << "@UT@Remove all data in protocols" << std::endl;
  pion::http::response http_response;
  adm->delete_all_protocol_json(http_response, "", response);

  std::cout << "@UT@Post iotagents without services" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "POST", "", "",
                       "application/json",
                       POST_PROTOCOLS1, headers, "", response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@Post iotagents with a service" << std::endl;
  std::cout << "@UT@POST" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "POST", "", "",
                       "application/json",
                       POST_PROTOCOLS2, headers, "", response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);
  std::cout << "@UT@Post iotagents with a service" << std::endl;
  std::cout << "@UT@POST" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "POST", "", "",
                       "application/json",
                       POST_PROTOCOLS3, headers, "", response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@POST reregister POST_PROTOCOLS2" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "POST", "", "",
                       "application/json",
                       POST_PROTOCOLS2, headers, "", response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
 //TODO IOTASSERT(code_res == POST_RESPONSE_CODE);


  std::cout << "@UT@GET" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "GET", "", "",
                       "application/json", "",
                       headers, "", response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == GET_RESPONSE_CODE);
  IOTASSERT_MESSAGE("only return one",
                    response.find("\"count\": 2,\"protocols\"")!= std::string::npos);

  std::cout << "@UT@ no delete for protocols, by now " << std::endl;

  std::cout << "END@UT@ testProtocol" << std::endl;

}

void AdminMgmTest::testServiceManagement(){
  srand(time(NULL));
  std::cout << "START @UT@START testServiceManagement" << std::endl;
  std::map<std::string, std::string> headers;
  std::string query_string;
  int code_res;
  std::string response;

  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG_MONGO);

  std::string service= "service" ;
  service.append(boost::lexical_cast<std::string>(rand()));
  std::cout << "@UT@service " << service << std::endl;

  std::cout << "@UT@GET get empty" << std::endl;
  code_res = http_test(URI_SERVICES_MANAGEMET , "GET", service, "",
                           "application/json", "",
                           headers, query_string, response);
  boost::algorithm::trim(response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\": 0,\"services\": []}") == 0);

  std::cout << "@UT@POST" << std::endl;
  code_res = http_test(URI_SERVICES_MANAGEMET, "POST", service, "", "application/json",
                       POST_SERVICE_MANAGEMENT1, headers, "", response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@GET only return one" << std::endl;
  code_res = http_test(URI_SERVICES_MANAGEMET, "GET", service, "/*",
                       "application/json", "",
                       headers, "", response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == GET_RESPONSE_CODE);
  IOTASSERT_MESSAGE("only return one",
                    response.find("\"count\": 1,\"services\"")!= std::string::npos);

  std::cout << "@UT@POST" << std::endl;
  code_res = http_test(URI_SERVICES_MANAGEMET, "POST", service, "/ssrv2",
                       "application/json",
                       POST_SERVICE_MANAGEMENT2, headers, "", response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@GET only return two" << std::endl;
  code_res = http_test(URI_SERVICES_MANAGEMET, "GET", service, "/*",
                       "application/json", "",
                       headers, "", response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == GET_RESPONSE_CODE);
  IOTASSERT_MESSAGE("only return one",
                    response.find("\"count\": 2,\"services\"")!= std::string::npos);

  /*std::cout << "@UT@DELETE " << service << std::endl;
  code_res = http_test(URI_SERVICES_MANAGEMET, "DELETE", service, "/*",
                       "application/json", "",
                       headers, query_string, response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == DELETE_RESPONSE_CODE);

  std::cout << "@UT@GET" << std::endl;
  code_res = http_test(URI_SERVICE2, "GET", service, "/*",
                       "application/json", "",
                       headers, "", response);
  boost::algorithm::trim(response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\": 0,\"services\": []}") == 0);
*/

  std::cout << "END@UT@ testServiceManagement" << std::endl;

}
