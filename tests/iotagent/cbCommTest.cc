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
#include "cbCommTest.h"
#include "ngsi/Attribute.h"
#include "ngsi/ContextElement.h"
#include "ngsi/UpdateContext.h"
#include "ngsi/ContextResponses.h"
#include "rest/iot_cb_comm.h"
#include "util/alarm.h"
#include "services/admin_service.h"
#include <stdexcept>
#include <boost/property_tree/ptree.hpp>

#define  PATH_CONFIG "../../tests/iotagent/config.json"
#define  PATH_BAD_CONFIG "../../tests/iotagent/config_bad_mongo.json"

CPPUNIT_TEST_SUITE_REGISTRATION(cbCommTest);
namespace iota {
std::string logger("main");
std::string URL_BASE("/iot");
}
iota::AdminService* AdminService_ptr;

bool handler_invoked = false;
void handler_function(std::string response, int status) {
  std::cout << "Content in handler " << response << " " << status << std::endl;
  handler_invoked = true;

};


void cbCommTest::testSend() {
  boost::shared_ptr<HttpMock> http_mock;
  http_mock.reset(new HttpMock("/mock"));
  http_mock->init();
  std::string mock_port = boost::lexical_cast<std::string>(http_mock->get_port());
  boost::shared_ptr<boost::asio::io_service> io_service(new
      boost::asio::io_service());
  iota::Attribute attribute("name", "type", "value");
  iota::Attribute metadata("name", "typecheck", "valor");
  iota::Attribute metadata1("name1", "type1", "value1");
  attribute.add_metadata(metadata1);
  attribute.add_metadata(metadata);

  iota::ContextElement context_element("id", "type", "false");
  context_element.add_attribute(attribute);

  iota::UpdateContext operation(std::string("UPDATE"));
  operation.add_context_element(context_element);
  std::string response_cb;
  response_cb.append("{\"contextResponses\" : [{\"contextElement\" : {\"type\" : \"type\",\"isPattern\" : \"false\",\"id\" : \"id\",\"attributes\" : [{");
  response_cb.append("\"name\" : \"name\",\"type\" : \"type\",\"value\" : \"\",\"metadatas\" : [{");
  response_cb.append("\"name\" : \"name1\",\"type\" : \"type1\",\"value\" : \"value1\"},{\"name\" : \"name\",\"type\" : \"typecheck\",\"value\" : \"valor\"}]}]},");
  response_cb.append("\"statusCode\" : {\"code\" : \"200\",\"reasonPhrase\" : \"OK\"}}]}");
  http_mock->set_response(200, response_cb);
  iota::ContextBrokerCommunicator cb;
  //cb.start();
  //std::string url("http://10.95.168.57:1026/NGSI10/updateContext");
  std::string url = "http://127.0.0.1:"+ mock_port + "/mock";
  boost::property_tree::ptree pt1;
  pt1.put("service", "service");
  boost::property_tree::ptree c1;
  c1.put("timeout", 10);
  pt1.add_child("config.cbroker", c1);
  std::string response = cb.send(url, operation.get_string(), pt1);
  std::cout << "Received:" << response << std::endl;
  std::istringstream is(response);
  iota::ContextResponses responses(is);
  CPPUNIT_ASSERT(responses.get_context_responses().size() == 1);
  CPPUNIT_ASSERT(responses.get_context_responses()[0].get_code().compare("200") ==
                 0);
  http_mock->stop();

}

void cbCommTest::testSynchSendTimeout() {

  boost::shared_ptr<iota::HttpClient> http_client;
  std::cout << "START testSynchSendTimeout" << std::endl;
  std::string endpoint = "192.0.0.1";
  http_client.reset(new iota::HttpClient(endpoint, 9001));
  pion::http::request_ptr request(new pion::http::request());
  pion::http::response_ptr response_ptr = http_client->send(request, 5,
                                          "");
  CPPUNIT_ASSERT(response_ptr.get() == NULL);
  std::cout << "Received:"  << std::endl;
  std::cout << "STOP testSynchSendTimeout" << std::endl;
}

