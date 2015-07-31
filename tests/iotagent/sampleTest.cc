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
#include "util/iota_logger.h"
#include <pion/process.hpp>
#include <pion/http/request.hpp>
#include <boost/filesystem/operations.hpp>
#include "sample_test/test_service.h"
#include "sample_test/test_command_service.h"
#include <boost/shared_ptr.hpp>
#include <util/device.h>
#include <util/service_collection.h>
#include <boost/algorithm/string/replace.hpp>
#include "rest/oauth_filter.h"
#include "util_functions.h"
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

#define RESPONSE_MESSAGE_NGSI_OK "{\"contextResponses\":[{\"statusCode\":{\"code\":\"200\",\"reasonPhrase\":\"OK\",\"details\":\"\"},\"contextElement\":{\"id\":\"room_uttest\",\"type\":\"type2\""
#define RESPONSE_CODE_NGSI 200

#define  ASYNC_TIME_WAIT  boost::this_thread::sleep(boost::posix_time::milliseconds(100));

CPPUNIT_TEST_SUITE_REGISTRATION(SampleTest);

void SampleTest::setUp() {
  std::cout << "setUp SampleTest " << std::endl;
}
SampleTest::SampleTest() {
}

SampleTest::~SampleTest() {
}

void SampleTest::tearDown() {
  std::cout << "tearDown SampleTest " << std::endl;
}
/*
void SampleTest::start_cbmock(boost::shared_ptr<HttpMock>& cb_mock,
                              const std::string& type) {
  cb_mock->init();
  std::string mock_port = boost::lexical_cast<std::string>(cb_mock->get_port());
  std::cout << "mock with port:" << mock_port << std::endl;

  iota::Configurator::release();
  iota::Configurator* my_instance = iota::Configurator::instance();

  std::stringstream ss;
  ss << "{ \"ngsi_url\": {"
     <<   "     \"cbroker\": \"http//127.0.0.1:1026\","
     <<   "     \"updateContext\": \"/NGSI10/updateContext\","
     <<   "     \"registerContext\": \"/NGSI9/registerContext\","
     <<   "     \"queryContext\": \"/NGSI10/queryContext\""
     <<   "},"
     <<   "\"iota_manager\" : \"http://127.0.0.1:"<< mock_port << "/iot/protocols\","
     <<   "\"public_ip\" : \"127.0.0.1\","
     <<   "\"timeout\": 1,"
     <<   "\"dir_log\": \"/tmp/\","
     <<   "\"timezones\": \"/etc/iot/date_time_zonespec.csv\","
     <<   "\"schema_path\": \"../../schema\","
     <<   "\"storage\": {"
     <<   "\"host\": \"127.0.0.1\","
     <<   "\"type\": \"" <<  type << "\","
     <<   "\"pool_size\": \"2\","
     <<   "\"port\": \"27017\","
     <<   "\"dbname\": \"iotest\","
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
*/

/***
  *  POST http://10.95.26.51:8002/d?i=Device_UL2_0_RESTv2&k=4orh3jl3h40qkd7fk2qrc52ggb
  *     ${#Project#END_TIME2}|t|${Properties#value}
  *     ${#Project#END_TIME2}|t|${Properties#value}#t|${Properties#value2}
  *     ${#Project#END_TIME}|t|${Properties#value}#l|${Properties#value2}/${Properties#value2_1}
  **/
