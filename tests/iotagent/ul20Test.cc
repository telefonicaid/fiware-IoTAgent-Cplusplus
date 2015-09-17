/**
* Copyright 2015 Telefonica Investigaci√≥n y Desarrollo, S.A.U
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
#include "util/device_collection.h"

#include "ul20Test.h"
#include "util/FuncUtil.h"
#include "util/KVP.h"
#include "ultra_light/ULInsertObservation.h"
#include "util/RiotISO8601.h"
#include "util/command.h"
#include "util/device.h"
#include "util/command_cache.h"
#include "ngsi/ContextResponses.h"
#include "rest/command_handle.h"

#include "services/admin_service.h"
#include "services/ngsi_service.h"
#include "../mocks/http_mock.h"
#include <boost/property_tree/ptree.hpp>
#include "util/iota_logger.h"
#include <pion/process.hpp>
#include <pion/http/request.hpp>
#include <boost/filesystem/operations.hpp>
#include "ultra_light/ul20_service.h"
#include <boost/shared_ptr.hpp>
#include <util/device.h>
#include <util/iota_exception.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>
#include "boost/format.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include "rest/riot_conf.h"
#include <util/common.h>

#include <cmath>
#include <ctime>

#include "util/dev_file.h"

#define  PATH_CONFIG_MONGO "../../tests/iotagent/config_mongo.json"
#define  PATH_CONFIG "../../tests/iotagent/config.json"
#define  PATH_CONFIG_VPN "../../tests/iotagent/config_vpn.json"


#define  PATH_DEV_CFG "../../tests/iotagent/devices.json"

#define  IOTASSERT_MESSAGE(x,y) \
         std::cout << "@" << __LINE__ << "@" << x << std::endl; \
         CPPUNIT_ASSERT_MESSAGE(x,y)

#define  IOTASSERT(y) \
         std::cout << "@" << __LINE__ << "@" << std::endl; \
         CPPUNIT_ASSERT(y)

#define  ASYNC_TIME_WAIT  boost::this_thread::sleep(boost::posix_time::milliseconds(100));

const int Ul20Test::POST_RESPONSE_CODE = 201;

const std::string
Ul20Test::POST_SERVICE("{\"services\": [{"
                        "\"apikey\": \"apikey%s\",\"token\": \"token\","
                        "\"cbroker\": \"http://127.0.0.1:%s/mock\",\"entity_type\": \"thing\",\"resource\": \"/iot/d\"}]}");

const std::string
Ul20Test::POST_SERVICE_ENDPOINT("{\"services\": [{"
                        "\"apikey\": \"apikey3\",\"token\": \"token\","
                        "\"cbroker\": \"http://127.0.0.1:1026/mock\",\"entity_type\": \"thing\",\"resource\": \"/iot/d\"}]}");

const std::string
Ul20Test::POST_DEVICE_SIN("{\"devices\": "
                       "[{\"device_id\": \"dev_SIN\", \"protocol\": \"PDI-IoTA-UltraLight\", \"timezone\": \"America/Santiago\","
                       "\"commands\": [{\"name\": \"PING\",\"type\": \"command\",\"value\": \"\" }],"
                       "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }]"
                       ",\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
                       "}]}");

const std::string
Ul20Test::POST_DEVICE_SIN_ENTITY_NAME("{\"devices\": "
                       "[{\"device_id\": \"dev_SIN_ENTITY_NAME\",\"protocol\": \"PDI-IoTA-UltraLight\", \"entity_type\": \"type_SIN_ENTITY_NAME\",\"timezone\": \"America/Santiago\","
                       "\"commands\": [{\"name\": \"PING\",\"type\": \"command\",\"value\": \"\" }],"
                       "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }]"
                       ",\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
                       "}]}");

const std::string
Ul20Test::POST_DEVICE_SIN_ENTITY_TYPE("{\"devices\": "
                       "[{\"device_id\": \"dev_SIN_ENTITY_TYPE\",\"protocol\": \"PDI-IoTA-UltraLight\",\"entity_name\": \"ent_SIN_ENTITY_TYPE\",\"timezone\": \"America/Santiago\","
                       "\"commands\": [{\"name\": \"PING\",\"type\": \"command\",\"value\": \"\" }],"
                       "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }]"
                       ",\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
                       "}]}");

const std::string
Ul20Test::POST_DEVICE("{\"devices\": "
                       "[{\"device_id\": \"device_id\",\"protocol\": \"PDI-IoTA-UltraLight\",\"entity_name\": \"room_ut1\",\"entity_type\": \"type2\",\"endpoint\": \"http://127.0.0.1:9999/device\",\"timezone\": \"America/Santiago\","
                       "\"commands\": [{\"name\": \"PING\",\"type\": \"command\",\"value\": \"\" }],"
                       "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }]"
                       ",\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
                       "}]}");

const std::string
Ul20Test::POST_DEVICE_ENDPOINT("{\"devices\": "
                       "[{\"device_id\": \"%s\",\"protocol\": \"PDI-IoTA-UltraLight\",\"entity_name\": \"%s\",\"entity_type\": \"type2\",\"endpoint\": \"http://127.0.0.1:9999/device\",\"timezone\": \"America/Santiago\""
                        "}]}");

const std::string
Ul20Test::POST_DEVICE_NO_ENDPOINT("{\"devices\": "
                       "[{\"device_id\": \"%s\",\"protocol\": \"PDI-IoTA-UltraLight\",\"entity_name\": \"%s\",\"entity_type\": \"type2\",\"endpoint\": \"\",\"timezone\": \"America/Santiago\""
                        "}]}");

const std::string
Ul20Test::POST_DEVICE2("{\"devices\": "
                       "[{\"device_id\": \"device_id\",\"protocol\": \"PDI-IoTA-UltraLight\",\"entity_name\": \"room_ut1\",\"entity_type\": \"type2\",\"endpoint\": \"http://127.0.0.1:9999/device\",\"timezone\": \"America/Santiago\","
                       "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }]"
                       ",\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
                       "}]}");

const std::string
Ul20Test::POST_DEVICE_CON("{\"devices\": "
                       "[{\"device_id\": \"unitTest_dev3_polling\",\"protocol\": \"PDI-IoTA-UltraLight\",\"entity_name\": \"room_ut3\",\"entity_type\": \"type2\",\"timezone\": \"America/Santiago\","
                       "\"commands\": [{\"name\": \"PING\",\"type\": \"command\",\"value\": \"\" }],"
                       "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }]"
                       ",\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
                       "}]}");
const std::string
Ul20Test::POST_DEVICE_CON2("{\"devices\": "
                       "[{\"device_id\": \"unitTest_dev32_polling\",\"protocol\": \"PDI-IoTA-UltraLight\",\"entity_type\": \"type2\",\"timezone\": \"America/Santiago\","
                       "\"commands\": [{\"name\": \"PING\",\"type\": \"command\",\"value\": \"\" }],"
                       "\"attributes\": [{\"object_id\": \"temp\",\"name\": \"temperature\",\"type\": \"int\" }]"
                       ",\"static_attributes\": [{\"name\": \"humidity\",\"type\": \"int\", \"value\": \"50\"  }]"
                       "}]}");


const std::string
Ul20Test::PUT_DEVICE("{\"activate\": false ,"
                     " \"commands\": [{\"name\": \"PING\",\"type\": \"command\",\"value\": \"device_id@ping|%s\" }]"
                     " }");
const std::string
Ul20Test::PUT_DEVICE2("{ \"activate\": 0 } ");

const std::string
Ul20Test::UPDATE_CONTEXT("{\"updateAction\":\"UPDATE\","
      "\"contextElements\":[{\"id\":\"room_ut1\",\"type\":\"type2\",\"isPattern\":\"false\","
      "\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"22\","
      "\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}"
      "]} ]}");

const std::string
Ul20Test::UPDATE_CONTEXT_sin_param("{\"updateAction\":\"UPDATE\","
      "\"contextElements\":[{\"id\":\"room_ut1\",\"type\":\"type2\",\"isPattern\":\"false\","
      "\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"\","
      "\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}"
      "]} ]}");

const int
Ul20Test::RESPONSE_CODE_NGSI = 200;

const std::string
Ul20Test::RESPONSE_MESSAGE_NGSI_OK("{\"contextResponses\":[{\"statusCode\":{\"code\":\"200\",\"reasonPhrase\":\"OK\",\"details\":\"\"},\"contextElement\":{\"id\":");


const std::string Ul20Test::HOST("127.0.0.1");
const std::string Ul20Test::CONTENT_JSON("aplication/json");

CPPUNIT_TEST_SUITE_REGISTRATION(Ul20Test);
namespace iota {
std::string logger("main");
std::string URL_BASE("/iot");
}
iota::AdminService* AdminService_ptr;



void Ul20Test::setUp() {
  std::cout << "setUp Ul20Test " << std::endl;


  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
}

void Ul20Test::tearDown() {
  std::cout << "tearDown Ul20Test " << std::endl;
}

void Ul20Test::start_cbmock(boost::shared_ptr<HttpMock>& cb_mock,
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
     <<   "\"timeout\": 1,"
     <<   "\"dir_log\": \"/tmp/\","
     <<   "\"timezones\": \"/etc/iot/date_time_zonespec.csv\","
    <<   "\"schema_path\": \"../../schema\","
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
     <<   "\"cbroker\": \"http://127.0.0.1:" << mock_port << "/mock\", "
     <<   "\"entity_type\": \"thing_apikey3\","
     <<   "\"attributes\": ["
     <<     "{\"object_id\":\"t\",\"type\": \"string\",\"name\":\"temperature\"}"
     <<   "]"
     << "} ] } ] }";
     }
     else {
     ss << "{ \"ngsi_url\": {"
     <<   "     \"updateContext\": \"/NGSI10/updateContext\","
     <<   "     \"registerContext\": \"/NGSI9/registerContext\","
     <<   "     \"queryContext\": \"/NGSI10/queryContext\""
     <<   "},"
     <<   "\"timeout\": 1,"
     <<   "\"http_proxy\": \"127.0.0.1:8888\","
     <<   "\"dir_log\": \"/tmp/\","
     <<   "\"timezones\": \"/etc/iot/date_time_zonespec.csv\","
    <<   "\"schema_path\": \"../../schema\","
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


/***
  *  POST http://10.95.26.51:8002/d?i=Device_UL2_0_RESTv2&k=4orh3jl3h40qkd7fk2qrc52ggb
  *     ${#Project#END_TIME2}|t|${Properties#value}
  *     ${#Project#END_TIME2}|t|${Properties#value}#t|${Properties#value2}
  *     ${#Project#END_TIME}|t|${Properties#value}#l|${Properties#value2}/${Properties#value2_1}
  **/
