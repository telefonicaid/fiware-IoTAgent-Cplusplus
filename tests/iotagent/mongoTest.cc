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
#include "util/alarm.h"
#include "util/iota_exception.h"
#include <boost/property_tree/ptree.hpp>

#include "util/device.h"
#include "util/command.h"

#include <iostream>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>

#define  PATH_CONFIG "../../tests/iotagent/config_mongo.json"
#define  PATH_BAD_CONFIG "../../tests/iotagent/config_bad_mongo.json"
#define  PATH_REPLICA_CONFIG "../../tests/iotagent/config_mongo_replica.json"

CPPUNIT_TEST_SUITE_REGISTRATION(MongoTest);

namespace iota {
std::string logger("main");
std::string URL_BASE = "/iot";
}
iota::AdminService* AdminService_ptr;

void MongoTest::setUp() {
  std::cout << "setUp mongoTest " << std::endl;


  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
}

void MongoTest::tearDown() {
  std::cout << "tearDown mongoTest " << std::endl;
}

void MongoTest::testGenericCollection() {
  std::cout << "START testGenericCollection" << std::endl;
  iota::Configurator::initialize(PATH_CONFIG);

  const iota::JsonValue& storage=
    iota::Configurator::instance()->get(iota::store::types::STORAGE);

  std::string dbname;
  if (storage.HasMember(iota::store::types::DBNAME.c_str())) {
    dbname.assign(storage[iota::store::types::DBNAME.c_str()].GetString());
  }

  std::cout << "testGenericCollection2" << std::endl;
  iota::Collection table1("PRUEBA");

  mongo::BSONObj p = BSON("name" << "Joe" << "desc" << "ssss");
  std::cout << "before insert" << std::endl;
  table1.insert(p);

  iota::Collection q1("PRUEBA");
  mongo::BSONObj p2 = BSON("name" << "Joe");
  std::cout << "before find" << std::endl;
  int code_res = q1.find(p2);
  CPPUNIT_ASSERT_MESSAGE("no inserted data",
                         q1.more());
  mongo::BSONObj r1 =  q1.next();

  std::string name =  r1.getStringField("name");
  std::cout << "Object " << r1 << std::endl;
  CPPUNIT_ASSERT_MESSAGE("no inserted data",
                         name.compare("Joe") == 0);

  std::cout << "before remove" << std::endl;
  q1.remove(p2);

  std::cout << "before count" << std::endl;
  int num = q1.count(p2);
  CPPUNIT_ASSERT_MESSAGE("remove error", num == 0);

  mongo::BSONObj pe = BSON("name" << "Pepe" << "desc" << "Botella");
  q1.insert(pe);

  mongo::BSONObj no = BSON("name" << "Pepe");
  mongo::BSONObj ap = BSON("desc" << "Lopez");
  std::cout << "before update" << std::endl;
  q1.update(no, ap);
  std::cout << "before find with bson_fields" << std::endl;
  mongo::BSONObjBuilder bson_fields;
  bson_fields.append("_id", 0);
  code_res = q1.find(no, bson_fields);

  CPPUNIT_ASSERT_MESSAGE("no inserted data",
                         q1.more());
  mongo::BSONObj r2 =  q1.next();
  std::cout << "FDFDFD " << r2 << std::endl;
  std::string desc =  r2.getStringField("desc");
  std::cout << "desc:" << desc << std::endl;
  CPPUNIT_ASSERT_MESSAGE("no inserted data",
                         desc.compare("Lopez") == 0);

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
  std::cout << "START testCommandCollection" << std::endl;
  iota::Configurator::initialize(PATH_CONFIG);
  iota::CommandCollection table1;

    table1.createTableAndIndex();

    iota::Command all("","", "");
    table1.remove(all);

    boost::property_tree::ptree pt;
    pt.put("body", "command");


    iota::Command p("nodo","service","/");
    p.set_command(pt);
    p.set_entity_type("entity_type");
    p.set_id("id0");
    p.set_name("name");
    p.set_sequence("sequence");
    p.set_status(0);
    p.set_timeout(22);
    p.set_uri_resp("uri_resp");
    table1.insert(p);

    iota::Command p2("nodo2","service2","/");
    p2.set_command(pt);
    p2.set_entity_type("entity_type");
    p2.set_id("id2");
    p2.set_name("name");
    p2.set_sequence("sequence");
    p2.set_status(0);
    p2.set_timeout(22);
    p2.set_uri_resp("uri_resp");
    table1.insert(p2);

    iota::CommandCollection table2;
    iota::Command q1("nodo","service","/");
    int code_res = table2.find(q1);
    CPPUNIT_ASSERT_MESSAGE("no inserted data",
           table2.more());
    iota::Command r1 =  table2.next();

    std::string name =  r1.get_name();
    std::cout << "name:" << name << std::endl;
    CPPUNIT_ASSERT_MESSAGE("no inserted data", name.compare("name") == 0);

    CPPUNIT_ASSERT_MESSAGE("more data", table2.more() == false);

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
  std::string location_body = "{\"name\": \"position\", \"type\": \"coords\", \"metadatas\": [{\"name\": \"location\",\"type\": \"string\",\"value\": \"WGS84\"}]}";

    table1.createTableAndIndex();

    iota::Device all("","");
    table1.remove(all);

    iota::Device p("Joe", "service1" );
    p._service_path = "service_path";
    p._entity_type = "_entity_type";
    p._entity_name = "_entity_name";
    p._endpoint = "_endpoint";
    p._timezone = "timezone";

    //comando1 ping
    p._commands.insert(std::pair<std::string, std::string>("ping","ping_command"));
    p._commands.insert(std::pair<std::string, std::string>("set","set_command"));

    //attribute1 t
    p._attributes.insert(std::pair<std::string, std::string>("t","{\"name\": \"temperature\", \"type\": \"int\"}"));
    p._attributes.insert(std::pair<std::string, std::string>("h","{\"name\": \"humedity\", \"type\": \"%\"}"));
    p._attributes.insert(std::pair<std::string, std::string>("l",location_body));

    //static attribute1 model
    p._static_attributes.insert(std::pair<std::string, std::string>("model","{\"name\": \"att_name\",\"type\": \"xxxx\",\"value\": \"value\"}"));
    p._static_attributes.insert(std::pair<std::string, std::string>("uuid","{\"name\": \"att_name2\",\"type\": \"xxxx\",\"value\": \"value\"}"));

    std::cout << "insert with props" << std::endl;
    table1.insert(p);


    // ponemos registrationid
    std::cout << "@UT@UPDATE registration" << std::endl;
    iota::Device query("Joe", "service1" );
    query._service_path = "service_path";
    iota::Device sett("", "" );
    sett._registration_id = "registration_id";
    sett._duration_cb = 666;
    table1.update(query, sett);


    iota::DeviceCollection table2;
    iota::Device q1("Joe", "service1" );
    int code_res = table2.find(q1);
    CPPUNIT_ASSERT_MESSAGE("no inserted data",
           table2.more());
    iota::Device r1 =  table2.next();
    CPPUNIT_ASSERT_MESSAGE("no commands", r1._commands.size() == 2);
    CPPUNIT_ASSERT_MESSAGE("no attributes", r1._attributes.size() == 3);
    std::string location =  r1._attributes["l"];
    std::cout << "@UT@LOCATION " << location << std::endl;
    //CPPUNIT_ASSERT_MESSAGE("bad location", location.compare(location_body) != std::string::npos);
    CPPUNIT_ASSERT_MESSAGE("no _registration_id", r1._registration_id.compare("registration_id") == 0);
    CPPUNIT_ASSERT_MESSAGE("no _duration_cb", r1._duration_cb == 666);

    CPPUNIT_ASSERT_MESSAGE("no static attributes", r1._static_attributes.size() == 2);

    std::string name =  r1._name;
    std::cout << "name:" << name << std::endl;
    CPPUNIT_ASSERT_MESSAGE("no inserted data", name.compare("Joe") == 0);

    table2.remove(q1);

    int num = table2.count(q1);
    CPPUNIT_ASSERT_MESSAGE("remove error", num == 0);

  std::cout << "END testDeviceCollection " << std::endl;
}