void SampleTest::testNormalPOST() {
  std::cout << "START testNormalPOST" << std::endl;
  std::string service(get_service_name(__FUNCTION__));
  std::string service_path("/");
  service_path.append(service);

  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock = (MockService*)
                         iota::Process::get_process().get_service("/mock");
  std::string cbroker_url("http://127.0.0.1:");
  cbroker_url.append(boost::lexical_cast<std::string>(port));
  cbroker_url.append("/mock/testNormalPOST");
  std::string
  post_service("{\"services\": [{"
               "\"apikey\": \"apikey-test\",\"token\": \"token\","
               "\"cbroker\": \"" + cbroker_url +
               "\",\"entity_type\": \"thing\",\"resource\": \"/TestSample/test\"}]}");
  boost::shared_ptr<iota::ServiceCollection> col(new iota::ServiceCollection());
  pion::http::response http_r;
  std::string r;
  ((iota::AdminService*)
   iota::Process::get_process().get_service("/TestSample"))->delete_service_json(
     col, service, "/", service, "apikey-test", "/TestSample/test", true,
     http_r, r, "1234", "4444");
  ((iota::AdminService*)
   iota::Process::get_process().get_service("/TestSample"))->post_service_json(col,
       service, "/", post_service,
       http_r, r, "apikey-test", "5678");
  std::string cb_last;
  iota::TestService* sampleserv = (iota::TestService*)
                                  iota::Process::get_process().get_service("/TestSample/test");
  std::string querySTR = "i=dev_1&k=apikey-test";
  std::string bodySTR = "Hello World";
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/TestSample/test"));
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
    sampleserv->service(http_request, url_args, query_parameters,
                        http_response, response);
    ASYNC_TIME_WAIT
    std::cout << "POST fecha + temperatura " << http_response.get_status_code() <<
              std::endl;
    CPPUNIT_ASSERT(http_response.get_status_code() == 200);
    //updateContext to CB
    cb_last = cb_mock->get_last("/mock/testNormalPOST/NGSI10/updateContext");
    std::cout << "@UT@CB"<< cb_last << std::endl;
    CPPUNIT_ASSERT(cb_last.find("\"id\":\"dev_1\",\"type\":\"thing\"") !=
                   std::string::npos);

  }

  r.clear();
  ((iota::AdminService*)
   iota::Process::get_process().get_service("/TestSample"))->delete_service_json(
     col, service, "/", service, "apikey-test", "/TestSample/test", true,
     http_r, r, "1234", "4444");
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

  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock = (MockService*)
                         iota::Process::get_process().get_service("/mock");

  std::string service(get_service_name(__FUNCTION__));
  std::string service_path("/");
  service_path.append(service);

  std::string cbroker_url("http://127.0.0.1:");
  cbroker_url.append(boost::lexical_cast<std::string>(port));
  cbroker_url.append("/mock/testCommandNormalPOST");
  std::string
  post_service("{\"services\": [{"
               "\"apikey\": \"apikey-test-1\",\"token\": \"token\","
               "\"cbroker\": \"" + cbroker_url +
               "\",\"entity_type\": \"thing\",\"resource\": \"/TestSample/cmdtest\"}]}");
  boost::shared_ptr<iota::ServiceCollection> col(new iota::ServiceCollection());
  pion::http::response http_r;
  std::string r;
  ((iota::AdminService*)
   iota::Process::get_process().get_service("/TestSample"))->delete_service_json(
     col, service, service_path, service, "apikey-test-1", "/TestSample/cmdtest", true,
     http_r, r, "1234", "4444");
  ((iota::AdminService*)
   iota::Process::get_process().get_service("/TestSample"))->post_service_json(col,
       service, service_path, post_service,
       http_r, r, "apikey-test-1", "5678");
  std::string cb_last;
  iota::TestCommandService* sampleserv = (iota::TestCommandService*)
                                  iota::Process::get_process().get_service("/TestSample/cmdtest");



  std::string querySTR = "i=dev_1&k=apikey-test-1";
  std::string bodySTR = "Hello World";
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/TestSample/cmdtest"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "dev_1"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey-test-1"));
    pion::http::response http_response;
    std::string response;
    sampleserv->service(http_request, url_args, query_parameters,
                        http_response, response);
    ASYNC_TIME_WAIT
    std::cout << "POST fecha + temperatura " << http_response.get_status_code() <<
              std::endl;
    CPPUNIT_ASSERT(http_response.get_status_code() == 200);
    //updateContext to CB
    cb_last = cb_mock->get_last("/mock/testCommandNormalPOST/NGSI10/updateContext");
    std::cout << "@UT@CB"<< cb_last << std::endl;
    CPPUNIT_ASSERT(cb_last.find("\"id\":\"thing:dev_1\",\"type\":\"thing\"") !=
                   std::string::npos);

  }

  // Device
  std::string device = get_device("unitTest_devtest_endpoint", sampleserv->get_protocol_data().protocol);
  add_device(device, service);
  querySTR = "i=unitTest_devtest_endpoint&k=apikey-test-1";
  bodySTR = "Hello World2";
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/TestSample/cmdtest"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_devtest_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey-test-1"));
    pion::http::response http_response;
    std::string response;
    sampleserv->service(http_request, url_args, query_parameters,
                        http_response, response);
    ASYNC_TIME_WAIT
    std::cout << "POST fecha + temperatura " << http_response.get_status_code() <<
              std::endl;
    CPPUNIT_ASSERT(http_response.get_status_code() == 200);
    //updateContext to CB
    cb_last = cb_mock->get_last("/mock/testCommandNormalPOST/NGSI10/updateContext");
    std::cout << "@UT@CB"<< cb_last << std::endl;
    CPPUNIT_ASSERT(cb_last.find("\"id\":\"room_uttest\",\"type\":\"thing\"") !=
                   std::string::npos);
    CPPUNIT_ASSERT(cb_last.find("\"temperature") !=
                   std::string::npos);
  }
  delete_device(device, service);
  r.clear();
  ((iota::AdminService*)
   iota::Process::get_process().get_service("/TestSample"))->delete_service_json(
     col, service, service_path, service, "apikey-test-1", "/TestSample/cmdtest", true,
     http_r, r, "1234", "4444");
  std::cout << "END testCommandNormalPOST " << std::endl;
}