void Ul20Test::testNormalPOST() {
  std::cout << "START testNormalPOST" << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);
  std::string cb_last;

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");


  // fecha  + temperatura
  std::string querySTR = "i=unitTest_dev1_endpoint&k=apikey3";
  std::string bodySTR = "2014-02-18T16:41:20Z|t|23";
  std::cout << "@UT@284 " << querySTR << "<-->" << bodySTR << std::endl;
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);
    http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST fecha + temperatura " <<
        http_response.get_status_code() << response <<
              std::endl;
    IOTASSERT_MESSAGE("response code not is 200",
                           http_response.get_status_code() == RESPONSE_CODE_NGSI);
    ASYNC_TIME_WAIT
    // updateContext to CB
    cb_last = cb_mock->get_last();
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT_MESSAGE("translate the name of device",
                           cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                           std::string::npos);
    IOTASSERT_MESSAGE("translate alias of observation",
                           cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"23\"")
                           !=
                           std::string::npos);
    IOTASSERT_MESSAGE("add statis attributes",
                           cb_last.find("{\"name\":\"att_name_static\",\"type\":\"string\",\"value\":\"value\"")
                           !=
                           std::string::npos);

  }
  // No attribute mapping
  {
    std::string bodySTR_no_mapping = "2014-02-18T16:41:20Z|nomap|23";
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR_no_mapping);
    http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST fecha + nomap " <<
        http_response.get_status_code() << response <<
              std::endl;
    IOTASSERT_MESSAGE("response code not is 200",
                           http_response.get_status_code() == RESPONSE_CODE_NGSI);
    ASYNC_TIME_WAIT
    // updateContext to CB
    cb_last = cb_mock->get_last();
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT_MESSAGE("translate the name of device",
                           cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                           std::string::npos);
    IOTASSERT_MESSAGE(" observation",
                           cb_last.find("{\"name\":\"nomap\",\"type\":\"string\",\"value\":\"23\"")
                           !=
                           std::string::npos);
    IOTASSERT_MESSAGE("add statis attributes",
                           cb_last.find("{\"name\":\"att_name_static\",\"type\":\"string\",\"value\":\"value\"")
                           !=
                           std::string::npos);

  }
  std::cout << "@UT@366 " << bodySTR << std::endl;
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);
    http_request->add_header(iota::types::FIWARE_SERVICE, "service22");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv22");

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "no_device"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey33"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST fecha + temperatura " <<
        http_response.get_status_code() << response <<
              std::endl;
    IOTASSERT_MESSAGE("response code not is 200",
                           http_response.get_status_code() == RESPONSE_CODE_NGSI);
    ASYNC_TIME_WAIT
    // updateContext to CB
    cb_last = cb_mock->get_last();
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT_MESSAGE("translate the name of device",
                           cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                           std::string::npos);
    IOTASSERT_MESSAGE("translate alias of observation",
                           cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"23\"")
                           !=
                           std::string::npos);
    IOTASSERT_MESSAGE("add statis attributes",
                           cb_last.find("{\"name\":\"att_name_static\",\"type\":\"string\",\"value\":\"value\"")
                           !=
                           std::string::npos);

  }
  // Dos Medidas
  bodySTR = "2014-02-18T16:41:20Z|t|23#t|24";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST dos medida " << http_response.get_status_code() << std::endl;
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);
    ASYNC_TIME_WAIT
    // updateContext to CB
    cb_last = cb_mock->get_last();
    cb_last.append(cb_mock->get_last());
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"24\"")
      !=
      std::string::npos);
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"23\"")
      !=
      std::string::npos);

  }

  // medida localizacion
  bodySTR = "2014-02-18T16:41:20Z|t|23#l|-3.3423/2.345";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST medida + localizacion " << http_response.get_status_code() <<
              std::endl;
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);

    ASYNC_TIME_WAIT
    // we don't know the order of meassurements to CB, so we join the two observations
    cb_last = cb_mock->get_last();
    cb_last.append(cb_mock->get_last());

    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(cb_last.find("\"name\":\"position\",\"type\":\"coords\""
                                ",\"value\":\"-3.3423,2.345\",\"metadatas\":[{\"name\":\"location\""
                                ",\"type\":\"string\",\"value\":\"WGS84\"") !=
                   std::string::npos);

    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"23\"")
      !=
      std::string::npos);
  }

  // sin fecha dos medidas
  bodySTR = "|t|23#|t|24";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    ASYNC_TIME_WAIT
    std::cout << "POST dos medidas sin fecha " << http_response.get_status_code() <<
              std::endl;
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);
    // updateContext to CB
    // we don't know the order of meassurements to CB, so we join the two observations
    cb_last = cb_mock->get_last();
    cb_last.append(cb_mock->get_last());

    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"24\"")
      !=
      std::string::npos);
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT(
      cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"23\"")
      !=
      std::string::npos);
  }

  // sin fecha dos medidas
  bodySTR = "|t|23#|l|-3.3423/2.345";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    ASYNC_TIME_WAIT
    std::cout << "POST sin fecha medida + localizacion " <<
              http_response.get_status_code() << std::endl;
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);
    // updateContext to CB
    // we don't know the order of meassurements to CB, so we join the two observations
    cb_last = cb_mock->get_last();
    cb_last.append(cb_mock->get_last());

    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(cb_last.find("\"name\":\"position\",\"type\":\"coords\""
                                ",\"value\":\"-3.3423,2.345\",\"metadatas\":[{\"name\":\"location\""
                                ",\"type\":\"string\",\"value\":\"WGS84\"") !=
                   std::string::npos);

    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"23\"")
      !=
      std::string::npos);

  }
  cb_mock->stop();
  std::cout << "END testNormalPOST " << std::endl;
}

/***
  *  GET http://10.95.26.51:8002/d?i=Device_UL2_0_RESTv2&k=4orh3jl3h40qkd7fk2qrc52ggb
  *       parameters   i define device
  *                    d parameter to define the data. It is a key and a value separated by |.
  *                    k apikey of the service
  *                    t UTC ISO8601) for the observation
  *                    ip to define IP address
  *                    getCmd  if getCmd=1, in the response add all commands for this device
  **/
void Ul20Test::testFileGET() {
  std::cout << "START testFileGET" << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);
  std::string cb_last;

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");


  // fecha  + temperatura
  std::string querySTR = "i=unitTest_dev1_endpoint&k=apikey3&t=2014-02-18T16:41:20Z&d=t|23";
  std::cout << "@UT@i=unitTest_dev1_endpoint&k=apikey3&t=2014-02-18T16:41:20Z&d=t|23" << std::endl;
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("GET");
    http_request->set_query_string(querySTR);
    http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    query_parameters.insert(std::pair<std::string,std::string>("t","2014-02-18T16:41:20Z"));
    query_parameters.insert(std::pair<std::string,std::string>("d","t|23"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "GET fecha + temperatura " <<
        http_response.get_status_code() << response <<
              std::endl;
    IOTASSERT_MESSAGE("response code not is 200",
                           http_response.get_status_code() == RESPONSE_CODE_NGSI);
    ASYNC_TIME_WAIT
    // updateContext to CB
    cb_last = cb_mock->get_last();
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT_MESSAGE("translate the name of device",
                           cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                           std::string::npos);
    IOTASSERT_MESSAGE("translate alias of observation",
                           cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"23\"")
                           !=
                           std::string::npos);
    IOTASSERT_MESSAGE("add statis attributes",
                           cb_last.find("{\"name\":\"att_name_static\",\"type\":\"string\",\"value\":\"value\"")
                           !=
                           std::string::npos);

  }
  // No attribute mapping
  {
    querySTR = "i=unitTest_dev1_endpoint&k=apikey3&t=2014-02-18T16:41:20Z&d=nomap|23";
    std::cout << "@UT@i=unitTest_dev1_endpoint&k=apikey3&t=2014-02-18T16:41:20Z&d=nomap|23" << std::endl;
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("GET");
    http_request->set_query_string(querySTR);
    http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    query_parameters.insert(std::pair<std::string,std::string>("t","2014-02-18T16:41:20Z"));
    query_parameters.insert(std::pair<std::string,std::string>("d","nomap|23"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "GET fecha + nomap " <<
        http_response.get_status_code() << response <<
              std::endl;
    IOTASSERT_MESSAGE("response code not is 200",
                           http_response.get_status_code() == RESPONSE_CODE_NGSI);
    ASYNC_TIME_WAIT
    // updateContext to CB
    cb_last = cb_mock->get_last();
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT_MESSAGE("translate the name of device",
                           cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                           std::string::npos);
    IOTASSERT_MESSAGE(" observation",
                           cb_last.find("{\"name\":\"nomap\",\"type\":\"string\",\"value\":\"23\"")
                           !=
                           std::string::npos);
    IOTASSERT_MESSAGE("add statis attributes",
                           cb_last.find("{\"name\":\"att_name_static\",\"type\":\"string\",\"value\":\"value\"")
                           !=
                           std::string::npos);

  }
  {
    querySTR = "i=no_device&k=apikey3&t=2014-02-18T16:41:20Z&d=t|23";
    std::cout << "@UT@i=no_device&k=apikey3&t=2014-02-18T16:41:20Z&d=t|23" << std::endl;
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("GET");
    http_request->set_query_string(querySTR);
    http_request->add_header(iota::types::FIWARE_SERVICE, "service22");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv22");

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "no_device"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey33"));
    query_parameters.insert(std::pair<std::string,std::string>("t","2014-02-18T16:41:20Z"));
    query_parameters.insert(std::pair<std::string,std::string>("d","t|23"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST fecha + temperatura " <<
        http_response.get_status_code() << response <<
              std::endl;
    IOTASSERT_MESSAGE("response code not is 200",
                           http_response.get_status_code() == RESPONSE_CODE_NGSI);
    ASYNC_TIME_WAIT
    // updateContext to CB
    cb_last = cb_mock->get_last();
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT_MESSAGE("translate the name of device",
                           cb_last.find("\"id\":\"thing_apikey3:no_device\",\"type\":\"thing_apikey3\"") !=
                           std::string::npos);
    IOTASSERT_MESSAGE("translate alias of observation",
                           cb_last.find("\"name\":\"temperature\",\"type\":\"string\",\"value\":\"23\"")
                           !=
                           std::string::npos);

  }
  // Dos Medidas
  querySTR = "i=unitTest_dev1_endpoint&k=apikey3&t=2014-02-18T16:41:20Z&d=t|23&d=t|24";
  std::cout << "@UT@i=unitTest_dev1_endpoint&k=apikey3&t=2014-02-18T16:41:20Z&d=t|23&d=t|24" << std::endl;
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("GET");
    http_request->set_query_string(querySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    query_parameters.insert(std::pair<std::string,std::string>("t","2014-02-18T16:41:20Z"));
    query_parameters.insert(std::pair<std::string,std::string>("d","t|23"));
    query_parameters.insert(std::pair<std::string,std::string>("d","t|24"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "@UT@POST dos medida " << http_response.get_status_code() << std::endl;
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);
    ASYNC_TIME_WAIT
    // updateContext to CB
    cb_last = cb_mock->get_last();
    cb_last.append(cb_mock->get_last());
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"24\"")
      !=
      std::string::npos);
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"23\"")
      !=
      std::string::npos);


  }

  // medida localizacion
  querySTR = "i=unitTest_dev1_endpoint&k=apikey3&t=2014-02-18T16:41:20Z&d=t|23&d=l|-3.3423/2.345";
  std::cout << "@UT@" << querySTR << std::endl;
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("GET");
    http_request->set_query_string(querySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    query_parameters.insert(std::pair<std::string,std::string>("t","2014-02-18T16:41:20Z"));
    query_parameters.insert(std::pair<std::string,std::string>("d","t|23"));
    query_parameters.insert(std::pair<std::string,std::string>("d","l|-3.3423/2.345"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST medida + localizacion " << http_response.get_status_code() <<
              std::endl;
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);

    ASYNC_TIME_WAIT
    // we don't know the order of meassurements to CB, so we join the two observations
    cb_last = cb_mock->get_last();
    cb_last.append(cb_mock->get_last());

    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(cb_last.find("\"name\":\"position\",\"type\":\"coords\""
                                ",\"value\":\"-3.3423,2.345\",\"metadatas\":[{\"name\":\"location\""
                                ",\"type\":\"string\",\"value\":\"WGS84\"") !=
                   std::string::npos);

    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"23\"")
      !=
      std::string::npos);
  }

  cb_mock->stop();
  std::cout << "END testFileGET " << std::endl;
}

void Ul20Test::testMongoGET() {
  std::cout << "START testMongoGET" << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock, "mongodb");
  std::string cb_last;

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");
  boost::property_tree::ptree srv_ptree;
  std::cout << "get_service_by_apiKey" << std::endl;
  //TODO ul20serv.get_service_by_apiKey(srv_ptree, "ddd");

  // fecha  + temperatura
  std::string querySTR = "i=unitTest_dev1_endpoint&k=apikey3&t=2014-02-18T16:41:20Z&d=t|23";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("GET");
    http_request->set_query_string(querySTR);
    http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    query_parameters.insert(std::pair<std::string,std::string>("t","2014-02-18T16:41:20Z"));
    query_parameters.insert(std::pair<std::string,std::string>("d","t|23"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "GET fecha + temperatura " <<
        http_response.get_status_code() << response <<
              std::endl;
    IOTASSERT_MESSAGE("response code not is 200",
                           http_response.get_status_code() == RESPONSE_CODE_NGSI);
    ASYNC_TIME_WAIT
    // updateContext to CB
    cb_last = cb_mock->get_last();
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT_MESSAGE("translate the name of device",
                           cb_last.find("\"id\":\"thing_apikey3:unitTest_dev1_endpoint\",\"type\":\"thing_apikey3\"") !=
                           std::string::npos);
    IOTASSERT_MESSAGE("translate alias of observation",
                           cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"23\"")
                           !=
                           std::string::npos);

  }
  // No attribute mapping
  {
    querySTR = "i=unitTest_dev1_endpoint&k=apikey3&t=2014-02-18T16:41:20Z&d=nomap|23";
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("GET");
    http_request->set_query_string(querySTR);
    http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    query_parameters.insert(std::pair<std::string,std::string>("t","2014-02-18T16:41:20Z"));
    query_parameters.insert(std::pair<std::string,std::string>("d","nomap|23"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "GET fecha + nomap " <<
        http_response.get_status_code() << response <<
              std::endl;
    IOTASSERT_MESSAGE("response code not is 200",
                           http_response.get_status_code() == RESPONSE_CODE_NGSI);
    ASYNC_TIME_WAIT
    // updateContext to CB
    cb_last = cb_mock->get_last();
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT_MESSAGE("translate the name of device",
                           cb_last.find("\"id\":\"thing_apikey3:unitTest_dev1_endpoint\",\"type\":\"thing_apikey3\"") !=
                           std::string::npos);
    IOTASSERT_MESSAGE(" observation",
                           cb_last.find("{\"name\":\"nomap\",\"type\":\"string\",\"value\":\"23\"")
                           !=
                           std::string::npos);

  }
  {
    querySTR = "i=no_device&k=apikey3&t=2014-02-18T16:41:20Z&d=t|23";
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("GET");
    http_request->set_query_string(querySTR);
    http_request->add_header(iota::types::FIWARE_SERVICE, "service22");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv22");

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "no_device"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey33"));
    query_parameters.insert(std::pair<std::string,std::string>("t","2014-02-18T16:41:20Z"));
    query_parameters.insert(std::pair<std::string,std::string>("d","t|23"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST fecha + temperatura " <<
        http_response.get_status_code() << response <<
              std::endl;
    IOTASSERT_MESSAGE("response code not is 200",
                           http_response.get_status_code() == RESPONSE_CODE_NGSI);
    ASYNC_TIME_WAIT
    // updateContext to CB
    cb_last = cb_mock->get_last();
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT_MESSAGE("translate the name of device",
                           cb_last.find("\"id\":\"thing_apikey3:no_device\",\"type\":\"thing_apikey3\"") !=
                           std::string::npos);
    IOTASSERT_MESSAGE("translate alias of observation",
                           cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"23\"")
                           !=
                           std::string::npos);

  }
  // Dos Medidas
  querySTR = "i=unitTest_dev1_endpoint&k=apikey3&t=2014-02-18T16:41:20Z&d=t|23&d=t|24";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("GET");
    http_request->set_query_string(querySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    query_parameters.insert(std::pair<std::string,std::string>("t","2014-02-18T16:41:20Z"));
    query_parameters.insert(std::pair<std::string,std::string>("d","t|23"));
    query_parameters.insert(std::pair<std::string,std::string>("d","t|24"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST dos medida " << http_response.get_status_code() << std::endl;
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);
    ASYNC_TIME_WAIT
    // updateContext to CB
    cb_last = cb_mock->get_last();
    cb_last.append(cb_mock->get_last());
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"thing_apikey3:unitTest_dev1_endpoint\",\"type\":\"thing_apikey3\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"24\"")
      !=
      std::string::npos);
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"thing_apikey3:unitTest_dev1_endpoint\",\"type\":\"thing_apikey3\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"23\"")
      !=
      std::string::npos);

  }

  // medida localizacion
  querySTR = "i=unitTest_dev1_endpoint&k=apikey3&t=2014-02-18T16:41:20Z&d=t|23&d=l|-3.3423/2.345";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("GET");
    http_request->set_query_string(querySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    query_parameters.insert(std::pair<std::string,std::string>("t","2014-02-18T16:41:20Z"));
    query_parameters.insert(std::pair<std::string,std::string>("d","t|23"));
    query_parameters.insert(std::pair<std::string,std::string>("d","l|-3.3423/2.345"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST medida + localizacion " << http_response.get_status_code() <<
              std::endl;
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);

    ASYNC_TIME_WAIT
    // we don't know the order of meassurements to CB, so we join the two observations
    cb_last = cb_mock->get_last();
    cb_last.append(cb_mock->get_last());

    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"thing_apikey3:unitTest_dev1_endpoint\",\"type\":\"thing_apikey3\"") !=
                   std::string::npos);
    IOTASSERT(cb_last.find("\"name\":\"l\",\"type\":\"string\",\"value\":\"-3.3423/2.345\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"23\"")
      !=
      std::string::npos);
  }

  cb_mock->stop();
  std::cout << "END testMongoGET " << std::endl;
}


