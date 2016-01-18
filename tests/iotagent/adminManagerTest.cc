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
#include "adminManagerTest.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <rest/process.h>
#include "../mocks/http_mock.h"
#include <cppunit/extensions/HelperMacros.h>
#include "util/device_collection.h"
#include <cmath>
#include <ctime>
#include "services/admin_mgmt_service.h"
#include "util/iota_exception.h"
#include "util/FuncUtil.h"
#include <sys/time.h>

#define PATH_CONFIG_MONGO "../../tests/iotagent/config_mongo.json"

#define IOTASSERT_MESSAGE(x, y)                          \
  std::cout << "@" << __LINE__ << "@" << x << std::endl; \
  CPPUNIT_ASSERT_MESSAGE(x, y)

#define IOTASSERT(y)                                \
  std::cout << "@" << __LINE__ << "@" << std::endl; \
  CPPUNIT_ASSERT(y)

//#define  PATH_CONFIG "../../tests/iotagent/config_mongo.json"

using ::testing::Return;
using ::testing::NotNull;
using ::testing::StrEq;
using ::testing::_;
using ::testing::Invoke;

CPPUNIT_TEST_SUITE_REGISTRATION(AdminManagerTest);

const std::string AdminManagerTest::HOST("127.0.0.1");
const std::string AdminManagerTest::CONTENT_JSON("application/json");

const int AdminManagerTest::POST_RESPONSE_CODE = 201;
// GET ALL empty
const int AdminManagerTest::GET_RESPONSE_CODE = 200;
const int AdminManagerTest::GET_RESPONSE_CODE_NOT_FOUND = 404;
const int AdminManagerTest::DELETE_RESPONSE_CODE = 204;

////////////////////
////  PROTOCOLS
const std::string AdminManagerTest::URI_PROTOCOLS("/TestManager/protocols");

const std::string AdminManagerTest::POST_PROTOCOLS_NO_AGENT(
    "{\"iotagent\": \"http://127.0.0.1:1000/iot\","
    "\"resource\": \"/iot/d\","
    "\"protocol\": \"UL20\","
    "\"description\": \"Ultralight 2.0\""
    "}");
