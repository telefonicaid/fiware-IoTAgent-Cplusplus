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
#include "mongoTest.h"

#include "services/admin_service.h"

#include "util/collection.h"
#include "util/command_collection.h"
#include "util/device_collection.h"
#include "util/protocol.h"
#include "util/protocol_collection.h"
#include "util/service_mgmt_collection.h"
#include "util/alarm.h"
#include "util/iota_exception.h"
#include <boost/property_tree/ptree.hpp>
#include <ctime>
#include <iostream>
#include <boost/chrono/thread_clock.hpp>
#include "util/mongo_connection.h"

#include "util/device.h"
#include "util/command.h"

#include <iostream>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <boost/property_tree/json_parser.hpp>

#define PATH_CONFIG "../../tests/iotagent/config_mongo.json"
#define PATH_BAD_CONFIG "../../tests/iotagent/config_bad_mongo.json"
#define PATH_REPLICA_CONFIG "../../tests/iotagent/config_mongo_replica.json"
#define PATH_NO_MONGO_CONFIG "../../tests/iotagent/config_no_mongo.json"

CPPUNIT_TEST_SUITE_REGISTRATION(MongoTest);
namespace iota {
std::string logger("main");
std::string URL_BASE = "/iot";
}

void MongoTest::setUp() {
  std::cout << "setUp mongoTest " << std::endl;

  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
}

void MongoTest::tearDown() { std::cout << "tearDown mongoTest " << std::endl; }

void MongoTest::testGenericCollection() {
  std::cout << "START testGenericCollection" << std::endl;
  iota::Configurator::initialize(PATH_CONFIG);

  const iota::JsonValue& storage =
      iota::Configurator::instance()->get(iota::store::types::STORAGE);

  std::string dbname;
  if (storage.HasMember(iota::store::types::DBNAME.c_str())) {
    dbname.assign(storage[iota::store::types::DBNAME.c_str()].GetString());
  }

  std::cout << "testGenericCollection2" << std::endl;
  iota::Collection table1("PRUEBA");

  mongo::BSONObj p = BSON("name"
                          << "Joe"
                          << "desc"
                          << "ssss");
  std::cout << "before insert" << std::endl;
  table1.insert(p);

  iota::Collection q1("PRUEBA");
  mongo::BSONObj p2 = BSON("name"
                           << "Joe");
  std::cout << "before find" << std::endl;
  int code_res = q1.find(p2);
  CPPUNIT_ASSERT_MESSAGE("no inserted data", q1.more());
  mongo::BSONObj r1 = q1.next();

  std::string name = r1.getStringField("name");
  std::cout << "Object " << r1 << std::endl;
  CPPUNIT_ASSERT_MESSAGE("no inserted data", name.compare("Joe") == 0);

  std::cout << "before remove" << std::endl;
  q1.remove(p2);

  std::cout << "before count" << std::endl;
  int num = q1.count(p2);
  CPPUNIT_ASSERT_MESSAGE("remove error", num == 0);

  mongo::BSONObj pe = BSON("name"
                           << "Pepe"
                           << "desc"
                           << "Botella");
  q1.insert(pe);

  mongo::BSONObj no = BSON("name"
                           << "Pepe");
  mongo::BSONObj ap = BSON("desc"
                           << "Lopez");
  std::cout << "before update" << std::endl;
  q1.update(no, ap);
  std::cout << "before find with bson_fields" << std::endl;
  mongo::BSONObjBuilder bson_fields;
  bson_fields.append("_id", 0);
  code_res = q1.find(no, bson_fields);

  CPPUNIT_ASSERT_MESSAGE("no inserted data", q1.more());
  mongo::BSONObj r2 = q1.next();
  std::cout << "FDFDFD " << r2 << std::endl;
  std::string desc = r2.getStringField("desc");
  std::cout << "desc:" << desc << std::endl;
  CPPUNIT_ASSERT_MESSAGE("no inserted data", desc.compare("Lopez") == 0);

  // Checking if _id is returned
  std::cout << "Object returned " << r2 << std::endl;
  CPPUNIT_ASSERT_MESSAGE("Checking if _id is returned",
                         r2.getField("_id").eoo() == true);

  std::cout << "before remove" << std::endl;
  q1.remove(no);
  num = q1.count(no);
  CPPUNIT_ASSERT_MESSAGE("remove error", num == 0);

  std::cout << "END testGenericCollection " << std::endl;
}