/**
 * //POST http://10.95.26.51:8002/d?t=2014-02-18T16:41:20Z&i=Device_UL2_0_RESTv2&k=4orh3jl3h40qkd7fk2qrc52ggb HTTP/1.1
   //     t|${Properties#value}

   //t|${Properties#value}#t|${Properties#value2}#t|${Properties#value3}
 */
void Ul20Test::testTimePOST() {

  std::cout << "START testTimePOST " << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");

  // fecha  + temperatura
  std::string querySTR =
    "t=2014-02-18T16:41:20Z&i=unitTest_dev1_endpoint&k=apikey3";
  std::string bodySTR;

  bodySTR.assign("t|23#t|24");
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("t",
                            "2014-02-18T16:41:20Z"));
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST for t|23#t|24 " << http_response.get_status_code() <<
              std::endl;
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);
  }

  bodySTR.assign("t|23#l|-3.3423/2.345");
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("t",
                            "2014-02-18T16:41:20Z"));
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST for  t|23#l|-3.3423/2.345 " <<
              http_response.get_status_code() << std::endl;
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);
  }
  cb_mock->stop();
  std::cout << "END testTimePOST " << std::endl;
}

void Ul20Test::testBadPost() {

  std::cout << "START testBadPost" << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");

  // Medida sin alias
  std::string querySTR = "i=unitTest_dev1_endpoint&k=apikey3";
  std::string bodySTR = "|22";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST sin alias " << http_response.get_status_code() << std::endl;
    IOTASSERT(http_response.get_status_code() == 400);
  }
  // Medida sin value
  bodySTR = "t|";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST sin medida " << http_response.get_status_code() << std::endl;
    IOTASSERT(http_response.get_status_code() == 400);
  }

  // Horas erroneas
  bodySTR = "2014-02-181641:20Z|t|22";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST hora erronea 2014-02-181641:20Z|t|22 " <<
              http_response.get_status_code() << std::endl;
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);;
  }

  bodySTR = "2014-02-18T1641:20Z|t|22";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST hora erronea 2014-02-18T1641:20Z|t|22 " <<
              http_response.get_status_code() << std::endl;
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);;
  }
  // falta barra
  bodySTR = "t22";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_dev1_endpoint"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST pegote " << http_response.get_status_code() << std::endl;
    IOTASSERT(http_response.get_status_code() == 400);
  }

  cb_mock->stop();
  std::cout << "END testBadPost " << std::endl;
}

/**
 * POST of a measure with a device no registered
 **/
void Ul20Test::testNoDevicePost() {
  std::cout << "START testNoDevicePost" << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);
  std::string cb_last;

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");

  std::string querySTR = "i=unitTest_device_tegistered&k=apikey3";
  std::string bodySTR = "t|22#l|-3.3423/2.345";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",
                            "unitTest_device_tegistered"));
    query_parameters.insert(std::pair<std::string,std::string>("k","apikey3"));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    ASYNC_TIME_WAIT
    std::cout << "POST device no registered " << http_response.get_status_code() <<
              std::endl;
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);
    ASYNC_TIME_WAIT

    cb_last = cb_mock->get_last();
    cb_last.append(cb_mock->get_last());
    // updateContext to CB
    std::cout << "@UT@CB"<< cb_last << std::endl;
    IOTASSERT(
      cb_last.find("\"id\":\"thing_apikey3:unitTest_device_tegistered\",\"type\":\"thing_apikey3\"")
      !=
      std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"l\",\"type\":\"string\",\"value\":\"-3.3423/2.345\"") !=
      std::string::npos);
    IOTASSERT(
      cb_last.find("\"id\":\"thing_apikey3:unitTest_device_tegistered\",\"type\":\"thing_apikey3\"")
      !=
      std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"temperature\",\"type\":\"string\",\"value\":\"22\"") !=
      std::string::npos);

  }

  cb_mock->stop();
  std::cout << "END testNoDevicePost " << std::endl;
}

void Ul20Test::testRiotISO8601() {

  std::cout << "START testRiotISO8601 " << std::endl;

  iota::RiotISO8601 firstIso(std::string("2014-10-18T16:41:20Z"));

  IOTASSERT(firstIso.get_year() == 2014);
  IOTASSERT(firstIso.get_month() == 10);
  IOTASSERT(firstIso.get_day() == 18);
  IOTASSERT(firstIso.get_hours() == 16);
  IOTASSERT(firstIso.get_minutes() == 41);
  IOTASSERT(firstIso.get_seconds() == 20);

  iota::RiotISO8601 secondIso("2014", "10", "18", "20", "10", "10");
  iota::RiotISO8601 thirdIso(secondIso);

  if (firstIso != secondIso) {
    IOTASSERT(1 == 1);
  }
  if (firstIso == secondIso) {
    IOTASSERT(1 == 0);
  }

  IOTASSERT(secondIso.toString() == thirdIso.toString());
  IOTASSERT(secondIso.toSimpleISOString() == thirdIso.toSimpleISOString());
  IOTASSERT(secondIso.toIdString() == thirdIso.toIdString());
  IOTASSERT(secondIso.get_microseconds() == thirdIso.get_microseconds());


  iota::RiotISO8601 badIso(std::string("2014-10-18T1040:20Z"));
  std::cout << badIso.get_day() << "-" << badIso.get_month() << "-"
            <<badIso.get_year()
            << " " << badIso.get_hours() << ":" <<badIso.get_minutes() << ":" <<
            badIso.get_seconds() << std::endl;
  IOTASSERT(badIso.get_hours() == 10);

  try {
    iota::RiotISO8601 badIso(std::string("2014-10-181040:20Z"));
    CPPUNIT_FAIL("No exception with 2014-10-181040:20Z");
  }
  catch (std::runtime_error& e) {
    std::cout << e.what() << std::endl;
  }

  std::cout << "END testRiotISO8601 " << std::endl;
}

void Ul20Test::testTranslate() {
  boost::shared_ptr<iota::Device> dev;
  std::cout << "START testTranslate " << std::endl;;
  boost::property_tree::ptree service_ptree;


  iota::ULInsertObservation io_ul_idas;

  std::string content = "2014-02-18T16:41:20Z|t|23|t|23";
  std::vector<iota::KVP> querySBC;
  iota::KVP id("ID", "unitTest_dev1_endpoint");
  querySBC.push_back(id);
  iota::KVP k("apikey", "apikey3");
  querySBC.push_back(k);

  std::vector<iota::ContextElement>cb_elto;
  try {
    io_ul_idas.translate(content, dev, service_ptree, querySBC, cb_elto, 0);
    std::cout << "res1: " << std::endl;
    for (int index=0; index< cb_elto.size(); ++index) {
      std::cout << cb_elto[index].get_string() << std::endl;
    }
    IOTASSERT_MESSAGE("No Protocol Exception for 2014-02-18T16:41:20Z|t|23|t|23",
                           true);
  }
  catch (std::runtime_error& e) {
    std::cout << e.what() << std::endl;
  }

  try {
    std::vector<iota::ContextElement> cb_elto2;
    content = "t|23#t|23";
    io_ul_idas.translate(content, dev, service_ptree, querySBC, cb_elto2, 0);
    std::cout << "res2: " << std::endl;
    for (int index=0; index< cb_elto2.size(); ++index) {
      std::cout << cb_elto2[index].get_string() << std::endl;
    }

    std::vector<iota::ContextElement> cb_elto3;
    content = "|t|23#t|23";
    io_ul_idas.translate(content, dev, service_ptree, querySBC, cb_elto3, 0);
    std::cout << "res3: " <<  std::endl;
    for (int index=0; index< cb_elto3.size(); ++index) {
      std::cout << cb_elto3[index].get_string() << std::endl;
    }

    std::vector<iota::ContextElement> cb_elto4;
    content = "|t|23#|t|23";
    io_ul_idas.translate(content, dev, service_ptree, querySBC, cb_elto4, 0);
    std::cout << "res4: "  << std::endl;
    for (int index=0; index< cb_elto4.size(); ++index) {
      std::cout << cb_elto4[index].get_string() << std::endl;
    }

  }
  catch (std::runtime_error& e) {
    std::cout << " Exception for " << content <<" -->" << e.what() << std::endl;
    IOTASSERT_MESSAGE(content,true);
  }

  std::cout << "END testTranslate " << std::endl;

}

void Ul20Test::testGetAllCommand() {
  std::cout << "START testGetAllCommand " << std::endl;
  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");
  try {
    std::string command_name="ping";
    std::string command_id;
    int timeout = 22;
    boost::property_tree::ptree pt;
    pt.put("body", "dev1@ping");
    std::string item_dev_name="dev1";
    std::string entity_type;
    std::string endpoint;
    std::string sequence;
    int   status=0;
    std::string apikey = "apikey3";
    boost::property_tree::ptree service_ptree;
    ul20serv.get_service_by_apiKey(service_ptree, apikey);
    std::string service = service_ptree.get<std::string>(iota::store::types::SERVICE, "");
    std::string service_path = service_ptree.get<std::string>(iota::store::types::SERVICE_PATH, "");

    boost::shared_ptr<iota::Device> dev;
    dev.reset(new iota::Device(item_dev_name, service));
    dev->_entity_type = entity_type;

    // guardamos elcomando en cache
    ul20serv.save_command(command_name, command_id,
                          timeout,pt,
                          dev, entity_type,
                          endpoint, service_ptree,
                          sequence, status);

    // le devolvemos los comandos ready_for_read al device
    iota::CommandVect cmdPtes = ul20serv.get_all_command(dev, service_ptree);
    IOTASSERT(cmdPtes.size() ==1);

    // han pasado a DELIVERED  y si se vuelve a preguntar por comandos ya no hay
    iota::CommandVect cmdPtes2 = ul20serv.get_all_command(dev, service_ptree);
    IOTASSERT(cmdPtes2.size() == 0);

    //llega la respuesta y borramos el comando
    ul20serv.remove_command(command_id, service, service_path);
  }
  catch (std::exception& exc) {
    std::cout << exc.what() << std::endl;
  }
  catch (...) {
    std::cout << "EXCEPTION" << std::endl;
  }
  //comprobar que ya no hay comandos en la cache

  std::cout << "END testGetAllCommand " << std::endl;
}