void SampleTest::testPUSHCommand() {
  std::cout << "@UT@START testPUSHCommand" << std::endl;

  std::string cb_last;
  iota::TestCommandService* plugin = (iota::TestCommandService*)
                                     iota::Process::get_process().get_service("/TestSample/cmdtest");

  std::string service(get_service_name(__FUNCTION__));
  std::string service_path("/");
  service_path.append(service);

  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* mock = (MockService*)
                      iota::Process::get_process().get_service("/mock");
  std::string cbroker_url("http://127.0.0.1:");
  cbroker_url.append(boost::lexical_cast<std::string>(port));
  cbroker_url.append("/mock/testPUSHCommand");
  std::string
  post_service("{\"services\": [{"
               "\"apikey\": \"apikey-test-2\",\"token\": \"token\","
               "\"cbroker\": \"" + cbroker_url +
               "\",\"entity_type\": \"thing\",\"resource\": \"/TestSample/cmdtest\"}]}");
  boost::shared_ptr<iota::ServiceCollection> col(new iota::ServiceCollection());
  pion::http::response http_r;
  std::string r;
  ((iota::AdminService*)
   iota::Process::get_process().get_service("/TestSample"))->delete_service_json(
     col, service, service_path, service, "apikey-test-2", "/TestSample/cmdtest", true,
     http_r, r, "1234", "4444");
  ((iota::AdminService*)
   iota::Process::get_process().get_service("/TestSample"))->post_service_json(col,
       service, service_path, post_service,
       http_r, r, "apikey-test-2", "5678");
  std::string device = get_device("unitTest_devtest_endpoint", plugin->get_protocol_data().protocol);
  add_device(device, service);

  // updateContext
  std::string querySTR = "";
  std::string bodySTR = "{\"updateAction\":\"UPDATE\",";
  bodySTR.append("\"contextElements\":[{\"id\":\"room_uttest\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR.append("\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"unitTest_devtest_endpoint@command|22\",");
  bodySTR.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR.append("]} ]}");
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/TestSample/ngsi/cmdtest/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, service);
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, service_path);
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    plugin->default_op_ngsi(http_request, url_args, query_parameters,
                            http_response, response);
    ASYNC_TIME_WAIT
    //respuesta al update de contextBroker
    std::cout << "@UT@RESPONSE" << http_response.get_status_code() << " " <<
              response << std::endl;
    IOTASSERT(response.find(RESPONSE_MESSAGE_NGSI_OK) !=
              std::string::npos);
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);

    std::string cb_last = mock->get_last("/mock/testPUSHCommand/NGSI10/updateContext");
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

    cb_last = mock->get_last("/mock/testPUSHCommand/NGSI10/updateContext");
    // pending
    std::cout << "@UT@Pending"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_uttest\",\"type\":\"thing\"") !=
              std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"pending\"")
      !=
      std::string::npos);
  }
  delete_device(device, service);
  r.clear();
  ((iota::AdminService*)
   iota::Process::get_process().get_service("/TestSample"))->delete_service_json(
     col, service, service_path, service, "apikey-test-2", "/TestSample/cmdtest", true,
     http_r, r, "1234", "4444");
  std::cout << "@UT@END testPUSHCommand " << std::endl;
}