void MongoTest::testCommandCollection() {
  std::cout << "@UT@START testCommandCollection" << std::endl;
  iota::Configurator::initialize(PATH_CONFIG);
  iota::CommandCollection table1;

  table1.createTableAndIndex();

  std::cout << "@UT@remove all" << std::endl;
  iota::Command all("", "", "");
  table1.remove(all);

  boost::property_tree::ptree pt;
  pt.put("body", "command");
  pt.put("kkdlvaca", "kkdlvaca");

  iota::Command p("nodo", "service", "/");
  p.set_command(pt);
  p.set_entity_type("entity_type");
  p.set_id("id0");
  p.set_name("name");
  p.set_sequence("sequence");
  p.set_status(0);
  p.set_timeout(22);
  p.set_uri_resp("uri_resp");
  std::cout << "@UT@insert1" << std::endl;
  table1.insert(p);

  iota::Command p2("nodo2", "service2", "/");
  p2.set_command(pt);
  p2.set_entity_type("entity_type");
  p2.set_id("id2");
  p2.set_name("name");
  p2.set_sequence("sequence");
  p2.set_status(0);
  p2.set_timeout(22);
  p2.set_uri_resp("uri_resp");
  std::cout << "@UT@insert2" << std::endl;
  table1.insert(p2);

  iota::CommandCollection table2;
  iota::Command q1("nodo", "service", "/");
  int code_res = table2.find(q1);
  CPPUNIT_ASSERT_MESSAGE("no inserted data", table2.more());
  iota::Command r1 = table2.next();

  std::string name = r1.get_name();
  std::cout << "name:" << name << std::endl;
  CPPUNIT_ASSERT_MESSAGE("no inserted data", name.compare("name") == 0);
  std::cout << "@UT@check get_command" << std::endl;
  boost::property_tree::ptree ptcommand = r1.get_command();

  boost::property_tree::json_parser::write_json(std::cout, ptcommand);
  std::string body =   ptcommand.get<std::string>("body", "");
  CPPUNIT_ASSERT_MESSAGE("no inserted data", body.compare("command") == 0);
  std::string body2 =   ptcommand.get<std::string>("kkdlvaca", "");
  CPPUNIT_ASSERT_MESSAGE("no inserted data", body2.compare("kkdlvaca") == 0);
  CPPUNIT_ASSERT_MESSAGE("more data", table2.more() == false);

  std::cout << "@UT@remove" << std::endl;
  table1.remove(p2);
  table1.remove(p);

  int num = table1.count(p2);
  CPPUNIT_ASSERT_MESSAGE("remove error", num == 0);
  std::cout << "END testCommandCollection " << std::endl;
}

