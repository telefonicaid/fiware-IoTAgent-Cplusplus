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
#include "simpleplugingTest.h"


#include "util/FuncUtil.h"
#include "util/KVP.h"
#include "ultra_light/ULInsertObservation.h"
#include "util/RiotISO8601.h"
#include "ngsi/ContextResponses.h"

#include "services/admin_service.h"
#include "services/ngsi_service.h"

#include <boost/property_tree/ptree.hpp>
#include "util/iota_logger.h"
#include <pion/process.hpp>
#include <boost/filesystem/operations.hpp>
#include "ultra_light/ul20_service.h"
#include <boost/shared_ptr.hpp>
#include <util/device.h>
#include "rest/oauth_filter.h"

#include <cmath>
#include <ctime>

#include "util/dev_file.h"

#define  PATH_CONFIG "../../tests/iotagent/config.json"
#define  PATH_DEV_CFG "../../tests/iotagent/devices.json"
#define  SERVICE  "service7"

#define  ASYNC_TIME_WAIT  boost::this_thread::sleep(boost::posix_time::milliseconds(100));

CPPUNIT_TEST_SUITE_REGISTRATION(SimplePluginTest);
namespace iota {
std::string logger("main");
std::string URL_BASE("/iot");
}
iota::AdminService* AdminService_ptr;


SimplePluginTest::SimplePluginTest() {
}

SimplePluginTest::~SimplePluginTest() {
  wserver.reset();
}
void SimplePluginTest::tearDown() {
}

void SimplePluginTest::start_cbmock(boost::shared_ptr<HttpMock>& cb_mock,
                                    const std::string& type,
                                    bool vpn) {
  cb_mock->init();
  std::string mock_port = boost::lexical_cast<std::string>(cb_mock->get_port());
  std::cout << "mock with port:" << mock_port << std::endl;

  iota::Configurator::release();
  iota::Configurator* my_instance = iota::Configurator::instance();

  std::stringstream ss;
  if (!vpn) {
    ss << "{ \"ngsi_url\": {"
       <<   "     \"updateContext\": \"/NGSI10/updateContext\","
       <<   "     \"registerContext\": \"/NGSI9/registerContext\","
       <<   "     \"queryContext\": \"/NGSI10/queryContext\""
       <<   "},"
       <<   "\"public_ip\": \"127.0.0.1:8888\","

       <<   "\"timeout\": 1,"
       <<   "\"iota_manager\": \"http://127.0.0.1:" << mock_port << "/protocols\", "
       <<   "\"dir_log\": \"/tmp/\","
       <<   "\"timezones\": \"/etc/iot/date_time_zonespec.csv\","
       <<   "\"storage\": {"
       <<   "\"host\": \"127.0.0.1\","
       <<   "\"type\": \"" <<  type << "\","
       <<   "\"port\": \"27017\","
       <<   "\"dbname\": \"iotest\","
       <<   "\"file\": \"../../tests/iotagent/devices.json\""
       << "},"
       << "\"resources\":[{\"resource\": \"/iot/sp\","
       << "  \"options\": {\"FileName\": \"SPService\" },"
       <<    " \"services\":[ {"
       <<   "\"apikey\": \"apikey3\","
       <<   "\"service\": \"service2\","
       <<   "\"service_path\": \"/ssrv2\","
       <<   "\"token\": \"token2\","
       <<   "\"cbroker\": \"http://127.0.0.1:" << mock_port << "/mock\", "
       <<   "\"entity_type\": \"thing\""
       << "} ] } ] }";
  }
  else {
    ss << "{ \"ngsi_url\": {"
       <<   "     \"updateContext\": \"/NGSI10/updateContext\","
       <<   "     \"registerContext\": \"/NGSI9/registerContext\","
       <<   "     \"queryContext\": \"/NGSI10/queryContext\""
       <<   "},"
       <<   "\"public_ip\": \"127.0.0.1:8888\","
       <<   "\"timeout\": 1,"
       <<   "\"http_proxy\": \"127.0.0.1:8888\","
       <<   "\"dir_log\": \"/tmp/\","
       <<   "\"timezones\": \"/etc/iot/date_time_zonespec.csv\","
       <<   "\"storage\": {"
       <<   "\"host\": \"127.0.0.1\","
       <<   "\"type\": \"" <<  type << "\","
       <<   "\"port\": \"27017\","
       <<   "\"dbname\": \"iotest\","
       <<   "\"file\": \"../../tests/iotagent/devices.json\""
       << "},"
       << "\"resources\":[{\"resource\": \"/iot/d\","
       << "  \"options\": {\"FileName\": \"UL20Service\" },"
       <<    " \"services\":[ {"
       <<   "\"apikey\": \"apikey3\","
       <<   "\"service\": \"service2\","
       <<   "\"service_path\": \"/ssrv2\","
       <<   "\"token\": \"token2\","
       <<   "\"outgoing_route\": \"gretunnel-service2\","
       <<   "\"cbroker\": \"http://127.0.0.1:" << mock_port << "/mock\", "
       <<   "\"entity_type\": \"thing\""
       << "} ] } ] }";
  }
  //my_instance->update_conf(ss);
  std::string err = my_instance->read_file(ss);
  std::cout << "GET CONF " << my_instance->getAll() << std::endl;
  if (!err.empty()) {
    std::cout << "start_cbmock:" << err << std::endl;
    std::cout << "start_cbmock_data:" << ss.str() << std::endl;
  }


}