void SampleTest::testPollingCommand() {
  std::cout << __LINE__ << "@UT@START testPollingCommand" << std::endl;
  std::string service(get_service_name(__FUNCTION__));
  std::string service_path("/");
  service_path.append(service);

  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* mock = (MockService*)
                      iota::Process::get_process().get_service("/mock");
  std::string cbroker_url("http://127.0.0.1:");
  cbroker_url.append(boost::lexical_cast<std::string>(port));
  cbroker_url.append("/mock/testPollingCommand");
  std::string
  post_service("{\"services\": [{"
               "\"apikey\": \"apikey-test-4\",\"token\": \"token\","
               "\"cbroker\": \"" + cbroker_url +
               "\",\"entity_type\": \"thing\",\"resource\": \"/TestSample/cmdtest\"}]}");
  boost::shared_ptr<iota::ServiceCollection> col(new iota::ServiceCollection());
  pion::http::response http_r;
  std::string r;
  ((iota::AdminService*)
   iota::Process::get_process().get_service("/TestSample"))->delete_service_json(
     col, service, service_path, service, "apikey-test-4", "/TestSample/cmdtest", true,
     http_r, r, "1234", "4444");
  ((iota::AdminService*)
   iota::Process::get_process().get_service("/TestSample"))->post_service_json(col,
       service, service_path, post_service,
       http_r, r, "apikey-test-4", "5678");

  iota::TestCommandService* plugin = (iota::TestCommandService*)
                                     iota::Process::get_process().get_service("/TestSample/cmdtest");

  std::string device = get_device("unitTest_devtest_noendpoint", plugin->get_protocol_data().protocol);
  add_device(device, service);

  // updateContext
  std::string querySTR = "";
  std::string bodySTR = "{\"updateAction\":\"UPDATE\",";
  bodySTR.append("\"contextElements\":[{\"id\":\"room_uttestno\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR.append("\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"unitTest_devtest_noendpoint@ping6|22\",");
  bodySTR.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR.append("]} ]}");
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/TestSample/ngsi/cmdtest/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, service);
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, service_path);
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    plugin->default_op_ngsi(http_request, url_args, query_parameters,
                            http_response, response);
    ASYNC_TIME_WAIT

    std::cout << "@UT@POST updateContext " << http_response.get_status_code() <<
              std::endl;
    IOTASSERT_MESSAGE("@UT@POST, response code no 200" ,
                      http_response.get_status_code() == 200);

    std::string cb_last = mock->get_last("/mock/testPollingCommand/NGSI10/updateContext");
    std::cout << "@UT@READY_FOR_READ" <<cb_last << std::endl;
    // ready_for_read
    IOTASSERT_MESSAGE("@UT@READY_FOR_READ, entity or entity_type in not correct"
                      ,
                      cb_last.find("\"id\":\"room_uttestno\",\"type\":\"thing\"") !=
                      std::string::npos);
    IOTASSERT_MESSAGE("@UT@READY_FOR_READ, name of command or value is not correct",
                      cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"pending\"")
                      !=
                      std::string::npos);

  }

  // GET desde el device
  querySTR = "i=unitTest_devtest_noendpoint&k=apikey-test-4";
  bodySTR = "";
  std::string command;
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/TestSample/cmdtest"));
    http_request->set_method("GET");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_devtest_noendpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey-test-4"));
    pion::http::response http_response;
    std::string response;
    plugin->service(http_request, url_args, query_parameters,
                    http_response, response);

    ASYNC_TIME_WAIT

    std::cout << "@UT@GET command " << http_response.get_status_code() <<
              ":" <<response << std::endl;
    command = response;
    IOTASSERT_MESSAGE("@UT@GET response code no 200" ,
                      http_response.get_status_code() == 200);

    std::string cb_last = mock->get_last("/mock/testPollingCommand/NGSI10/updateContext");
    std::cout << "@UT@DELIVERED" << cb_last << std::endl;
    // delivered
    IOTASSERT_MESSAGE("@UT@DELIVERED, entity or entity_type in not correct" ,
                      cb_last.find("\"id\":\"room_uttestno\",\"type\":\"thing\"") !=
                      std::string::npos);
    IOTASSERT_MESSAGE("@UT@DELIVERED, name of command or value is not correct"
                      ,
                      cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"delivered\"")
                      !=std::string::npos);

  }

  // POST resultado del comando
  std::size_t found = command.find("|@|");
  if (found!=std::string::npos) {
    command = command.substr(0, found);
  }

  querySTR = "i=unitTest_devtest_noendpoint&k=apikey-test-4";
  bodySTR = command;
  bodySTR.append("|command_result");
  std::cout << "@UT@POST command result " << bodySTR << std::endl;
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/TestSample/cmdtest"));
    http_request->set_method("PUT");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_devtest_noendpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey-test-4"));
    pion::http::response http_response;
    std::string response;
    plugin->service(http_request, url_args, query_parameters,
                    http_response, response);
    ASYNC_TIME_WAIT
    std::cout << "@UT@POST command result " << http_response.get_status_code() <<
              std::endl;
    IOTASSERT_MESSAGE("@UT@GET response code no 200" ,
                      http_response.get_status_code() == 200);

    std::string cb_last = mock->get_last("/mock/testPollingCommand/NGSI10/updateContext");
    std::cout << "@UT@INFO" << cb_last << std::endl;
    // info
    IOTASSERT_MESSAGE("@UT@INFO, entity or entity_type in not correct" ,
                      cb_last.find("\"id\":\"room_uttestno\",\"type\":\"thing\"") !=
                      std::string::npos);
    IOTASSERT_MESSAGE("@UT@INFO, name of command or value is not correct" ,
                      cb_last.find("|command_result")
                      !=
                      std::string::npos);

    std::cout << "@UT@OK" << cb_last << std::endl;
    // OK
    IOTASSERT_MESSAGE("@UT@OK, entity or entity_type in not correct" ,
                      cb_last.find("\"id\":\"room_uttestno\",\"type\":\"thing\"") !=
                      std::string::npos);
    IOTASSERT_MESSAGE("@UT@OK, name of command or value is not correct" ,
                      cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"OK\"")
                      !=
                      std::string::npos);

  }
  delete_device(device, service);
  r.clear();
  ((iota::AdminService*)
   iota::Process::get_process().get_service("/TestSample"))->delete_service_json(
     col, service, service_path, service, "apikey-test-4",
     "/TestSample/test", true,
     http_r, r, "1234", "4444");

  std::cout << "@UT@END testPollingCommand " << std::endl;
}