void MongoTest::testDeviceCollection() {
  std::cout << "START testDeviceCollection" << std::endl;
  iota::Configurator::initialize(PATH_CONFIG);
  iota::DeviceCollection table1;
  std::string location_body =
      "{\"name\": \"position\", \"type\": \"coords\", \"metadatas\": "
      "[{\"name\": \"location\",\"type\": \"string\",\"value\": \"WGS84\"}]}";

  table1.createTableAndIndex();

  iota::Device all("", "");
  table1.removed(all);

  iota::Device p("Joe", "service1");
  p._service_path = "service_path";
  p._entity_type = "_entity_type";
  p._entity_name = "_entity_name";
  p._endpoint = "_endpoint";
  p._timezone = "timezone";

  // comando1 ping
  p._commands.insert(
      std::pair<std::string, std::string>("ping", "ping_command"));
  p._commands.insert(std::pair<std::string, std::string>("set", "set_command"));

  // attribute1 t
  p._attributes.insert(std::pair<std::string, std::string>(
      "t", "{\"name\": \"temperature\", \"type\": \"int\"}"));
  p._attributes.insert(std::pair<std::string, std::string>(
      "h", "{\"name\": \"humedity\", \"type\": \"%\"}"));
  p._attributes.insert(std::pair<std::string, std::string>("l", location_body));

  // static attribute1 model
  p._static_attributes.insert(std::pair<std::string, std::string>(
      "model",
      "{\"name\": \"att_name\",\"type\": \"xxxx\",\"value\": \"value\"}"));
  p._static_attributes.insert(std::pair<std::string, std::string>(
      "uuid",
      "{\"name\": \"att_name2\",\"type\": \"xxxx\",\"value\": \"value\"}"));

  std::cout << "insert with props" << std::endl;
  table1.insertd(p);

  // ponemos registrationid
  std::cout << "@UT@UPDATE registration" << std::endl;
  iota::Device query("Joe", "service1");
  query._service_path = "service_path";
  iota::Device sett("", "");
  sett._registration_id = "registration_id";
  sett._duration_cb = 666;
  table1.updated(query, sett);

  iota::DeviceCollection table2;
  iota::Device q1("Joe", "service1");
  int code_res = table2.findd(q1);
  CPPUNIT_ASSERT_MESSAGE("no inserted data", table2.more());
  iota::Device r1 = table2.nextd();
  CPPUNIT_ASSERT_MESSAGE("no commands", r1._commands.size() == 2);
  CPPUNIT_ASSERT_MESSAGE("no attributes", r1._attributes.size() == 3);
  std::string location = r1._attributes["l"];
  std::cout << "@UT@LOCATION " << location << std::endl;
  // CPPUNIT_ASSERT_MESSAGE("bad location", location.compare(location_body) !=
  // std::string::npos);
  CPPUNIT_ASSERT_MESSAGE("no _registration_id",
                         r1._registration_id.compare("registration_id") == 0);
  CPPUNIT_ASSERT_MESSAGE("no _duration_cb", r1._duration_cb == 666);

  CPPUNIT_ASSERT_MESSAGE("no static attributes",
                         r1._static_attributes.size() == 2);

  std::string name = r1._name;
  std::cout << "name:" << name << std::endl;
  CPPUNIT_ASSERT_MESSAGE("no inserted data", name.compare("Joe") == 0);

  table2.removed(q1);

  int num = table2.countd(q1);
  CPPUNIT_ASSERT_MESSAGE("remove error", num == 0);

  std::cout << "create a device with update" << std::endl;
  iota::DeviceCollection device_table;
  iota::Device device_query("name_update", "service_update");
  device_query._service_path = "/srv_path";
  device_query._protocol = "protocol";
  iota::Device device_timestamp("", "");
  device_timestamp._timestamp_data = 123456789;
  device_table.updated(device_query, device_timestamp, true);

  code_res = table2.findd(device_query);
  CPPUNIT_ASSERT_MESSAGE("no inserted data", table2.more());
  r1 = table2.nextd();
  CPPUNIT_ASSERT_MESSAGE("no timestam", r1._timestamp_data == 123456789);

  table2.removed(device_query);
  num = table2.countd(device_query);
  CPPUNIT_ASSERT_MESSAGE("remove error", num == 0);

  std::cout << "END testDeviceCollection " << std::endl;
}

void MongoTest::testNoMongo() {
  std::cout << "START testNoMongo" << std::endl;
  iota::Configurator::initialize(PATH_BAD_CONFIG);
  try {
    iota::MongoConnection::instance()->reconnect();

    const iota::JsonValue& storage =
        iota::Configurator::instance()->get(iota::store::types::STORAGE);

    std::cout << "testGenericCollection2" << std::endl;
    iota::Collection table1("PRUEBA");
    CPPUNIT_ASSERT_MESSAGE("no exception mongo", true);
  } catch (std::exception e) {
    std::cout << "throw exception:" << e.what() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("alarm not found",
                           iota::Alarm::instance()->size() == 1);
  }

  std::cout << "END testNoMongo " << std::endl;
}