const std::string AdminManagerTest::POST_PROTOCOLS2(
    "{\"iotagent\": \"host2\","
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
const std::string AdminManagerTest::POST_PROTOCOLS3(
    "{\"iotagent\": \"host2\","
    "\"resource\": \"/iot/d\","
    "\"protocol\": \"UL20\","
    "\"description\": \"Ultralight 2.0\","
    "\"services\": [{"
    "\"apikey\": \"apikey3\","
    "\"service\": \"service2\","
    "\"service_path\": \"/ssrv2\","
    "\"token\": \"token2\","
    "\"resource\": \"/iot/d\","
    "\"entity_type\": \"thing\""
    "}]}");
const std::string AdminManagerTest::POST_PROTOCOLS2_RERE(
    "{\"iotagent\": \"host2\","
    "\"resource\": \"/iot/mqtt\","
    "\"protocol\": \"MQTT\","
    "\"description\": \"mqtt example\","
    "\"services\": [{"
    "\"apikey\": \"apikey3\","
    "\"service\": \"service2\","
    "\"service_path\": \"/ssrv2\","
    "\"token\": \"token2rere\","
    "\"cbroker\": \"\","
    "\"resource\": \"/iot/mqtt\","
    "\"entity_type\": \"thingrere\""
    "},{"
    "\"apikey\": \"apikey3\","
    "\"service\": \"service2\","
    "\"service_path\": \"/ssrv2re\","
    "\"token\": \"token2\","
    "\"cbroker\": \"http://127.0.0.1:1026\","
    "\"resource\": \"/iot/mqtt\","
    "\"entity_type\": \"thing\""
    "}]}");

const std::string AdminManagerTest::POST_PROTOCOLS2_RERERE(
    "{\"iotagent\": \"host2\","
    "\"resource\": \"/iot/mqtt\","
    "\"protocol\": \"MQTT\","
    "\"description\": \"mqtt example\","
    "\"services\": [{"
    "\"apikey\": \"apikey3\","
    "\"service\": \"service2\","
    "\"service_path\": \"/ssrv2\","
    "\"token\": \"token2rere\","
    "\"cbroker\": \"http://127.0.0.1:1026\","
    "\"resource\": \"/iot/mqtt\","
    "\"entity_type\": \"thingrere\""
    "}]}");

const std::string AdminManagerTest::POST_PROTOCOLS2_RERERE_EMPTY(
    "{\"iotagent\": \"host2\","
    "\"resource\": \"/iot/mqtt\","
    "\"protocol\": \"MQTT\","
    "\"description\": \"mqtt example\","
    "\"services\": []}");

const std::string AdminManagerTest::POST_PROTOCOLS4("TODO");
const std::string AdminManagerTest::GET_PROTOCOLS_RESPONSE(
    "{ \"count\": 0,\"devices\": []}");

////////////////////
////  SERVICE_MANAGEMENT
const std::string AdminManagerTest::URI_SERVICES_MANAGEMET(
    "/TestManager/services");
// POST
const std::string AdminManagerTest::POST_SERVICE_MANAGEMENT1(
    "{\"services\": [{"
    "\"protocol\": [\"UL20\", \"MQTT\"],"
    "\"apikey\": \"apikey\",\"token\": \"token\","
    "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\""
    "}]}");
const std::string AdminManagerTest::POST_SERVICE_MANAGEMENT2(
    "{\"services\": [{"
    "\"protocol\": [\"UL20\", \"MQTT\"],"
    "\"apikey\": \"apikey\",\"token\": \"token\","
    "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\""
    "}]}");

const std::string AdminManagerTest::POST_BAD_SERVICE_MANAGEMENT1(
    "{\"services\": [{"
    "\"protocol\": [\"no_exists\", \"MQTT\"],"
    "\"apikey\": \"apikey\",\"token\": \"token\","
    "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\""
    "}]}");

const std::string AdminManagerTest::POST_SERVICE_MANAGEMENT3(
    "{\"services\": [{"
    "\"protocol\": [\"UL20\"],"
    "\"apikey\": \"apikey\",\"token\": \"token\","
    "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\""
    "}]}");

const std::string AdminManagerTest::GET_SERVICE_MANAGEMENT_RESPONSE(
    "{ \"count\": 0,\"devices\": []}");

////////////////////
////  DEVICE _MANAGEMENT
const std::string AdminManagerTest::URI_DEVICES_MANAGEMEMT(
    "/TestManager/devices");
// POST
const std::string AdminManagerTest::POST_DEVICE_MANAGEMENT1(
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

const std::string AdminManagerTest::PUT_DEVICE(
    "{\"protocol\": \"PDI-IoTA-UltraLight\",\"entity_name\": "
    "\"entity_name_mod\"}");

const std::string AdminManagerTest::PUT_DEVICE2(
    "{\"protocol\": \"PDI-IoTA-UltraLight\",\"entity_name\": "
    "\"entity_name_org\"}");
//
const std::string AdminManagerTest::POST_DEVICE(
    "{\"devices\": "
    "[{\"device_id\": \"device_id_new\",\"protocol\": "
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

const std::string AdminManagerTest::POST_DEVICE_NO_PROTOCOL(
    "{\"devices\": "
    "[{\"device_id\": \"device_id_new\",\"entity_name\": "
    "\"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": "
    "\"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
    "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
    "\"device_id@ping|%s\" }],"
    "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
    "\"temperature\",\"type\": \"int\" }],"
    "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
    "\"value\": \"50\"  }]"
    "}]}");

const std::string AdminManagerTest::GET_DEVICE_MANAGEMENT_RESPONSE(
    "{ \"count\": 0,\"devices\": []}");

std::string s1_d_host2(
    "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
    "10.95.213.36:1026\","
    "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"host2\","
    "\"protocol\":\"UL20\",\"service\": "
    "\"testmanagerservice\",\"service_path\":\"/testmanagersubservice\"}");

std::string s1_d2(
    "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
    "10.95.213.36:1026\","
    "\"entity_type\":\"thing\",\"resource\":\"/iot/d2\",\"iotagent\":\"host1\","
    "\"protocol\":\"UL20\",\"service\": "
    "\"testmanagerservice\",\"service_path\":\"/testmanagersubservice\"}");

std::string s1_mqtt(
    "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
    "10.95.213.36:1026\","
    "\"entity_type\":\"thing\",\"resource\":\"/iot/"
    "mqtt\",\"iotagent\":\"host1\","
    "\"protocol\":\"MQTT\",\"service\": "
    "\"testmanagerservice\",\"service_path\":\"/testmanagersubservice\"}");

std::string s2_d(
    "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
    "10.95.213.36:1026\","
    "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"host1\","
    "\"protocol\":\"UL20\",\"service\": "
    "\"testmanagerservice\",\"service_path\":\"/ss2\"}");

std::string s3_mqtt(
    "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
    "10.95.213.36:1026\","
    "\"entity_type\":\"thing\",\"resource\":\"/iot/"
    "mqtt\",\"iotagent\":\"host1\","
    "\"protocol\":\"MQTT\",\"service\": \"s3\",\"service_path\":\"/ss3\"}");

std::string s4_agus(
    "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
    "10.95.213.36:1026\","
    "\"entity_type\":\"thing\",\"resource\":\"/iot/fake\",\"iotagent\":\"http:/"
    "/127.0.0.1:7070/mock/iot1\","
    "\"protocol\":\"UL20\",\"service\": "
    "\"s4_agus\",\"service_path\":\"/ss3\"}");

std::string s5_agus(
    "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
    "10.95.213.36:1026\","
    "\"entity_type\":\"thing\",\"resource\":\"/iot/fake\",\"iotagent\":\"http:/"
    "/127.0.0.1:7070/mock/iot2\","
    "\"protocol\":\"UL20\",\"service\": "
    "\"s4_agus\",\"service_path\":\"/ss3\"}");
std::string s6_agus(
    "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
    "10.95.213.36:1026\","
    "\"entity_type\":\"thing\",\"resource\":\"/iot/fake\",\"iotagent\":\"http:/"
    "/127.0.0.1:7070/mock/iot3\","
    "\"protocol\":\"UL20\",\"service\": "
    "\"s4_agus\",\"service_path\":\"/ss3\"}");
std::string s7_agus_error(
    "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
    "10.95.213.36:1026\","
    "\"entity_type\":\"thing\",\"resource\":\"/iot/fake\",\"iotagent\":\"http:/"
    "/127.0.0.1:1000/iot\","
    "\"protocol\":\"UL20\",\"service\": "
    "\"s4_agus\",\"service_path\":\"/ss3\"}");

std::string devices(
    "{\"devices\":[{\"protocol\":\"PDI-IoTA-UltraLight\",\"device_id\": "
    "\"device_id_post\",\"entity_name\": \"entity_name\",\"entity_type\": "
    "\"entity_type\",\"endpoint\": \"http://device_endpoint\",\"timezone\": "
    "\"America/Santiago\","
    "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
    "\"device_id@ping|%s\" }],"
    "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
    "\"temperature\",\"type\": \"int\" }],"
    "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
    "\"value\": \"50\"  }]"
    "}]}");

AdminManagerTest::AdminManagerTest() {
  cleanDB();
  unsigned int port = iota::Process::get_process().get_http_port();
  std::string service("testmanagerservice");
  std::string service_path("/testmanagersubservice");
  pion::http::response http_response_;
  std::string response_;
  std::map<std::string, std::string> headers;
  std::string service_s1(
      "{\"services\": [{"
      "\"apikey\": \"apikey\",\"token\": \"token\","
      "\"cbroker\": \"http://cbroker\",\"entity_type\": "
      "\"thing\",\"resource\": \"/iot/d\"}]}");
  int code_res =
      http_test("/iotagent/services", "POST", service, service_path,
                "application/json", service_s1, headers, "", response_);
  /*

  boost::shared_ptr<iota::ServiceCollection> col(new iota::ServiceCollection());
  iota::Process::get_process().get_admin_service()->post_service_json(col,
  service, service_path, service_s1, http_response,
                         response, "1234", "5678");
  */
}

void AdminManagerTest::cleanDB() {
  iota::ServiceCollection table_service;
  table_service.createTableAndIndex();
  mongo::BSONObj all;
  table_service.remove(all);
}

AdminManagerTest::~AdminManagerTest() {}

void AdminManagerTest::setUp() {}

void AdminManagerTest::tearDown() {
  // delete cbPublish; //Already deleted inside MqttService.
}

void AdminManagerTest::testDeviceToBeAdded() {
  iota::DeviceToBeAdded test("protocol", "device_json");
  iota::DeviceToBeAdded test2 = test;

  CPPUNIT_ASSERT(test == test2);
}

void AdminManagerTest::testGetEndpointsFromDevices() {
  iota::AdminManagerService manager_service;

  std::cout << "Test: testGetEndpointsFromDevices... starting" << std::endl;

  iota::ServiceMgmtCollection table1;
  table1.createTableAndIndex();

  mongo::BSONObj all;

  table1.remove(all);

  table1.insert(mongo::fromjson(s1_d_host2));
  table1.insert(mongo::fromjson(s1_d2));
  table1.insert(mongo::fromjson(s1_mqtt));
  table1.insert(mongo::fromjson(s2_d));
  table1.insert(mongo::fromjson(s3_mqtt));

  std::vector<iota::DeviceToBeAdded> v_endpoints_devices;
  std::string devices(
      "{\"devices\": "
      "[{\"protocol\":\"UL20\",\"device_id\": \"device_id\",\"entity_name\": "
      "\"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": "
      "\"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
      "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
      "\"device_id@ping|%s\" }],"
      "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
      "\"temperature\",\"type\": \"int\" }],"
      "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
      "\"value\": \"50\"  }]"
      "},{\"protocol\":\"UL20\",\"device_id\": "
      "\"device_id_2\",\"entity_name\": \"entity_name\",\"entity_type\": "
      "\"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": "
      "\"America/Santiago\","
      "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
      "\"device_id@ping|%s\" }],"
      "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
      "\"temperature\",\"type\": \"int\" }],"
      "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
      "\"value\": \"50\"  }]"
      "}]}");

  manager_service.resolve_endpoints(v_endpoints_devices, devices,
                                    "testmanagerservice",
                                    "/testmanagersubservice");

  std::cout << "Test: testGetEndpointsFromDevices: result: ["
            << v_endpoints_devices.size() << "] endpoints" << std::endl;
  CPPUNIT_ASSERT(v_endpoints_devices.size() == 4);
  for (int i = 0; i < v_endpoints_devices.size(); i++) {
    std::cout << "Test: host: " << v_endpoints_devices[i].get_endpoint()
              << std::endl;
  }

  // Test bad json
  std::string bad_devices("{\"devices\": [");
  CPPUNIT_ASSERT_THROW_MESSAGE(
      "Resolve endpoints with bad json ",
      manager_service.resolve_endpoints(v_endpoints_devices, bad_devices,
                                        "testmanagerservice",
                                        "/testmanagersubservice"),
      iota::IotaException);

  std::cout << "Test: END" << std::endl;
}

void AdminManagerTest::testAddDevicesToEndpoints() {
  MockService* http_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  unsigned int port = iota::Process::get_process().get_http_port();
  std::string response;

  iota::AdminManagerService manager_service;
  manager_service.set_timeout(5);
  std::cout << "testAddDevicesToEndpoints: STARTING... " << std::endl;
  std::map<std::string, std::string> h;
  http_mock->set_response("testAddDevicesToEndpoints", 200, "{}", h);

  std::string device(
      "{\"protocol\":\"UL20\",\"device_id\": \"device_id\",\"entity_name\": "
      "\"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": "
      "\"http://device_endpoint\",\"timezone\": \"America/Santiago\","
      "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
      "\"device_id@ping|%s\" }],"
      "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
      "\"temperature\",\"type\": \"int\" }],"
      "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
      "\"value\": \"50\"  }]"
      "}");

  std::string endpoint("http://127.0.0.1:" +
                       boost::lexical_cast<std::string>(port) +
                       "/mock/testAddDevicesToEndpoints");
  // TEST:

  std::string temp;
  std::cout << "Endpoint: " << endpoint << std::endl;
  int res = manager_service.operation_device_iotagent(
      endpoint, device, "testmanagerservice", "/testmanagersubservice", "test",
      "POST", temp);
  CPPUNIT_ASSERT(res == 200);

  sleep(4);
  // http_mock->stop();
}

void AdminManagerTest::testGetDevices() {
  std::cout << "@UT@START testGetDevices" << std::endl;
  iota::ServiceMgmtCollection table1;
  table1.createTableAndIndex();

  table1.remove(BSON("service"
                     << "s4_agus"));
  table1.insert(mongo::fromjson(s4_agus));
  table1.insert(mongo::fromjson(s5_agus));
  table1.insert(mongo::fromjson(s6_agus));
  table1.insert(mongo::fromjson(s7_agus_error));

  MockService* http_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  std::string mock_response(
      "{\"count\": 2,\"devices\": "
      "[{\"protocol\":\"UL20\",\"device_id\": \"device_id\",\"entity_name\": "
      "\"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": "
      "\"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
      "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
      "\"device_id@ping|%s\" }],"
      "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
      "\"temperature\",\"type\": \"int\" }],"
      "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
      "\"value\": \"50\"  }]"
      "},{\"protocol\":\"UL20\",\"device_id\": "
      "\"device_id_2\",\"entity_name\": \"entity_name\",\"entity_type\": "
      "\"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": "
      "\"America/Santiago\","
      "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
      "\"device_id@ping|%s\" }],"
      "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
      "\"temperature\",\"type\": \"int\" }],"
      "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
      "\"value\": \"50\"  }]"
      "}]}");
  std::string mock_response_one_device(
      "{\"protocol\":\"UL20\",\"device_id\": \"device_id\",\"entity_name\": "
      "\"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": "
      "\"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
      "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
      "\"device_id@ping|%s\" }],"
      "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
      "\"temperature\",\"type\": \"int\" }],"
      "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
      "\"value\": \"50\"  }]"
      "}");
  std::string mock_response_iot1(
      "{\"count\": 4,\"devices\": "
      "[{\"protocol\":\"UL20\",\"device_id\": \"device_id\",\"entity_name\": "
      "\"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": "
      "\"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
      "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
      "\"device_id@ping|%s\" }],"
      "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
      "\"temperature\",\"type\": \"int\" }],"
      "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
      "\"value\": \"50\"  }]"
      "},{\"protocol\":\"UL20\",\"device_id\": "
      "\"device_id_2\",\"entity_name\": \"entity_name\",\"entity_type\": "
      "\"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": "
      "\"America/Santiago\","
      "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
      "\"device_id@ping|%s\" }],"
      "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
      "\"temperature\",\"type\": \"int\" }],"
      "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
      "\"value\": \"50\"  }]"
      "}]}");
  std::string mock_response_iot2("{\"count\": 4,\"devices\": []}");

  std::map<std::string, std::string> h;
  // Two endpoints. Repeat response for test
  http_mock->set_response("/mock/iot1/devices", 200, mock_response, h);
  http_mock->set_response("/mock/iot2/devices", 200, mock_response, h);

  iota::AdminManagerService* manager_service =
      (iota::AdminManagerService*)iota::Process::get_process()
          .get_admin_service();
  pion::http::request_ptr http_request(new pion::http::request("/"));
  http_request->add_header("Fiware-Service", "s4_agus");
  http_request->add_header("Fiware-ServicePath", "/ss3");
  // http_request->add_header("X-Trace-Message", "12345");
  http_request->set_method("GET");

  pion::http::response http_response;
  std::string response;
  //  manager_service.get_devices(http_request, args, query, "s4_agus", "/ss3",
  //  0, 0,
  //                              "on", "", http_response, response);

  manager_service->get_all_devices_json("s4_agus", "/ss3", 0, 0, "on", "",

                                        http_response, response, "12345",
                                        "token", "UL20");

  std::cout << "@UT@get_all_devices" << response << std::endl;
  CPPUNIT_ASSERT_MESSAGE("Expected 4 devices ",
                         response.find("\"count\" : 4") != std::string::npos);

  response.clear();
  http_mock->set_response("/mock/iot1/devices/device_id", 200,
                          mock_response_one_device, h);
  manager_service->get_a_device_json("s4_agus", "/ss3", "device_id",
                                     http_response, response, "12345", "token",
                                     "UL20");
  std::cout << "@UT@get_a_device_json" << response << std::endl;
  CPPUNIT_ASSERT_MESSAGE(
      "Expected device_id ",
      response.find("\"device_id\" : \"device_id\"") != std::string::npos);
  CPPUNIT_ASSERT_MESSAGE("Expected  count ",
                         response.find("\"count\" : 1") != std::string::npos);
  std::cout << "@UT@get_all_devices  offset 2  limit 4" << std::endl;

  http_mock->set_response("/mock/iot1/devices", 200, mock_response_iot1, h);
  http_mock->set_response("/mock/iot2/devices", 200, mock_response_iot1, h);
  http_mock->set_response("/mock/iot3/devices", 200, mock_response_iot2, h);
  response.clear();
  int code = manager_service->get_all_devices_json(
      "s4_agus", "/ss3", 4, 2, "on", "", http_response, response, "12345",
      "token", "UL20");
  CPPUNIT_ASSERT_MESSAGE(
      "Expected device_id ",
      response.find("\"device_id\" : \"device_id\"") != std::string::npos);

  std::cout << "@UT@get_all_devices" << response << std::endl;
  CPPUNIT_ASSERT_MESSAGE("Expected  count ",
                         response.find("\"count\" : 12") != std::string::npos);
  mongo::BSONObj resObj = mongo::fromjson(response);
  std::vector<mongo::BSONElement> devicesBSON =
      (resObj.getField("devices")).Array();
  CPPUNIT_ASSERT_MESSAGE("200 ", code);
  CPPUNIT_ASSERT_MESSAGE("Expected  count ", 4 == devicesBSON.size());
  CPPUNIT_ASSERT_MESSAGE(
      "Checking error ",
      response.find("Connection refused") != std::string::npos);
  std::cout << "STOP testGetDevices" << std::endl;
  // sleep(2);

  table1.remove(BSON("service"
                     << "s4_agus"));

  std::cout << "@UT@END testGetDevices" << std::endl;
}

void AdminManagerTest::testMultiplePostsWithResponse() {
  std::cout << "@UT@START testMultiplePostsWithResponse" << std::endl;

  iota::AdminManagerService* manager_service =
      (iota::AdminManagerService*)iota::Process::get_process()
          .get_admin_service();
  MockService* http_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  unsigned int port = iota::Process::get_process().get_http_port();
  std::string response;
  std::map<std::string, std::string> h;
  // Two endpoints. Repeat response for test
  http_mock->set_response("/mock/testMultiplePostsWithResponse/devices", 201,
                          "", h);
  http_mock->set_response("/mock/testMultiplePostsWithResponse/devices", 201,
                          "", h);

  std::string device_1(
      "{\"protocol\":\"UL20\",\"device_id\": \"device_1\",\"entity_name\": "
      "\"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": "
      "\"http://device_endpoint\",\"timezone\": \"America/Santiago\","
      "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
      "\"device_id@ping|%s\" }],"
      "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
      "\"temperature\",\"type\": \"int\" }],"
      "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
      "\"value\": \"50\"  }]"
      "}");

  std::string device_2(
      "{\"protocol\":\"UL20\",\"device_id\": \"device_2\",\"entity_name\": "
      "\"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": "
      "\"http://device_endpoint\",\"timezone\": \"America/Santiago\","
      "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
      "\"device_id@ping|%s\" }],"
      "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
      "\"temperature\",\"type\": \"int\" }],"
      "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
      "\"value\": \"50\"  }]"
      "}");

  std::string endpoint("http://127.0.0.1:" +
                       boost::lexical_cast<std::string>(port) +
                       "/mock/testMultiplePostsWithResponse");

  std::vector<iota::DeviceToBeAdded> v_devices;

  v_devices.push_back(iota::DeviceToBeAdded(device_1, endpoint));
  v_devices.push_back(iota::DeviceToBeAdded(device_2, endpoint));
  v_devices.push_back(
      iota::DeviceToBeAdded(device_2, "http://127.0.0.1:1000/iot"));
  std::string temp;
  int res = manager_service->post_multiple_devices(
      v_devices, "testmanagerservice", "/testmanagersubservice", "", temp);

  std::cout << "Test: testMultiplePostsWithResponse: result: " << res
            << std::endl;

  CPPUNIT_ASSERT(res == 200);
  CPPUNIT_ASSERT_MESSAGE("Checking error ",
                         temp.find("Connection refused") != std::string::npos);
  // sleep(2);
}

void AdminManagerTest::testPostJSONDevices() {
  iota::AdminManagerService* manager_service =
      (iota::AdminManagerService*)iota::Process::get_process()
          .get_admin_service();
  MockService* http_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  unsigned int port = iota::Process::get_process().get_http_port();
  std::string s1_d(
      "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
      "10.95.213.36:1026\","
      "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"http://"
      "127.0.0.1:" +
      boost::lexical_cast<std::string>(port) +
      "/iotagent\","
      "\"protocol\":\"PDI-IoTA-UltraLight\",\"service\": "
      "\"testmanagerservice\",\"service_path\":\"/testmanagersubservice\"}");
  std::string s1_d_error(
      "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
      "10.95.213.36:1026\","
      "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"http://"
      "127.0.0.1:1000/mock/iotagent\","
      "\"protocol\":\"PDI-IoTA-UltraLight\",\"service\": "
      "\"testmanagerservice\",\"service_path\":\"/testmanagersubservice\"}");

  iota::ServiceMgmtCollection table1;
  iota::DeviceCollection table_device;

  table1.createTableAndIndex();
  table_device.createTableAndIndex();
  mongo::BSONObj all;

  table1.remove(all);

  iota::Device borrar("", "");

  table_device.removed(borrar);

  table1.insert(mongo::fromjson(s1_d));
  table1.insert(mongo::fromjson(s1_d_error));
  // only one endpoint will be added.

  std::string response;
  pion::http::response http_response;
  std::map<std::string, std::string> h;
  http_mock->set_response("/mock/iotagent/devices", 201, "", h);
  std::cout << "Test testPostJSONDevices STARTING" << std::endl;
  manager_service->post_device_json("testmanagerservice",
                                    "/testmanagersubservice", devices,
                                    http_response, response, "");

  std::cout << "Result " << response << std::endl;
  std::cout << iota::http2string(http_response) << std::endl;

  CPPUNIT_ASSERT(http_response.get_status_code() == 200);
  CPPUNIT_ASSERT_MESSAGE(
      "Checking error ",
      response.find("Connection refused") != std::string::npos);

  // Now checking if the device has been added to the collection.
  iota::Device q1("device_id_post", "testmanagerservice");
  q1._service_path.assign("/testmanagersubservice");
  int code = table_device.findd(q1);
  std::cout << "DEVICE FOUND?: " << (code < 0 ? "NO" : "YES") << std::endl;
  CPPUNIT_ASSERT(code >= 0);

  std::cout << "Test delete STARTING" << std::endl;
  manager_service->delete_device_json(
      "testmanagerservice", "/testmanagersubservice", "device_id_post",
      http_response, response, "", "PDI-IoTA-UltraLight");

  std::cout << "Test testPostJSONDevices DONE" << std::endl;
}

void AdminManagerTest::testProtocol() {
  std::cout << "START @UT@START testProtocol" << std::endl;
  std::map<std::string, std::string> headers;
  std::string query_string;
  int code_res;
  std::string response, cb_last;
  std::string service = "service2";
  std::map<std::string, std::string> h;

  MockService* http_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  unsigned int port = iota::Process::get_process().get_http_port();

  std::string POST_PROTOCOLS1(
      "{\"iotagent\": \"http://127.0.0.1:7070/mock/testProtocol\","
      "\"resource\": \"/iot/d\","
      "\"protocol\": \"UL20\","
      "\"description\": \"Ultralight 2.0\""
      "}");

  std::cout << "@UT@Remove all data in protocols" << std::endl;
  pion::http::response http_response;
  ((iota::AdminManagerService*)(iota::Process::get_process()
                                    .get_admin_service()))
      ->delete_all_protocol_json(http_response, "", response);

  http_mock->set_response("/mock/testProtocol/services", 201, "", h);
  std::cout << "@UT@Post iotagents without services" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "POST", "ss", "", "application/json",
                       POST_PROTOCOLS1, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  http_mock->set_response("/mock/testProtocol/services", 201, "", h);
  std::cout << "@UT@POST One endpoint" << std::endl;
  code_res =
      http_test(URI_SERVICES_MANAGEMET, "POST", service, "", "application/json",
                POST_SERVICE_MANAGEMENT3, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  CPPUNIT_ASSERT_MESSAGE("Waiting 201", code_res == 201);
  std::cout << "@UT@DELETE" << std::endl;
  http_mock->set_response("/mock/testProtocol/services", 204, "{}", h);
  code_res =
      http_test(URI_SERVICES_MANAGEMET, "DELETE", service, "",
                "application/json", "", headers, "protocol=UL20", response);
  std::cout << "@UT@RESPONSE: XXXXXXXXX" << code_res << " " << response
            << std::endl;
  std::cout << "@UT@Post agent unreacheable" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "POST", "ss", "", "application/json",
                       POST_PROTOCOLS_NO_AGENT, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@GET " << std::endl;
  code_res = http_test(URI_PROTOCOLS, "GET", "ss", "", "application/json", "",
                       headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == GET_RESPONSE_CODE);
  IOTASSERT_MESSAGE(
      "only return one",
      response.find("\"count\": 1,\"protocols\"") != std::string::npos);
  code_res = http_test(URI_SERVICES_MANAGEMET, "GET", service, "",
                       "application/json", "", headers, query_string, response);
  IOTASSERT(code_res == GET_RESPONSE_CODE);
  IOTASSERT_MESSAGE(
      "only return one",
      response.find("\"count\": 0,\"services\"") != std::string::npos);

  std::cout << "@UT@Post iotagents with a service" << std::endl;
  std::cout << "@UT@POST" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "POST", "ss", "", "application/json",
                       POST_PROTOCOLS2, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);
  std::cout << "@UT@Post iotagents with a service" << std::endl;
  std::cout << "@UT@POST" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "POST", "ss", "", "application/json",
                       POST_PROTOCOLS3, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  std::cout << "@UT@POST reregister POST_PROTOCOLS2_RERE" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "GET", "ss", "", "application/json", "",
                       headers, "", response);
  std::cout << "@UT@count " << response << std::endl;
  std::cout << "@UT@POST change host but no identifier" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "POST", "ss", "", "application/json",
                       POST_PROTOCOLS2_RERE, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);
  code_res = http_test(URI_PROTOCOLS, "GET", "ss", "", "application/json", "",
                       headers, "", response);
  std::cout << "@UT@count " << response << std::endl;

  std::cout << "@UT@GET" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "GET", "ss", "", "application/json", "",
                       headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == GET_RESPONSE_CODE);
  IOTASSERT_MESSAGE(
      "only return one",
      response.find("\"count\": 2,\"protocols\"") != std::string::npos);

  std::cout << "@UT@GET services" << std::endl;
  code_res = http_test(URI_SERVICES_MANAGEMET, "GET", "service2", "/*",
                       "application/json", "", headers, query_string, response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == GET_RESPONSE_CODE);
  IOTASSERT_MESSAGE(
      "no modified data count ",
      response.find("\"count\": 3,\"services\"") != std::string::npos);
  IOTASSERT_MESSAGE("no modified data new service",
                    response.find("/ssrv2re") != std::string::npos);
  IOTASSERT_MESSAGE("no modified data reregister",
                    response.find("token2rere") != std::string::npos);
  IOTASSERT_MESSAGE("cbroker modified",
                    response.find("\"cbroker\" : \"\"") != std::string::npos);

  std::cout << "END@UT@ testProtocol" << std::endl;
}