void Ul20Test::testDevices() {
  std::cout << "START.. testDevices " << std::endl;

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");

  std::string service = "service2";
  std::string subservice = "/ssrv2";
  {
    std::cout << "@UT@searh with subservice" << std::endl;
    std::string ent1 = "unitTest_dev1_endpoint";
    std::cout << "unitTest_dev1_endpoint " << std::endl;
    boost::shared_ptr<iota::Device> dev = ul20serv.get_device(ent1, service, subservice);
    IOTASSERT(dev.get() != NULL);

  }

  {
    std::cout << "@UT@searh without subservice" << std::endl;
    std::string ent1 = "unitTest_dev1_endpoint";
    std::cout << "unitTest_dev1_endpoint " << std::endl;
    boost::shared_ptr<iota::Device> dev = ul20serv.get_device(ent1, service);
    CPPUNIT_ASSERT(dev.get() != NULL);

  }

  {
    std::string ent1 = "room_ut1";
    std::cout << "room_ut1 " << std::endl;
    boost::shared_ptr<iota::Device> dev = ul20serv.get_device_by_entity(ent1, "type2",
                                    service, subservice);
    IOTASSERT(dev.get() != NULL);
  }

  {
    try {
      std::string ent1 = "room_ut2";
      std::cout << "room_ut2" << std::endl;
      boost::shared_ptr<iota::Device> dev = ul20serv.get_device_by_entity(ent1, "type2",
                                      service, subservice);
      std::cout << "inactive must throw an exceptionnn" << std::endl;
      CPPUNIT_FAIL("inactive must throw an exception");
    }
    catch (std::runtime_error& e) {
      std::cout << "inactive throws an runtime_error" << std::endl;
      std::string errSTR = e.what();
      std::cout << errSTR << std::endl;
    }
    catch (...) {
      std::cout << "inactive throws an exceptionnnn" << std::endl;
    }
  }

  std::cout << "END testDevices " << std::endl;
}

void Ul20Test::testTransformCommand() {
  std::cout << "START testTransformCommand " << std::endl;

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");

  std::string service = "service2";
  std::string id, res1;

  std::string command_name="PING";
  std::string sequence_id;
  boost::shared_ptr<iota::Device> item_dev(new iota::Device("dev1", "service"));
  const boost::property_tree::ptree ptreeservice;
  std::string command_id;
  std::string command_line;

 {
    std::cout << "@UT@normal command " << std::endl;
    std::string provisioned_data = "";
    std::string updateContext_data = "";
    boost::property_tree::ptree pt;
    ul20serv.transform_command(command_name, provisioned_data, updateContext_data,
                        sequence_id, item_dev, ptreeservice, id, pt);
    res1 = pt.get("body", "");
    std::cout << "@UT@res:" << res1 << std::endl;
    IOTASSERT(res1.compare("dev1@PING") == 0);
  }

  {
    std::cout << "@UT@normal command with parameters " << std::endl;
    std::string provisioned_data = "";
    std::string updateContext_data = "param1|param2";
    boost::property_tree::ptree pt;
    ul20serv.transform_command(command_name, provisioned_data, updateContext_data,
                        sequence_id, item_dev, ptreeservice, id, pt);
    res1 = pt.get("body", "");
    std::cout << "@UT@res:" << res1 << std::endl;
    IOTASSERT(res1.compare("dev1@PING|param1|param2") == 0);
  }

 {
    std::cout << "@UT@raw command " << std::endl;
    std::string provisioned_data = "@@RAW@@";
    std::string updateContext_data = "updateContextValue@command";
    boost::property_tree::ptree pt;
    ul20serv.transform_command(command_name, provisioned_data, updateContext_data,
                        sequence_id, item_dev, ptreeservice, id, pt);
    res1 = pt.get("body", "");
    std::cout << "@UT@res:" << res1 << std::endl;
    IOTASSERT(res1.compare(updateContext_data) == 0);
  }

  {
    std::cout << "@UT@format command " << std::endl;
    std::string provisioned_data = "dev1@ping";
    std::string updateContext_data = "";
    boost::property_tree::ptree pt;
    std::string parameters1;
    ul20serv.transform_command(command_name, provisioned_data, updateContext_data,
                        sequence_id, item_dev, ptreeservice, id, pt);
    res1 = pt.get("body", "");
    std::cout << "@UT@res:" << res1 << std::endl;
    IOTASSERT(res1.compare(provisioned_data) == 0);
  }
  {
    std::cout << "@UT@format command2 " << std::endl;
    boost::property_tree::ptree pt;
    std::string provisioned_data = "dev1@set|%sfin";
    std::string updateContext_data = "2014-02-18T16:41:20Z";
    ul20serv.transform_command(command_name, provisioned_data, updateContext_data,
                        sequence_id, item_dev, ptreeservice, id, pt);
    res1 = pt.get("body", "");
    std::cout << "@UT@res:" << res1 << std::endl;
    IOTASSERT(res1.compare("dev1@set|2014-02-18T16:41:20Zfin") == 0);
  }
  {
    boost::property_tree::ptree pt;
    std::string cmd1 = "dev1@set|fecha:%s|p1:%sfin";
    std::cout << "@UT@command:" <<cmd1<< std::endl;
    std::string parameters1 = "2014-02-18T16:41:20Z|22";
    ul20serv.transform_command(command_name, cmd1, parameters1,
                        sequence_id, item_dev, ptreeservice, id, pt);
    res1 = pt.get("body", "");
    std::cout << "@UT@res:" << res1 << std::endl;
    IOTASSERT(res1.compare("dev1@set|fecha:2014-02-18T16:41:20Z|p1:22fin") == 0);
  }
  {
    boost::property_tree::ptree pt;
    std::string cmd1 = "dev1@set|fecha:%s|p1:%s|p2:%sfin";
    std::cout << "@UT@command:" <<cmd1<< std::endl;
    std::string parameters1 = "2014-02-18T16:41:20Z|22|33";
    ul20serv.transform_command(command_name, cmd1, parameters1,
                        sequence_id, item_dev, ptreeservice, id, pt);
    res1 = pt.get("body", "");
    std::cout << "@UT@res:" << res1 << std::endl;
    IOTASSERT(res1.compare("dev1@set|fecha:2014-02-18T16:41:20Z|p1:22|p2:33fin") == 0);
  }
  {
    boost::property_tree::ptree pt;
    std::string cmd1 = "dev1@set|fecha:%s|p1:%s|p2:%s|p3:%sfin";
    std::cout << "@UT@command:" <<cmd1<< std::endl;
    std::string parameters1 = "2014-02-18T16:41:20Z|22|33|44";
    ul20serv.transform_command(command_name, cmd1, parameters1,
                        sequence_id, item_dev, ptreeservice, id, pt);
    res1 = pt.get("body", "");
    std::cout << "@UT@res:" << res1 << std::endl;
    IOTASSERT(res1.compare("dev1@set|fecha:2014-02-18T16:41:20Z|p1:22|p2:33|p3:44fin") == 0);
  }
  {
    boost::property_tree::ptree pt;
    std::string cmd1 = "dev1@set|fecha:%s|p1:%s|p2:%s|p3:%s|p4:%sfin";
    std::cout << "@UT@command:" <<cmd1<< std::endl;
    std::string parameters1 = "2014-02-18T16:41:20Z|22|33|44|55";
    ul20serv.transform_command(command_name, cmd1, parameters1,
                        sequence_id, item_dev, ptreeservice, id, pt);
    res1 = pt.get("body", "");
    std::cout << "@UT@res:" << res1 << std::endl;
    IOTASSERT(res1.compare("dev1@set|fecha:2014-02-18T16:41:20Z|p1:22|p2:33|p3:44|p4:55fin") == 0);
  }
  std::cout << "END testTransformCommand " << std::endl;
}

void Ul20Test::testisCommandResponse() {
  std::cout << "START testisCommandResponse " << std::endl;

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");
  std::string service = "service2";

  std::string command_response;
  std::string id_command;
  int code = ul20serv.isCommandResp("dev1@ping|ping OK", 2000, command_response,
                                    id_command);
  std::cout <<  "dev1@ping|ping OK" << " is " << code << ":" <<  id_command <<
            "##" << command_response << std::endl;
  IOTASSERT(command_response.compare("dev1@ping|ping OK") == 0);
  IOTASSERT(id_command.compare("dev1@ping") == 0);

  std::cout << "END testisCommandResponse" << std::endl;

}


void Ul20Test::testCommand() {
  std::cout << "START testCommand " << std::endl;

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");
  std::string service = "service2";
  std::string apikey = "apikey3";
  boost::property_tree::ptree service_ptree;

  ul20serv.get_service_by_apiKey(service_ptree, apikey);

  // registramos el device en la cache
  boost::shared_ptr<iota::Device> item1(new iota::Device("dev1_endpoint", service));

  //comando recibido por updateContext
  iota::ContextElement cb_elto("pruDevice1", "thing", "false");
  iota::Attribute att("PING", "command", "22");
  cb_elto.add_attribute(att);

  std::string oper="updateContext";
  iota::UpdateContext op(oper);
  op.add_context_element(cb_elto);



  iota::ContextResponses  context_responses;
  ul20serv.updateContext(op, service_ptree, "", context_responses);
  std::cout << context_responses.get_string() << std::endl;

  std::cout << "END testCommand " << std::endl;
}

void Ul20Test::testFindService() {

  std::cout << "START testFindService " << std::endl;

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");
  ul20serv.init_services_by_resource();

  IOTASSERT(ul20serv.find_service_name("service2") == true);

  std::cout << "END testFindService " << std::endl;
}

void Ul20Test::testSendUnRegister() {
  std::cout << "START testSendRegister " << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");

  boost::property_tree::ptree pt_cb;
  try {
    ul20serv.get_service_by_name(pt_cb, "service2", "/");
    IOTASSERT(true);
  }
  catch (std::exception& e) {
    std::cout << "exception " << e.what() << std::endl;
  }

  ul20serv.get_service_by_name(pt_cb, "service2", "");
  std::string uri_cb = pt_cb.get<std::string>("cbroker", "");
  std::cout << "uri_cb:" << uri_cb << std::endl;

  std::vector<iota::ContextRegistration> context_registrations;
  iota::ContextRegistration  cr;
  std::string cb_response;
  std::string reg_id;

  iota::Entity entity("entity1", "thingf", "false");
  cr.add_entity(entity);

  context_registrations.push_back(cr);
  boost::shared_ptr<iota::Device> device(new iota::Device("dev1", "thingf"));

  ul20serv.send_unregister(pt_cb,
                         device,
                         reg_id,
                         cb_response);

  std::string cb_last = cb_mock->get_last();
  std::cout << "@UT@INFO" << cb_last << std::endl;
  IOTASSERT(cb_last.find("\"id\":\"entity1\",\"type\":\"thingf\"") !=
                 std::string::npos);
  IOTASSERT(
    cb_last.find("\"providingApplication\":\"http://0.0.0.0/1026/iot/d\"") !=
    std::string::npos);

  boost::shared_ptr<iota::Device> device2(new iota::Device("entity1", "thingf", "service1"));

  cb_mock->reset();
  ul20serv.send_register(context_registrations,
                         pt_cb,
                         device2,
                         reg_id,
                         cb_response);

  std::string cb_last1 = cb_mock->get_last();
  std::cout << "@UT@INFO1" << cb_last1 << std::endl;
  std::string cb_last2 = cb_mock->get_last();
  std::cout << "@UT@INFO2" << cb_last2 << std::endl;
  IOTASSERT(cb_last.compare("{\"updateAction\":\"APPEND\",\"contextElements\":[{\"id\":\"entity1\",\"type\":\"thingf\",\"isPattern\":\"false\"}]}") !=0);
  IOTASSERT(cb_last.compare("{\"updateAction\":\"APPEND\",\"contextElements\":[{\"id\":\"ent1\",\"type\":\"thingf\",\"isPattern\":\"false\"}]}") !=0);

  cb_mock->stop();
  std::cout << "END testSendUnRegister " << std::endl;
}

void Ul20Test::testSendRegister() {
  std::cout << "START testSendRegister " << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");

  boost::property_tree::ptree pt_cb;
  try {
    ul20serv.get_service_by_name(pt_cb, "service2", "/");
    IOTASSERT(true);
  }
  catch (std::exception& e) {
    std::cout << "exception " << e.what() << std::endl;
  }

  ul20serv.get_service_by_name(pt_cb, "service2", "");
  std::string uri_cb = pt_cb.get<std::string>("cbroker", "");
  std::cout << "uri_cb:" << uri_cb << std::endl;

  std::vector<iota::ContextRegistration> context_registrations;
  iota::ContextRegistration  cr;
  std::string cb_response;
  std::string reg_id;

  iota::Entity entity("entity1", "thingf", "false");
  cr.add_entity(entity);
  cr.add_provider("http://0.0.0.0/1026/iot/d");

  iota::AttributeRegister attribute("attr1", "type1", "false");
  cr.add_attribute(attribute);

  context_registrations.push_back(cr);
  boost::shared_ptr<iota::Device> device(new iota::Device("dev1", "thingf"));

  ul20serv.send_register(context_registrations,
                         pt_cb,
                         device,
                         reg_id,
                         cb_response);

  std::string cb_last = cb_mock->get_last();
  std::cout << "@UT@INFO" << cb_last << std::endl;
  IOTASSERT(cb_last.find("\"id\":\"entity1\",\"type\":\"thingf\"") !=
                 std::string::npos);
  IOTASSERT(
    cb_last.find("\"providingApplication\":\"http://0.0.0.0/1026/iot/d\"") !=
    std::string::npos);

  boost::shared_ptr<iota::Device> device2(new iota::Device("entity1", "thingf", "service1"));

  cb_mock->reset();
  ul20serv.send_register(context_registrations,
                         pt_cb,
                         device2,
                         reg_id,
                         cb_response);

  std::string cb_last1 = cb_mock->get_last();
  std::cout << "@UT@INFO1" << cb_last1 << std::endl;
  std::string cb_last2 = cb_mock->get_last();
  std::cout << "@UT@INFO2" << cb_last2 << std::endl;
  IOTASSERT(cb_last.compare("{\"updateAction\":\"APPEND\",\"contextElements\":[{\"id\":\"entity1\",\"type\":\"thingf\",\"isPattern\":\"false\"}]}") !=0);
  IOTASSERT(cb_last.compare("{\"updateAction\":\"APPEND\",\"contextElements\":[{\"id\":\"ent1\",\"type\":\"thingf\",\"isPattern\":\"false\"}]}") !=0);

  cb_mock->stop();
  std::cout << "END testSendRegister " << std::endl;
}