/**
 *  export variables for multi threads
 *  UNIT_TEST_THREADS  number of threads
 *  UNIT_TEST_MILLIS   miliseconds  waiting between  threads starting
 *  UNIT_TEST_ACTIONS  numbers os operations with the same connection
 **/
void MongoTest::testReplica() {
  std::cout << "START testReplica" << std::endl;
  std::clock_t start;
  start = std::clock();

  iota::Configurator::initialize(PATH_REPLICA_CONFIG);
  int num_threads = 1;
  const char* val = ::getenv("UNIT_TEST_THREADS");
  if (val != 0) {
    num_threads = boost::lexical_cast<int>(val);
  }
  int milis_threads = 1;
  const char* valmilis = ::getenv("UNIT_TEST_MILLIS");
  if (valmilis != 0) {
    milis_threads = boost::lexical_cast<int>(valmilis);
  }

  boost::thread_group g;
  std::cout << "nonum threads:" << num_threads << " milis: " << milis_threads
            << std::endl;
  for (int i = 0; i < num_threads; i++) {
    g.add_thread(new boost::thread(MongoTest::workerFunc));
    boost::this_thread::sleep(boost::posix_time::milliseconds(milis_threads));
  }

  g.join_all();
  std::cout << "Time testReplica: "
            << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms"
            << std::endl;
  std::cout << "END testReplica" << std::endl;
}

void MongoTest::testMongoAlone() {
  std::cout << "START testMongoAlone" << std::endl;
  std::clock_t start;
  start = std::clock();

  iota::Configurator::initialize(PATH_CONFIG);
  int num_threads = 1;
  const char* val = ::getenv("UNIT_TEST_THREADS");
  if (val != 0) {
    num_threads = boost::lexical_cast<int>(val);
  }
  int milis_threads = 1;
  const char* valmilis = ::getenv("UNIT_TEST_MILLIS");
  if (valmilis != 0) {
    milis_threads = boost::lexical_cast<int>(valmilis);
  }
  iota::Collection table1("PRUEBA");

  mongo::BSONObj p = BSON("name"
                          << "Inicio prueba testMongoAlone");
  table1.insert(p);

  boost::thread_group g;
  std::cout << "nonum threads:" << num_threads << " milis: " << milis_threads
            << std::endl;
  for (int i = 0; i < num_threads; i++) {
    g.add_thread(new boost::thread(MongoTest::workerFunc));
    boost::this_thread::sleep(boost::posix_time::milliseconds(milis_threads));
  }

  g.join_all();
  std::cout << "Time testMongoAlone: "
            << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms"
            << std::endl;
  std::cout << "END testMongoAlone" << std::endl;
}

void MongoTest::workerFunc() {
  std::cout << "START workerFunc" << std::endl;
  std::clock_t start;
  start = std::clock();

  int num_actions_inserts = 1;
  const char* val = ::getenv("UNIT_TEST_ACTIONS_INSERTS");
  if (val != 0) {
    num_actions_inserts = boost::lexical_cast<int>(val);
  }
  int num_actions_find = 1;
  val = ::getenv("UNIT_TEST_ACTIONS_FIND");
  if (val != 0) {
    num_actions_find = boost::lexical_cast<int>(val);
  }
  int num_actions_delete = 1;
  val = ::getenv("UNIT_TEST_ACTIONS_DELETE");
  if (val != 0) {
    num_actions_delete = boost::lexical_cast<int>(val);
  }

  int num_actions_col = 1;
  val = ::getenv("UNIT_TEST_ACTIONS_COL");
  if (val != 0) {
    num_actions_col = boost::lexical_cast<int>(val);
  }

  try {
    for (int j = 0; j < num_actions_col; j++) {
      iota::Collection table1("PRUEBA");

      mongo::BSONObj p = BSON("name"
                              << "Joe"
                              << "desc"
                              << "ssss");
      std::cout << "before insert" << std::endl;
      for (int i = 0; i < num_actions_inserts; i++) {
        table1.insert(p);
      }

      for (int i = 0; i < num_actions_find; i++) {
        iota::Collection q1("PRUEBA");
        mongo::BSONObj p2 = BSON("name"
                                 << "Joe");
        std::cout << "before find" << std::endl;
        int code_res = q1.find(p2);
        CPPUNIT_ASSERT_MESSAGE("no inserted data", q1.more());
        if (q1.more()) {
          q1.next();
        }
      }

      for (int i = 0; i < num_actions_delete; i++) {
        iota::Collection q1("PRUEBA");
        mongo::BSONObj p2 = BSON("name"
                                 << "Joe");
        int code_res = q1.find(p2);
        CPPUNIT_ASSERT_MESSAGE("no inserted data", q1.more());
        if (q1.more()) {
          mongo::BSONObj o = q1.next();
          std::cout << "before remove" << o.jsonString() << std::endl;
          q1.remove(o);
        }
      }
    }
  } catch (std::exception exc) {
    std::cout << "ERROR " << exc.what() << std::endl;
  }
  std::cout << "Time workerFunc: "
            << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms"
            << std::endl;
  std::cout << "END workerFunc " << std::endl;
}