void MongoTest::testNoMongo() {
  std::cout << "START testNoMongo" << std::endl;
  iota::Configurator::initialize(PATH_BAD_CONFIG);

  const iota::JsonValue& storage=
    iota::Configurator::instance()->get(iota::store::types::STORAGE);

  try {
     std::cout << "testGenericCollection2" << std::endl;
     iota::Collection table1("PRUEBA");
     CPPUNIT_ASSERT_MESSAGE("no exception mongo", true);
  }catch(std::exception e){
     CPPUNIT_ASSERT_MESSAGE("alarm not found", iota::Alarm::instance()->size() == 1);
  }

  iota::Configurator::initialize(PATH_CONFIG);

  //TODO no se quita la alarma porque el endpoint es distinto, pero funciona
  std::cout << "testGenericCollection22" << std::endl;
  iota::Collection table1("PRUEBA");
  std::cout << "num alarms:" <<  iota::Alarm::instance()->size() << std::endl;
  CPPUNIT_ASSERT_MESSAGE("alarm not found", iota::Alarm::instance()->size() == 1);

  std::cout << "END testNoMongo " << std::endl;
}

void MongoTest::testReplica() {
  std::cout << "START testReplica" << std::endl;

  iota::Configurator::initialize(PATH_REPLICA_CONFIG);
  int num_threads=1;
  const char * val = ::getenv( "UNIT_TEST_THREADS" );
  if ( val != 0 ) {
      num_threads = boost::lexical_cast<int>(val);
  }
  int milis_threads=1;
  const char * valmilis = ::getenv( "UNIT_TEST_MILLIS" );
  if ( valmilis != 0 ) {
      milis_threads = boost::lexical_cast<int>(valmilis);
  }

  boost::thread_group g;
  std::cout << "nonum threads:" << num_threads << " milis: " << milis_threads << std::endl;
  for (int i =0; i < num_threads; i++){
    g.add_thread(new boost::thread(MongoTest::workerFunc));
    boost::this_thread::sleep(boost::posix_time::milliseconds(milis_threads));
  }

  g.join_all();
  std::cout << "END testReplica" << std::endl;
}