void AdminManagerTest::testErrorsManager() {
  std::cout << "START @UT@START testErrorsManager" << std::endl;
  std::map<std::string, std::string> headers;
  std::string query_string;
  int code_res;
  std::string response, cb_last;
  std::string service = "service2";
  std::map<std::string, std::string> h;

  MockService* http_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  unsigned int port = iota::Process::get_process().get_http_port();
  http_mock->set_response("/mock/testErrorsManager/services", 201, "", h);
  std::cout << "START @UT@START testErrorsManager" << std::endl;

  std::cout << "@UT@POST" << std::endl;
  code_res =
      http_test(URI_SERVICES_MANAGEMET, "POST", service, "", "application/json",
                POST_SERVICE_MANAGEMENT1, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  CPPUNIT_ASSERT_MESSAGE("Checking response code no iotagent ",
                         code_res == 500);
  code_res =
      http_test(URI_SERVICES_MANAGEMET, "PUT", service, "", "application/json",
                POST_SERVICE_MANAGEMENT1, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  CPPUNIT_ASSERT_MESSAGE("Checking response code no iotagent ",
                         code_res == 404);
  code_res = http_test(URI_SERVICES_MANAGEMET, "DELETE", service, "",
                       "application/json", POST_SERVICE_MANAGEMENT1, headers,
                       "protocol=UL20", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  CPPUNIT_ASSERT_MESSAGE("Checking response code no iotagent ",
                         code_res == 404);

  std::cout << "END@UT@ testErrorsManager" << std::endl;
}

void AdminManagerTest::testServiceManagement() {
  std::cout << "START @UT@START testServiceManagementt" << std::endl;
  std::map<std::string, std::string> headers;
  std::string query_string;
  int code_res;
  std::string response, cb_last;
  std::string service = "service2";
  std::map<std::string, std::string> h;

  MockService* http_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  unsigned int port = iota::Process::get_process().get_http_port();

  std::string POST_PROTOCOLS1(
      "{\"iotagent\": \"http://127.0.0.1:7070/mock/testServiceManagement\","
      "\"resource\": \"/iot/d\","
      "\"protocol\": \"UL20\","
      "\"description\": \"Ultralight 2.0\""
      "}");

  std::string POST_PROTOCOLS2(
      "{\"iotagent\": \"http://127.0.0.1:7070/mock/testServiceManagement\","
      "\"resource\": \"/iot/mqtt\","
      "\"protocol\": \"MQTT\","
      "\"description\": \"mqtt example\""
      "}");

  std::cout << "@UT@Remove all data in protocols" << std::endl;
  pion::http::response http_response;
  ((iota::AdminManagerService*)(iota::Process::get_process()
                                    .get_admin_service()))
      ->delete_all_protocol_json(http_response, "", response);

  http_mock->set_response("/mock/testServiceManagement/services", 201, "", h);
  std::cout << "@UT@Post iotagents without services" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "POST", "ss", "", "application/json",
                       POST_PROTOCOLS1, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  http_mock->set_response("/mock/testServiceManagement/services", 201, "", h);
  std::cout << "@UT@Post iotagents without services" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "POST", "ss", "", "application/json",
                       POST_PROTOCOLS2, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  srand(time(NULL));

  std::cout << "START @UT@START testServiceManagement" << std::endl;

  service = "service";
  service.append(boost::lexical_cast<std::string>(rand()));
  std::cout << "@UT@service " << service << std::endl;

  std::cout << "@UT@GET get empty" << std::endl;
  code_res = http_test(URI_SERVICES_MANAGEMET, "GET", service, "",
                       "application/json", "", headers, query_string, response);
  boost::algorithm::trim(response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\": 0,\"services\": []}") == 0);

  std::cout << "@UT@POST" << std::endl;
  http_mock->set_response("/mock/testServiceManagement/services", 201, "", h);
  code_res =
      http_test(URI_SERVICES_MANAGEMET, "POST", service, "", "application/json",
                POST_SERVICE_MANAGEMENT1, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 201);

  std::cout << "@UT@GET IDAS-20462 nodevice" << std::endl;
  http_mock->set_response("/mock/testServiceManagement/devices/nodevice", 404,
                          "", h);
  code_res = http_test(URI_DEVICES_MANAGEMEMT + "/nodevice", "GET", service, "",
                       "application/json", "", headers, "", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\" : 0, \"devices\" : [] }") == 0);

  std::cout << "@UT@PUT" << std::endl;
  http_mock->set_response("/mock/testServiceManagement/services", 204, "", h);
  code_res =
      http_test(URI_SERVICES_MANAGEMET, "PUT", service, "", "application/json",
                "{\"services\": [{\"protocol\":[\"MQTT\"],\"cbroker\": "
                "\"http://cbrokerPUT\"}]}",
                headers, "", response);
  std::cout << "@UT@PUTRESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 204);

  code_res = http_test(URI_SERVICES_MANAGEMET, "GET", service, "",
                       "application/json", "", headers, query_string, response);
  boost::algorithm::trim(response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\": 0,\"services\": []}") == 0);

  http_mock->set_response("/mock/testServiceManagement/services", 400,
                          "{\"reason\": \"hola\", \"details\": \"adios\"}");
  code_res = http_test(URI_SERVICES_MANAGEMET, "PUT", "noservice", "",
                       "application/json", POST_SERVICE_MANAGEMENT1, headers,
                       "", response);
  std::cout << "PUT RESPONSE BAD " << response << std::endl;
  IOTASSERT(code_res == 404);

  std::cout << "@UT@DELETE" << std::endl;
  http_mock->set_response("/mock/testServiceManagement/services", 204, "", h);
  code_res =
      http_test(URI_SERVICES_MANAGEMET, "DELETE", service, "/ssrv2",
                "application/json", "", headers, "protocol=UL20", response);
  std::cout << "@UT@RESPONSE: XXXXXXXXX" << code_res << " " << response
            << std::endl;
  IOTASSERT(code_res == 204);
  cb_last =
      http_mock->get_last("/mock/testServiceManagement/protocols/services");
  std::cout << "@UT@iotagent mock: " << cb_last << std::endl;

  std::cout << "@UT@POST reregister POST_PROTOCOLS2_RERERE" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "POST", "ss", "", "application/json",
                       POST_PROTOCOLS2_RERERE, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);
  // debe de haber un servicio menos

  std::cout << "@UT@POST reregister POST_PROTOCOLS2_RERERE_EMPTY" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "POST", "ss", "", "application/json",
                       POST_PROTOCOLS2_RERERE, headers, "", response);
  std::cout << "@UT@RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);
  // http_mock->stop();

  std::cout << "END@UT@ testServiceManagement" << std::endl;
}

