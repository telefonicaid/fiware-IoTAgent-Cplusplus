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
#include "util/device_collection.h"

#include "sampleTest.h"
#include "ngsi/ContextResponses.h"

#include "services/admin_service.h"
#include "services/ngsi_service.h"
#include "../mocks/http_mock.h"
#include <boost/property_tree/ptree.hpp>
#include <pion/logger.hpp>
#include <pion/process.hpp>
#include <pion/http/request.hpp>
#include <boost/filesystem/operations.hpp>
#include "sample_test/test_service.h"
#include "sample_test/test_command_service.h"
#include <boost/shared_ptr.hpp>
#include <util/device.h>
#include <boost/algorithm/string/replace.hpp>

#include <cmath>
#include <ctime>

#include "util/dev_file.h"

#define  PATH_CONFIG "../../tests/iotagent/config.json"

#define  PATH_DEV_CFG "../../tests/iotagent/devices.json"

#define  IOTASSERT_MESSAGE(x,y) \
         std::cout << "@" << __LINE__ << "@" << x << std::endl; \
         CPPUNIT_ASSERT_MESSAGE(x,y)

#define  IOTASSERT(y) \
         std::cout << "@" << __LINE__ << "@" << std::endl; \
         CPPUNIT_ASSERT(y)

#define RESPONSE_MESSAGE_NGSI_OK "{ \"statusCode\" : { \"code\":200, \"reasonPhrase\":\"OK\"}}"
#define RESPONSE_CODE_NGSI 200

CPPUNIT_TEST_SUITE_REGISTRATION(SampleTest);
namespace iota {
std::string logger("main");
std::string URL_BASE("/iot");
}
iota::AdminService* AdminService_ptr;



void SampleTest::setUp() {
  std::cout << "setUp SampleTest " << std::endl;


  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
}

void SampleTest::tearDown() {
  std::cout << "tearDown SampleTest " << std::endl;
}
void SampleTest::start_cbmock(boost::shared_ptr<HttpMock>& cb_mock,
                              const std::string& type) {
  cb_mock->init();
  std::string mock_port = boost::lexical_cast<std::string>(cb_mock->get_port());
  std::cout << "mock with port:" << mock_port << std::endl;

  iota::Configurator::release();
  iota::Configurator* my_instance = iota::Configurator::instance();

  std::stringstream ss;
  ss << "{ \"ngsi_url\": {"
     <<   "     \"updateContext\": \"/NGSI10/updateContext\","
     <<   "     \"registerContext\": \"/NGSI9/registerContext\","
     <<   "     \"queryContext\": \"/NGSI10/queryContext\""
     <<   "},"
     <<   "\"timeout\": 1,"
     <<   "\"dir_log\": \"/tmp/\","
     <<   "\"timezones\": \"/etc/iot/date_time_zonespec.csv\","
     <<   "\"storage\": {"
     <<   "\"host\": \"127.0.0.1\","
     <<   "\"type\": \"" <<  type << "\","
     <<   "\"port\": \"27017\","
     <<   "\"dbname\": \"iot\","
     <<   "\"file\": \"../../tests/iotagent/devices.json\""
     << "},"
     << "\"resources\":[{\"resource\": \"/iot/test\","
     << "  \"options\": {\"FileName\": \"TestService\" },"
     <<    " \"services\":[ {"
     <<   "\"apikey\": \"apikey-test\","
     <<   "\"service\": \"srvtest\","
     <<   "\"service_path\": \"/srvpathtest\","
     <<   "\"token\": \"token-test\","
     <<   "\"cbroker\": \"http://127.0.0.1:" << mock_port << "/mock\", "
     <<   "\"entity_type\": \"thing\""
     << "} ] } ] }";
  //my_instance->update_conf(ss);
  std::string err = my_instance->read_file(ss);
  std::cout << "GET CONF " << my_instance->getAll() << std::endl;
  if (!err.empty()) {
    std::cout << "start_cbmock:" << err << std::endl;
    std::cout << "start_cbmock_data:" << ss.str() << std::endl;
  }


}


/***
  *  POST http://10.95.26.51:8002/d?i=Device_UL2_0_RESTv2&k=4orh3jl3h40qkd7fk2qrc52ggb
  *     ${#Project#END_TIME2}|t|${Properties#value}
  *     ${#Project#END_TIME2}|t|${Properties#value}#t|${Properties#value2}
  *     ${#Project#END_TIME}|t|${Properties#value}#l|${Properties#value2}/${Properties#value2_1}
  **/