void MongoTest::testException() {
  std::cout << "START testException" << std::endl;
  iota::Configurator::initialize(PATH_CONFIG);

  iota::Collection table1("PRUEBA");
  iota::Collection tableDev("DEVICE");

  // remove all data
  mongo::BSONObj all;
  table1.remove(all);

  mongo::BSONObj p = BSON("_id"
                          << "Joe"
                          << "desc"
                          << "ssss");
  std::cout << "before insert" << std::endl;
  table1.insert(p);

  iota::Collection q1("PRUEBA");
  mongo::BSONObj p2 = BSON("_id"
                           << "Joe");
  std::cout << "before find" << std::endl;
  int code_res = q1.find(p2);
  CPPUNIT_ASSERT_MESSAGE("no inserted data", q1.more());

  // object DUPLICATED insert
  try {
    std::cout << "before insert with duplicated" << std::endl;
    table1.insert(p);
    CPPUNIT_ASSERT_MESSAGE("no exception with duplicate", false);
  } catch (iota::IotaException e) {
    std::cout << "IotaException" << e.what() << std::endl;
    std::cout << "IotaException" << e.status() << "|" << e.reason()
              << std::endl;
    CPPUNIT_ASSERT_MESSAGE("no code duplicate", e.status() == 409);
    CPPUNIT_ASSERT_MESSAGE("no message duplicate", e.reason().compare(""));
  }

  // object NO exists update
  std::cout << "before NO exists update" << std::endl;
  mongo::BSONObj q = BSON("_id"
                          << "Joeee");
  mongo::BSONObj pdata = BSON("data"
                              << "Joeee");
  int n = table1.update(q, pdata);
  std::cout << "num update " << n << std::endl;
  CPPUNIT_ASSERT_MESSAGE("no updates", n == 0);

  // BAD BSON

  std::cout << "END testException" << std::endl;
}

void MongoTest::testProtocolCollection() {
  std::cout << "START testProtocolCollection" << std::endl;
  iota::Configurator::initialize(PATH_CONFIG);
  iota::ProtocolCollection table1;

  table1.createTableAndIndex();

  iota::Protocol all("");
  table1.remove(all);

  iota::Protocol p("protocol1");
  iota::Protocol::resource_endpoint endp1;
  endp1.endpoint = "endpoint1";
  endp1.resource = "resource1";
  p.add(endp1);

  p.set_description("descripcion p1");

  std::cout << "insert1" << std::endl;
  table1.insert(p);

  iota::Protocol p2("protocol2");
  iota::Protocol::resource_endpoint endp2;
  endp2.endpoint = "endpoint2";
  endp2.resource = "resource2";

  p2.add(endp2);

  p2.set_name("p2");

  std::cout << "insert2" << std::endl;
  table1.insert(p2);

  // ponemos registrationid
  std::cout << "@UT@UPDATE registration" << std::endl;
  iota::Protocol query("protocol1");

  iota::Protocol sett;
  sett.set_description("descripcion 2");
  iota::Protocol::resource_endpoint endpU;
  endpU.endpoint = "endpointU";
  endpU.resource = "resourceU";
  p.add(endp1);
  table1.update(query, sett);

  iota::ProtocolCollection table2;
  iota::Protocol q1("protocol1");
  int code_res = table2.find(q1);
  CPPUNIT_ASSERT_MESSAGE("no inserted data", table2.more());
  iota::Protocol r1 = table2.next();
  CPPUNIT_ASSERT_MESSAGE("no all endpoints", r1.get_endpoints().size() == 1);

  std::cout << "@UT@Todos los protocolos " << std::endl;
  iota::Protocol all2;
  std::vector<iota::Protocol> protocols = table2.get_all();
  CPPUNIT_ASSERT_MESSAGE("no all endpoints2 ", protocols.size() == 2);

  table2.remove(all2);

  int num = table2.count(q1);
  CPPUNIT_ASSERT_MESSAGE("remove error", num == 0);

  std::cout << "END testProtocolCollection " << std::endl;
}