void AdminManagerTest::testBADServiceManagement() {
  srand(static_cast<unsigned int>(time(0)));
  std::cout << "START @UT@START testBADServiceManagement" << std::endl;
  std::map<std::string, std::string> headers;
  std::string query_string;

  int code_res;
  std::string response;
  std::string service = "service";
  service.append(boost::lexical_cast<std::string>(rand()));
  std::cout << "@UT@service " << service << std::endl;

  // no existe servicio al hacer POST de device
  std::cout << "@UT@1POST" << std::endl;
  code_res =
      http_test(URI_SERVICES_MANAGEMET, "POST", service, "", "application/json",
                POST_BAD_SERVICE_MANAGEMENT1, headers, "", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.compare("{\"reason\":\"The request is not well "
                             "formed\",\"details\":\"No exists protocol "
                             "no_exists\"}") == 0);

  std::cout << "@UT@GET IDAS-20462 nodevice" << std::endl;
  code_res = http_test(URI_DEVICES_MANAGEMEMT + "/nodevice", "GET", service, "",
                       "application/json", "", headers, "", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\" : 0, \"devices\" : [] }") == 0);

  std::cout << "@UT@GET IDAS-20462 nodevice" << std::endl;
  code_res = http_test(URI_SERVICES_MANAGEMET, "GET", service, "",
                       "application/json", "", headers, "", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\": 0,\"services\": []}") == 0);

  code_res = http_test(URI_SERVICES_MANAGEMET + "/nodevice", "GET", "nodevice",
                       "", "application/json", "", headers, "", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\": 0,\"services\": []}") == 0);

  std::cout << "@UT@bad limit" << std::endl;
  code_res =
      http_test(URI_SERVICES_MANAGEMET + "/nodevice", "GET", "nodevice", "",
                "application/json", "", headers, "limit=-22", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\": 0,\"services\": []}") == 0);

  std::cout << "@UT@bad limit2" << std::endl;
  code_res =
      http_test(URI_SERVICES_MANAGEMET + "/nodevice", "GET", "nodevice", "",
                "application/json", "", headers, "limit=badlimit", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\": 0,\"services\": []}") == 0);

  std::cout << "@UT@bad offset" << std::endl;
  code_res =
      http_test(URI_SERVICES_MANAGEMET + "/nodevice", "GET", "nodevice", "",
                "application/json", "", headers, "offset=-22", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\": 0,\"services\": []}") == 0);

  std::cout << "@UT@bad limit2" << std::endl;
  code_res =
      http_test(URI_SERVICES_MANAGEMET + "/nodevice", "GET", "nodevice", "",
                "application/json", "", headers, "offset=badlimit", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\": 0,\"services\": []}") == 0);

  code_res = http_test(URI_DEVICES_MANAGEMEMT + "/nodevice", "GET", "noservice",
                       "", "application/json", "", headers, "", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\" : 0, \"devices\" : [] }") == 0);

  std::cout << "@UT@bad limit" << std::endl;
  code_res =
      http_test(URI_DEVICES_MANAGEMEMT + "/nodevice", "GET", "noservice", "",
                "application/json", "", headers, "limit=-22", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\" : 0, \"devices\" : [] }") == 0);

  std::cout << "@UT@bad limit" << std::endl;
  code_res =
      http_test(URI_DEVICES_MANAGEMEMT + "/nodevice", "GET", "noservice", "",
                "application/json", "", headers, "limit=badlimit", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\" : 0, \"devices\" : [] }") == 0);

  std::cout << "@UT@bad offset" << std::endl;
  code_res =
      http_test(URI_DEVICES_MANAGEMEMT + "/nodevice", "GET", "noservice", "",
                "application/json", "", headers, "offset=-22", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\" : 0, \"devices\" : [] }") == 0);

  std::cout << "@UT@bad offset2" << std::endl;
  code_res =
      http_test(URI_DEVICES_MANAGEMEMT + "/nodevice", "GET", "noservice", "",
                "application/json", "", headers, "offset=bad", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 200);
  IOTASSERT(response.compare("{ \"count\" : 0, \"devices\" : [] }") == 0);

  std::cout << "END@UT@ testBADServiceManagement" << std::endl;
}

