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
#include "ApikeyTest.h"

#include "rest/riot_conf.h"

#include <sstream>
#include <stdexcept>
#include <pion/http/types.hpp>
#include "util/collection.h"
#include "rest/rest_handle.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define PATH_CONFIG "../../tests/iotagent/config.json"
#define ALT_PATH_CONFIG "../../config.json"
#define  PATH_CONFIG_MONGO "../../tests/iotagent/config_mongo.json"

CPPUNIT_TEST_SUITE_REGISTRATION(ApiKeyTest);

void ApiKeyTest::testBADConfigurator() {
  std::cout << "START apiKeyTest testBADConfigurator" << std::endl;

  iota::Configurator* conf = iota::Configurator::instance();

  std::string configuration("");


  std::stringstream ss;
  ss << configuration;
  conf->read_file(ss);
  std::string all = conf->getAll();

  std::string error = conf->getError();
  std::cout << "data2:" << error << std::endl;
  CPPUNIT_ASSERT(
    error.compare("Configurator: Text only contains white space(s)[0]") == 0);

  conf->release();
  std::cout << "delete:" << std::endl;
  //delete conf;
  std::cout << "END  apiKeyTest testBADConfigurator" << std::endl;
}

void ApiKeyTest::testConfigurator() {
  std::cout << "START apiKeyTest testConfigurator" << std::endl;
  iota::Configurator* conf = iota::Configurator::instance();

  std::string
  configuration("{\"timeout\": 5, \"resources\": [{\"resource\": \"/iot/res\",\"services\": [{\"apikey\": \"apikey-1\",\"cbroker\": \"http:://0.0.0.0:1026\", \"service\": \"service-1\",\"entity_type\": \"thing\",\"service_path\": \"servicepath\",\"token\": \"token1\"},");
  configuration.append("{\"apikey\": \"apikeyduplicada\",\"cbroker\": \"http:://0.0.0.0:1026\", \"service\": \"serviceduplicado\",\"entity_type\": \"thing\",\"service_path\": \"servicepath1\",\"token\": \"token111\"},");
  configuration.append("{\"apikey\": \"apikeyduplicada\",\"cbroker\": \"http:://0.0.0.0:1026\", \"service\": \"serviceduplicado\",\"entity_type\": \"thing\",\"service_path\": \"servicepath2\",\"token\": \"token222\"},");
  configuration.append("{\"apikey\": \"\",\"service\": \"service-2\",\"service_path\": \"servicepath-2\",\"token\": \"token2\", \"entity_type\": \"thing\", \"cbroker\": \"http:://0.0.0.0:1026\"}]},");
  configuration.append("{\"resource\": \"/iot/repsol\",\"services\": [{\"apikey\": \"\",\"service\": \"service-3\",\"service_path\": \"servicepath-3\",\"cbroker\": \"http:://0.0.0.0:1026\", \"entity_type\": \"thing\",\"token\": \"token3\"}]}], \"ngsi_url\": {\"updateContext\": \"/NGSI10/updateContext\"}}");


  std::stringstream ss;
  ss << configuration;
  //std::cout << "read_file" << std::endl;
  conf->read_file(ss);
  //std::cout << "read_file end" << std::endl;

  std::string all = conf->getAll();
  //std::cout << "data:" << all << std::endl;

  std::size_t found = all.find("apikey-1");
  CPPUNIT_ASSERT(found!=std::string::npos);

  try {
    std::cout << " getServicebyApiKey -> noexists " << std::endl;
    const iota::JsonValue& pt2 = conf->getServicebyApiKey("/iot/res", "/noexists");
    CPPUNIT_ASSERT(true);
  }
  catch (std::exception exc) {
    std::cout  << "noexists excepcion" << std::endl;
    std::cout  << exc.what() << std::endl;
  }

  std::cout << " getServicebyApiKey -> /iot/res apikeyduplicada" << std::endl;
  try {
    const iota::JsonValue& pt2 = conf->getServicebyApiKey("/iot/res", "apikeyduplicada");
    CPPUNIT_ASSERT(true);
  }
  catch (std::exception exc) {
    std::cout  << "apikeduplicada excepcion" << std::endl;
    std::cout  << exc.what() << std::endl;
  }

  std::cout << " getService -> /iot/res serviceduplicado" << std::endl;
  try {
    const iota::JsonValue& pt2 = conf->getService("/iot/res", "serviceduplicado",
                                            "servicepath1");
    std::string s(pt2["token"].GetString());
    std::cout << "token " << s << std::endl;
    CPPUNIT_ASSERT(s.compare("token111") == 0);
  }
  catch (std::exception exc) {
    std::cout  << "serviceduplicado excepcion" << std::endl;
    std::cout  << exc.what() << std::endl;
    CPPUNIT_ASSERT(true);
  }

  std::cout << " getService -> /iot/res serviceduplicado" << std::endl;
  try {
    const iota::JsonValue& pt2 = conf->getService("/iot/res", "serviceduplicado", "");
    CPPUNIT_ASSERT(true);
  }
  catch (std::exception exc) {
    std::cout  << "serviceduplicado excepcion" << std::endl;
    std::cout  << exc.what() << std::endl;
  }

  std::cout << " getServicebyApiKey -> /iot/res apikey-1" << std::endl;
  const iota::JsonValue& pt = conf->getServicebyApiKey("/iot/res", "apikey-1");
  std::string s(pt["service"].GetString());
  CPPUNIT_ASSERT(s.compare("service-1") == 0);
  CPPUNIT_ASSERT_THROW(conf->getServicebyApiKey("/iot/res", "noapikey"),
                       std::runtime_error);
  CPPUNIT_ASSERT_THROW(conf->getService("/iot/res", "noservice", "/"),
                       std::runtime_error);

  std::cout << " getService -> /iot/res service-2" << std::endl;
  const iota::JsonValue& pt_cb = conf->getService("/iot/res", "service-2", "");
  std::string sp(pt_cb["service_path"].GetString());
  std::cout << "servic_path: " << sp << std::endl;
  CPPUNIT_ASSERT(sp.compare("servicepath-2") == 0);

  std::cout << " getServicebyApiKey -> /iot/res apikey-2  /kk" << std::endl;
  CPPUNIT_ASSERT_THROW(conf->getService("/iot/res", "service-2", "/kk"),
                       std::runtime_error);

  std::cout << " getServicebyApiKey -> /iot/repsol " << std::endl;
  const iota::JsonValue& pt_empty = conf->getServicebyApiKey("/iot/repsol", "");
  std::string sp1(pt_empty["service"].GetString());
  CPPUNIT_ASSERT(sp1.compare("service-3") == 0);
  std::string url(pt_cb["cbroker"].GetString());
  CPPUNIT_ASSERT(url.compare("http:://0.0.0.0:1026") == 0);

  // First fileds in JSON

  const iota::JsonValue& timeout = conf->get("timeout");
  if (timeout.IsNumber()) {
    CPPUNIT_ASSERT(timeout.GetInt64() == 5);
  }

  // No existe
  CPPUNIT_ASSERT_THROW(conf->get("FieldNotExists"), std::runtime_error);


  // std::cout << conf->getAll() << std::endl;
  //liberamos la memoria del configurador
  conf->release();

  std::cout << "END  apiKeyTest testConfigurator" << std::endl;
}