void MongoTest::testServiceMgmtCollection() {
  std::cout << "START testServiceMgmtCollection" << std::endl;
  iota::Configurator::initialize(PATH_CONFIG);
  iota::ServiceMgmtCollection table1;

  table1.createTableAndIndex();

  mongo::BSONObj all;
  table1.remove(all);

  std::string s1_d(
      "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
      "10.95.213.36:1026\","
      "\"entity_type\":\"thing\",\"resource\":\"/iot/"
      "d\",\"iotagent\":\"host1\","
      "\"protocol\":\"UL20\",\"service\": \"s1\",\"service_path\":\"/ss1\"}");

  std::string s1_d_host2(
      "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
      "10.95.213.36:1026\","
      "\"entity_type\":\"thing\",\"resource\":\"/iot/"
      "d\",\"iotagent\":\"host2\","
      "\"protocol\":\"UL20\",\"service\": \"s1\",\"service_path\":\"/ss1\"}");

  std::string s1_mqtt(
      "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
      "10.95.213.36:1026\","
      "\"entity_type\":\"thing\",\"resource\":\"/iot/"
      "mqtt\",\"iotagent\":\"host1\","
      "\"protocol\":\"MQTT\",\"service\": \"s1\",\"service_path\":\"/ss1\"}");

  std::string s2_d(
      "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
      "10.95.213.36:1026\","
      "\"entity_type\":\"thing\",\"resource\":\"/iot/"
      "d\",\"iotagent\":\"host1\","
      "\"protocol\":\"UL20\",\"service\": \"s1\",\"service_path\":\"/ss2\"}");

  std::string s3_mqtt(
      "{\"apikey\":\"apikey\",\"token\":\"token\",\"cbroker\":\"http://"
      "10.95.213.36:1026\","
      "\"entity_type\":\"thing\",\"resource\":\"/iot/"
      "mqtt\",\"iotagent\":\"host1\","
      "\"protocol\":\"MQTT\",\"service\": \"s3\",\"service_path\":\"/ss3\"}");

  std::cout << "inserts" << std::endl;
  table1.insert(mongo::fromjson(s1_d));
  table1.insert(mongo::fromjson(s1_d_host2));
  table1.insert(mongo::fromjson(s1_mqtt));
  table1.insert(mongo::fromjson(s2_d));
  table1.insert(mongo::fromjson(s3_mqtt));

  std::cout << "@UT@Todos los servicios de protocolo UL20" << std::endl;
  iota::ServiceMgmtCollection table2;
  std::vector<iota::ServiceType> services =
      table2.get_services_by_protocol("UL20");
  for (std::vector<iota::ServiceType>::iterator it = services.begin();
       it != services.end(); ++it) {
    std::cout << it->first << ":" << it->second << std::endl;
  }
  std::cout << "@UT@fin" << std::endl;

  std::cout << "@UT@Todos los iotagentS del servicio s1 /ss1 " << std::endl;
  iota::ServiceMgmtCollection table3;
  std::vector<iota::IotagentType> iotagents =
      table3.get_iotagents_by_service("s1", "/ss1", "UL20");
  int iot_count = 0;
  std::string iot;
  for (std::vector<iota::IotagentType>::iterator it = iotagents.begin();
       it != iotagents.end(); ++it) {
    iot.append(*it);
    std::cout << "|" << iot << "|" << std::endl;
    iot_count++;
  }
  CPPUNIT_ASSERT_MESSAGE("count iotagent", iot_count == 2);
  CPPUNIT_ASSERT_MESSAGE("no found host1",
                         iot.find("host1") != std::string::npos);
  CPPUNIT_ASSERT_MESSAGE("no found host2",
                         iot.find("host2") != std::string::npos);

  std::cout << "@UT@OR" << std::endl;
  int n = table1.count(all);
  std::cout << "En la tabla hay " << n << " eltos " << std::endl;
  table1.remove(BSON("service" << BSON("$in" << BSON_ARRAY("s1"
                                                           << "s3"))));
  int n2 = table1.count(all);
  std::cout << "despues de borrar con OR quedan " << n2 << " eltos "
            << std::endl;

  CPPUNIT_ASSERT_MESSAGE("remove error", n2 == 0);

  std::cout << "END testServiceMgmtCollection " << std::endl;
}