void Ul20Test::testDevicesConfig() {

  std::cout << "START testDevicesConfig " << std::endl;

  iota::DevicesFile* dev_cfg = iota::DevicesFile::initialize(PATH_DEV_CFG);

  iota::Cache my_cache(30, false);

  dev_cfg->parse_to_cache(&my_cache);

  IOTASSERT(my_cache.size() > 0);

  boost::shared_ptr<iota::Device> itemQ(new iota::Device("", "service2"));
  itemQ->_entity_name = "room_ut1";
  itemQ->_entity_type = "type2";
  itemQ->_service_path = "/ssrv2";
  boost::shared_ptr<iota::Device> result = my_cache.get_by_entity(itemQ);

  if (result.get() != NULL) {
    std::cout << result->_name << std::endl;
  }
  else {
    std::cout << "ERROR not founded room_ut1 of service2" << std::endl;
    IOTASSERT(result.get() != NULL);
  }

  dev_cfg->release();

  std::cout << "END testDevicesConfig " << std::endl;
}
void Ul20Test::testNoDeviceFile() {

  std::cout << "START testNoDeviceFile " << std::endl;

  CPPUNIT_ASSERT_THROW(iota::DevicesFile::initialize("fake.json"),
                       std::runtime_error);

  std::cout << "END testNoDeviceFile " << std::endl;
}

void Ul20Test::testRegisterDuration() {

  std::cout << "START testRegisterDuration " << std::endl;

  int resu;
  std::string data;

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");

  data = "PT1H";
  resu = ul20serv.get_duration_seconds(data);
  IOTASSERT(resu == 3600);

  data = "PT1M";
  resu = ul20serv.get_duration_seconds(data);
  IOTASSERT(resu == 60);

  data = "PT1S";
  resu = ul20serv.get_duration_seconds(data);
  IOTASSERT(resu == 1);

  data = "P1Y";
  resu = ul20serv.get_duration_seconds(data);
  IOTASSERT(resu == 365 * 24 * 3600);

  data = "P1M";
  resu = ul20serv.get_duration_seconds(data);
  IOTASSERT(resu == 30 * 24 * 3600);

  data = "P1W";
  resu = ul20serv.get_duration_seconds(data);
  IOTASSERT(resu == 7 * 24 * 3600);

  data = "P1D";
  resu = ul20serv.get_duration_seconds(data);
  IOTASSERT(resu == 24 * 3600);

  std::cout << "END testRegisterDuration " << std::endl;
}

void Ul20Test::testKVP() {
  std::cout << "START testKVP " << std::endl;

  std::vector<iota::KVP> queryK;
  iota::KVP k1("key1", "val1");
  queryK.push_back(k1);
  iota::KVP k2("key2", "val2");
  queryK.push_back(k2);

  for (int i = 0; i < queryK.size(); i++) {
    if (queryK[i].getKey().compare("key1") == 0) {
      IOTASSERT(queryK[i].getValue().compare("val1") == 0);
    }
    else {
      IOTASSERT(queryK[i].getValue().compare("val2") == 0);
    }
  }

  std::cout << "END testKVP " << std::endl;
}

void Ul20Test::testCacheMongoGet() {
  std::cout << "START testCacheMongoGet " << std::endl;

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG_MONGO);

  iota::Device p("dev1", "service2");
  p._service_path = "service_path";
  p._entity_type = "entity_type";
  p._entity_name = "entity_name";
  p._endpoint = "endpoint";

  iota::DeviceCollection table1;
  try {
    table1.insertd(p);
  }
  catch (std::exception& e) {
  }
  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");

  std::cout << "before ul20serv.get_device" << std::endl;
  boost::shared_ptr<iota::Device> aux = ul20serv.get_device("dev1", "service2");

  if (aux.get() != NULL) {
    std::string name =  aux->_name;
    std::cout << "name" << name << std::endl;
    IOTASSERT(name.compare("dev1") == 0);
  }
  else {
    IOTASSERT(true);
  }


  std::cout << "END testCacheMongoGet " << std::endl;
}

void Ul20Test::testCacheMongoGetNotFound() {
  std::cout << "START testCacheMongoGetNotFound " << std::endl;

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG_MONGO);

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");

  std::cout << "before ul20serv.get_device" << std::endl;
  boost::shared_ptr<iota::Device> aux = ul20serv.get_device("devNO", "service2");

  IOTASSERT(aux.get() == NULL);

  std::cout << "END testCacheMongoGetNotFound " << std::endl;
}


/***  devices.json
  *  {
            "id": "unitTest_dev1_endpoint",
            "type": "type2",
            "endpoint": "http://localhost:1026/NGSI10/updateContext",
            "entity": "room_ut1",
            "service": "service2",
            "commands": [
                {
                    "name": "PING",
                    "type": "command",
                    "command": "dest@PING|%s"
                }
            ]

        }

  **/

void Ul20Test::testPUSHCommand() {
  std::cout << "@UT@START testPUSHCommand" << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  //simulador del device
  boost::shared_ptr<HttpMock> device_mock;
  device_mock.reset(new HttpMock(9999, "/device"));
  device_mock->init();

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");

  // updateContext
  std::string querySTR = "";
  std::string bodySTR = "{\"updateAction\":\"UPDATE\",";
  bodySTR.append("\"contextElements\":[{\"id\":\"room_ut1\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR.append("\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"unitTest_dev1_endpoint@PING|22\",");
  bodySTR.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR.append("]} ]}");
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    ASYNC_TIME_WAIT
    //respuesta al update de contextBroker
    std::cout << "@UT@RESPONSE" << http_response.get_status_code() << " " <<
              response << std::endl;
    IOTASSERT(response.find(RESPONSE_MESSAGE_NGSI_OK) !=
                   std::string::npos);
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);

    std::string cb_last = cb_mock->get_last();
    // info
    std::cout << "@UT@INFO" << cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_info\",\"type\":\"string\",\"value\":\"unitTest_dev1_endpoint@PING|22\"")
      !=
      std::string::npos);
    // OK
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"OK\"")
      !=
      std::string::npos);

    cb_last = device_mock->get_last();
    std::cout << "@UT@comando que llega al simulador "<< cb_last << std::endl;
    IOTASSERT(cb_last.find("unitTest_dev1_endpoint@PING|22") !=
                   std::string::npos);

    cb_last = cb_mock->get_last();
    // pending
    std::cout << "@UT@Pending"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"pending\"")
      !=
      std::string::npos);
  }

  cb_mock->stop();
  device_mock->stop();
  std::cout << "@UT@END testPUSHCommand " << std::endl;
}

void Ul20Test::testPUSHCommandProxyAndOutgoingRoute() {
  std::cout << "@UT@START testPUSHCommandProxyAndOutgoingRoute" << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock, "file", true);

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG_VPN);

  //simulador del device
  boost::shared_ptr<HttpMock> device_mock;
  device_mock.reset(new HttpMock(9999, "/device"));
  //device_mock->set_extended_echo();
  device_mock->init();

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");

  // updateContext
  std::string querySTR = "";
  std::string bodySTR = "{\"updateAction\":\"UPDATE\",";
  bodySTR.append("\"contextElements\":[{\"id\":\"room_ut1\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR.append("\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"dev1@command|22\",");
  bodySTR.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR.append("]} ]}");
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    ASYNC_TIME_WAIT
    //respuesta al update de contextBroker
    std::cout << "@UT@RESPONSE" << http_response.get_status_code() << " " <<
              response << std::endl;
    IOTASSERT(response.find(RESPONSE_MESSAGE_NGSI_OK) !=
                   std::string::npos);
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);

    std::string cb_last = cb_mock->get_last();
    // info
    std::cout << "@UT@INFO" << cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_info\",\"type\":\"string\",\"value\":\"dev1@command|22\"")
      !=
      std::string::npos);
    // OK
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"OK\"")
      !=
      std::string::npos);

    cb_last = device_mock->get_last();
    std::cout << "@UT@comando que llega al simulador "<< cb_last << std::endl;
    IOTASSERT(cb_last.find("dev1@command|22") !=
                   std::string::npos);

    cb_last = cb_mock->get_last();
    // pending
    std::cout << "@UT@Pending"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"pending\"")
      !=
      std::string::npos);
  }

  cb_mock->stop();
  device_mock->stop();
  std::cout << "@UT@END testPUSHCommandProxyAndOutgoingRoute " << std::endl;
}

void Ul20Test::testPUSHCommandAsync() {
  std::cout << "@UT@START testPUSHCommandAsync" << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  //simulador del device
  boost::shared_ptr<HttpMock> device_mock;
  device_mock.reset(new HttpMock(9999, "/device"));
  device_mock->init();

  iota::UL20Service ul20serv;
  ul20serv.set_async_commands();
  ul20serv.set_resource("/iot/d");

  // updateContext
  std::string querySTR = "";
  std::string bodySTR = "{\"updateAction\":\"UPDATE\",";
  bodySTR.append("\"contextElements\":[{\"id\":\"room_ut1\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR.append("\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"dev1@command|22\",");
  bodySTR.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR.append("]} ]}");
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    ASYNC_TIME_WAIT
    ASYNC_TIME_WAIT
    ASYNC_TIME_WAIT
    //respuesta al update de contextBroker
    std::cout << "@UT@RESPONSE" << http_response.get_status_code() << " " <<
              response << std::endl;
    IOTASSERT(response.find(RESPONSE_MESSAGE_NGSI_OK) !=
                   std::string::npos);
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);

    std::string cb_last = cb_mock->get_last();
    std::cout << "CB_LAST " << cb_last << std::endl;

    // info
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_info\",\"type\":\"string\",\"value\":\"dev1@command|22\"")
      !=
      std::string::npos);

    // OK
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"OK\"")
      !=
      std::string::npos);

    cb_last = device_mock->get_last();
    std::cout << "@UT@comando que llega al simulador "<< cb_last << std::endl;
    IOTASSERT(cb_last.find("dev1@command|22") !=
                   std::string::npos);

    cb_last = cb_mock->get_last();
    // pending
    std::cout << "@UT@Pending"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"pending\"")
      !=
      std::string::npos);
  }

  cb_mock->stop();
  device_mock->stop();
  std::cout << "@UT@END testPUSHCommand " << std::endl;
}

void Ul20Test::testBADPUSHCommand() {
  std::cout << "@UT@START testBADPUSHCommand" << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);

  std::string cb_last;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  //simulador del device
  boost::shared_ptr<HttpMock> device_mock;
  device_mock.reset(new HttpMock(9999, "/device"));
  device_mock->init();

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");

  // updateContext bad command
  std::string querySTR = "";
  std::string bodySTR = "{\"updateAction\":\"UPDATE\",";
  bodySTR.append("\"contextElements\":[{\"id\":\"room_ut1\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR.append("\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"22\",");
  bodySTR.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR.append("]} ]}");
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    //respuesta al update de contextBroker
    // UL20 Command is not correct, it must be  name_device@command_name, but it is 22
    std::cout << "@UT@RESPONSE" << http_response.get_status_code() << " " <<
              response << std::endl;
    IOTASSERT_MESSAGE("Waiting for 400 and receive ",
                           response.find(iota::types::RESPONSE_MESSAGE_COMMAND_BAD) != std::string::npos);
    IOTASSERT(http_response.get_status_code() == 200);
    IOTASSERT_MESSAGE("@UT@DELIVERED, 400" ,
                           response.find("400") !=std::string::npos);
  }

  // update context inactive device
  std::string bodySTR2 = "{\"updateAction\":\"UPDATE\",";
  bodySTR2.append("\"contextElements\":[{\"id\":\"room_ut2\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR2.append("\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"X@Y|22\",");
  bodySTR2.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR2.append("]} ]}");
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR2);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    //respuesta al update de contextBroker
    // UL20 Command is not correct, it must be  name_device@command_name, but it is 22
    std::cout << "@UT@RESPONSE" << http_response.get_status_code() << " " <<
              response << std::endl;
    IOTASSERT_MESSAGE("Waiting for 409 and receive ",
                           response.find("The device is not active") != std::string::npos);
    IOTASSERT(http_response.get_status_code() == 200);
    IOTASSERT_MESSAGE("@UT@DELIVERED, 409" ,
                           response.find("409") !=std::string::npos);
  }

  //update context bad endpoint
  std::string bodySTR3 = "{\"updateAction\":\"UPDATE\",";
  bodySTR3.append("\"contextElements\":[{\"id\":\"room_utbad\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR3.append("\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"X@Y|22\",");
  bodySTR3.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR3.append("]} ]}");
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR3);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    ASYNC_TIME_WAIT
    //respuesta al update de contextBroker
    // UL20 Command is not correct, it must be  name_device@command_name, but it is 22
    std::cout << "@UT@RESPONSE" << http_response.get_status_code() << " " <<
              response << std::endl;
    IOTASSERT_MESSAGE("Waiting for 200 and receive ",
                           response.find(RESPONSE_MESSAGE_NGSI_OK) != std::string::npos);
    IOTASSERT(http_response.get_status_code() == 200);
    IOTASSERT_MESSAGE("@UT@DELIVERED, 200" ,
                           response.find("200") !=std::string::npos);

    //al CB le llega el resultado del comando
    cb_last = cb_mock->get_last();
    std::cout << "@UT@INFO" << cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_utbad\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"error\"")
      !=
      std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_info\",\"type\":\"string\",\"value\":\"no ul20 response command:This seems to be like an error connection to 127.0.0.1:1026//path\"")
      !=
      std::string::npos);
  }


  cb_mock->stop();
  device_mock->stop();
  std::cout << "@UT@END testBADPUSHCommand " << std::endl;
}