void ApiKeyTest::testNgsiOperation() {

  std::cout << "START testNgsiOperationn" << std::endl;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);
  const iota::JsonValue& v = conf->get("ngsi_url");
  //std::cout << conf->getAll() << std::endl;
  std::string uc = v["updateContext"].GetString();
  std::cout << "ngsi_url.updateContext " <<  uc << std::endl;
  CPPUNIT_ASSERT(uc.compare("/NGSI10/updateContext") == 0);

  conf->release();
  std::cout << "END testNgsiOperation" << std::endl;
}

void ApiKeyTest::testHttpMessages() {
  std::cout << "START testHttpMessages" << std::endl;
  iota::Configurator* conf = iota::Configurator::instance();
  std::string men = iota::Configurator::instance()->getHttpMessage(
                      pion::http::types::RESPONSE_CODE_NOT_FOUND);
  CPPUNIT_ASSERT(men.compare(pion::http::types::RESPONSE_MESSAGE_NOT_FOUND) == 0);
  std::cout << iota::Configurator::instance()->getHttpMessage(404) << std::endl;

  conf->release();
  std::cout << "END testHttpMessages" << std::endl;
}

void ApiKeyTest::testConversionMap() {
  std::cout << "Start testConversionMap" << std::endl;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);
  std::map<std::string, std::string> to_map;
  conf->get("ngsi_url", to_map);

  CPPUNIT_ASSERT_MESSAGE("Checking json object to map", to_map.size() > 0);
  CPPUNIT_ASSERT_MESSAGE("Checking map value",
                         to_map["updateContext"].compare("/NGSI10/updateContext") == 0);
  std::cout << "End testConversionMap" << std::endl;
}