/*
void cbCommTest::testAsynchSendTimeout() {

  std::cout << "START testAsyncSendTimeout" << std::endl;
  boost::shared_ptr<boost::asio::io_service> io_service(new
      boost::asio::io_service());

  boost::shared_ptr<iota::ContextBrokerCommunicator> cb(new
      iota::ContextBrokerCommunicator(
        *io_service));
  //cb->start();
  //cb.start();
  //std::string url("http://10.95.168.57:1026/NGSI10/updateContext");
  std::string url = "http://192.0.0.1:9001/mock";
  boost::property_tree::ptree pt1;
  pt1.put("service", "service");
  boost::property_tree::ptree c1;
  c1.put("timeout", 10);
  pt1.add_child("config.cbroker", c1);
  std::cout << "Async send" << std::endl;
  cb->async_send(url, "UPDATE", pt1, boost::bind(handler_function,
                 _1, _2));

  while (!handler_invoked) {
    io_service->run();
  }
  CPPUNIT_ASSERT_MESSAGE("Handler invoked ", handler_invoked);
  std::cout << "END testAsyncSend" << std::endl;
}*/

void cbCommTest::testAsynchSendTimeout() {

  std::cout << "START testAsyncSendTimeout2" << std::endl;

  boost::shared_ptr<iota::ContextBrokerCommunicator> cb(new
      iota::ContextBrokerCommunicator());
  cb->start();

  //std::string url("http://10.95.168.57:1026/NGSI10/updateContext");
  std::string url = "http://192.0.0.1:9001/mock";
  boost::property_tree::ptree pt1;
  pt1.put("service", "service");
  boost::property_tree::ptree c1;
  c1.put("timeout", 10);
  pt1.add_child("config.cbroker", c1);
  std::cout << "Async send" << std::endl;
  cb->async_send(url, "UPDATE", pt1, boost::bind(handler_function,
                 _1, _2));

  /*while (!handler_invoked) {
    io_service->run();
  }*/
  CPPUNIT_ASSERT_MESSAGE("Handler invoked ", handler_invoked);
  std::cout << "END testAsyncSend2" << std::endl;
}


void cbCommTest::testAsyncSend() {
  boost::shared_ptr<HttpMock> http_mock;
  http_mock.reset(new HttpMock("/mock"));
  http_mock->init();
  std::string mock_port = boost::lexical_cast<std::string>(http_mock->get_port());

  std::cout << "START testAsyncSend" << std::endl;
  iota::Attribute attribute("name", "type", "value");
  iota::Attribute metadata("name", "typecheck", "valor");
  iota::Attribute metadata1("name1", "type1", "value1");
  attribute.add_metadata(metadata1);
  attribute.add_metadata(metadata);

  iota::ContextElement context_element("id", "type", "false");
  context_element.add_attribute(attribute);

  iota::UpdateContext operation(std::string("UPDATE"));
  operation.add_context_element(context_element);
  std::string response_cb;
  response_cb.append("{\"contextResponses\" : [{\"contextElement\" : {\"type\" : \"type\",\"isPattern\" : \"false\",\"id\" : \"id\",\"attributes\" : [{");
  response_cb.append("\"name\" : \"name\",\"type\" : \"type\",\"value\" : \"\",\"metadatas\" : [{");
  response_cb.append("\"name\" : \"name1\",\"type\" : \"type1\",\"value\" : \"value1\"},{\"name\" : \"name\",\"type\" : \"typecheck\",\"value\" : \"valor\"}]}]},");
  response_cb.append("\"statusCode\" : {\"code\" : \"200\",\"reasonPhrase\" : \"OK\"}}]}");
  http_mock->set_response(200, response_cb);
  boost::shared_ptr<iota::ContextBrokerCommunicator> cb(new
      iota::ContextBrokerCommunicator());
  cb->start();
  //cb.start();
  //std::string url("http://10.95.168.57:1026/NGSI10/updateContext");
  std::string url = "http://127.0.0.1:"+ mock_port + "/mock";
  boost::property_tree::ptree pt1;
  pt1.put("service", "service");
  boost::property_tree::ptree c1;
  c1.put("timeout", 10);
  c1.put("token", "MyToken");
  c1.put("oauth.on_behalf_trust_url", "http://127.0.0.1/auth");
  c1.put("oauth.on_behalf_user", "iotagent");
  c1.put("oauth.on_behalf_password", "iotagent");
  pt1.add_child("config.cbroker", c1);
  std::cout << "Async send" << std::endl;
  cb->async_send(url, operation.get_string(), pt1, boost::bind(handler_function,
                 _1, _2));
  /*
  std::cout << response << std::endl;
  std::istringstream is(response);
  iota::ContextResponses responses(is);
  CPPUNIT_ASSERT(responses.get_context_responses().size() == 1);
  CPPUNIT_ASSERT(responses.get_context_responses()[0].get_code().compare("200") ==
                 0);
  cb->async_send(url, operation.get_string(), pt1, boost::bind(handler_function,
                             boost::ref(req), boost::ref(connection), oauth));
                             */
  std::cout << __FUNCTION__ << " CB" << cb.use_count() << std::endl;
  /*while (!handler_invoked) {
    io_service->run();
  }*/
  http_mock->stop();
  CPPUNIT_ASSERT_MESSAGE("Handler invoked ", handler_invoked);
  std::cout << "END testAsyncSend" << std::endl;

}

