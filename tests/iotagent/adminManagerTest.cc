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


    std::cout << "inserts" << std::endl;
    table1.insert(mongo::fromjson(s1_d));
    table1.insert(mongo::fromjson(s1_d_host2));
    table1.insert(mongo::fromjson(s1_d2));
    table1.insert(mongo::fromjson(s1_mqtt));
    table1.insert(mongo::fromjson(s2_d));
    table1.insert(mongo::fromjson(s3_mqtt));

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
  iota::AdminManagerService manager_service;

  std::cout << "Test: testGetEndpointsFromDevices... starting" << std::endl;


  std::vector<iota::DeviceToBeAdded> v_endpoints;
  std::string devices("{\"devices\": "
                       "[{\"protocol\":\"UL20\",\"device_id\": \"device_id\",\"entity_name\": \"entity_name\",\"entity_type\": \"entity_type\",\"endpoint\": \"htp://device_endpoint\",\"timezone\": \"America/Santiago\","
                       "\"commands\": [{\"name\": \"ping\",\"type\": \"command\",\"value\": \"device_id@ping|%s\" }],"
                       "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }],"
                       "\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
                       "}]}");

  manager_service.resolve_endpoints(v_endpoints,devices,"s1","/ss1");

  //Expected, "host1" and "host2"

  std::cout << "Test: testGetEndpointsFromDevices: result: ["<< v_endpoints.size() << "] endpoints" << std::endl;
  CPPUNIT_ASSERT (v_endpoints.size() == 2);
  CPPUNIT_ASSERT (v_endpoints[0].get_endpoint().compare("host1")==0);
  CPPUNIT_ASSERT (v_endpoints[1].get_endpoint().compare("host2")==0);
  std::cout << "Test: END" << std::endl;

}

