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


#include <cppunit/extensions/HelperMacros.h>

#include <cmath>
#include <ctime>

//#define  PATH_CONFIG "../../tests/iotagent/config_mongo.json"

using ::testing::Return;
using ::testing::NotNull;
using ::testing::StrEq;
using ::testing::_;
using ::testing::Invoke;

CPPUNIT_TEST_SUITE_REGISTRATION(AdminManagerTest);


AdminManagerTest::AdminManagerTest() {

  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;

}

AdminManagerTest::~AdminManagerTest() {
  // cb_mock.stop();
}

void AdminManagerTest::setUp() {

  iota::Configurator::initialize("../../tests/iotagent/config_mongo.json");
  iota::ServiceMgmtCollection table1;

    table1.createTableAndIndex();

    mongo::BSONObj all;
    table1.remove(all);

    std::string s1_d("{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://10.95.213.36:1026\","
                   "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"host1\","
                   "\"protocol\":\"UL20\",\"service\": \"s1\",\"service_path\":\"/ss1\"}");

    std::string s1_d_host2("{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://10.95.213.36:1026\","
                   "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"host2\","
                   "\"protocol\":\"UL20\",\"service\": \"s1\",\"service_path\":\"/ss1\"}");

    std::string s1_d2("{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://10.95.213.36:1026\","
                   "\"entity_type\":\"thing\",\"resource\":\"/iot/d2\",\"iotagent\":\"host1\","
                   "\"protocol\":\"UL20\",\"service\": \"s1\",\"service_path\":\"/ss1\"}");

    std::string s1_mqtt("{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://10.95.213.36:1026\","
                   "\"entity_type\":\"thing\",\"resource\":\"/iot/mqtt\",\"iotagent\":\"host1\","
                   "\"protocol\":\"MQTT\",\"service\": \"s1\",\"service_path\":\"/ss1\"}");

    std::string s2_d("{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://10.95.213.36:1026\","
                   "\"entity_type\":\"thing\",\"resource\":\"/iot/d\",\"iotagent\":\"host1\","
                   "\"protocol\":\"UL20\",\"service\": \"s1\",\"service_path\":\"/ss2\"}");

    std::string s3_mqtt("{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://10.95.213.36:1026\","
                   "\"entity_type\":\"thing\",\"resource\":\"/iot/mqtt\",\"iotagent\":\"host1\","
                   "\"protocol\":\"MQTT\",\"service\": \"s3\",\"service_path\":\"/ss3\"}");

    std::string s4_agus("{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://10.95.213.36:1026\","
                   "\"entity_type\":\"thing\",\"resource\":\"/iot/fake\",\"iotagent\":\"http://127.0.0.1:7777/iot\","
                   "\"protocol\":\"UL20\",\"service\": \"s4_agus\",\"service_path\":\"/ss3\"}");


    std::cout << "inserts" << std::endl;
    table1.insert(mongo::fromjson(s1_d));
    table1.insert(mongo::fromjson(s1_d_host2));
    table1.insert(mongo::fromjson(s1_d2));
    table1.insert(mongo::fromjson(s1_mqtt));
    table1.insert(mongo::fromjson(s2_d));
    table1.insert(mongo::fromjson(s3_mqtt));
    table1.insert(mongo::fromjson(s4_agus));

}

void AdminManagerTest::tearDown() {
  // delete cbPublish; //Already deleted inside MqttService.

}


void AdminManagerTest::testDeviceToBeAdded(){

  iota::DeviceToBeAdded test("protocol","device_json");
  iota::DeviceToBeAdded test2 = test;

  CPPUNIT_ASSERT(test == test2);
}

void AdminManagerTest::testGetEndpointsFromDevices()
{
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

  std::cout << "Test: testGetEndpointsFromDevices: result: ["<< v_endpoints_devices.size() << "] endpoints" << std::endl;
  CPPUNIT_ASSERT (v_endpoints_devices.size() == 4);
  for (int i=0;i<v_endpoints_devices.size(); i++){
    std::cout << "Test: host: " << v_endpoints_devices[i].get_endpoint() << std::endl;
  }
  CPPUNIT_ASSERT (v_endpoints_devices[0].get_endpoint().compare("host1")==0);
  CPPUNIT_ASSERT (v_endpoints_devices[1].get_endpoint().compare("host2")==0);
  CPPUNIT_ASSERT (v_endpoints_devices[2].get_endpoint().compare("host1")==0);
  CPPUNIT_ASSERT (v_endpoints_devices[3].get_endpoint().compare("host2")==0);
  std::cout << "Test: END" << std::endl;

}

void AdminManagerTest::testGetDevices() {
  std::cout << "START testGetDevices" << std::endl;
  boost::shared_ptr<HttpMock> http_mock;
  http_mock.reset(new HttpMock(7777, "/iot/devices", false));
  http_mock->init();
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
  manager_service.get_devices(http_request, args, query, http_response, response);
  CPPUNIT_ASSERT(http_response.get_status_code() == 200);
  std::cout << http_response.get_content() << std::endl;
  std::cout << "STOP testGetDevices" << std::endl;

  sleep(10);
}