void AdminManagerTest::testPutJSONDevice_Wrong() {
  std::cout << "START @UT@START testPutJSONDevice_Wrong (missing protocol)"
            << std::endl;
  std::string device_put(
      "{\"device_id\": \"device_1\",\"entity_name\": "
      "\"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": "
      "\"http://device_endpoint\",\"timezone\": \"America/Santiago\","
      "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": "
      "\"device_id@ping|%s\" }],"
      "\"attributes\": [{\"object_id\": \"temp\",\"name\": "
      "\"temperature\",\"type\": \"int\" }],"
      "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", "
      "\"value\": \"50\"  }]"
      "}");

  std::string protocol("PDI-Ultra-Light");

  iota::AdminManagerService* manager_service =
      (iota::AdminManagerService*)iota::Process::get_process()
          .get_admin_service();
  pion::http::response http_response;
  std::string response;
  std::string token("1234");

  CPPUNIT_ASSERT_THROW_MESSAGE(
      "Missing protocol, Exception expected.",
      manager_service->put_device_json(
          "testmanagerservice", "/testmanagersubservice", "device_1",
          device_put, http_response, response, token, ""),
      iota::IotaException);

  CPPUNIT_ASSERT_THROW_MESSAGE(
      "Empty Services. Exception expected",
      manager_service->put_device_json("", "/testmanagersubservice", "device_1",
                                       device_put, http_response, response,
                                       token, protocol),
      iota::IotaException);

  std::cout << "END@UT@ testPutJSONDevice_Wrong" << std::endl;
}