///SimplePlugin Test
void SampleTest::testRegisterIoTA() {
  std::cout << "START testRegisterIoTA" << std::endl;


  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock = (MockService*)
                         iota::Process::get_process().get_service("/mock");
  std::string mock_port = boost::lexical_cast<std::string>(port);
  std::cout << "@UT@create server" << std::endl;
  std::cout << "@UT@create pluging" << std::endl;

  std::string service ="srv1_ut";
  std::string service_path="/";
  std::string body("{\"services\": [{"
                   "\"apikey\": \"apikey\",\"token\": \"token\","
                   "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\",\"resource\": \"/TestSample/test\"}]}");
  pion::http::response http_response;
  std::string response;
  std::string token, apikey;
  std::string request_identifier;
  iota::AdminService& adminserv = *
                                  (iota::Process::get_process().get_admin_service());
  boost::shared_ptr<iota::ServiceCollection> table;
  adminserv.create_collection(table);
  adminserv.delete_service_json(table, service, service_path,
                                service, apikey, "/TestSample/test", false, http_response,
                                response, token, request_identifier);
  std::string manager("http://127.0.0.1:");
  iota::RestHandle* spserv = (iota::RestHandle*)
                             iota::Process::get_process().get_service("/TestSample/test");
  spserv->set_iota_manager_endpoint(manager + mock_port +
                                    "/mock/TestSample/protocols");
  adminserv.post_service_json(table,service, service_path, body, http_response,
                              response, token, request_identifier);

  std::cout << "@UT@manager_endpoint:" <<spserv->get_iota_manager_endpoint() <<
            std::endl;

  CPPUNIT_ASSERT_MESSAGE("Manager endpoint ",
                         spserv->get_iota_manager_endpoint().find("/protocols") !=
                         std::string::npos);

  ASYNC_TIME_WAIT

  std::string r_1 = cb_mock->get_last("/mock/TestSample/protocols");
  std::cout << "@UT@register:" <<r_1 << std::endl;
  CPPUNIT_ASSERT_MESSAGE("POST manager ", r_1.find(
                           "{ \"protocol\" : \"PDI-IoTA-test\", \"description\" : \"test Protocol\", \"iotagent\" : \"http://127.0.0.1/TestSample\"")
                         != std::string::npos);

  std::cout << "END testRegisterIoTA" << std::endl;
}
void SampleTest::testFilter() {
  std::cout << "START testFilter" << std::endl;
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock = (MockService*)
                         iota::Process::get_process().get_service("/mock");
  std::string mock_port = boost::lexical_cast<std::string>(port);

  boost::shared_ptr<iota::OAuthFilter> filter(new iota::OAuthFilter(
        iota::Process::get_process().get_io_service()));
  filter->set_filter_url_base("/TestSample/sp_auth");
  std::map<std::string, std::string> map;
  map[ iota::types::CONF_FILE_OAUTH_VALIDATE_TOKEN_URL] = "http://127.0.0.1:"
      +mock_port+"/mock/v3/auth/tokens";
  map[ iota::types::CONF_FILE_OAUTH_ROLES_URL] = "http://127.0.0.1:"+mock_port
      +"/mock/v3/role_assignments";
  map[ iota::types::CONF_FILE_OAUTH_PROJECTS_URL] = "http://127.0.0.1:"+mock_port
      +"/mock/v3/projects";
  map[ iota::types::CONF_FILE_ACCESS_CONTROL] = "http://127.0.0.1:"+mock_port;
  map[ iota::types::CONF_FILE_PEP_USER] = "pep";
  map[ iota::types::CONF_FILE_PEP_PASSWORD] = "pep";
  map[ iota::types::CONF_FILE_PEP_DOMAIN] = "admin_domain";
  map[ iota::types::CONF_FILE_OAUTH_TIMEOUT] = "3";
  filter->set_configuration(map);
  iota::RestHandle* spserv_auth = (iota::RestHandle*)
                                  iota::Process::get_process().get_service("/TestSample/sp_auth");
  spserv_auth->add_pre_filter(filter);
  spserv_auth->add_statistic_counter("traffic", true);

  boost::system::error_code error_code;

  std::string bodySTR = "BodyToTest";

  {
    pion::tcp::connection_ptr tcp_conn_1(new pion::tcp::connection(
                                           iota::Process::get_process().get_io_service()));

    std::cout << "No headers " << port << std::endl;
    // No headers fiware-service, fiware-servicepath....
    error_code = tcp_conn_1->connect(
                   boost::asio::ip::address::from_string("127.0.0.1"), port);
    pion::http::request http_request1("/TestSample/sp_auth/devices/device_test_filter");
    http_request1.set_method("POST");
    http_request1.set_content(bodySTR);
    http_request1.send(*tcp_conn_1, error_code);
    pion::http::response http_response1(http_request1);
    http_response1.receive(*tcp_conn_1, error_code);
    CPPUNIT_ASSERT_MESSAGE("No authorized 401",
                           http_response1.get_status_code() == 401);
  }

  {
    pion::tcp::connection_ptr tcp_conn_2(new pion::tcp::connection(
                                           iota::Process::get_process().get_io_service()));
    std::cout << "With headers" << std::endl;
    std::map<std::string, std::string> h;
    std::string
    content_validate_token("{\"token\": {\"issued_at\": \"2014-10-06T08:20:13.484880Z\",\"extras\": {},\"methods\": [\"password\"],\"expires_at\": \"2014-10-06T09:20:13.484827Z\",");
    content_validate_token.append("\"user\": {\"domain\": {\"id\": \"f7a5b8e303ec43e8a912fe26fa79dc02\",\"name\": \"SmartValencia\"},\"id\": \"5e817c5e0d624ee68dfb7a72d0d31ce4\",\"name\": \"alice\"}}}");
    h["X-Subject-Token"] = "x-subject-token";
    cb_mock->set_response("/mock/v3/auth/tokens", 200, "", h);
    cb_mock->set_response("/mock/v3/auth/tokens", 200, content_validate_token);
    std::string
    projects("{\"projects\": [{\"description\": \"SmartValencia Subservicio Electricidad\",\"links\": {\"self\": \"http://${KEYSTONE_HOST}/v3/projects/c6851f8ef57c4b91b567ab62ca3d0aef\"},\"enabled\": true,\"id\": \"c6851f8ef57c4b91b567ab62ca3d0aef\",\"domain_id\": \"f7a5b8e303ec43e8a912fe26fa79dc02\",\"name\": \"Electricidad\"}]}");
    cb_mock->set_response("/mock/v3/projects",200, projects, h);
    std::string
    user_roles("{\"role_assignments\": [{\"scope\": {\"project\": {\"id\": \"c6851f8ef57c4b91b567ab62ca3d0aef\"}},\"role\": {\"id\": \"a6407b6c597e4e1dad37a3420b6137dd\"},\"user\": {\"id\": \"5e817c5e0d624ee68dfb7a72d0d31ce4\"},\"links\": {\"assignment\": \"puthere\"}}],\"links\": {\"self\": \"http://${KEYSTONE_HOST}/v3/role_assignments\",\"previous\": null,\"next\": null}}");
    cb_mock->set_response("/mock/v3/role_assignments",200, user_roles);
    error_code = tcp_conn_2->connect(
                   boost::asio::ip::address::from_string("127.0.0.1"), port);
    pion::http::request http_request2("/TestSample/sp_auth/devices/device_test_filter");
    http_request2.add_header("Fiware-Service", "SmartValencia");
    http_request2.add_header("Fiware-ServicePath", "Electricidad");
    http_request2.add_header("X-Auth-Token", "a");
    http_request2.set_method("POST");
    http_request2.set_content(bodySTR);
    http_request2.send(*tcp_conn_2, error_code);
    pion::http::response http_response2(http_request2);
    http_response2.receive(*tcp_conn_2, error_code);
    std::cout << http_response2.get_status_code() << std::endl;

    // Forbidden
    CPPUNIT_ASSERT_MESSAGE("Filter progress 403",
                           http_response2.get_status_code() == 403);
  }

  std::cout << "END testFilter " << std::endl;
}