void cbCommTest::testAlarm() {

  std::cout << "START testAlarm" << std::endl;
  iota::Configurator::initialize(PATH_BAD_CONFIG);

  const iota::JsonValue& storage=
    iota::Configurator::instance()->get(iota::store::types::STORAGE);

  std::string command_name = "PING";
  std::string command_att  ="status";
  std::string  type = "type2";
  std::string value = "value";
  boost::shared_ptr<iota::Device> item_dev;
  item_dev.reset(new iota::Device("dev1", "service"));
  item_dev->_entity_type = "entity_type";

  boost::property_tree::ptree  service;
  service.put("cbroker", "http://127.0.0.1:9999/d");
  service.put("service", "service2");
  service.put("service_path", "/ssrv2");

  std::string opSTR = "APPEND";

  try {
    std::cout << "testGenericCollection2" << std::endl;
    iota::ContextBrokerCommunicator cb_comm;
    cb_comm.send_updateContext(command_name, command_att,
                               type,
                               value, item_dev,
                               service, opSTR);
  }
  catch (std::exception& e) {
    CPPUNIT_ASSERT_MESSAGE("alarm not found", iota::Alarm::instance()->size() == 1);
  }

  {


    //simulador del device
    boost::shared_ptr<HttpMock> device_mock;
    device_mock.reset(new HttpMock(9999, "/d"));
    device_mock->init();


    iota::ContextBrokerCommunicator cb_comm;
    cb_comm.send_updateContext(command_name, command_att,
                               type,
                               value, item_dev,
                               service, opSTR);
    CPPUNIT_ASSERT_MESSAGE("alarm not found", iota::Alarm::instance()->size() == 0);
    device_mock->stop();
  }

  std::cout << "END testAlarm " << std::endl;



}