/**
  *  IDAS-20145  en los timeout vencidos de pull no se separa dev@comando para obtener el nombre del comando
  **/
void Ul20Test::testPollingCommandTimeout() {
  std::cout << "@UT@START testPollingCommandTimeout" << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");

  // updateContext
  std::string querySTR = "";
  std::string bodySTR = "{\"updateAction\":\"UPDATE\",";
  bodySTR.append("\"contextElements\":[{\"id\":\"room_ut3\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR.append("\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"X@Y|22\",");
  bodySTR.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR.append("]} ]}");
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);
    ASYNC_TIME_WAIT
    std::cout << "@UT@POST updateContext " << http_response.get_status_code() <<
              std::endl;
    IOTASSERT_MESSAGE("@UT@POST, response code no 200" ,
                           http_response.get_status_code() == 200);

    std::string cb_last = cb_mock->get_last();
    std::cout << "@UT@READY_FOR_READ" <<cb_last << std::endl;
    // ready_for_read
    IOTASSERT_MESSAGE("@UT@READY_FOR_READ, entity or entity_type in not correct"
                           ,
                           cb_last.find("\"id\":\"room_ut3\",\"type\":\"type2\"") !=std::string::npos);
    IOTASSERT_MESSAGE("@UT@READY_FOR_READ, name of command or value is not correct",
                           cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"pending\"")
                           !=
                           std::string::npos);


    ///esperamos un segundo
    sleep(2);
    cb_last = cb_mock->get_last();
      std::cout << "@UT@TIMEOUT" <<cb_last << std::endl;
      IOTASSERT_MESSAGE("@UT@READY_FOR_READ, entity or entity_type in not correct"
                             ,
                            cb_last.find("\"id\":\"room_ut3\",\"type\":\"type2\"") !=std::string::npos);
      IOTASSERT_MESSAGE("@UT@READY_FOR_READ, name of command or value is not correct",
            cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"expired read\"") !=std::string::npos);
      cb_last = cb_mock->get_last();
      std::cout << "@UT@TIMEOUT_STATUS" <<cb_last << std::endl;

  }

  cb_mock->stop();
  std::cout << "@UT@END testPollingCommandTimeout" << std::endl;
}

/**
  * IDAS-20141   si se envian comandos distintos a X@Y  no se progresa info
  * al buscar id de comando no lo entiendo y no devuelvo info, habrÌa que dar error pero progresar la respuesta
  **/
void Ul20Test::testCommandNOUL() {
  std::cout << "@UT@START testCommandNOUL" << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);

  //simulador del device
  boost::shared_ptr<HttpMock> device_mock;
  device_mock.reset(new HttpMock(9999, "/device"));
  device_mock->init();

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");

  // updateContext
  std::string querySTR = "";
  std::string bodySTR = "{\"updateAction\":\"UPDATE\",";
  bodySTR.append("\"contextElements\":[{\"id\":\"room_ut1\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR.append("\"attributes\":[{\"name\":\"RAW\",\"type\":\"command\",\"value\":\"param=value\",");
  bodySTR.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR.append("]} ]}");
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    ASYNC_TIME_WAIT

    //respuesta al update de contextBroker
    std::cout << "@UT@RESPONSE" << http_response.get_status_code() << " " <<
              response << std::endl;
    IOTASSERT(response.find(iota::types::RESPONSE_MESSAGE_COMMAND_BAD) !=
                   std::string::npos);
    IOTASSERT(http_response.get_status_code() == 200);
    IOTASSERT_MESSAGE("@UT@DELIVERED, 400" ,
                           response.find("400") !=std::string::npos);


  }

  cb_mock->stop();
  device_mock->stop();
  std::cout << "@UT@END testCommandNOUL " << std::endl;
}


/**
  * IDAS-20142 si se manda comando x@Y|param=value   solo se devuelve param=value en el info
  **/
void Ul20Test::testPUSHCommandParam() {
  std::cout << "@UT@START testPUSHCommandParam" << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);

  //simulador del device
  boost::shared_ptr<HttpMock> device_mock;
  device_mock.reset(new HttpMock(9999, "/device"));
  device_mock->init();

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");

  // updateContext
  std::string querySTR = "";
  std::string bodySTR = "{\"updateAction\":\"UPDATE\",";
  bodySTR.append("\"contextElements\":[{\"id\":\"room_ut1\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR.append("\"attributes\":[{\"name\":\"RAW\",\"type\":\"command\",\"value\":\"X@Y|param=value\",");
  bodySTR.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR.append("]} ]}");
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, "service2");
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, "/ssrv2");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    ASYNC_TIME_WAIT
    //respuesta al update de contextBroker
    std::cout << "@UT@RESPONSE" << http_response.get_status_code() << " " <<
              response << std::endl;
    IOTASSERT(response.find(RESPONSE_MESSAGE_NGSI_OK) !=
                   std::string::npos);
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);

    std::string cb_last = cb_mock->get_last();
    // info
    std::cout << "@UT@INFO" << cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"RAW_info\",\"type\":\"string\",\"value\":\"X@Y|param=value\"")
      !=std::string::npos);

    // OK
    std::cout << "@UT@OK"<<cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"RAW_status\",\"type\":\"string\",\"value\":\"OK\"") !=
      std::string::npos);

    cb_last = device_mock->get_last();
    std::cout << "@UT@comando que llega al simulador "<< cb_last << std::endl;
    IOTASSERT(cb_last.find("X@Y|param=value") !=
                   std::string::npos);

    cb_last = cb_mock->get_last();
    // pending
    std::cout << "@UT@Pending"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"RAW_status\",\"type\":\"string\",\"value\":\"pending\"")
      !=
      std::string::npos);
  }

  cb_mock->stop();
  device_mock->stop();

  std::cout << "@UT@END testPUSHCommandParam " << std::endl;

}

void Ul20Test::testCommandHandle() {
  std::cout << "@UT@START testCommandHandle " << std::endl;
  std::string command_id="id";

  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);
  iota::UL20Service ul20serv;
  boost::property_tree::ptree pt;
  pt.put("timeout", 1);
  pt.put("service", "service");
  pt.put("service_path", "service_path");
  pt.put("cbroker", "cbroker");
  pt.put("token", "token");
  pt.put("entity_type", "entity_type");
  boost::shared_ptr<iota::Device> dev(new iota::Device("item_dev_name","service"));
  boost::property_tree::ptree ptcommand;
  ptcommand.put("body","command_to_send" );


  ul20serv.save_command("command_name",
                        command_id,
                        1,
                        ptcommand,
                        dev,
                        "entity_type",
                        "endpoint",
                        pt,
                        "sequence",
                        0);
  std::cout << "@UT@END testCommandHandle " << std::endl;
}


void Ul20Test::testPUSHCommand_MONGO() {
  std::cout << "@UT@START testPUSHCommand_MONGO" << std::endl;

  pion::http::response http_response;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock, "mongodb");

  //simulador del device
  boost::shared_ptr<HttpMock> device_mock;
  device_mock.reset(new HttpMock(9999, "/device"));
  device_mock->init();
  std::map<std::string, std::string> h;
  std::string mock_response = "device_id@PING|Ping OK";
  // device command response
  device_mock->set_response(200, mock_response, h);
  device_mock->set_response(200, mock_response, h);
  std::string mock_port = boost::lexical_cast<std::string>(cb_mock->get_port());

  iota::AdminService adminserv;
  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");
  ul20serv.start();
  adminserv.add_service("/iot/d", &ul20serv);


  boost::shared_ptr<iota::ServiceCollection> col(new iota::ServiceCollection());

  std::string response;
  int code_res;
  std::string service= "service" ;
  service.append(mock_port);
  std::string subservice= "/ssrv" ;
  subservice.append(mock_port);

  std::cout << "@UT@POST Service" << std::endl;


  std::string apikey = "apikey";
  apikey.append(mock_port);
  std::string postSRV = boost::str(boost::format(POST_SERVICE) % mock_port  % mock_port);
  code_res = adminserv.post_service_json( col, service, subservice, postSRV,
                     http_response, response, "1234", "5678");
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);


  std::cout << "@UT@POST Device" << std::endl;
  code_res = adminserv.post_device_json(service, subservice, POST_DEVICE,
                     http_response, response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);


  // updateContext con parametro
  std::string querySTR = "";
  std::string bodySTR = UPDATE_CONTEXT;
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, service);
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, subservice);
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    ASYNC_TIME_WAIT
    //respuesta al update de contextBroker
    std::cout << "@UT@RESPONSE" << http_response.get_status_code() << " " <<
              response << std::endl;
    IOTASSERT(response.find(RESPONSE_MESSAGE_NGSI_OK) !=
                   std::string::npos);
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);

    std::string cb_last = cb_mock->get_last();
    // info
    std::cout << "@UT@INFO" << cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_info\",\"type\":\"string\",\"value\":\"device_id@PING|Ping OK\"")
      !=
      std::string::npos);

    // OK
    std::cout << "@UT@OK"<<cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"OK\"")
      !=
      std::string::npos);

    cb_last = device_mock->get_last();
    std::cout << "@UT@comando que llega al simulador "<< cb_last << std::endl;
    IOTASSERT(cb_last.find("device_id@PING|22") !=
                   std::string::npos);

    cb_last = cb_mock->get_last();
    // pending
    std::cout << "@UT@Pending"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"pending\"")
      !=
      std::string::npos);
  }

 std::cout << "@UT@upadteContext@" << UPDATE_CONTEXT_sin_param << std::endl;
  // updateContext con parametro
  querySTR = "";
  bodySTR = UPDATE_CONTEXT_sin_param;
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, service);
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, subservice);
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    ASYNC_TIME_WAIT
    //respuesta al update de contextBroker
    std::cout << "@UT@RESPONSE" << http_response.get_status_code() << " " <<
              response << std::endl;
    IOTASSERT(response.find(RESPONSE_MESSAGE_NGSI_OK) !=
                   std::string::npos);
    IOTASSERT(http_response.get_status_code() == RESPONSE_CODE_NGSI);

    std::string cb_last = cb_mock->get_last();
    // info
    std::cout << "@UT@INFO" << cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_info\",\"type\":\"string\",\"value\":\"device_id@PING|Ping OK\"")
      !=
      std::string::npos);

    // OK
    std::cout << "@UT@OK"<<cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"OK\"")
      !=
      std::string::npos);

    cb_last = device_mock->get_last();
    std::cout << "@UT@comando que llega al simulador "<< cb_last << std::endl;
    IOTASSERT(cb_last.find("device_id@PING") !=
                   std::string::npos);

    cb_last = cb_mock->get_last();
    // pending
    std::cout << "@UT@Pending"<< cb_last << std::endl;
    IOTASSERT(cb_last.find("\"id\":\"room_ut1\",\"type\":\"type2\"") !=
                   std::string::npos);
    IOTASSERT(
      cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"pending\"")
      !=
      std::string::npos);
  }

  std::cout << "@UT@DELETE Service" << std::endl;
  std::string token, trace_message;
  code_res = adminserv.delete_service_json(col, service, "/", service, apikey, "/iot/d", true,
                     http_response, response, token, trace_message);


  cb_mock->stop();
  device_mock->stop();

  std::cout << "@UT@END testPUSHCommand_MONGO " << std::endl;
}