void AdminManagerTest::testPutJSONDevice() {
  unsigned int port = iota::Process::get_process().get_http_port();
  std::string s1_d(
      "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
      "10.95.213.36:1026\","
      "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"http://"
      "127.0.0.1:" +
      boost::lexical_cast<std::string>(port) +
      "/iotagent\","
      "\"protocol\":\"PDI-IoTA-UltraLight\",\"service\": "
      "\"testmanagerservice\",\"service_path\":\"/testmanagersubservice\"}");
  std::string s1_d_error(
      "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
      "10.95.213.36:1026\","
      "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"http://"
      "127.0.0.1:1000/iotagent\","
      "\"protocol\":\"PDI-IoTA-UltraLight\",\"service\": "
      "\"testmanagerservice\",\"service_path\":\"/testmanagersubservice\"}");

  std::string device_put(
      "{\"protocol\":\"PDI-IoTA-UltraLight\",\"device_id\": \"device_id_post\","
      "\"entity_name\": \"entity_put\"}");

  std::string device_id("device_id_post");
  std::string trace("12345");
  std::string token("");
  std::string protocol("PDI-IoTA-UltraLight");
  iota::ServiceMgmtCollection table1;
  iota::DeviceCollection table_device;

  table1.createTableAndIndex();
  table_device.createTableAndIndex();

  mongo::BSONObj all;

  table1.remove(all);

  iota::Device borrar("", "");

  table_device.removed(borrar);

  table1.insert(mongo::fromjson(s1_d));
  table1.insert(mongo::fromjson(s1_d_error));

  iota::AdminManagerService* manager_service =
      (iota::AdminManagerService*)iota::Process::get_process()
          .get_admin_service();

  std::string response;
  pion::http::response http_response;

  std::cout << "Test testPutJSONDevice STARTING" << std::endl;
  // DEVICE POSTED using Manager's API.
  manager_service->post_device_json("testmanagerservice",
                                    "/testmanagersubservice", devices,
                                    http_response, response, "");

  int res = manager_service->put_device_json(
      "testmanagerservice", "/testmanagersubservice", device_id, device_put,
      http_response, response, "", protocol);
  CPPUNIT_ASSERT(res == 200);
  CPPUNIT_ASSERT_MESSAGE(
      "Checking error ",
      response.find("Connection refused") != std::string::npos);
  std::cout << "Response " << res << std::endl;

  std::cout << "Test testPutJSONDevice DONE" << std::endl;
}

