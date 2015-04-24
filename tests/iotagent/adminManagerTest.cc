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
#include <pion/process.hpp>

#include <cppunit/extensions/HelperMacros.h>
#include "util/device_collection.h"
#include <cmath>
#include <ctime>
#include "services/admin_mgmt_service.h"

#define PATH_CONFIG_MONGO "../../tests/iotagent/config_mongo.json"

#define  IOTASSERT_MESSAGE(x,y) \
         std::cout << "@" << __LINE__ << "@" << x << std::endl; \
         CPPUNIT_ASSERT_MESSAGE(x,y)

#define  IOTASSERT(y) \
         std::cout << "@" << __LINE__ << "@" << std::endl; \
         CPPUNIT_ASSERT(y)

//#define  PATH_CONFIG "../../tests/iotagent/config_mongo.json"

using ::testing::Return;
using ::testing::NotNull;
using ::testing::StrEq;
using ::testing::_;
using ::testing::Invoke;

iota::AdminService* AdminService_ptr;
CPPUNIT_TEST_SUITE_REGISTRATION(AdminManagerTest);

/*namespace iota {
std::string URL_BASE = "/iot";
std::string logger("main");
}*/

const std::string AdminManagerTest::HOST("127.0.0.1");
const std::string AdminManagerTest::CONTENT_JSON("application/json");

const int AdminManagerTest::POST_RESPONSE_CODE = 201;
//GET ALL empty
const int AdminManagerTest::GET_RESPONSE_CODE = 200;
const int AdminManagerTest::GET_RESPONSE_CODE_NOT_FOUND = 404;
const int AdminManagerTest::DELETE_RESPONSE_CODE = 204;

////////////////////
////  PROTOCOLS
const std::string AdminManagerTest::URI_PROTOCOLS("/iot/protocols");
//POST
const std::string
AdminManagerTest::POST_PROTOCOLS1("{\"iotagent\": \"host1\","
                          "\"resource\": \"/iot/d\","
                          "\"protocol\": \"UL20\","
                          "\"description\": \"Ultralight 2.0\""
                          "}");