void MongoTest::workerFunc(){
  std::cout << "START workerFunc" << std::endl;
  iota::Collection table1("PRUEBA");

  mongo::BSONObj p = BSON("name" << "Joe" << "desc" << "ssss");
  std::cout << "before insert" << std::endl;
  table1.insert(p);

  iota::Collection q1("PRUEBA");
  mongo::BSONObj p2 = BSON("name" << "Joe");
  std::cout << "before find" << std::endl;
  int code_res = q1.find(p2);
  CPPUNIT_ASSERT_MESSAGE("no inserted data",
                         q1.more());
  if (q1.more()){
      q1.next();
  }

  std::cout << "END workerFunc " << std::endl;
}

void MongoTest::testException() {
  std::cout << "START testException" << std::endl;
  iota::Configurator::initialize(PATH_CONFIG);

  iota::Collection table1("PRUEBA");
  iota::Collection tableDev("DEVICE");

  //remove all data
  mongo::BSONObj all;
  table1.remove(all);

  mongo::BSONObj p = BSON("_id" << "Joe" << "desc" << "ssss");
  std::cout << "before insert" << std::endl;
  table1.insert(p);

  iota::Collection q1("PRUEBA");
  mongo::BSONObj p2 = BSON("_id" << "Joe");
  std::cout << "before find" << std::endl;
  int code_res = q1.find(p2);
  CPPUNIT_ASSERT_MESSAGE("no inserted data",
                         q1.more());

  // object DUPLICATED insert
  try{
    std::cout << "before insert with duplicated" << std::endl;
    table1.insert(p);
    CPPUNIT_ASSERT_MESSAGE("no exception with duplicate", false);
  }catch( iota::IotaException e){
    std::cout << "IotaException" << e.what() << std::endl;
    std::cout << "IotaException" << e.status() << "|" << e.reason() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("no code duplicate",
                         e.status() == 409);
    CPPUNIT_ASSERT_MESSAGE("no message duplicate",
                         e.reason().compare(""));
  }


  // object NO exists update
    std::cout << "before NO exists update" << std::endl;
    mongo::BSONObj q = BSON("_id" << "Joeee");
    mongo::BSONObj pdata = BSON("data" << "Joeee");
    int n = table1.update(q, pdata);
    std::cout << "num update " << n << std::endl;
    CPPUNIT_ASSERT_MESSAGE("no updates", n ==0);

  //BAD BSON

  std::cout << "END testException" << std::endl;
}