void SampleTest::testNormalPOST() {
  std::cout << "START testNormalPOST" << std::endl;

  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);
  std::string cb_last;


  iota::TestService sampleserv;
  sampleserv.set_resource("/iot/test");

  std::string querySTR = "i=dev_1&k=apikey-test";
  std::string bodySTR = "Hello World";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/test"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "dev_1"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey-test"));
    pion::http::response http_response;
    std::string response;
    sampleserv.service(http_request, url_args, query_parameters,
                       http_response, response);

    std::cout << "POST fecha + temperatura " << http_response.get_status_code() <<
              std::endl;
    CPPUNIT_ASSERT(http_response.get_status_code() == 200);
    //updateContext to CB
    cb_last = cb_mock->get_last();
    std::cout << "@UT@CB"<< cb_last << std::endl;
    CPPUNIT_ASSERT(cb_last.find("\"id\":\"dev_1\",\"type\":\"thing\"") !=
                   std::string::npos);

  }
  cb_mock->stop();
  std::cout << "END testNormalPOST " << std::endl;
}

/***
  *  POST http://10.95.26.51:8002/d?i=Device_UL2_0_RESTv2&k=4orh3jl3h40qkd7fk2qrc52ggb
  *     ${#Project#END_TIME2}|t|${Properties#value}
  *     ${#Project#END_TIME2}|t|${Properties#value}#t|${Properties#value2}
  *     ${#Project#END_TIME}|t|${Properties#value}#l|${Properties#value2}/${Properties#value2_1}
  **/
void SampleTest::testCommandNormalPOST() {
  std::cout << "START testCommandNormalPOST" << std::endl;

  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);
  std::string cb_last;


  iota::TestCommandService sampleserv;
  sampleserv.set_resource("/iot/test");

  std::string querySTR = "i=dev_1&k=apikey-test";
  std::string bodySTR = "Hello World";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/test"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "dev_1"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey-test"));
    pion::http::response http_response;
    std::string response;
    sampleserv.service(http_request, url_args, query_parameters,
                       http_response, response);

    std::cout << "POST fecha + temperatura " << http_response.get_status_code() <<
              std::endl;
    CPPUNIT_ASSERT(http_response.get_status_code() == 200);
    //updateContext to CB
    cb_last = cb_mock->get_last();
    std::cout << "@UT@CB"<< cb_last << std::endl;
    CPPUNIT_ASSERT(cb_last.find("\"id\":\"thing:dev_1\",\"type\":\"thing\"") !=
                   std::string::npos);

  }
  querySTR = "i=unitTest_devtest_endpoint&k=apikey-test";
  bodySTR = "Hello World2";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/test"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_devtest_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey-test"));
    pion::http::response http_response;
    std::string response;
    sampleserv.service(http_request, url_args, query_parameters,
                       http_response, response);

    std::cout << "POST fecha + temperatura " << http_response.get_status_code() <<
              std::endl;
    CPPUNIT_ASSERT(http_response.get_status_code() == 200);
    //updateContext to CB
    cb_last = cb_mock->get_last();
    std::cout << "@UT@CB"<< cb_last << std::endl;
    CPPUNIT_ASSERT(cb_last.find("\"id\":\"room_uttest\",\"type\":\"thing\"") !=
                   std::string::npos);
    CPPUNIT_ASSERT(cb_last.find("\"temperature") !=
                   std::string::npos);
  }


  cb_mock->stop();
  std::cout << "END testCommandNormalPOST " << std::endl;
}

void SampleTest::testPUSHCommand() {
  std::cout << "@UT@START testPUSHCommand" << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  //simulador del device
  boost::shared_ptr<HttpMock> device_mock;
  device_mock.reset(new HttpMock(9999, "/device"));
  device_mock->init();

  iota::TestCommandService plugin;
  plugin.set_resource("/iot/test");

  // updateContext
  std::string querySTR = "";
  std::string bodySTR = "{\"updateAction\":\"UPDATE\",";
  bodySTR.append("\"contextElements\":[{\"id\":\"room_uttest\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR.append("\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"unitTest_devtest_endpoint@command|22\",");
  bodySTR.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR.append("]} ]}");
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, "srvtest");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/srvpathtest");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    plugin.default_op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    //respuesta al update de contextBroker
    std::cout << "@UT@RESPONSE" << http_response.get_status_code() << " " <<
              response << std::endl;
    IOTASSERT(response.find(RESPONSE_MESSAGE_NGSI_OK) !=
                   std::string::npos);
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);

    std::string cb_last = cb_mock->get_last();
    // info
    std::cout << "@UT@INFO" << cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_uttest\",\"type\":\"thing\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_info\",\"type\":\"string\",\"value\":\"unitTest_devtest_endpoint@command|22|command_response\"")
      !=
      std::string::npos);
    // OK
    IOTASSERT(cb_last.find("\"id\":\"room_uttest\",\"type\":\"thing\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"OK\"")
      !=
      std::string::npos);

    cb_last = cb_mock->get_last();
    // pending
    std::cout << "@UT@Pending"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_uttest\",\"type\":\"thing\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"pending\"")
      !=
      std::string::npos);
  }

  cb_mock->stop();
  device_mock->stop();
  std::cout << "@UT@END testPUSHCommand " << std::endl;
}