void SampleTest::testGetDevice() {
  std::cout << "START testGetDevice" << std::endl;
  iota::RestHandle* spserv_auth = (iota::RestHandle*)
                                  iota::Process::get_process().get_service("/TestSample/test");
  std::cout << "get_device 8934075379000039321 serviceTT  /subservice " <<
            std::endl;
  boost::shared_ptr<iota::Device> dev =
    spserv_auth->get_device("8934075379000039321", "serviceTT", "/subservice");
  if (dev.get() == NULL) {
    std::cout << "Not found device 8934075379000039321" << std::endl;
    CPPUNIT_ASSERT_MESSAGE("Not found device 8934075379000039321", true);
  }
  else {
    std::cout << " entity: " << dev->_entity_name  << std::endl;
    // CPPUNIT_ASSERT_MESSAGE("device has not correct entity_name", dev->_entity_name.compare("room1") == 0);
  }

  std::cout << "get_device 8934075379000039321  /subservice" << std::endl;
  dev = spserv_auth->get_device("8934075379000039321", "", "/subservice");
  if (dev.get() == NULL) {
    std::cout << "Not found device 8934075379000039321" << std::endl;
    CPPUNIT_ASSERT_MESSAGE("Not found device 8934075379000039321", true);
  }
  else {
    std::cout << " entity: " << dev->_entity_name  << std::endl;
    // CPPUNIT_ASSERT_MESSAGE("device has not correct entity_name", dev->_entity_name.compare("room1") == 0);
  }

  std::cout << "get_device 8934075379000039321" << std::endl;
  dev = spserv_auth->get_device("8934075379000039321");
  if (dev.get() == NULL) {
    std::cout << "Not found device 8934075379000039321" << std::endl;
    CPPUNIT_ASSERT_MESSAGE("Not found device 8934075379000039321", true);
  }
  else {
    std::cout << " entity: " << dev->_entity_name  << std::endl;
    // CPPUNIT_ASSERT_MESSAGE("device has not correct entity_name", dev->_entity_name.compare("room1") == 0);
  }

  std::cout << "END testGetDevice" << std::endl;
}