const std::string
AdminManagerTest::POST_PROTOCOLS2("{\"iotagent\": \"host2\","
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
AdminManagerTest::POST_PROTOCOLS3("{\"iotagent\": \"host3\","
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
AdminManagerTest::POST_PROTOCOLS2_RERE("{\"iotagent\": \"host2\","
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
                          "},{"
                          "\"apikey\": \"apikey3\","
                                "\"service\": \"service2\","
                                "\"service_path\": \"/ssrv2re\","
                                "\"token\": \"token2\","
                                "\"cbroker\": \"http://127.0.0.1:1026\","
                                "\"resource\": \"/iot/mqtt\","
                                "\"entity_type\": \"thing\""
                          "}]}");


const std::string
AdminManagerTest::POST_PROTOCOLS4("TODO");
const std::string
AdminManagerTest::GET_PROTOCOLS_RESPONSE("{ \"count\": 0,\"devices\": []}");


////////////////////
////  SERVICE_MANAGEMENT
const std::string AdminManagerTest::URI_SERVICES_MANAGEMET("/iot/services");
//POST
const std::string
AdminManagerTest::POST_SERVICE_MANAGEMENT1("{\"services\": [{"
       "\"protocol\": [\"UL20\", \"MQTT\"],"
       "\"apikey\": \"apikey\",\"token\": \"token\","
       "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\""
     "}]}");
const std::string
AdminManagerTest::POST_SERVICE_MANAGEMENT2("{\"services\": [{"
       "\"protocol\": [\"UL20\", \"MQTT\"],"
       "\"apikey\": \"apikey\",\"token\": \"token\","
       "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\""
     "}]}");


const std::string
AdminManagerTest::POST_BAD_SERVICE_MANAGEMENT1("{\"services\": [{"
       "\"protocol\": [\"no_exists\", \"MQTT\"],"
       "\"apikey\": \"apikey\",\"token\": \"token\","
       "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\""
     "}]}");

const std::string
AdminManagerTest::GET_SERVICE_MANAGEMENT_RESPONSE("{ \"count\": 0,\"devices\": []}");

////////////////////
////  DEVICE _MANAGEMENT
const std::string AdminManagerTest::URI_DEVICES_MANAGEMEMT("/iot/devices");
//POST
const std::string
AdminManagerTest::POST_DEVICE_MANAGEMENT1("{\"devices\": "
                           "[{\"device_id\": \"device_id\",\"entity_name\": \"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
                           "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": \"device_id@ping|%s\" }],"
                           "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }],"
                           "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
                           "}]}");

const std::string
AdminManagerTest::GET_DEVICE_MANAGEMENT_RESPONSE("{ \"count\": 0,\"devices\": []}");

std::string
s1_d_host2("{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://10.95.213.36:1026\","
           "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"host2\","
           "\"protocol\":\"UL20\",\"service\": \"s1\",\"service_path\":\"/ss1\"}");

std::string
s1_d2("{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://10.95.213.36:1026\","
      "\"entity_type\":\"thing\",\"resource\":\"/iot/d2\",\"iotagent\":\"host1\","
      "\"protocol\":\"UL20\",\"service\": \"s1\",\"service_path\":\"/ss1\"}");

std::string
s1_mqtt("{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://10.95.213.36:1026\","
        "\"entity_type\":\"thing\",\"resource\":\"/iot/mqtt\",\"iotagent\":\"host1\","
        "\"protocol\":\"MQTT\",\"service\": \"s1\",\"service_path\":\"/ss1\"}");

std::string
s2_d("{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://10.95.213.36:1026\","
     "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"host1\","
     "\"protocol\":\"UL20\",\"service\": \"s1\",\"service_path\":\"/ss2\"}");

std::string
s3_mqtt("{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://10.95.213.36:1026\","
        "\"entity_type\":\"thing\",\"resource\":\"/iot/mqtt\",\"iotagent\":\"host1\","
        "\"protocol\":\"MQTT\",\"service\": \"s3\",\"service_path\":\"/ss3\"}");

std::string
s4_agus("{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://10.95.213.36:1026\","
        "\"entity_type\":\"thing\",\"resource\":\"/iot/fake\",\"iotagent\":\"http://127.0.0.1:7070/iot\","
        "\"protocol\":\"UL20\",\"service\": \"s4_agus\",\"service_path\":\"/ss3\"}");

std::string
s5_agus("{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://10.95.213.36:1026\","
        "\"entity_type\":\"thing\",\"resource\":\"/iot/fake\",\"iotagent\":\"http://127.0.0.1:7070/iot1\","
        "\"protocol\":\"UL20\",\"service\": \"s4_agus\",\"service_path\":\"/ss3\"}");

std::string
devices("{\"devices\":[{\"protocol\":\"PDI-IoTA-UltraLight\",\"device_id\": \"device_id_post\",\"entity_name\": \"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": \"http://device_endpoint\",\"timezone\": \"America/Santiago\","
        "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": \"device_id@ping|%s\" }],"
        "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }],"
        "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
        "}]}");


AdminManagerTest::AdminManagerTest() {
  iota::Configurator::initialize("../../tests/iotagent/config_mongo.json");
  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  cleanDB();
  pion::process::initialize();

  adm = new iota::AdminManagerService();
  //adm->set_manager();

  // ul20serv_ptr = new iota::UL20Service();
  // ul20serv_ptr->set_resource("/iot/d");

  AdminService_ptr = adm;
  AdminService_ptr->add_service("/iot/res", AdminService_ptr);
  wserver.reset(new pion::http::plugin_server(scheduler));
  wserver->add_service("/iot", adm);



  // wserver->add_service("/iot/d",ul20serv_ptr);

  wserver->start();

  std::string service("s1");
  std::string service_path("/ss1");
  pion::http::response http_response;
  std::string response;
  std::string service_s1("{\"services\": [{"
                         "\"apikey\": \"apikey\",\"token\": \"token\","
                         "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\",\"resource\": \"/iot/d\"}]}");
  boost::shared_ptr<iota::ServiceCollection> col(new iota::ServiceCollection());
  adm->post_service_json(col, service,service_path,service_s1,http_response,
                         response);

}

void AdminManagerTest::cleanDB() {

  iota::ServiceCollection table_service;

  table_service.createTableAndIndex();

  mongo::BSONObj all;

  table_service.remove(all);
}


AdminManagerTest::~AdminManagerTest() {

  wserver->stop();
  scheduler.shutdown();

}

void AdminManagerTest::setUp() {


}

void AdminManagerTest::tearDown() {
  // delete cbPublish; //Already deleted inside MqttService.

}


void AdminManagerTest::testDeviceToBeAdded() {

  iota::DeviceToBeAdded test("protocol","device_json");
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
  std::string devices("{\"devices\": "
                      "[{\"protocol\":\"UL20\",\"device_id\": \"device_id\",\"entity_name\": \"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
                      "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": \"device_id@ping|%s\" }],"
                      "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }],"
                      "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
                      "},{\"protocol\":\"UL20\",\"device_id\": \"device_id_2\",\"entity_name\": \"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
                      "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": \"device_id@ping|%s\" }],"
                      "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }],"
                      "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
                      "}]}");

  manager_service.resolve_endpoints(v_endpoints_devices,devices,"s1","/ss1");

  //Expected, "host1" and "host2"

  std::cout << "Test: testGetEndpointsFromDevices: result: ["<<
            v_endpoints_devices.size() << "] endpoints" << std::endl;
  CPPUNIT_ASSERT(v_endpoints_devices.size() == 4);
  for (int i=0; i<v_endpoints_devices.size(); i++) {
    std::cout << "Test: host: " << v_endpoints_devices[i].get_endpoint() <<
              std::endl;
  }
  // Following asserts have to change

  /*CPPUNIT_ASSERT(v_endpoints_devices[0].get_endpoint().compare("host1")==0);
  CPPUNIT_ASSERT(v_endpoints_devices[1].get_endpoint().compare("host2")==0);
  CPPUNIT_ASSERT(v_endpoints_devices[2].get_endpoint().compare("host1")==0);
  CPPUNIT_ASSERT(v_endpoints_devices[3].get_endpoint().compare("host2")==0);
*/
  std::cout << "Test: END" << std::endl;

}

void AdminManagerTest::testAddDevicesToEndpoints() {

  boost::shared_ptr<HttpMock> http_mock;
  http_mock.reset(new HttpMock(7777, "/iot/devices", false));
  http_mock->init();



  iota::AdminManagerService manager_service;
  std::cout << "testAddDevicesToEndpoints: STARTING... " << std::endl;
  std::map<std::string, std::string> h;
  http_mock->set_response(200, "{}", h);

  std::string
  device("{\"protocol\":\"UL20\",\"device_id\": \"device_id\",\"entity_name\": \"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": \"http://device_endpoint\",\"timezone\": \"America/Santiago\","
         "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": \"device_id@ping|%s\" }],"
         "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }],"
         "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
         "}");


  std::string endpoint("http://127.0.0.1:7777/iot/devices");
  //TEST:


  std::cout << "Endpoint: " << endpoint << std::endl;
  int res = manager_service.add_device_iotagent(endpoint,device,"s1","/ss1",
            "test");
  CPPUNIT_ASSERT(res == 200);
  //sleep(4);
  //CLEAN UP
  sleep(4);
  http_mock->stop();
}

void AdminManagerTest::testGetDevices() {
  std::cout << "@UT@START testGetDevices" << std::endl;
  iota::ServiceMgmtCollection table1;
  table1.createTableAndIndex();

  table1.remove(BSON("service" << "s4_agus"));
  table1.insert(mongo::fromjson(s4_agus));
  table1.insert(mongo::fromjson(s5_agus));

  boost::shared_ptr<HttpMock> http_mock;
  http_mock.reset(new HttpMock(7070, "/", false));
  http_mock->init();
  std::string mock_response("{\"count\": 2,\"devices\": "
                            "[{\"protocol\":\"UL20\",\"device_id\": \"device_id\",\"entity_name\": \"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
                            "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": \"device_id@ping|%s\" }],"
                            "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }],"
                            "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
                            "},{\"protocol\":\"UL20\",\"device_id\": \"device_id_2\",\"entity_name\": \"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
                            "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": \"device_id@ping|%s\" }],"
                            "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }],"
                            "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
                            "}]}");
  std::string
  mock_response_one_device("{\"protocol\":\"UL20\",\"device_id\": \"device_id\",\"entity_name\": \"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
                           "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": \"device_id@ping|%s\" }],"
                           "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }],"
                           "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
                           "}");
  std::map<std::string, std::string> h;
  // Two endpoints. Repeat response for test
  http_mock->set_response(200, mock_response, h);
  http_mock->set_response(200, mock_response, h);
  std::string mock_port = boost::lexical_cast<std::string>(http_mock->get_port());
  //pion::one_to_one_scheduler scheduler;
  //scheduler.startup();

  iota::AdminManagerService manager_service;
  pion::http::request_ptr http_request(new pion::http::request("/"));
  http_request->add_header("Fiware-Service", "s4_agus");
  http_request->add_header("Fiware-ServicePath", "/ss3");
  // http_request->add_header("X-Trace-Message", "12345");
  http_request->set_method("GET");

  pion::http::response http_response;
  std::string response;
  //  manager_service.get_devices(http_request, args, query, "s4_agus", "/ss3", 0, 0,
  //                              "on", "", http_response, response);

  manager_service.get_all_devices_json("s4_agus", "/ss3", 0, 0, "on", "",
                                       http_response, response, "12345","token", "UL20");

  std::cout << "@UT@get_all_devices" <<  response << std::endl;
  CPPUNIT_ASSERT_MESSAGE("Expected 4 devices ",
                         response.find("\"count\" : 4") != std::string::npos);


  http_mock->set_response(200, mock_response_one_device, h);
  manager_service.get_a_device_json("s4_agus", "/ss3",
                                    "device_id", http_response, response,"12345","token","UL20");
  std::cout << "@UT@get_a_device_json" <<  response << std::endl;
  CPPUNIT_ASSERT_MESSAGE("Expected device_id ",
                         response.find("\"device_id\" : \"device_id\"") != std::string::npos);
  CPPUNIT_ASSERT_MESSAGE("Expected  count ",
                         response.find("\"count\" : 1") != std::string::npos);
  std::cout << "STOP testGetDevices" << std::endl;
  sleep(2);
  http_mock->stop();

  table1.remove(BSON("service" << "s4_agus"));

  std::cout << "@UT@END testGetDevices" << std::endl;
}

void AdminManagerTest::testMultiplePostsWithResponse() {
  //boost::asio::io_service io_service;
  std::cout << "@UT@START testMultiplePostsWithResponse" << std::endl;
  boost::shared_ptr<HttpMock> http_mock;
  http_mock.reset(new HttpMock(7777, "/iot/devices", false));
  http_mock->init();
  //pion::one_to_one_scheduler scheduler;
  //scheduler.startup();

  iota::AdminManagerService manager_service;

  std::map<std::string, std::string> h;
  // Two endpoints. Repeat response for test
  http_mock->set_response(204, "{}", h);
  http_mock->set_response(204, "{}", h);





  std::string
  device_1("{\"protocol\":\"UL20\",\"device_id\": \"device_1\",\"entity_name\": \"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": \"http://device_endpoint\",\"timezone\": \"America/Santiago\","
           "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": \"device_id@ping|%s\" }],"
           "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }],"
           "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
           "}");


  std::string
  device_2("{\"protocol\":\"UL20\",\"device_id\": \"device_2\",\"entity_name\": \"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": \"http://device_endpoint\",\"timezone\": \"America/Santiago\","
           "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": \"device_id@ping|%s\" }],"
           "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }],"
           "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
           "}");

  std::string endpoint("http://127.0.0.1:7777");

  std::vector<iota::DeviceToBeAdded> v_devices;

  v_devices.push_back(iota::DeviceToBeAdded(device_1,endpoint));
  v_devices.push_back(iota::DeviceToBeAdded(device_2,endpoint));

  std::string response(manager_service.post_multiple_devices(v_devices,"s1",
                       "/ss1",""));

  std::cout << "Test: testMultiplePostsWithResponse: result: "<< response<<
            std::endl;

  CPPUNIT_ASSERT(
    !response.empty());  //<- I have to change this and use a proper assertion
  sleep(2);
  http_mock->stop();
}

/**
There's something wrong with the protocol provisioning. Check again once Fago has tested his code.
*/
/*
void AdminManagerTest::testPostJSONDevices() {

  std::string
  s1_d("{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://10.95.213.36:1026\","
       "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"http://127.0.0.1:"
       +boost::lexical_cast<std::string>(wserver->get_port())+"\","
       "\"protocol\":\"PDI-IoTA-UltraLight\",\"service\": \"s1\",\"service_path\":\"/ss1\"}");

  iota::ServiceMgmtCollection table1;
  iota::DeviceCollection table_device;

  table1.createTableAndIndex();
  table_device.createTableAndIndex();

  mongo::BSONObj all;

  table1.remove(all);

  iota::Device borrar("","");

  table_device.removed(borrar);

  table1.insert(mongo::fromjson(s1_d));

  //only one endpoint will be added.

  iota::AdminManagerService manager_service;

  std::string response;
  pion::http::response http_response;

  std::cout << "Test testPostJSONDevices STARTING" << std::endl;
  manager_service.post_device_json("s1","/ss1",devices,http_response,
                                   response,"");

  std::cout << "Result " << response << std::endl;

  CPPUNIT_ASSERT(http_response.get_status_code() == 200);
  CPPUNIT_ASSERT(!response.empty());

  //Now checking if the device has been added to the collection.
  iota::Device q1("device_id_post", "s1");
  q1._service_path.assign("/ss1");
  int code = table_device.findd(q1);
  std::cout << "DEVICE FOUND?: " << (code < 0?"NO":"YES") << std::endl;
  CPPUNIT_ASSERT(code > 0);
  std::cout << "Test testPostJSONDevices DONE" << std::endl;

}*/

int AdminManagerTest::http_test(const std::string& uri,
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


void AdminManagerTest::testProtocol_ServiceManagement(){
  std::cout << "START @UT@START testProtocol" << std::endl;
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

  std::cout << "@UT@POST reregister POST_PROTOCOLS2_RERE" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "POST", "", "",
                       "application/json",
                       POST_PROTOCOLS2_RERE, headers, "", response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);


  std::cout << "@UT@GET" << std::endl;
  code_res = http_test(URI_PROTOCOLS, "GET", "", "",
                       "application/json", "",
                       headers, "", response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == GET_RESPONSE_CODE);
  IOTASSERT_MESSAGE("only return one",
                    response.find("\"count\": 2,\"protocols\"")!= std::string::npos);

  std::cout << "@UT@GET services" << std::endl;
  code_res = http_test(URI_SERVICES_MANAGEMET , "GET", "service2", "/*",
                           "application/json", "",
                           headers, query_string, response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == GET_RESPONSE_CODE);
  IOTASSERT_MESSAGE("no modified data count ",
                    response.find("\"count\": 3,\"services\"")!= std::string::npos);
  IOTASSERT_MESSAGE("no modified data new service",
                    response.find("/ssrv2re")!= std::string::npos);
  IOTASSERT_MESSAGE("no modified data reregister",
                    response.find("token2rere")!= std::string::npos);

  std::cout << "@UT@ no delete for protocols, by now " << std::endl;

  std::cout << "END@UT@ testProtocol" << std::endl;

  srand(time(NULL));
  std::cout << "START @UT@START testServiceManagement" << std::endl;

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
                    response.find("\"count\": 2,\"services\"")!= std::string::npos);

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
                    response.find("\"count\": 4,\"services\"")!= std::string::npos);

  std::cout << "@UT@DELETE " << service << std::endl;
  code_res = http_test(URI_SERVICES_MANAGEMET, "DELETE", service, "/*",
                       "application/json", "",
                       headers, query_string, response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  //TODO IOTASSERT(code_res == DELETE_RESPONSE_CODE);

  std::cout << "@UT@GET" << std::endl;
  code_res = http_test(URI_SERVICES_MANAGEMET, "GET", service, "/*",
                       "application/json", "",
                       headers, "", response);
  boost::algorithm::trim(response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  //TODO IOTASSERT(code_res == 200);
  //TODO IOTASSERT(response.compare("{ \"count\": 0,\"services\": []}") == 0);


  std::cout << "END@UT@ testServiceManagement" << std::endl;

}

void AdminManagerTest::testBADServiceManagement() {
  srand(static_cast<unsigned int>(time(0)));
  std::cout << "START @UT@START testBADServiceManagement" << std::endl;
  std::map<std::string, std::string> headers;
  std::string query_string;

  int code_res;
  std::string response;
  std::string service= "service" ;
  service.append(boost::lexical_cast<std::string>(rand()));
  std::cout << "@UT@service " << service << std::endl;

  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG_MONGO);

  //no existe servicio al hacer POST de device
  std::cout << "@UT@1POST" << std::endl;
  code_res = http_test(URI_SERVICES_MANAGEMET, "POST", service, "", "application/json",
                       POST_BAD_SERVICE_MANAGEMENT1, headers, "", response);
  std::cout << "@UT@1RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == 400);
  IOTASSERT(response.compare(
  "{\"reason\":\"The request is not well formed\",\"details\":\"No exists protocol no_exists\"}") == 0);

  std::cout << "END@UT@ testBADServiceManagement" << std::endl;
}