void AdminManagerTest::testPutProtocolDevice() {
  std::cout << "START @UT@START testPutProtocolDevice" << std::endl;
  unsigned int port = iota::Process::get_process().get_http_port();
  std::map<std::string, std::string> headers;
  std::string query_string("");

  int code_res;
  std::string response;
  std::string service = "testmanagerservice";
  std::cout << "@UT@service " << service << std::endl;

  std::string uri_query(URI_DEVICES_MANAGEMEMT);
  uri_query.append("/device_id_new");

  // missing protocol in query
  std::cout << "@UT@1PUT" << std::endl;
  code_res = http_test(uri_query, "PUT", service, "", "application/json",
                       PUT_DEVICE, headers, query_string, response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.compare("{\"reason\":\"A parameter is missing in the "
                             "request\",\"details\":\"protocol parameter is "
                             "mandatory\"}") == 0);

  pion::http::response http_response;

  std::string s1_d(
      "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
      "10.95.213.36:1026\","
      "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"http://"
      "127.0.0.1:" +
      boost::lexical_cast<std::string>(port) +
      "/iotagent\","
      "\"protocol\":\"PDI-IoTA-UltraLight\",\"service\": "
      "\"testmanagerservice\",\"service_path\":\"/testmanagersubservice\"}");
  std::string s1_d_error(
      "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
      "10.95.213.36:1026\","
      "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"http://"
      "127.0.0.1:1000/iotagent\","
      "\"protocol\":\"PDI-IoTA-UltraLight\",\"service\": "
      "\"testmanagerservice\",\"service_path\":\"/testmanagersubservice\"}");

  // POST Device directly onto endpoint.
  code_res = http_test("/iotagent/devices", "POST", "testmanagerservice",
                       "/testmanagersubservice", "application/json",
                       POST_DEVICE, headers, "", response);

  iota::ServiceMgmtCollection table1;
  table1.createTableAndIndex();
  mongo::BSONObj all;

  table1.remove(all);
  // Insert endpoint for ss1 service
  table1.insert(mongo::fromjson(s1_d));
  table1.insert(mongo::fromjson(s1_d_error));

  std::multimap<std::string, std::string> query_parameters;
  query_parameters.insert(std::make_pair<std::string, std::string>(
      "protocol", "PDI-IoTA-UltraLight"));
  query_string = iota::make_query_string(query_parameters);
  code_res = http_test(uri_query, "PUT", service, "/testmanagersubservice",
                       "application/json", PUT_DEVICE, headers, query_string,
                       response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  // TODO  IOTASSERT(code_res == 200);
  CPPUNIT_ASSERT_MESSAGE(
      "Checking error ",
      response.find("Connection refused") != std::string::npos);
  std::cout << "END@UT@ testPutProtocolDevice" << std::endl;
}

void AdminManagerTest::testBadCharactersDeviceDM1179() {
  std::cout << "START @UT@START testBadCharactersDevice_DM1179" << std::endl;
  MockService* http_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  unsigned int port = iota::Process::get_process().get_http_port();
  std::string response;

  iota::AdminManagerService manager_service;
  manager_service.set_timeout(5);
  std::cout << "testAddDevicesToEndpoints: STARTING... " << std::endl;
  std::map<std::string, std::string> h;
  http_mock->set_response("testAddDevicesToEndpoints", 201, "{}", h);
  http_mock->set_response("testAddDevicesToEndpoints", 201, "{}", h);

  std::string endpoint("http://127.0.0.1:" +
                       boost::lexical_cast<std::string>(port) +
                       "/mock/testBadCharactersDeviceDM1179");

  std::map<std::string, std::string> headers;
  std::string query_string("");
  std::string POST_DEVICE_BAD_NAME(
    "{\"devices\": "
    "[{\"device_id\": \"device id\",\"entity_name\": "
    "\"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": "
    "\"http://device_endpoint\",\"timezone\": \"America/Santiago\","
    "\"protocol\": \"PDI-IoTA-UltraLight\""
    "}]}");

  const std::string POST_SERVICE_MANAGEMENT_BAD_NAME(
    "{\"services\": [{"
    "\"protocol\": [\"UL20\"],"
    "\"apikey\": \"apikey\",\"token\": \"token\","
    "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\""
    "}]}");

  pion::http::response http_response;
  int code_res;
  std::string service = "testmanagerservice22";
  std::string service_path = "/";
  std::cout << "@UT@service " << service << std::endl;

  std::string uri_query(URI_DEVICES_MANAGEMEMT);

  std::string body1("{\"iotagent\": \"");
  body1 += endpoint;
  body1 += "\",\"identifier\": \"id:80\",";
  body1 += "\"resource\": \"/iot/d\",";
  body1 += "\"protocol\": \"UL20\",";
  body1 += "\"services\": []}";
  int code_res1 = manager_service.post_protocol_json(
      service, service_path, body1, http_response, response);
  std::cout << "@UT@1RESPONSE: " << code_res1 << " " << response << std::endl;
  if (code_res1 == 500){
      IOTASSERT(response.compare("\"code\": \"200\"") == 0);
  }else {
      IOTASSERT(code_res1 == 201);
  }


  // create service
  std::cout << "@UT@1POST SERVICE" << std::endl;
  code_res = http_test("/TestManager/services", "POST", service, "", "application/json",
                       POST_SERVICE_MANAGEMENT_BAD_NAME, headers, query_string, response);
  std::cout << "@UT@2RESPONSE: " << code_res << " " << response << std::endl;
  if (code_res1 == 500){
      IOTASSERT(response.compare("\"code\": \"201\"") == 0);
  }else {
      IOTASSERT(code_res1 == 201);
  }

  // create device
  std::cout << "@UT@1POST" << std::endl;
  code_res = http_test(uri_query, "POST", service, "", "application/json",
                       POST_DEVICE_BAD_NAME, headers, query_string, response);
  std::cout << "@UT@3RESPONSE: " << code_res << " " << response << std::endl;
  if (code_res1 == 500){
      IOTASSERT(response.compare("\"code\": \"201\"") == 0);
  }else {
      IOTASSERT(code_res1 == 201);
  }

  // DELETE Device
  code_res = http_test("/iotagent/devices/device+id", "DELETE", "testmanagerservice",
                       "/testmanagersubservice", "application/json",
                       "", headers, "", response);

  std::cout << "@UT@22RESPONSE: " << code_res << " " << response << std::endl;
  if (code_res == 500){
      IOTASSERT(response.compare("\"code\": \"204\"") == 0);
  }else {
      IOTASSERT(code_res == 204);
  }

  std::cout << "END@UT@ testBadCharactersDevice_DM1179" << std::endl;
}


void AdminManagerTest::testPostJSONDeviceErrorHandling() {
  std::cout << "START @UT@START testPostJSONDeviceErrorHandling" << std::endl;
  std::map<std::string, std::string> headers;
  std::string query_string("");
  unsigned int port = iota::Process::get_process().get_http_port();

  std::string s1_d(
      "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
      "10.95.213.36:1026\","
      "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"http://"
      "127.0.0.1:" +
      boost::lexical_cast<std::string>(port) +
      "/iotagent\","
      "\"protocol\":\"PDI-IoTA-UltraLight\",\"service\": "
      "\"testmanagerservice\",\"service_path\":\"/testmanagersubservice\"}");

  int code_res;
  std::string response;
  std::string service = "testmanagerservice";
  std::cout << "@UT@service " << service << std::endl;

  std::string uri_query(URI_DEVICES_MANAGEMEMT);
  iota::ServiceMgmtCollection table1;
  table1.createTableAndIndex();
  mongo::BSONObj all;

  table1.remove(all);
  // Insert endpoint for ss1 service
  table1.insert(mongo::fromjson(s1_d));
  // missing protocol in query
  std::cout << "@UT@1POST" << std::endl;
  std::multimap<std::string, std::string> query_parameters;
  query_parameters.insert(std::make_pair<std::string, std::string>(
      "protocol", "PDI-IoTA-UltraLight"));
  query_string = iota::make_query_string(query_parameters);
  code_res = http_test(uri_query, "POST", service, "/testmanagersubservice",
                       "application/json", POST_DEVICE, headers, query_string,
                       response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 500);

  std::cout << "END@UT@ testPostJSONDeviceErrorHandling" << std::endl;
}

void AdminManagerTest::testNoEndpoints_Bug_IDAS20444() {
  std::cout << "START @UT@START testNoEndpoints_Bug_IDAS20444" << std::endl;
  std::map<std::string, std::string> headers;
  std::string query_string("");
  unsigned int port = iota::Process::get_process().get_http_port();

  std::string s1_d(
      "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
      "10.95.213.36:1026\","
      "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"http://"
      "127.0.0.1:" +
      boost::lexical_cast<std::string>(port) +
      "/iotagent\","
      "\"protocol\":\"PDI-IoTA-UltraLight\",\"service\": "
      "\"testmanagerservice\",\"service_path\":\"/testmanagersubservice\"}");

  int code_res;
  std::string response;
  std::string service = "testmanagerservice";
  std::cout << "@UT@service " << service << std::endl;

  std::string uri_query(URI_DEVICES_MANAGEMEMT);

  iota::ServiceMgmtCollection table1;
  table1.createTableAndIndex();
  mongo::BSONObj all;

  // First Scenario: no endpoints found, 400 expected.
  table1.remove(all);

  std::cout << "@UT@1POST" << std::endl;
  std::multimap<std::string, std::string> query_parameters;
  query_parameters.insert(std::make_pair<std::string, std::string>(
      "protocol", "PDI-IoTA-UltraLight"));
  query_string = iota::make_query_string(query_parameters);
  code_res = http_test(uri_query, "POST", service, "/testmanagersubservice",
                       "application/json", POST_DEVICE, headers, query_string,
                       response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.find("{\"reason\":\"The request is not well formed\"") !=
            std::string::npos);

  // Insert endpoint for ss1 service
  table1.insert(mongo::fromjson(s1_d));
  // Second Scenario: inexistent service
  code_res = http_test(uri_query, "POST", "nanana", "/testmanagersubservice",
                       "application/json", POST_DEVICE, headers, query_string,
                       response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 400);

  // Scenario: no protocol on endpoint
  code_res = http_test(uri_query, "POST", service, "/testmanagersubservice",
                       "application/json", POST_DEVICE, headers, "", response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 500);

  // Scenario: no protocol on JSON

  code_res = http_test(uri_query, "POST", service, "/testmanagersubservice",
                       "application/json", POST_DEVICE_NO_PROTOCOL, headers, "",
                       response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 400);

  std::cout << "END@UT@ testNoEndpoints_Bug_IDAS20444" << std::endl;
}

void AdminManagerTest::testNoDeviceError_Bug_IDAS20463() {
  std::cout << "START @UT@START testNoDeviceError_Bug_IDAS20463" << std::endl;
  std::map<std::string, std::string> headers;
  std::string query_string("");
  unsigned int port = iota::Process::get_process().get_http_port();

  std::string s1_d(
      "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
      "10.95.213.36:1026\","
      "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"http://"
      "127.0.0.1:" +
      boost::lexical_cast<std::string>(port) +
      "/iotagent\","
      "\"protocol\":\"PDI-IoTA-UltraLight\",\"service\": "
      "\"testmanagerservice\",\"service_path\":\"/testmanagersubservice\"}");

  int code_res;
  std::string response;
  std::string service = "testmanagerservice";
  std::cout << "@UT@service " << service << std::endl;

  std::string uri_query(URI_DEVICES_MANAGEMEMT);
  iota::ServiceMgmtCollection table1;
  table1.createTableAndIndex();
  mongo::BSONObj all;

  // First Scenario: no device found
  table1.remove(all);

  std::cout << "@UT@1POST" << std::endl;
  std::multimap<std::string, std::string> query_parameters;
  query_parameters.insert(std::make_pair<std::string, std::string>(
      "protocol", "PDI-IoTA-UltraLight"));

  query_string = iota::make_query_string(query_parameters);

  code_res = http_test(uri_query + "/device_id", "PUT", service,
                       "/testmanagersubservice", "application/json", PUT_DEVICE,
                       headers, query_string, response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 404);

  // Insert endpoint for ss1 service
  table1.insert(mongo::fromjson(s1_d));
  // Second Scenario: inexistent service
  code_res = http_test(uri_query + "/device_id", "PUT", service,
                       "/testmanagersubservice", "application/json",
                       PUT_DEVICE2, headers, query_string, response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 404);

  code_res = http_test(uri_query + "/device_id", "PUT", "nanan",
                       "/testmanagersubservice", "application/json", PUT_DEVICE,
                       headers, query_string, response);
  std::cout << "@UT@1RESPONSE: " << code_res << " " << response << std::endl;
  IOTASSERT(code_res == 404);

  std::cout << "END@UT@ testNoDeviceError_Bug_IDAS20463" << std::endl;
}

void AdminManagerTest::testReregistration_diff_protocol_description() {
  std::cout << "@UT@START testReregistration_diff_protocol_description"
            << std::endl;

  iota::AdminManagerService manager_service;

  std::string service = create_random_service("s");
  std::string service_path("/ss1");
  std::string description1("des1");
  std::string description2("des2");
  delete_mongo(iota::store::types::MANAGER_SERVICE_TABLE, service,
               service_path);
  pion::http::response http_response;
  std::string response;
  std::string body1(
      "{\"iotagent\": \"host2\","
      "\"resource\": \"/iot/mqtt\","
      "\"protocol\": \"MQTT\","
      "\"services\": [{"
      "\"apikey\": \"apikey3\","
      "\"token\": \"token2\","
      "\"cbroker\": \"http://127.0.0.1:1026\","
      "\"resource\": \"/iot/mqtt\","
      "\"entity_type\": \"thing\",");
  body1.append("\"service\":\"");
  body1.append(service);
  body1.append("\",\"service_path\":\"");
  body1.append(service_path);
  body1.append("\"}],");
  body1.append("\"description\":\"");
  std::string body2 = body1;
  body1.append(description1);
  body1.append("\"}");
  body2.append(description2);
  body2.append("\"}");

  int code_res1 = manager_service.post_protocol_json(
      service, service_path, body1, http_response, response);
  std::cout << "@UT@1RESPONSE: " << code_res1 << " " << response << std::endl;
  IOTASSERT(code_res1 == 201);
  IOTASSERT(response.empty());
  IOTASSERT(check_mongo("SERVICE_MGMT", service, service_path, "description",
                        description1) == 1);

  int code_res2 = manager_service.post_protocol_json(
      service, service_path, body2, http_response, response);
  std::cout << "@UT@2RESPONSE: " << code_res2 << " " << response << std::endl;
  IOTASSERT(code_res2 == 201);
  IOTASSERT(response.empty());
  IOTASSERT(check_mongo("SERVICE_MGMT", service, service_path, "description",
                        description1) == 0);
  IOTASSERT(check_mongo("SERVICE_MGMT", service, service_path, "description",
                        description2) == 1);

  delete_mongo(iota::store::types::MANAGER_SERVICE_TABLE, service,
               service_path);
  std::cout << "@UT@END testReregistration_diff_protocol_description"
            << std::endl;
}

void AdminManagerTest::testReregistration_changing_ip() {
  std::cout << "@UT@START testReregistration_changing_ip" << std::endl;

  iota::AdminManagerService manager_service;

  std::string service = create_random_service("s");
  std::string service_path("/ss1");
  std::string description1("des1");
  std::string description2("des2");
  delete_mongo(iota::store::types::MANAGER_SERVICE_TABLE, service,
               service_path);
  pion::http::response http_response;
  std::string response;
  std::string body1(
      "{\"iotagent\": \"host2ip\","
      "\"identifier\": \"id:80\","
      "\"resource\": \"/iot/mqtt\","
      "\"protocol\": \"MQTT\","
      "\"services\": []}");
  std::string body2(
      "{\"iotagent\": \"host2_new_ip\","
      "\"identifier\": \"id:80\","
      "\"resource\": \"/iot/mqtt\","
      "\"protocol\": \"MQTT\","
      "\"services\": []}");

  int code_res1 = manager_service.post_protocol_json(
      service, service_path, body1, http_response, response);
  std::cout << "@UT@1RESPONSE: " << code_res1 << " " << response << std::endl;
  IOTASSERT(code_res1 == 201);
  IOTASSERT(response.empty());
  IOTASSERT(
      check_mongo(
          iota::store::types::PROTOCOL_TABLE,
          "{ \"protocol\" : \"MQTT\", \"endpoints.endpoint\": \"host2ip\"}") ==
      1);

  int code_res2 = manager_service.post_protocol_json(
      service, service_path, body2, http_response, response);
  std::cout << "@UT@2RESPONSE: " << code_res2 << " " << response << std::endl;
  IOTASSERT(code_res2 == 201);
  IOTASSERT(response.empty());
  IOTASSERT(
      check_mongo(
          iota::store::types::PROTOCOL_TABLE,
          "{ \"protocol\" : \"MQTT\", \"endpoints.endpoint\": \"host2ip\"}") ==
      0);
  IOTASSERT(check_mongo(iota::store::types::PROTOCOL_TABLE,
                        "{ \"protocol\" : \"MQTT\", \"endpoints.endpoint\": "
                        "\"host2_new_ip\"}") == 1);

  delete_mongo(iota::store::types::PROTOCOL_TABLE, "", "");
  std::cout << "@UT@END testReregistration_changing_ip" << std::endl;
}

void AdminManagerTest::testReregistration_changing_identifier() {
  std::cout << "@UT@START testReregistration_changing_identifier" << std::endl;

  iota::AdminManagerService manager_service;

  std::string service = create_random_service("s");
  std::string service_path("/ss1");
  std::string description1("des1");
  std::string description2("des2");
  delete_mongo(iota::store::types::MANAGER_SERVICE_TABLE, service,
               service_path);
  pion::http::response http_response;
  std::string response;
  std::string body1(
      "{\"iotagent\": \"host2ip\","
      "\"identifier\": \"id1:80\","
      "\"resource\": \"/iot/mqtt\","
      "\"protocol\": \"MQTT\","
      "\"services\": []}");
  std::string body2(
      "{\"iotagent\": \"host2ip\","
      "\"identifier\": \"id2:80\","
      "\"resource\": \"/iot/mqtt\","
      "\"protocol\": \"MQTT\","
      "\"services\": []}");

  int code_res1 = manager_service.post_protocol_json(
      service, service_path, body1, http_response, response);
  std::cout << "@UT@1RESPONSE: " << code_res1 << " " << response << std::endl;
  IOTASSERT(code_res1 == 201);
  IOTASSERT(response.empty());
  IOTASSERT(
      check_mongo(
          iota::store::types::PROTOCOL_TABLE,
          "{ \"protocol\" : \"MQTT\", \"endpoints.identifier\": \"id1:80\"}") ==
      1);

  int code_res2 = manager_service.post_protocol_json(
      service, service_path, body2, http_response, response);
  std::cout << "@UT@2RESPONSE: " << code_res2 << " " << response << std::endl;
  IOTASSERT(code_res2 == 201);
  IOTASSERT(response.empty());
  IOTASSERT(
      check_mongo(
          iota::store::types::PROTOCOL_TABLE,
          "{ \"protocol\" : \"MQTT\", \"endpoints.identifier\": \"id1:80\"}") ==
      0);
  IOTASSERT(
      check_mongo(
          iota::store::types::PROTOCOL_TABLE,
          "{ \"protocol\" : \"MQTT\", \"endpoints.identifier\": \"id2:80\"}") ==
      1);

  delete_mongo(iota::store::types::PROTOCOL_TABLE, "", "");
  std::cout << "@UT@END testReregistration_changing_identifier" << std::endl;
}