void Ul20Test::testBAD_PUSHCommand_MONGO() {
  std::cout << "@UT@START testBAD_PUSHCommand_MONGO" << std::endl;

  pion::http::response http_response;
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock, "mongodb");
  std::string mock_port = boost::lexical_cast<std::string>(cb_mock->get_port());

  //simulador del device
  boost::shared_ptr<HttpMock> device_mock;
  device_mock.reset(new HttpMock(9999, "/device"));
  device_mock->init();

  boost::shared_ptr<iota::ServiceCollection> col(new iota::ServiceCollection());

  iota::AdminService adminserv;
  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");
  ul20serv.start();
  // asociamos
  adminserv.add_service("/iot/d", &ul20serv);

  std::string response;
  int code_res;
  std::string service= "service" ;
  service.append(mock_port);
  std::string subservice= "/ssrv" ;
  subservice.append(mock_port);

  // POST updateContext sin servicio
  std::string querySTR = "";
  std::string bodySTR = "{\"updateAction\":\"UPDATE\",";
  bodySTR.append("\"contextElements\":[{\"id\":\"room_ut1\",\"type\":\"type2\",\"isPattern\":\"false\",");
  bodySTR.append("\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"dev1@command|22\",");
  bodySTR.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR.append("]} ]}");
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, service);
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, subservice);
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    //respuesta al update de contextBroker
    std::cout << "@UT@RESPONSE no service:" << http_response.get_status_code() << " " <<
              response << std::endl;
    IOTASSERT_MESSAGE("Checking response " + response, response.find(
        "{ \"errorCode\" : { \"code\":404, \"reasonPhrase\":\"The service does not exist\"}") !=
                   std::string::npos);
    IOTASSERT(http_response.get_status_code() == 200);

  }

  //creamos le servicio
  std::cout << "@UT@POST create Service" << std::endl;
  std::string postSRV = boost::str(boost::format(POST_SERVICE) % mock_port % mock_port);
  code_res = adminserv.post_service_json(col, service, subservice, postSRV,
                     http_response, response, "1234", "5678");
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  //POST sin device
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, service);
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, subservice);
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    //respuesta al update de contextBroker
    std::cout << "@UT@RESPONSE no device:" << http_response.get_status_code() << " " <<
              response << std::endl;
    IOTASSERT(response.find(
    "{\"contextResponses\":[{\"statusCode\":{\"code\":\"404\",\"reasonPhrase\":\"The device does not exist\"") !=
                   std::string::npos);
    IOTASSERT(http_response.get_status_code() == 200);

  }

  //creamos el device
  std::cout << "@UT@POST Device" << ul20serv.get_cache_size() << std::endl;
  code_res = adminserv.post_device_json(service, subservice, POST_DEVICE2,
                     http_response, response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

  //POST con device sin command
  std::cout << "@UT@POST con device sin command" << ul20serv.get_cache_size() << std::endl;
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, service);
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, subservice);
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    //respuesta al update de contextBroker
    std::cout << "@UT@RESPONSE no command:" << http_response.get_status_code() << std::endl;
    std::cout << "@UT@RESPONSE " << response << std::endl;
    std::string resOK= "{\"contextResponses\":[{\"statusCode\":{\"code\":\"404\",\"reasonPhrase\":\"the device does not have implemented this command\"";
    std::cout << "@UT@RESPONSE " << resOK << std::endl;
    IOTASSERT(response.find(resOK) != std::string::npos);
    IOTASSERT(http_response.get_status_code() == 200);

  }

  //aÒadimos el comando
  std::cout << "@UT@PUT Device1" << ul20serv.get_cache_size() << std::endl;
  code_res = adminserv.put_device_json(service, subservice, "device_id", PUT_DEVICE,
                     http_response, response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == 204);

  std::cout << "@UT@NUM_CACHE: " << ul20serv.get_cache_size() << std::endl;

  //POST con device no activo
  {
    std::cout << "@UT@POST no active" << std::endl;
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, service);
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, subservice);
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    //respuesta al update de contextBroker
    std::cout << "@UT@RESPONSE no command:" << http_response.get_status_code() << " " <<
              response << std::endl;
    IOTASSERT(response.find(
    "{\"contextResponses\":[{\"statusCode\":{\"code\":\"409\",\"reasonPhrase\":\"The device is not active") !=
                   std::string::npos);
    IOTASSERT(http_response.get_status_code() == 200);

  }

  std::cout << "@UT@DELETE Service" << std::endl;
  std::string token, trace_message;
  code_res = adminserv.delete_service_json(col, service, "/", service, "apikey", "/iot/d", true,
                     http_response, response, token, trace_message);

  cb_mock->stop();
  device_mock->stop();

  std::cout << "@UT@END testBAD_PUSHCommand_MONGO " << std::endl;
}

void Ul20Test::testPollingCommand_MONGO_CON() {
  std::cout << "@UT@START testPollingCommand_MONGO_CON " << std::endl;


  pion::http::response http_response;
  std::string response;
  iota::AdminService adminserv;
  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");
  ul20serv.start();
  // asociamos
  adminserv.add_service("/iot/d", &ul20serv);
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock, "mongodb");
  std::string mock_port = boost::lexical_cast<std::string>(cb_mock->get_port());
  std::cout << "@UT@mock port " <<  mock_port << std::endl;

  boost::shared_ptr<iota::ServiceCollection> col(new iota::ServiceCollection());
  std::string service="service";
  service.append(mock_port);
  std::string subservice="/ssrv";
  subservice.append(mock_port);
  std::cout << "@UT@POST Service" <<  service << std::endl;

  std::string postSRV = boost::str(boost::format(POST_SERVICE) % mock_port % mock_port);
  int code_res = adminserv.post_service_json( col, service, subservice, postSRV,
                         http_response, response, "1234", "5678");

  std::cout << "@UT@First command" <<  service << std::endl;
  testPollingCommand_MONGO("unitTest_dev3_polling",
       "room_ut3", "type2", POST_DEVICE_CON, cb_mock);
  std::cout << "@UT@Second command" <<  service << std::endl;
  testPollingCommand_MONGO("unitTest_dev32_polling",
       "type2:unitTest_dev32_polling", "type2", POST_DEVICE_CON2, cb_mock);

  std::cout << "@UT@DELETE Service" << std::endl;
  std::string token, trace_message;
  code_res = adminserv.delete_service_json(col, service, subservice, service, "apikey", "/iot/d", true,
                         http_response, response, token, trace_message);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == 204);

  cb_mock->stop();
  std::cout << "@UT@END testPollingCommand_MONGO_CON " << std::endl;
}
void Ul20Test::testPollingCommand_MONGO_SIN_ENTITY_NAME() {
  std::cout << "@UT@START testPollingCommand_MONGO_SIN_ENTITY_NAME "  << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  testPollingCommand_MONGO("dev_SIN_ENTITY_NAME",
                 "type_SIN_ENTITY_NAME:dev_SIN_ENTITY_NAME",
                 "type_SIN_ENTITY_NAME",
                 POST_DEVICE_SIN_ENTITY_NAME, cb_mock);
  std::cout << "@UT@END testPollingCommand_MONGO_SIN_ENTITY_NAME "  << std::endl;
}
void Ul20Test::testPollingCommand_MONGO_SIN_ENTITY_TYPE() {
  std::cout << "@UT@START testPollingCommand_MONGO_SIN_ENTITY_TYPE " << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  testPollingCommand_MONGO("dev_SIN_ENTITY_TYPE",
                           "ent_SIN_ENTITY_TYPE",
                           "thing",
                           POST_DEVICE_SIN_ENTITY_TYPE, cb_mock);
  std::cout << "@UT@END testPollingCommand_MONGO_SIN_ENTITY_TYPE "  << std::endl;
}
void Ul20Test::testPollingCommand_MONGO_SIN() {
  std::cout << "@UT@START testPollingCommand_MONGO_SIN " << std::endl;
  boost::shared_ptr<HttpMock> cb_mock;
  testPollingCommand_MONGO("dev_SIN",
                           "thing:dev_SIN",
                           "thing",
                           POST_DEVICE_SIN, cb_mock);
  std::cout << "@UT@START testPollingCommand_MONGO_SIN " << std::endl;
}


void Ul20Test::testPollingCommand_MONGO(
                    const std::string &name_device,
                    const std::string &entity_name,
                    const std::string &entity_type,
                    const std::string &post_device,
                    const boost::shared_ptr<HttpMock> &create_mock) {
  std::cout << "@UT@device testPollingCommand_MONGO " << name_device << std::endl;

  pion::http::response http_response;
  std::string mock_port;
  boost::shared_ptr<HttpMock> cb_mock;
  if (create_mock.get() == NULL){
      cb_mock.reset(new HttpMock("/mock"));
      start_cbmock(cb_mock, "mongodb");
  }else{
     cb_mock = create_mock;
  }

  boost::shared_ptr<iota::ServiceCollection> col(new iota::ServiceCollection());
  mock_port = boost::lexical_cast<std::string>(cb_mock->get_port());
  //simulador del device
  boost::shared_ptr<HttpMock> device_mock;
  device_mock.reset(new HttpMock(9999, "/device"));
  device_mock->init();

  iota::AdminService adminserv;
  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");
  ul20serv.start();
  // asociamos
  adminserv.add_service("/iot/d", &ul20serv);
  std::string response;
  int code_res;
  std::string service= "service" ;
  std::string subservice= "/ssrv" ;
  std::string apikey = "apikey";

  service.append(mock_port);
  subservice.append(mock_port);
  apikey.append(mock_port);

  std::cout << "@UT@service " << service << " spath " << subservice << std::endl;

  if (create_mock.get() == NULL){
      std::cout << "@UT@POST Service" << std::endl;

      std::string postSRV = boost::str(boost::format(POST_SERVICE) % mock_port % mock_port);
      code_res = adminserv.post_service_json( col, service, subservice, postSRV,
                         http_response, response, "1234", "5678");
      std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
      IOTASSERT(code_res == POST_RESPONSE_CODE);
  }

  std::cout << "@UT@POST Device" << std::endl;
  code_res = adminserv.post_device_json( service, subservice, post_device,
                     http_response, response);
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);


  // updateContext
  std::string querySTR = "";
  std::string bodySTR = "{\"updateAction\":\"UPDATE\",";
  bodySTR.append("\"contextElements\":[{\"id\":\"");
  bodySTR.append(entity_name);
  bodySTR.append("\",\"type\":\"");
  bodySTR.append(entity_type);
  bodySTR.append("\",\"isPattern\":\"false\",");
  bodySTR.append("\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"22\",");
  bodySTR.append("\"metadatas\":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"2014-11-23T17:33:36.341305Z\"}]}");
  bodySTR.append("]} ]}");
  {
    pion::http::request_ptr http_request(new
                                         pion::http::request("/iot/ngsi/d/updateContext"));
    http_request->set_method("POST");
    http_request->add_header(iota::types::FIWARE_SERVICE, service);
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, subservice);
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    ASYNC_TIME_WAIT
    std::cout << "@UT@POST updateContext " << http_response.get_status_code() <<
              std::endl;
    IOTASSERT_MESSAGE("@UT@POST, response code no 200" ,
                           http_response.get_status_code() == 200);

    std::string cb_last = cb_mock->get_last();
    std::cout << "@UT@READY_FOR_READ" <<cb_last << std::endl;
    // ready_for_read
    std::string str_check = "\"id\":\"";
    str_check.append(entity_name);
    str_check.append("\",\"type\":\"");
    str_check.append(entity_type);
    str_check.append("\"");
    IOTASSERT_MESSAGE("@UT@READY_FOR_READ, entity or entity_type in not correct"
                           ,
                           cb_last.find(str_check) !=std::string::npos);
    IOTASSERT_MESSAGE("@UT@READY_FOR_READ, name of command or value is not correct",
                           cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"pending\"")
                           !=
                           std::string::npos);

  }

  // GET desde el device
  querySTR = "i=";
  querySTR.append(name_device);
  querySTR.append("&k=");
  querySTR.append(apikey);
  bodySTR = "";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("GET");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",name_device));
    query_parameters.insert(std::pair<std::string,std::string>("k",apikey));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    ASYNC_TIME_WAIT
    std::cout << "@UT@GET command " << http_response.get_status_code() <<
              ":" << http_response.get_content() << std::endl;
    IOTASSERT_MESSAGE("@UT@GET response code no 200" ,
                           http_response.get_status_code() == 200);

    std::string cb_last = cb_mock->get_last();
    std::cout << "@UT@DELIVERED" << cb_last << std::endl;
    // delivered
    std::string str_check = "\"id\":\"";
    str_check.append(entity_name);
    str_check.append("\",\"type\":\"");
    str_check.append(entity_type);
    str_check.append("\"");
    IOTASSERT_MESSAGE("@UT@DELIVERED, entity or entity_type in not correct" ,
                           cb_last.find(str_check) !=std::string::npos);
    IOTASSERT_MESSAGE("@UT@DELIVERED, name of command or value is not correct"
                           ,
                           cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"delivered\"")
                           !=std::string::npos);

  }

  // POST resultado del comando
  querySTR = "i=";
  querySTR.append(name_device);
  querySTR.append("&k=");
  querySTR.append(apikey);
  bodySTR = name_device;
  bodySTR.append("@PING|Ping ok");
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",name_device));
    query_parameters.insert(std::pair<std::string,std::string>("k",apikey));
    pion::http::response http_response;
    std::string response;
    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    ASYNC_TIME_WAIT
    std::cout << "@UT@POST command result " << http_response.get_status_code() <<
              std::endl;
    IOTASSERT_MESSAGE("@UT@GET response code no 200" ,
                           http_response.get_status_code() == 200);

    std::string cb_last = cb_mock->get_last();
    std::cout << "@UT@INFO" << cb_last << std::endl;
    // info
    std::string str_check = "\"id\":\"";
    str_check.append(entity_name);
    str_check.append("\",\"type\":\"");
    str_check.append(entity_type);
    str_check.append("\"");
    IOTASSERT_MESSAGE("@UT@INFO, entity or entity_type in not correct" ,
                           cb_last.find(str_check) !=std::string::npos);
    IOTASSERT_MESSAGE("@UT@INFO, name of command or value is not correct" ,
                           cb_last.find("{\"name\":\"PING_info\",\"type\":\"string\",\"value\":")
                           !=
                           std::string::npos);

    std::cout << "@UT@OK" << cb_last << std::endl;
    // OK
    std::string errSTR = "\"id\":\"";
    errSTR.append(entity_name);
    errSTR.append("\",\"type\":\"");
    errSTR.append(entity_type);
    IOTASSERT_MESSAGE("@UT@OK, entity or entity_type in not correct" ,
                           cb_last.find(errSTR) !=std::string::npos);
    IOTASSERT_MESSAGE("@UT@OK, name of command or value is not correct" ,
                           cb_last.find("{\"name\":\"PING_status\",\"type\":\"string\",\"value\":\"OK\"")
                           !=
                           std::string::npos);

  }

  if (create_mock.get() == NULL){
      std::cout << "@UT@Delete Service" << std::endl;

      std::string token, trace_message;
      code_res = adminserv.delete_service_json(  col, service, subservice, service, apikey, "/iot/d", true,
                         http_response, response, token, trace_message);
      std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
      IOTASSERT(code_res == 204);
  }

}