void SampleTest::testPollingCommand() {
  std::cout << __LINE__ << "@UT@START testPollingCommand" << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  iota::TestCommandService plugin;
  plugin.set_resource("/iot/test");

  // updateContext
  std::string querySTR = "";
  std::string bodySTR = "{\"updateAction\":\"UPDATE\",";
  bodySTR.append("\"contextElements\":[{\"id\":\"room_uttestno\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR.append("\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"unitTest_devtest_noendpoint@ping6|22\",");
  bodySTR.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR.append("]} ]}");
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, "srvtest");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/srvpathtest");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    plugin.default_op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "@UT@POST updateContext " << http_response.get_status_code() <<
              std::endl;
    IOTASSERT_MESSAGE("@UT@POST, response code no 200" ,
                           http_response.get_status_code() == 200);

    std::string cb_last = cb_mock->get_last();
    std::cout << "@UT@READY_FOR_READ" <<cb_last << std::endl;
    // ready_for_read
    IOTASSERT_MESSAGE("@UT@READY_FOR_READ, entity or entity_type in not correct"
                           ,
                           cb_last.find("\"id\":\"room_uttestno\",\"type\":\"thing\"") !=std::string::npos);
    IOTASSERT_MESSAGE("@UT@READY_FOR_READ, name of command or value is not correct",
                           cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"pending\"")
                           !=
                           std::string::npos);

  }

  // GET desde el device
  querySTR = "i=unitTest_devtest_noendpoint&k=apikey-test";
  bodySTR = "";
  std::string command;
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/test"));
    http_request->set_method("GET");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_devtest_noendpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey-test"));
    pion::http::response http_response;
    std::string response;
    plugin.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "@UT@GET command " << http_response.get_status_code() <<
              ":" <<response << std::endl;
    command = response;
    IOTASSERT_MESSAGE("@UT@GET response code no 200" ,
                           http_response.get_status_code() == 200);

    std::string cb_last = cb_mock->get_last();
    std::cout << "@UT@DELIVERED" << cb_last << std::endl;
    // delivered
    IOTASSERT_MESSAGE("@UT@DELIVERED, entity or entity_type in not correct" ,
                           cb_last.find("\"id\":\"room_uttestno\",\"type\":\"thing\"") !=std::string::npos);
    IOTASSERT_MESSAGE("@UT@DELIVERED, name of command or value is not correct"
                           ,
                           cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"delivered\"")
                           !=std::string::npos);

  }

  // POST resultado del comando
  std::size_t found = command.find("|@|");
  if (found!=std::string::npos){
      command = command.substr(0, found);
  }

  querySTR = "i=unitTest_devtest_noendpoint&k=apikey-test";
  bodySTR = command;
  bodySTR.append("|command_result");
  std::cout << "@UT@POST command result " << bodySTR << std::endl;
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/test"));
    http_request->set_method("PUT");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_devtest_noendpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey-test"));
    pion::http::response http_response;
    std::string response;
    plugin.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "@UT@POST command result " << http_response.get_status_code() <<
              std::endl;
    IOTASSERT_MESSAGE("@UT@GET response code no 200" ,
                           http_response.get_status_code() == 200);

    std::string cb_last = cb_mock->get_last();
    std::cout << "@UT@INFO" << cb_last << std::endl;
    // info
    IOTASSERT_MESSAGE("@UT@INFO, entity or entity_type in not correct" ,
                           cb_last.find("\"id\":\"room_uttestno\",\"type\":\"thing\"") !=std::string::npos);
    IOTASSERT_MESSAGE("@UT@INFO, name of command or value is not correct" ,
                           cb_last.find("|command_result")
                           !=
                           std::string::npos);

    std::cout << "@UT@OK" << cb_last << std::endl;
    // OK
    IOTASSERT_MESSAGE("@UT@OK, entity or entity_type in not correct" ,
                           cb_last.find("\"id\":\"room_uttestno\",\"type\":\"thing\"") !=std::string::npos);
    IOTASSERT_MESSAGE("@UT@OK, name of command or value is not correct" ,
                           cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"OK\"")
                           !=
                           std::string::npos);

  }

  cb_mock->stop();
  std::cout << "@UT@END testPollingCommand " << std::endl;
}