std::string MongoTest::testjoinCommands(const std::string &obj1,
                                        const std::string &obj2) {
  std::string res;

  std::string resobj2;
  std::string::size_type commandit2 =  obj2.find("\"commands\"", 0);
  std::string::size_type commanditfin2, commanditini2;
  bool obj2_commands = commandit2 != std::string::npos;
  if (obj2_commands){
     commanditfin2 =  obj2.find("]", commandit2);
     commanditini2 =  obj2.find("{");
  }else{
     commanditini2 =  obj2.find("{");
     commanditfin2 =  obj2.find_last_of("}");
  }
  if (commanditini2 != std::string::npos && commanditfin2 != std::string::npos){
     resobj2.append(obj2.substr(commanditini2+1, commanditfin2 - commanditini2 -1));
  }

  std::string::size_type commandit =  obj1.find("\"commands\"", 0);
  if (commandit != std::string::npos){
     std::cout << "@UT@ have commands" << std::endl;
     std::string::size_type commanditf =  obj1.find("[", commandit);
     if (commanditf != std::string::npos){
        res = obj1.substr(0, commandit);
        if (!resobj2.empty()){
          res.append(resobj2);
          res.append(",");
        }
        res.append(obj1.substr(commanditf+1));
     }
  }else {
    // no tiene comandos , cerramos el corchete de comandos
    if (obj2_commands){
       resobj2.append("]");
    }
    std::cout << "@UT@ no have commands" << std::endl;
    commandit =  obj1.find("{", 0);
    res.append("{");
    if (!resobj2.empty()){
       res.append(resobj2);
       res.append(",");
    }
    res.append(obj1.substr(commandit+1));
  }

  return res;
}