/**
Test to demonstrate what the example of multiple measures (bulk) in a updateContext should look like
*/
void cbCommTest::testMultiMeasuresUpdateContext(){

 /*std::string response_cb;

  response_cb.append("{\"contextResponses\" : [{\"contextElement\" : {\"type\" : \"type\",\"isPattern\" : \"false\",\"id\" : \"id\",\"attributes\" : [{");
  response_cb.append("\"name\" : \"name\",\"type\" : \"type\",\"value\" : \"\",\"metadatas\" : [{");
  response_cb.append("\"name\" : \"name1\",\"type\" : \"type1\",\"value\" : \"value1\"},{\"name\" : \"name\",\"type\" : \"typecheck\",\"value\" : \"valor\"}]}]},");
  response_cb.append("\"statusCode\" : {\"code\" : \"200\",\"reasonPhrase\" : \"OK\"}},");
  response_cb.append("{\"contextElement\" : {\"type\" : \"type\",\"isPattern\" : \"false\",\"id\" : \"id\",\"attributes\" : [{");
  response_cb.append("\"name\" : \"name2\",\"type\" : \"type\",\"value\" : \"33\",\"metadatas\" : [{");
  response_cb.append("\"name\" : \"name3\",\"type\" : \"type1\",\"value\" : \"value1\"},{\"name\" : \"name\",\"type\" : \"typecheck\",\"value\" : \"valor\"}]}]},");
  response_cb.append("\"statusCode\" : {\"code\" : \"200\",\"reasonPhrase\" : \"OK\"}}]}");
*/
  std::string response_cb;
  response_cb.append("{\"updateAction\":\"UPDATE\",\"contextElements\":[{\"id\":\"id\",\"type\":\"type\",\"isPattern\":\"false\",\"attributes\":[{");
  response_cb.append("\"name\":\"name\",\"type\":\"type\",\"value\":\"value\",\"metadatas\":[{\"name\":\"name1\",\"type\":\"type1\",\"value\":\"value1\"},");
  response_cb.append("{\"name\":\"name\",\"type\":\"typecheck\",\"value\":\"valor\"}]}]},{\"id\":\"id\",\"type\":\"type\",\"isPattern\":\"false\",\"attributes\":");
  response_cb.append("[{\"name\":\"name2\",\"type\":\"type\",\"value\":\"value\",\"metadatas\":[{\"name\":\"name\",\"type\":\"typecheck\",\"value\":\"valor\"},");
  response_cb.append("{\"name\":\"name3\",\"type\":\"type1\",\"value\":\"value1\"}]}]}]}");



 boost::shared_ptr<HttpMock> http_mock;
  http_mock.reset(new HttpMock("/mock"));
  http_mock->init();
  std::string mock_port = boost::lexical_cast<std::string>(http_mock->get_port());

  std::cout << "START testMultiMeasuresUpdateContext" << std::endl;
  iota::Attribute attribute("name", "type", "value");
  iota::Attribute metadata("name", "typecheck", "valor");
  iota::Attribute metadata1("name1", "type1", "value1");
  attribute.add_metadata(metadata1);
  attribute.add_metadata(metadata);

  iota::ContextElement context_element("id", "type", "false");
  context_element.add_attribute(attribute);



  iota::Attribute attribute2("name2", "type", "value");
  iota::Attribute metadata3("name", "typecheck", "valor");
  iota::Attribute metadata4("name3", "type1", "value1");
  attribute2.add_metadata(metadata3);
  attribute2.add_metadata(metadata4);

  iota::ContextElement context_element_2("id", "type", "false");
  context_element_2.add_attribute(attribute2);

   iota::UpdateContext operation(std::string("UPDATE"));

  operation.add_context_element(context_element);

  operation.add_context_element(context_element_2);//Another context element.

  std::cout <<"String to be sent: " <<  operation.get_string() << std::endl;


  iota::ContextBrokerCommunicator cb;
  //cb.start();
  //std::string url("http://10.95.168.57:1026/NGSI10/updateContext");
  std::string url = "http://127.0.0.1:"+ mock_port + "/mock";
  boost::property_tree::ptree pt1;
  pt1.put("service", "service");
  boost::property_tree::ptree c1;
  c1.put("timeout", 10);
  pt1.add_child("config.cbroker", c1);
  std::string response = cb.send(url, operation.get_string(), pt1);
  std::cout << "Received:" << response << std::endl;

  boost::trim(response);
  boost::erase_all(response,"\n");
  boost::erase_all(response,"\r");

  CPPUNIT_ASSERT_MESSAGE("UpdateContext incorrect",response.compare(response_cb) == 0);

  http_mock->stop();


}

