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

//#define  PATH_CONFIG "../../tests/iotagent/config_mongo.json"

using ::testing::Return;
using ::testing::NotNull;
using ::testing::StrEq;
using ::testing::_;
using ::testing::Invoke;

iota::AdminService* AdminService_ptr;
CPPUNIT_TEST_SUITE_REGISTRATION(AdminManagerTest);


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

  adm = new iota::AdminService();
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