void SimplePluginTest::testRegisterIoTA() {

  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/protocols"));
  start_cbmock(cb_mock, "mongodb");
  std::string mock_port = boost::lexical_cast<std::string>(cb_mock->get_port());
  scheduler.set_num_threads(1);
  wserver.reset(new pion::http::plugin_server(scheduler));
  spserv_auth = new iota::SPService();
  wserver->add_service("/iot/sp", spserv_auth);
  wserver->start();

  CPPUNIT_ASSERT_MESSAGE("Manager endpoint ",
                         spserv_auth->get_iota_manager_endpoint().find("/protocols") !=
                         std::string::npos);

  ASYNC_TIME_WAIT
  std::string r_1 = cb_mock->get_last();
  CPPUNIT_ASSERT_MESSAGE("POST manager ", r_1.find("HOLA") != std::string::npos);
  cb_mock->stop();
}
void SimplePluginTest::testFilter() {
  std::cout << "START testFilter" << std::endl;
  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);
  scheduler.set_num_threads(1);
  wserver.reset(new pion::http::plugin_server(scheduler));
  spserv_auth = new iota::SPService();
  wserver->add_service("/iot/sp_auth", spserv_auth);
  wserver->start();
  std::cout << "THREADS " << scheduler.get_num_threads() << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/"));
  cb_mock->init();
  std::string mock_port = boost::lexical_cast<std::string>(cb_mock->get_port());

  boost::shared_ptr<iota::OAuthFilter> filter(new iota::OAuthFilter());
  filter->set_filter_url_base("/iot/sp_auth");
  std::map<std::string, std::string> map;
  map[ iota::types::CONF_FILE_OAUTH_VALIDATE_TOKEN_URL] = "http://127.0.0.1:"
      +mock_port+"/v3/auth/tokens";
  map[ iota::types::CONF_FILE_OAUTH_ROLES_URL] = "http://127.0.0.1:"+mock_port
      +"/v3/role_assignments";
  map[ iota::types::CONF_FILE_OAUTH_PROJECTS_URL] = "http://127.0.0.1:"+mock_port
      +"/v3/projects";
  map[ iota::types::CONF_FILE_ACCESS_CONTROL] = "http://127.0.0.1:"+mock_port;
  map[ iota::types::CONF_FILE_PEP_USER] = "pep";
  map[ iota::types::CONF_FILE_PEP_PASSWORD] = "pep";
  map[ iota::types::CONF_FILE_PEP_DOMAIN] = "admin_domain";
  map[ iota::types::CONF_FILE_OAUTH_TIMEOUT] = "3";
  filter->set_configuration(map);
  spserv_auth->add_pre_filter(filter);
  spserv_auth->add_statistic_counter("traffic", true);

  boost::system::error_code error_code;

  std::string bodySTR = "BodyToTest";

  {
    pion::tcp::connection_ptr tcp_conn_1(new pion::tcp::connection(
                                           scheduler.get_io_service()));

    std::cout << "No headers " << wserver->get_port() << std::endl;
    // No headers fiware-service, fiware-servicepath....
    error_code = tcp_conn_1->connect(
                   boost::asio::ip::address::from_string("127.0.0.1"), wserver->get_port());
    pion::http::request http_request1("/iot/sp_auth/devices/device_1");
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
                                           scheduler.get_io_service()));
    std::cout << "With headers" << std::endl;
    std::map<std::string, std::string> h;
    std::string
    content_validate_token("{\"token\": {\"issued_at\": \"2014-10-06T08:20:13.484880Z\",\"extras\": {},\"methods\": [\"password\"],\"expires_at\": \"2014-10-06T09:20:13.484827Z\",");
    content_validate_token.append("\"user\": {\"domain\": {\"id\": \"f7a5b8e303ec43e8a912fe26fa79dc02\",\"name\": \"SmartValencia\"},\"id\": \"5e817c5e0d624ee68dfb7a72d0d31ce4\",\"name\": \"alice\"}}}");
    h["X-Subject-Token"] = "x-subject-token";
    cb_mock->set_response(200, "", h);
    cb_mock->set_response(200, content_validate_token);
    std::string
    projects("{\"projects\": [{\"description\": \"SmartValencia Subservicio Electricidad\",\"links\": {\"self\": \"http://${KEYSTONE_HOST}/v3/projects/c6851f8ef57c4b91b567ab62ca3d0aef\"},\"enabled\": true,\"id\": \"c6851f8ef57c4b91b567ab62ca3d0aef\",\"domain_id\": \"f7a5b8e303ec43e8a912fe26fa79dc02\",\"name\": \"Electricidad\"}]}");
    cb_mock->set_response(200, projects, h);
    std::string
    user_roles("{\"role_assignments\": [{\"scope\": {\"project\": {\"id\": \"c6851f8ef57c4b91b567ab62ca3d0aef\"}},\"role\": {\"id\": \"a6407b6c597e4e1dad37a3420b6137dd\"},\"user\": {\"id\": \"5e817c5e0d624ee68dfb7a72d0d31ce4\"},\"links\": {\"assignment\": \"puthere\"}}],\"links\": {\"self\": \"http://${KEYSTONE_HOST}/v3/role_assignments\",\"previous\": null,\"next\": null}}");
    cb_mock->set_response(200, user_roles);
    error_code = tcp_conn_2->connect(
                   boost::asio::ip::address::from_string("127.0.0.1"), wserver->get_port());
    pion::http::request http_request2("/iot/sp_auth/devices/device_1");
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

  // Statistic
  std::cout << spserv_auth->get_statistics() << std::endl;


  wserver->stop();
  cb_mock->stop();
  conf->release();

  std::cout << "END testFilter " << wserver->get_connections() << std::endl;
}

void SimplePluginTest::testStatistic() {

}

void SimplePluginTest::testGetDevice() {
  std::cout << "START testGetDevice" << std::endl;
  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);
  scheduler.set_num_threads(1);
  wserver.reset(new pion::http::plugin_server(scheduler));
  spserv_auth = new iota::SPService();
  wserver->add_service("/iot/tt", spserv_auth);
  wserver->start();
  std::cout << "THREADS " << scheduler.get_num_threads() << std::endl;

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
