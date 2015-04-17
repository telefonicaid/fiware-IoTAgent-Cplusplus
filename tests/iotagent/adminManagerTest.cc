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

#include <cmath>
#include <ctime>

//#define  PATH_CONFIG "../../tests/iotagent/config_mongo.json"

using ::testing::Return;
using ::testing::NotNull;
using ::testing::StrEq;
using ::testing::_;
using ::testing::Invoke;

iota::AdminService* AdminService_ptr;
CPPUNIT_TEST_SUITE_REGISTRATION(AdminManagerTest);

std::string
s1_d("{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://10.95.213.36:1026\","
     "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"http://127.0.0.1:7777\","
     "\"protocol\":\"UL20\",\"service\": \"s1\",\"service_path\":\"/ss1\"}");

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
AdminManagerTest::AdminManagerTest() {
  iota::Configurator::initialize("../../tests/iotagent/config_mongo.json");
  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  cleanDB();
  /*pion::process::initialize();

  adm = new iota::AdminService();
  adm->set_manager();



  AdminService_ptr = adm;
  AdminService_ptr->add_service("/iot/res", AdminService_ptr);
  wserver.reset(new pion::http::plugin_server(scheduler));
  wserver->add_service("/iot", adm);
  wserver->start();

   std::string service("s1");
  std::string service_path("/ss1");
  pion::http::response http_response;
  std::string response;
  std::string service_s1("{\"services\": [{"
                        "\"apikey\": \"apikey\",\"token\": \"token\","
                        "\"cbroker\": \"http://cbroker\",\"entity_type\": \"thing\",\"resource\": \"/iot/d\"}]}");
  adm->post_service_json(service,service_path,service_s1,http_response,response);
  */
}

void AdminManagerTest::cleanDB() {

  iota::ServiceCollection table_service;

  table_service.createTableAndIndex();

  mongo::BSONObj all;

  table_service.remove(all);
}


AdminManagerTest::~AdminManagerTest() {


  cleanDB();
  // wserver->stop();
  // scheduler.shutdown();
}

void AdminManagerTest::setUp() {

/*
  iota::ServiceCollection table1;

  table1.createTableAndIndex();
  mongo::BSONObj all = BSON("service" << "s1");

  table1.remove(all);

  std::cout << "inserts" << std::endl;
  table1.insert(mongo::fromjson(s1_d));
  table1.insert(mongo::fromjson(s1_d_host2));
  table1.insert(mongo::fromjson(s1_d2));
  table1.insert(mongo::fromjson(s1_mqtt));
  table1.insert(mongo::fromjson(s2_d));
  table1.insert(mongo::fromjson(s3_mqtt));
*/
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
  boost::asio::io_service io_service;
  iota::AdminManagerService manager_service(io_service);

  std::cout << "Test: testGetEndpointsFromDevices... starting" << std::endl;


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
  CPPUNIT_ASSERT(v_endpoints_devices.size() == 6);
  for (int i=0; i<v_endpoints_devices.size(); i++) {
    std::cout << "Test: host: " << v_endpoints_devices[i].get_endpoint() <<
              std::endl;
  }
  // Following asserts have to change
  /*
  CPPUNIT_ASSERT (v_endpoints_devices[0].get_endpoint().compare("host1")==0);
  CPPUNIT_ASSERT (v_endpoints_devices[1].get_endpoint().compare("host2")==0);
   CPPUNIT_ASSERT (v_endpoints_devices[2].get_endpoint().compare("host1")==0);
   CPPUNIT_ASSERT (v_endpoints_devices[3].get_endpoint().compare("host2")==0);
   */

  std::cout << "Test: END" << std::endl;

}

void AdminManagerTest::testAddDevicesToEndpoints() {

  boost::shared_ptr<HttpMock> http_mock;
  http_mock.reset(new HttpMock(7777, "/iot/devices", false));
  http_mock->init();

  pion::one_to_one_scheduler scheduler;
  scheduler.startup();

  iota::AdminManagerService manager_service(scheduler.get_io_service());
  std::cout << "testAddDevicesToEndpoints: STARTING... " << std::endl;
  std::map<std::string, std::string> h;
  http_mock->set_response(204, "{}", h);

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
  CPPUNIT_ASSERT(res == 204);
  //sleep(4);
  //CLEAN UP
  sleep(4);
  http_mock->stop();
}

void AdminManagerTest::testGetDevices() {
  std::cout << "START testGetDevices" << std::endl;
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
  std::map<std::string, std::string> h;
  // Two endpoints. Repeat response for test
  http_mock->set_response(200, mock_response, h);
  http_mock->set_response(200, mock_response, h);
  std::string mock_port = boost::lexical_cast<std::string>(http_mock->get_port());
  pion::one_to_one_scheduler scheduler;
  scheduler.startup();

  iota::AdminManagerService manager_service(scheduler.get_io_service());
  pion::http::request_ptr http_request(new pion::http::request("/"));
  http_request->add_header("Fiware-Service", "s4_agus");
  http_request->add_header("Fiware-ServicePath", "/ss3");
  http_request->add_header("X-Trace-Message", "12345");
  http_request->set_method("GET");
  std::map<std::string, std::string> args;
  std::multimap<std::string, std::string> query;
  query.insert(std::pair<std::string, std::string>("protocol", "UL20"));
  pion::http::response http_response;
  std::string response;
  manager_service.get_devices(http_request, args, query, "s4_agus", "/ss3", 0, 0,
                              "on", "", http_response, response);
  CPPUNIT_ASSERT_MESSAGE("Expected 4 devices ",
                         response.find("\"count\" : 4") != std::string::npos);

  std::cout << "STOP testGetDevices" << std::endl;
  sleep(4);
  http_mock->stop();

  table1.remove(BSON("service" << "s4_agus"));
  //table1.remove(BSON("service" << BSON( "$in" << BSON_ARRAY("s1" << "s6"))));
}

void AdminManagerTest::testMultiplePostsWithResponse() {
  //boost::asio::io_service io_service;

  boost::shared_ptr<HttpMock> http_mock;
  http_mock.reset(new HttpMock(7777, "/iot/devices", false));
  http_mock->init();
  pion::one_to_one_scheduler scheduler;
  scheduler.startup();

  iota::AdminManagerService manager_service(scheduler.get_io_service());

  std::map<std::string, std::string> h;
  // Two endpoints. Repeat response for test
  http_mock->set_response(204, "{}", h);
  http_mock->set_response(204, "{}", h);

  //NOT USED
  std::string
  devices("{\"devices\":[{\"protocol\":\"UL20\",\"device_id\": \"device_id\",\"entity_name\": \"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": \"http://device_endpoint\",\"timezone\": \"America/Santiago\","
          "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": \"device_id@ping|%s\" }],"
          "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }],"
          "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
          "}]}");



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
  sleep(5);
  http_mock->stop();
}