void ApiKeyTest::testAuthInfo() {
  std::cout << "Start testAuthInfo" << std::endl;
  iota::Configurator* conf = iota::Configurator::initialize(ALT_PATH_CONFIG);
  std::map<std::string, std::string> to_map;
  conf->get("oauth", to_map);

  CPPUNIT_ASSERT_MESSAGE("Checking json object to map", to_map.size() > 0);
  std::cout << to_map["pep_user"] << std::endl;
  CPPUNIT_ASSERT_MESSAGE("Checking map value",
                         to_map["pep_user"].compare("pep") == 0);
  std::cout << "End testAuthInfo" << std::endl;
}


void ApiKeyTest::testConfiguratorMongo() {
  std::cout << "START apiKeyTest testConfiguratorMongo" << std::endl;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG_MONGO);

  std::cout << "before inserts" << std::endl;
  iota::Collection table1("SERVICE");

  mongo::BSONObj p1 = BSON("apikey" << "apikey-1" << "cbroker" <<
                           "http:://0.0.0.0:1026"
                           << "service" << "service-1" << "entity_type" << "thing" <<
                           "service_path" << "servicepath" << "token" << "token1");
  table1.insert(p1);

  mongo::BSONObj p2 = BSON("apikey" << "apikeyduplicada" << "cbroker" <<
                           "http:://0.0.0.0:1026"
                           << "service" << "serviceduplicado" << "entity_type" << "thing" <<
                           "service_path" << "servicepath1" << "token" << "token111");
  table1.insert(p2);

  mongo::BSONObj p3 = BSON("apikey" << "apikeyduplicada" << "cbroker" <<
                           "http:://0.0.0.0:1026"
                           << "service" << "serviceduplicado" << "entity_type" << "thing" <<
                           "service_path" << "servicepath2" << "token" << "token222");
  table1.insert(p3);

  mongo::BSONObj p4 = BSON("apikey" << "service-2" << "cbroker" <<
                           "http:://0.0.0.0:1026"
                           << "service" << "serviceduplicado" << "entity_type" << "thing" <<
                           "service_path" << "servicepath-2" << "token" << "token111");
  table1.insert(p4);

  mongo::BSONObj p5 = BSON("apikey" << "" << "cbroker" << "http:://0.0.0.0:1026"
                           << "service" << "repsol" << "entity_type" << "thing" <<
                           "service_path" << "servicepath-repsol" << "token" << "token111");
  table1.insert(p5);

  mongo::BSONObj p6= BSON("apikey" << "apikey-6" << "cbroker" <<
                           "http:://0.0.0.0:1026"
                           << "service" << "service-6" << "entity_type" << "thing" <<
                           "service_path" << "servicepath" << "token" << "token1");
  mongo::BSONObj p_att = BSON("name" << "GPS" << "type" << "string" << "value" << "10");
  mongo::BSONObjBuilder o_p6;
  o_p6.appendElements(p6);
  mongo::BSONArrayBuilder a_obj;
  a_obj.append(p_att);
  o_p6.append("static_attributes", a_obj.arr());
  table1.insert(o_p6.obj());
  iota::RestHandle rh;
  boost::property_tree::ptree pt_cb;
  std::string apiKey;

  try {
    apiKey.assign("noexists");

    boost::property_tree::ptree pt_cb;
    std::cout << " getServicebyApiKey -> noexists " << std::endl;
    rh.get_service_by_apiKey(pt_cb, apiKey);
    std::cout << " getServicebyApiKey -> noexists " << std::endl;
    CPPUNIT_ASSERT(true);
  }
  catch (std::exception exc) {
    std::cout  << "noexists excepcion" << std::endl;
    std::cout  << exc.what() << std::endl;
  }

  std::cout << " getServicebyApiKey -> /iot/res apikeyduplicada" << std::endl;
  try {
    apiKey.assign("apikeyduplicada");
    rh.get_service_by_apiKey(pt_cb, apiKey);
    CPPUNIT_ASSERT(true);
  }
  catch (std::exception exc) {
    std::cout  << "apikeduplicada excepcion" << std::endl;
    std::cout  << exc.what() << std::endl;
  }

  std::cout << " getService -> /iot/res serviceduplicado" << std::endl;
  try {
    rh.get_service_by_name(pt_cb, "serviceduplicado", "servicepath1");
    std::string s(pt_cb.get<std::string>("service", ""));
    std::cout << "token " << s << std::endl;
    CPPUNIT_ASSERT(s.compare("token111") == 0);
  }
  catch (std::exception exc) {
    std::cout  << "serviceduplicado excepcion" << std::endl;
    std::cout  << exc.what() << std::endl;
    CPPUNIT_ASSERT(true);
  }

  std::cout << " getService -> /iot/res serviceduplicado" << std::endl;
  try {
    rh.get_service_by_name(pt_cb, "serviceduplicado", "");
    CPPUNIT_ASSERT(true);
  }
  catch (std::exception exc) {
    std::cout  << "serviceduplicado excepcion" << std::endl;
    std::cout  << exc.what() << std::endl;
  }

  try {
    std::cout << " getServicebyApiKey -> /iot/res apikey-1" << std::endl;
    rh.get_service_by_apiKey(pt_cb, "apikey-1");
    std::string s(pt_cb.get<std::string>("service", ""));
    CPPUNIT_ASSERT(s.compare("service-1") == 0);
    CPPUNIT_ASSERT_THROW(rh.get_service_by_apiKey(pt_cb, "noapikey"),
                         std::runtime_error);
    CPPUNIT_ASSERT_THROW(rh.get_service_by_name(pt_cb, "noservice", "/"),
                         std::runtime_error);

    std::cout << " getService -> /iot/res service-2" << std::endl;
    rh.get_service_by_name(pt_cb, "service-2", "");
    std::string sp(pt_cb.get<std::string>("service_path", ""));
    std::cout << "servic_path: " << sp << std::endl;
    CPPUNIT_ASSERT(sp.compare("servicepath-2") == 0);

    std::cout << " getServicebyApiKey -> /iot/res apikey-2  /kk" << std::endl;
    CPPUNIT_ASSERT_THROW(rh.get_service_by_name(pt_cb, "service-2", "/kk"),
                         std::runtime_error);

  }
  catch (std::exception exc) {
    std::cout  << "catch a borrar" << std::endl;
    std::cout  << exc.what() << std::endl;
  }

  try {
    std::cout << " getServicebyApiKey -> /iot/res apikey-6" << std::endl;
    rh.get_service_by_apiKey(pt_cb, "apikey-6");
    std::string s(pt_cb.get<std::string>("service", ""));
    CPPUNIT_ASSERT(s.compare("service-6") == 0);
    boost::property_tree::ptree s_a = pt_cb.get_child("static_attributes");
    CPPUNIT_ASSERT_MESSAGE("Checking static attribute GPS", s_a.get<std::string>("name", "").compare("GPS") == 0);
    CPPUNIT_ASSERT_MESSAGE("Checking static attribute GPS", s_a.get<std::string>("value", "").compare("10") == 0);
    CPPUNIT_ASSERT_MESSAGE("Checking cbroker", pt_cb.get<std::string>("cbroker", "").compare("http:://0.0.0.0:1026") == 0);
  }
  catch (std::exception exc) {
  }

  // std::cout << conf->getAll() << std::endl;
  //liberamos la memoria del configurador
  conf->release();

  mongo::BSONObjBuilder pobj;
  table1.remove(pobj.obj());
  std::cout << "END  apiKeyTest testConfiguratorMongo" << std::endl;
}