/***
  *  POST http://10.95.26.51:8002/d?i=Device_UL2_0_RESTv2&k=4orh3jl3h40qkd7fk2qrc52ggb
  *     ${#Project#END_TIME2}|t|${Properties#value}
  *     ${#Project#END_TIME2}|t|${Properties#value}#t|${Properties#value2}
  *     ${#Project#END_TIME}|t|${Properties#value}#l|${Properties#value2}/${Properties#value2_1}
  **/
void Ul20Test::testQueryContext() {
  std::cout << "START testQueryContext" << std::endl;
  iota::Configurator* conf = iota::Configurator::initialize(PATH_CONFIG);
  boost::shared_ptr<HttpMock> cb_mock;
  cb_mock.reset(new HttpMock("/mock"));
  start_cbmock(cb_mock);
  std::string cb_last;
  std::string responseOK( "{\"contextResponses\":[{\"statusCode\":{\"code\":\"200\",\"reasonPhrase\":\"OK\",\"details\":\"\"},\"contextElement\":{\"id\":\"room_ut1\",\"type\":\"type2\",\"isPattern\":\"false\",\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"@@RAW@@\"},{\"name\":\"RAW\",\"type\":\"command\",\"value\":\"@@RAW@@\"}]}}]}");

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");
  std::string service = "service2";
  std::string subservice = "/ssrv2";
  boost::property_tree::ptree service_ptree;
  ul20serv.get_service_by_name(service_ptree, service, subservice);

  // queryContext  recibido del CB
  iota::QueryContext op;
  iota::Entity entity("room_ut1", "type2", "false");
  op.add_entity(entity);
  //q.add_attribute("ping");

  iota::ContextResponses  context_responses;
  ul20serv.queryContext(op, service_ptree, context_responses);
  std::string response = context_responses.get_string();
  std::cout << "@UT@response: " << response << std::endl;
  IOTASSERT_MESSAGE("@UT@OK, response is not correct" ,
            response.compare(responseOK) ==0);

  std::cout << "END testQueryContext" << std::endl;
}


/***
  *  POST http://10.95.26.51:8002/d?i=Device_UL2_0_RESTv2&k=4orh3jl3h40qkd7fk2qrc52ggb
  *     ${#Project#END_TIME2}|t|${Properties#value}
  *     ${#Project#END_TIME2}|t|${Properties#value}#t|${Properties#value2}
  *     ${#Project#END_TIME}|t|${Properties#value}#l|${Properties#value2}/${Properties#value2_1}
  **/
void Ul20Test::testQueryContextAPI() {
  std::cout << "START testQueryContextAPI" << std::endl;

  std::string responseNOservice("{ \"errorCode\" : { \"code\":404, \"reasonPhrase\":\"The service does not exist\"}}");
  std::string responseOK( "{\"contextResponses\":[{\"statusCode\":{\"code\":\"200\",\"reasonPhrase\":\"OK\",\"details\":\"\"},\"contextElement\":{\"id\":\"room_ut111\",\"type\":\"type2\",\"isPattern\":\"false\",\"attributes\":[{\"name\":\"PING\",\"type\":\"command\",\"value\":\"%s\"},{\"name\":\"RAW\",\"type\":\"command\",\"value\":\"%s\"}]}}]}");

  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/ngsi/d/queryContext");
  std::string service = "service2";
  std::string subservice = "service2";
  std::string apikey = "apikey3";

  // POST  queryContext
  std::string querySTR = "";
  std::string bodySTR = "{\"entities\":[{\"type\":\"type2\",\"isPattern\":\"false\",\"id\":\"room_ut111\" }]}";
  {
    pion::http::request_ptr http_request(new pion::http::request("/iot/ngsi/d/queryContext"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content(bodySTR);
    http_request->add_header(iota::types::FIWARE_SERVICE, service);
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, subservice);

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::http::response http_response;
    std::string response;
    ul20serv.op_ngsi(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "POST queryContext " <<
        http_response.get_status_code() <<
                 ":" << response << std::endl;
    IOTASSERT_MESSAGE("response code not is 200",
                           http_response.get_status_code() == RESPONSE_CODE_NGSI);

    std::cout << responseNOservice << std::endl;
    std::cout << response << std::endl;
    IOTASSERT_MESSAGE("@UT@OK, response is not correct" ,
            responseNOservice.compare(response) ==0);

  }

  std::cout << "END testQueryContextAPI " << std::endl;
}


void Ul20Test::testChangeIPDevice(){

  /*
  This test will prove that a device with existing endpoint can change that attribute by means
  of a http request to the service (coming from the own device).


  USING  mongodb.

            "device_id": "unitTest_dev1_endpoint",
            "endpoint": "http://127.0.0.1:9999/device",
  */
  std::cout << "START testChangeIPDevice" << std::endl;

  iota::Configurator::initialize("../../tests/iotagent/config_mongo.json");
  std::string dev_name = "unitTest_dev1_change_endpoint";
  std::string service = "service2";
  std::string subservice = "/ssrv2";
  std::string apikey = "apikey3";
  std::string new_endpoint = "http://127.0.0.1:5555/new";

  boost::shared_ptr<iota::ServiceCollection> col(new iota::ServiceCollection());
  iota::DeviceCollection table_device;

//REMOVING previous data from mongo
  try{
  iota::Device borrar("", "");

  table_device.removed(borrar);

  mongo::BSONObj all;

  col->remove(all);

  }catch(std::exception exc){

  }


  pion::http::response http_response;
  iota::AdminService adminService;
  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");
  adminService.add_service("/iot/d",&ul20serv);



  std::string response;
  int code_res;

  std::cout << "@UT@POST Service" << std::endl;


  code_res = adminService.post_service_json( col, service, subservice, POST_SERVICE_ENDPOINT,
                     http_response, response, apikey, "5678");
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);


  std::cout << "@UT@POST Device" << std::endl;
   std::string post_device = boost::str(boost::format(POST_DEVICE_ENDPOINT) % dev_name % dev_name);
  code_res = adminService.post_device_json(service, subservice, post_device,
                     http_response, response);


  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);


  std::string encoded_endpoint = pion::algorithm::url_encode(new_endpoint);
  std::string querySTR = "i="+dev_name+"&k=apikey3&ip="+encoded_endpoint;
  pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content("");

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",dev_name));
    query_parameters.insert(std::pair<std::string,std::string>("k",apikey));
    //include the IP parameter to change it
    query_parameters.insert(std::pair<std::string,std::string>("ip",new_endpoint));



    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

    std::cout << "RESPONSE: " << response << std::endl;

    boost::shared_ptr<iota::Device> dev;

    ASYNC_TIME_WAIT

    //change of endpoint parameter should have happend, let's check it.


    dev = ul20serv.get_device(dev_name,service,subservice);

    code_res = adminService.delete_service_json(col, service, "/ssrv2", service, apikey, "/iot/d", true,
                     http_response, response, "1234", "4444");

    std::string protocol = "";
    code_res = adminService.delete_device_json(service,subservice,dev_name,http_response,response,"12334",protocol);

    if (dev.get() != NULL){

      IOTASSERT_MESSAGE("endpoint hasn't changed for device: " + dev->_endpoint ,dev->_endpoint.compare(new_endpoint)== 0);

    }else{
      IOTASSERT_MESSAGE("Device doesn't exist: ERROR",false);
    }

  std::cout << "END testChangeIPDevice " << std::endl;
}

void Ul20Test::testChangeIPDevice_empty(){

  /*
  This test covers the opposite scenario to the previous one, so in this case, device's endpoint can't be changed
  if input value is empty (in order not to change PUSH/PULL commands behaviour).

  USING  mongodb.

            "device_id": "unitTest_dev1_endpoint",
            "endpoint": "",
  */
  std::cout << "START testChangeIPDevice_empty" << std::endl;

  iota::Configurator::initialize("../../tests/iotagent/config_mongo.json");
  std::string dev_name = "unitTest_dev1_change_endpoint";
  std::string service = "service2";
  std::string subservice = "/ssrv2";
  std::string apikey = "apikey3";
  std::string new_endpoint = "";

  boost::shared_ptr<iota::ServiceCollection> col(new iota::ServiceCollection());
  iota::DeviceCollection table_device;

//REMOVING previous data from mongo
  try{
  iota::Device borrar("", "");

  table_device.removed(borrar);

  mongo::BSONObj all;

  col->remove(all);

  }catch(std::exception exc){

  }


  pion::http::response http_response;
  iota::AdminService adminService;
  iota::UL20Service ul20serv;
  ul20serv.set_resource("/iot/d");
  adminService.add_service("/iot/d",&ul20serv);



  std::string response;
  int code_res;

  std::cout << "@UT@POST Service" << std::endl;


  code_res = adminService.post_service_json( col, service, subservice, POST_SERVICE_ENDPOINT,
                     http_response, response, apikey, "5678");
  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);

//FIRST SUB-TEST: new_endpoint is empty, but device already has one endpoint

  std::cout << "@UT@POST Device" << std::endl;
   std::string post_device = boost::str(boost::format(POST_DEVICE_ENDPOINT) % dev_name % dev_name);
  code_res = adminService.post_device_json(service, subservice, post_device,
                     http_response, response);


  std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
  IOTASSERT(code_res == POST_RESPONSE_CODE);


 {
  std::string encoded_endpoint = "";
  std::string querySTR = "i="+dev_name+"&k=" + apikey + "&ip="+encoded_endpoint;
  pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content("");

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    query_parameters.insert(std::pair<std::string,std::string>("i",dev_name));
    query_parameters.insert(std::pair<std::string,std::string>("k",apikey));
    //include the IP parameter to change it
    query_parameters.insert(std::pair<std::string,std::string>("ip",new_endpoint));



    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);

  }

    std::cout << "RESPONSE: " << response << std::endl;

    boost::shared_ptr<iota::Device> dev;

    ASYNC_TIME_WAIT

    //change of endpoint parameter should NOT have happend, let's check it.



    dev = ul20serv.get_device(dev_name,service,subservice);



    if (dev.get() != NULL){

      IOTASSERT_MESSAGE("endpoint Should not have changed for device: " + dev->_endpoint ,dev->_endpoint.compare(new_endpoint) != 0);

    }else{
      IOTASSERT_MESSAGE("Device doesn't exist: ERROR",false);
    }


     std::string protocol = "";
    code_res = adminService.delete_device_json(service,subservice,dev_name,http_response,response,"12334",protocol);


//SECOND SUB-TEST: new_endpoint is not empty

    std::cout << "@UT@POST Device with no endpoint" << std::endl;
   post_device = boost::str(boost::format(POST_DEVICE_NO_ENDPOINT) % dev_name % dev_name);
    code_res = adminService.post_device_json(service, subservice, post_device,
                     http_response, response);

    std::cout << "@UT@RESPONSE: " <<  code_res << " " << response << std::endl;
    IOTASSERT(code_res == POST_RESPONSE_CODE);


   {
    std::string encoded_endpoint = "";
    encoded_endpoint = pion::algorithm::url_encode(new_endpoint);
    std::string querySTR = "i="+dev_name+"&k=" + apikey + "&ip="+encoded_endpoint;

    pion::http::request_ptr http_request(new pion::http::request("/iot/d"));
    http_request->set_method("POST");
    http_request->set_query_string(querySTR);
    http_request->set_content("");

    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;

    query_parameters.insert(std::pair<std::string,std::string>("i",dev_name));
    query_parameters.insert(std::pair<std::string,std::string>("k",apikey));
    //include the IP parameter to change it
    query_parameters.insert(std::pair<std::string,std::string>("ip",new_endpoint));



    ul20serv.service(http_request, url_args, query_parameters,
                     http_response, response);
  }

    ASYNC_TIME_WAIT

    dev = ul20serv.get_device(dev_name,service,subservice);


    code_res = adminService.delete_service_json(col, service, "/ssrv2", service, apikey, "/iot/d", true,
                     http_response, response, "1234", "4444");


    //change of endpoint parameter should NOT have happend, let's check it.

   code_res = adminService.delete_device_json(service,subservice,dev_name,http_response,response,"12334",protocol);



    if (dev.get() != NULL){

      IOTASSERT_MESSAGE("endpoint Should not have changed for device: " + dev->_endpoint ,dev->_endpoint.compare(new_endpoint) != 0);

    }else{
      IOTASSERT_MESSAGE("Device doesn't exist: ERROR",false);
    }


  std::cout << "END testChangeIPDevice_empty " << std::endl;

}