void MongoTest::testArray() {
  std::cout << "@UT@START testArray" << std::endl;

  {
    std::cout << "@UT@commands in two" << std::endl;
    std::string cmd1 = "{\"device_id\":\"sensor_ts\",\"protocol\":\"PDI-IoTA-test\",\"commands\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"\"}]}";
    std::string cmd2 = "{\"myfield\":\"myfield1\",\"commands\":[{\"name\":\"PING2\",\"type\":\"command\",\"value\":\"\"}]}";

    std::string res = testjoinCommands(cmd1, cmd2);
    std::cout << "@UT@RES1: " << res << std::endl;
    CPPUNIT_ASSERT_MESSAGE("no commands in device",
                         res.find("{\"device_id\":\"sensor_ts\",\"protocol\":\"PDI-IoTA-test\"") != std::string::npos);
    CPPUNIT_ASSERT_MESSAGE("no commands in device",
                         res.find("{\"name\":\"PING\",\"type\":\"command\",\"value\":\"\"}]}") != std::string::npos);
    CPPUNIT_ASSERT_MESSAGE("no commands in device",
                         res.find("{\"name\":\"PING2\",\"type\":\"command\",\"value\":\"\"") != std::string::npos);
    mongo::BSONObj objProtocol1 = mongo::fromjson(res);
    std::cout << "@UT@SERV1:" << objProtocol1.jsonString() << std::endl;
  }

  {
    std::cout << "@UT@same commands in two, commands duyplicated" << std::endl;
    std::string cmd1 = "{\"device_id\":\"sensor_ts\",\"protocol\":\"PDI-IoTA-test\",\"commands\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"\"}]}";
    std::string cmd2 = "{\"myfield\":\"myfield1\",\"commands\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"44\"}]}";

    std::string res = testjoinCommands(cmd1, cmd2);
    std::cout << "@UT@RES2: " << res << std::endl;
    CPPUNIT_ASSERT_MESSAGE("no commands in device",
                         res.find("{\"device_id\":\"sensor_ts\",\"protocol\":\"PDI-IoTA-test\"") != std::string::npos);
    CPPUNIT_ASSERT_MESSAGE("no commands in device",
                         res.find("{\"name\":\"PING\",\"type\":\"command\",\"value\":\"\"}]}") != std::string::npos);
    CPPUNIT_ASSERT_MESSAGE("no commands in device",
                         res.find("{\"name\":\"PING\",\"type\":\"command\",\"value\":\"44\"") != std::string::npos);

    mongo::BSONObj objProtocol1 = mongo::fromjson(res);
    std::cout << "@UT@SERV2:" << objProtocol1.jsonString() << std::endl;
  }

  {
    std::cout << "@UT@same no commands in two" << std::endl;
    std::string cmd1 = "{\"device_id\":\"sensor_ts\",\"protocol\":\"PDI-IoTA-test\"}";
    std::string cmd2 = "{\"myfield\":\"myfield1\"}";

    std::string res = testjoinCommands(cmd1, cmd2);
    std::cout << "@UT@RES3: " << res << std::endl;
    CPPUNIT_ASSERT_MESSAGE("no commands in device",
                         res.find("{\"myfield\":\"myfield1\"") != std::string::npos);
    CPPUNIT_ASSERT_MESSAGE("no commands in device",
                         res.find("\"device_id\":\"sensor_ts\",\"protocol\":\"PDI-IoTA-test\"}") != std::string::npos);
    mongo::BSONObj objProtocol1 = mongo::fromjson(res);
    std::cout << "@UT@SERV3:" << objProtocol1.jsonString() << std::endl;
  }

  {
    std::cout << "@UT@commands in device" << std::endl;
    std::string cmd1 = "{\"device_id\":\"sensor_ts\",\"protocol\":\"PDI-IoTA-test\"}";
    std::string cmd2 = "{\"myfield\":\"myfield1\",\"commands\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"44\"}]}";

    std::string res = testjoinCommands(cmd1, cmd2);
    std::cout << "@UT@RES4: " << res << std::endl;
    CPPUNIT_ASSERT_MESSAGE("no commands in device",
                         res.find("{\"myfield\":\"myfield1\",\"commands\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"44\"}") != std::string::npos);
    CPPUNIT_ASSERT_MESSAGE("no commands in device",
                         res.find("\"device_id\":\"sensor_ts\",\"protocol\":\"PDI-IoTA-test\"") != std::string::npos);
    mongo::BSONObj objProtocol1 = mongo::fromjson(res);
    std::cout << "@UT@SERV4:" << objProtocol1.jsonString() << std::endl;
  }

  {
    std::cout << "@UT@same commands in device" << std::endl;
    std::string cmd1 = "{\"device_id\":\"sensor_ts\",\"protocol\":\"PDI-IoTA-test\"}";
    std::string cmd2 = "";

    std::string res = testjoinCommands(cmd1, cmd2);
    std::cout << "@UT@RES5: " << res << std::endl;
    CPPUNIT_ASSERT_MESSAGE("no found device",
                         res.find(cmd1) != std::string::npos);

    mongo::BSONObj objProtocol1 = mongo::fromjson(res);
    std::cout << "@UT@SERV5:" << objProtocol1.jsonString() << std::endl;
  }

  std::cout << "@UT@END testArray" << std::endl;
}


