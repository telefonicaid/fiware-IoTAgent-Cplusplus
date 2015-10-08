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
#include "ttTest.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <cppunit/extensions/HelperMacros.h>

#include <cmath>
#include <ctime>

#include <pion/algorithm.hpp>

#include "CC_Logger.h"
#include "input_buffer/ESP_Plugin_Input_Buffer.h"
#include "thinkingthings/DecodeTTJSON.h"

#include "../mocks/util_functions.h"

using ::testing::Return;
using ::testing::NotNull;
using ::testing::StrEq;
using ::testing::_;
using ::testing::Invoke;

CPPUNIT_TEST_SUITE_REGISTRATION(TTTest);

TTTest::TTTest() {}

TTTest::~TTTest() {}

void TTTest::setUp() {}

void TTTest::tearDown() {}

void TTTest::testSearchResponses() {
  iota::esp::TTService* ttService =
      (iota::esp::TTService*)iota::Process::get_process().get_service(
          "/TestTT/tt");
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");

  TestSetup test_setup(get_service_name(__FUNCTION__), "/TestTT/tt");

  ESPLib esplib;
  iota::esp::tt::TTResponse* myResponse;
  iota::esp::tt::SearchResponse* seeker = new iota::esp::tt::SearchResponse();

  CC_Logger::getSingleton()->logDebug(
      "TEST: testSearchResponses starting ... ");

  int idsensor = esplib.createSensor("../../tests/iotagent/sensortt-open.xml");

  std::string response;

  /*{
  "contextResponses": [
      {
          "contextElement": {
              "attributes": [
                  {
                      "name": "amp",
                      "type": "string",
                      "value": "3"
                  },
                  {
                      "name": "refresh",
                      "type": "string",
                      "value": "44"
                  }
              ],
              "id": "8934075379000039321",
              "isPattern": "false",
              "type": "thing"
          },
          "statusCode": {
              "code": "200",
              "reasonPhrase": "OK"
          }
      }
  ]
  }*/

  std::string mockResponse;
  mockResponse.assign("{\"contextResponses\": [");
  mockResponse.append("{ \"contextElement\":{");
  mockResponse.append("\"attributes\": [{");
  mockResponse.append("\"name\":\"amp\",\"type\":\"string\",");
  mockResponse.append("\"value\":\"3\"");
  mockResponse.append("},{");
  mockResponse.append("\"name\":\"refresh\",\"type\":\"string\",");
  mockResponse.append("\"value\":\"44\"");
  mockResponse.append("}],");
  mockResponse.append("\"id\":\"8934075379000039321\",");
  mockResponse.append("\"isPattern\":\"false\",\"type\":\"");
  mockResponse.append(get_service_name(__FUNCTION__));
  mockResponse.append("\"},");
  mockResponse.append(
      "\"statusCode\":{\"code\":\"200\",\"reasonPhrase\":\"OK\"}}]}");

  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/queryContext", 200,
      mockResponse);

  if (idsensor > 0) {
    esplib.startSensor(idsensor, "main");

    ESP_Input_Buffer ibuffer;
    std::map<std::string, std::string> params;
    // Test input
    params.clear();

    std::string input =
        "cadena=#8934075379000039321,#0,GM,amp,3,#0,GC,refresh,33,#1,GM,"
        "inexistent,55,23$none";

    ibuffer.context.addInputData(input.c_str(), input.length());

    ESP_Result myResult =
        esplib.executeRunnerFromInput(idsensor, "main", params, &ibuffer);

    CC_Logger::getSingleton()->logDebug("Result size: %d",
                                        myResult.ppresults.size());

    std::string device_id = myResult.findInputAttributeAsString(
        "request",
        true);  // The keyword "request" is defined somewhere in the ESP Xml
                // file, how could I get it?

    CPPUNIT_ASSERT_EQUAL(device_id, std::string("8934075379000039321"));

    boost::property_tree::ptree pb_tree;
    ttService->get_service_by_apiKey(pb_tree, get_service_name(__FUNCTION__));

    iota::esp::tt::QueryContextWrapper* queryC =
        new iota::esp::tt::QueryContextWrapper(&pb_tree);

    response.assign(seeker->searchTTResponse(myResult.attresults, device_id,
                                             get_service_name(__FUNCTION__),
                                             queryC));

    CPPUNIT_ASSERT_EQUAL(
        std::string("#0,GM,amp,$,#0,GC,refresh,44,$,#1,GM,inexistent,23$none"),
        response);

    CC_Logger::getSingleton()->logDebug("TEST: testSearchResponses DONE");
  }
  esplib.stopSensor(idsensor);
  esplib.destroySensor(idsensor);

  CPPUNIT_ASSERT(idsensor > 0);
}

void TTTest::testTTResponses() {
  CC_Logger::getSingleton()->logDebug("testTTResponse: starting");
  // GENERIC strings
  std::string idDevice("8934075379000039321");
  std::string entity_type("thing");
  std::string busid("0");
  std::string module("GM");
  std::string param1("amp");
  std::string param2("11");  // This is the received value, it may not be the
                             // same as the stored one (from CB), but that's
                             // fine

  std::string expected("#0,GM,amp,$");  // We expect the returned value from
                                        // ContextBroker, not the one coming
                                        // from ESP

  // INPUT FROM ESP
  CC_AttributesType mapAttr;

  ESP_Attribute esp_busid;
  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));

  // INPUT from ContextBroker
  ::iota::ContextElement cElement(idDevice, entity_type, "false");

  ::iota::Attribute attrib("amp", "string", "33");
  cElement.add_attribute(attrib);

  iota::esp::tt::TTResponse* ttResponse =
      iota::esp::tt::TTResponse::decodeStringToTTResponse(module);

  ttResponse->setBusId(busid);
  // TEST
  ttResponse->processContextElement(cElement, mapAttr);
  CC_Logger::getSingleton()->logDebug("testTTResponse: output: DONE");
  CPPUNIT_ASSERT_EQUAL(expected, ttResponse->toTTString());

  delete ttResponse;
}

void TTTest::testTTResponsesP1() {
  CC_Logger::getSingleton()->logDebug("testTTResponsesP1: starting");
  // GENERIC strings
  std::string idDevice("8934075379000039321");
  std::string entity_type("thing");
  std::string busid("0");
  std::string module("P1");
  // These are the received values,
  std::string param1("11");
  std::string param2("23");
  std::string param3("33");
  std::string param4("44");

  std::string expected("#0,P1,$");  // We expect DEFAULT response

  // INPUT FROM ESP
  CC_AttributesType mapAttr;

  ESP_Attribute esp_busid;
  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param3;
  esp_param3.setValue("param3", param3.c_str(), param3.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param4;
  esp_param2.setValue("param4", param4.c_str(), param4.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));
  mapAttr.insert(CC_AttributesPair("param_3", esp_param2));
  mapAttr.insert(CC_AttributesPair("param_4", esp_param2));

  // INPUT from ContextBroker
  ::iota::ContextElement cElement(idDevice, entity_type, "false");

  ::iota::Attribute attrib1("P1", "string", "T");

  cElement.add_attribute(attrib1);

  iota::esp::tt::TTResponse* ttResponse =
      iota::esp::tt::TTResponse::decodeStringToTTResponse(module);

  ttResponse->setBusId(busid);
  // TEST: INPUT from CB and INPUT from ESP (what comes from device) are
  // compared
  ttResponse->processContextElement(cElement, mapAttr);
  CC_Logger::getSingleton()->logDebug("testTTResponsesP1: output: DONE");
  CPPUNIT_ASSERT_EQUAL(expected, ttResponse->toTTString());

  delete ttResponse;
}

void TTTest::testTTResponsesP1NoTResponse() {
  CC_Logger::getSingleton()->logDebug("testTTResponsesP1NoTResponse: starting");
  // GENERIC strings
  std::string idDevice("8934075379000039321");
  std::string entity_type("thing");
  std::string busid("0");
  std::string module("P1");
  // These are the received values,
  std::string param1("11");
  std::string param2("23");
  std::string param3("33");
  std::string param4("44");

  std::string expected("#0,P1,$");  // We expect the returned values from
                                    // ContextBroker, not the one coming from
                                    // ESP

  // INPUT FROM ESP
  CC_AttributesType mapAttr;

  ESP_Attribute esp_busid;
  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param3;
  esp_param3.setValue("param3", param3.c_str(), param3.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param4;
  esp_param2.setValue("param4", param4.c_str(), param4.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));
  mapAttr.insert(CC_AttributesPair("param_3", esp_param2));
  mapAttr.insert(CC_AttributesPair("param_4", esp_param2));

  // INPUT from ContextBroker, what we need to put into the response.
  ::iota::ContextElement cElement(idDevice, entity_type, "false");

  ::iota::Attribute attrib1("P1", "string", "");

  cElement.add_attribute(attrib1);

  iota::esp::tt::TTResponse* ttResponse =
      iota::esp::tt::TTResponse::decodeStringToTTResponse(module);

  ttResponse->setBusId(busid);
  // TEST: INPUT from CB and INPUT from ESP (what comes from device) are
  // compared
  ttResponse->processContextElement(cElement, mapAttr);

  CC_Logger::getSingleton()->logDebug(
      "testTTResponsesP1NoTResponse: output: %s",
      ttResponse->toTTString().c_str());
  CPPUNIT_ASSERT_EQUAL(expected, ttResponse->toTTString());
  CC_Logger::getSingleton()->logDebug(
      "testTTResponsesP1NoTResponse: output: DONE");
  delete ttResponse;
}

void TTTest::testTTResponsesB() {
  CC_Logger::getSingleton()->logDebug("testTTResponsesB: starting");
  // GENERIC strings
  std::string idDevice("8934075379000039321");
  std::string entity_type("thing");
  std::string busid("0");
  std::string module("B");
  // These are the received values,not really relevant for this test
  std::string param1("11");
  std::string param2("23");
  std::string param3("33");
  std::string param4("44");
  std::string param5("55");
  std::string param6("66");

  std::string expected("#0,B,23,55,66,$");  // Expecting DEFAULT RESPONSE

  // INPUT FROM ESP
  CC_AttributesType mapAttr;

  ESP_Attribute esp_busid;
  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param3;
  esp_param3.setValue("param3", param3.c_str(), param3.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param4;
  esp_param4.setValue("param4", param4.c_str(), param4.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param5;
  esp_param5.setValue("param5", param5.c_str(), param5.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param6;
  esp_param6.setValue("param6", param6.c_str(), param6.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));
  mapAttr.insert(CC_AttributesPair("param_3", esp_param3));
  mapAttr.insert(CC_AttributesPair("param_4", esp_param4));
  mapAttr.insert(CC_AttributesPair("param_5", esp_param5));
  mapAttr.insert(CC_AttributesPair("param_6", esp_param6));

  // INPUT from ContextBroker
  ::iota::ContextElement cElement(idDevice, entity_type, "false");

  ::iota::Attribute compound("B", "compound", "[]");

  ::iota::Attribute attrib1(TT_B_CHARGING_MODE, "string", "mode_value");
  ::iota::Attribute attrib2(TT_B_DISCONN_STACK_TIME, "string", "discon_value");
  ::iota::Attribute attrib3(TT_B_STATUS_CHARGE_ACTIVATION, "string",
                            "status_value");

  compound.add_value_compound(attrib1);
  compound.add_value_compound(attrib2);
  compound.add_value_compound(attrib3);

  cElement.add_attribute(compound);

  CC_Logger::getSingleton()->logDebug(
      "testTTResponsesB: Attribte compound: length %d",
      compound.get_value_compound().size());
  CC_Logger::getSingleton()->logDebug(
      "testTTResponsesB: ContextElement attributes:  %d",
      cElement.get_attributes().size());

  iota::esp::tt::TTResponse* ttResponse =
      iota::esp::tt::TTResponse::decodeStringToTTResponse(module);

  ttResponse->setBusId(busid);

  ttResponse->processContextElement(cElement, mapAttr);
  CC_Logger::getSingleton()->logDebug("testTTResponsesB: output: DONE");
  CPPUNIT_ASSERT_EQUAL(expected, ttResponse->toTTString());

  delete ttResponse;
}

void TTTest::testTTCBPublisher() {
  iota::esp::TTService* ttService =
      (iota::esp::TTService*)iota::Process::get_process().get_service(
          "/TestTT/tt");
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  CC_Logger::getSingleton()->logDebug("testTTCBPublisher: starting");

  TestSetup test_setup(get_service_name(__FUNCTION__), "/TestTT/tt");

  std::string entity("thing");  // This test works without a provisioned device

  std::string expected = "";

  boost::property_tree::ptree pb_tree;
  ttService->get_service_by_apiKey(pb_tree, get_service_name(__FUNCTION__));

  iota::RiotISO8601 timeInstant;
  std::string strTime = timeInstant.toUTC().toString();

  expected.append("{\"updateAction\":\"APPEND\",\"contextElements\":");
  expected.append("[{\"id\":\"dev01\",\"type\":\"");
  expected.append(entity);
  expected.append("\",\"isPattern\":\"false\",\"attributes\"");
  expected.append(
      ":[{\"name\":\"te\",\"type\":\"string\",\"value\":\"234\",\"metadatas\"");
  expected.append(
      ":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"");
  expected.append(strTime);
  expected.append("\"}]},");
  expected.append(
      "{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"");
  expected.append(strTime);
  expected.append("\"}]}]}");

  std::string actual = "";

  std::string jsonTT = "";

  jsonTT.append(std::string("{\"name\" : \""));
  jsonTT.append("te");
  jsonTT.append("\",\"type\":\"string\",");
  jsonTT.append(std::string("\"value\" : \""));
  jsonTT.append("234");
  jsonTT.append(std::string("\"}"));

  iota::tt::TTCBPublisher* ttPublisher = new iota::tt::TTCBPublisher();
  ::iota::ContextElement cElement("dev01", "thing", "false");

  // iota::RiotISO8601 timeInstant("2014","5","7","14","35","35");
  std::vector<std::string> vJsonTT;

  vJsonTT.push_back(jsonTT);

  actual.assign(ttPublisher->publishContextBroker(cElement, vJsonTT, pb_tree,
                                                  timeInstant));
  boost::trim(actual);
  boost::erase_all(actual, "\n");
  boost::erase_all(actual, "\r");

  CPPUNIT_ASSERT_EQUAL(expected, actual);
  CC_Logger::getSingleton()->logDebug("testTTCBPublisher: DONE");
  delete ttPublisher;
}

/*
Simple test to prove if decoding both Postprocessor outputs is working as
expected
*/
void TTTest::testDecodeTTJSONS() {
  CC_Logger::getSingleton()->logDebug("testDecodeTTJSONS: starting");
  std::string processed(
      "{\"name\":\"__TTCurrent_te\",\"type\":\"string\",\"value\":\"234\","
      "\"metadatas\"");
  processed.append(
      ":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"");
  processed.append("time");
  processed.append("\"}]}");

  std::string plain(
      "{\"name\":\"te\",\"type\":\"string\",\"value\":\"234\",\"metadatas\"");
  plain.append(":[{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"");
  plain.append("time");
  plain.append("\"}]}");

  // This is what postprocessor will do.
  std::string actual("{ \"processed\": ");
  actual.append(processed);
  actual.append(",");
  actual.append("\"plain\": ");
  actual.append(plain);
  actual.append("}");

  CC_Logger::getSingleton()->logDebug("testDecodeTTJSONS: JSON:  %s ",
                                      actual.c_str());
  iota::esp::tt::DecodeTTJSON* decode = new iota::esp::tt::DecodeTTJSON();
  decode->parse(actual);
  CPPUNIT_ASSERT_EQUAL(plain, decode->getPlainJSON());
  CPPUNIT_ASSERT_EQUAL(processed, decode->getProcessedJSON());
  CC_Logger::getSingleton()->logDebug("testDecodeTTJSONS: DONE");
  delete decode;
}

void TTTest::testDecodeTTJSONsError() {
  std::string actual("{ \"processed\": ");
  actual.append("{}");
  actual.append("}");
  iota::esp::tt::DecodeTTJSON* decode = new iota::esp::tt::DecodeTTJSON();
  CPPUNIT_ASSERT_THROW(decode->parse(actual), std::runtime_error);
  actual.assign("{ \"plain\": ");
  actual.append("{}");
  actual.append("}");
  CPPUNIT_ASSERT_THROW(decode->parse(actual), std::runtime_error);
  actual.assign("{ \"processed\": ");
  CPPUNIT_ASSERT_THROW(decode->parse(actual), std::runtime_error);
  delete decode;
}

/*This test shows the scenario where it's the very first time a TT device
communicates with this iotagent. The first thing is going to happen is that
QueryContext will return empty responses for previous attributes (there are none
yet). So the TTService has to publish them
*/
void TTTest::testFirstTimeTTAttributes() {
  std::cout << "@UT@START testFirstTimeTTAttributes" << std::endl;

  iota::esp::TTService* ttService =
      (iota::esp::TTService*)iota::Process::get_process().get_service(
          "/TestTT/tt");
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  TestSetup test_setup(get_service_name(__FUNCTION__), "/TestTT/tt");

  test_setup.add_device("8934075379000039321",
                        ttService->get_protocol_data().protocol);

  // Setting responses for http_mocks
  // queryContext should not find any attributes:

  std::string mockResponseError;
  mockResponseError.assign("{\"errorCode\": {\"code\" : \"404\",");
  mockResponseError.append("\"reasonPhrase\" : \"No context element found\"}}");
  // Query
  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/queryContext", 404,
      mockResponseError);  // First response, no contextElements found

  std::string mockResponse;  // This response lacks P1.
  mockResponse.assign("{\"contextResponses\": [");
  mockResponse.append("{ \"contextElement\":{");
  mockResponse.append("\"attributes\": [{");
  mockResponse.append("\"name\":\"config_time\",\"type\":\"string\",");
  mockResponse.append("\"value\":\"88\"");  // ATTENTION TO THIS: this has to
                                            // come in the response.
  mockResponse.append("},{");
  mockResponse.append("\"name\":\"posicion\",\"type\":\"string\",");
  mockResponse.append("\"value\":\"44\"");
  mockResponse.append("}],");
  mockResponse.append("\"id\":\"8934075379000039321\",");
  mockResponse.append("\"isPattern\":\"false\",\"type\":\"");
  mockResponse.append(test_setup.get_service());
  mockResponse.append("\"},");
  mockResponse.append(
      "\"statusCode\":{\"code\":\"200\",\"reasonPhrase\":\"OK\"}}]}");

  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/updateContext", 200,
      mockResponse);

  // Query
  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/queryContext", 200,
      mockResponse);  // Second query.

  std::string mockResponseOK;

  std::cout << "@UT@ testFirstTimeTTAttributes: starting" << std::endl;

  std::string query =
      "cadena=#8934075379000039321,#0,GC,config_time,34,#0,GM,posicion,33.000/"
      "-3.234234,#0,P1,214,07,33f,633c,#0,B,11,22,33,44,55,66";

  query.assign(pion::algorithm::url_encode(query));

  pion::http::request_ptr http_request(new pion::http::request("/TestTT/tt"));
  http_request->set_method("POST");
  http_request->set_content(query);
  std::map<std::string, std::string> url_args;
  std::multimap<std::string, std::string> query_parameters;

  pion::http::response http_response;
  std::string response;
  ttService->service(http_request, url_args, query_parameters, http_response,
                     response);

  CC_Logger::getSingleton()->logDebug("testFirstTimeTTAttributes: CODE:  %d ",
                                      http_response.get_status_code());
  CC_Logger::getSingleton()->logDebug(
      "testFirstTimeTTAttributes: Response:  %s ", response.c_str());
  CPPUNIT_ASSERT_EQUAL(
      std::string(
          "#0,GC,config_time,88,$,#0,GM,posicion,$,#0,P1,$,#0,B,22,55,66,$"),
      response);
  CPPUNIT_ASSERT(http_response.get_status_code() == 200);

  std::string actual_mock(cb_mock->get_last(
      "/mock/" + test_setup.get_service() + "/NGSI10/updateContext"));
  std::cout << "@UT@CB: " << actual_mock << std::endl;
  CPPUNIT_ASSERT(!actual_mock.empty());
  std::string expected_p1;
  expected_p1.append("{\"name\":\"P1\",\"type\":\"compound\",\"value\":");
  expected_p1.append(
      "[{\"name\":\"mcc\",\"type\":\"string\",\"value\":\"214\"},");
  expected_p1.append(
      "{\"name\":\"mnc\",\"type\":\"string\",\"value\":\"07\"},");
  expected_p1.append(
      "{\"name\":\"cell-id\",\"type\":\"string\",\"value\":\"33f\"},");
  expected_p1.append(
      "{\"name\":\"lac\",\"type\":\"string\",\"value\":\"633c\"}");

  std::string expected_b;
  expected_b.append("{\"name\":\"B\",\"type\":\"compound\",\"value\":");
  expected_b.append(
      "[{\"name\":\"voltaje\",\"type\":\"string\",\"value\":\"11\"},");
  expected_b.append(
      "{\"name\":\"estado_activacion\",\"type\":\"string\",\"value\":\"22\"},");
  expected_b.append(
      "{\"name\":\"hay_cargador\",\"type\":\"string\",\"value\":\"33\"},");
  expected_b.append(
      "{\"name\":\"estado_carga\",\"type\":\"string\",\"value\":\"44\"},");
  expected_b.append(
      "{\"name\":\"modo_carga\",\"type\":\"string\",\"value\":\"55\"},");
  expected_b.append(
      "{\"name\":\"tiempo_desco_stack\",\"type\":\"string\",\"value\":\"66\"}");

  CPPUNIT_ASSERT(actual_mock.find(expected_b) != std::string::npos);
  CPPUNIT_ASSERT(actual_mock.find(expected_p1) != std::string::npos);

  std::cout << "@UT@END testFirstTimeTTAttributes" << std::endl;
}

/*
Check with ThinkingThings Team, but I think K1 attribute does not get published
as it has no value.
*/

void TTTest::testFirstTimeTTAttributesWithK() {
  CC_Logger::getSingleton()->logDebug(
      "testFirstTimeTTAttributesWithK: starting");
  iota::esp::TTService* ttService =
      (iota::esp::TTService*)iota::Process::get_process().get_service(
          "/TestTT/tt");
  TestSetup test_setup(get_service_name(__FUNCTION__), "/TestTT/tt");
  test_setup.add_device("8934075379000039321",
                        ttService->get_protocol_data().protocol);

  /*
    boost::shared_ptr<HttpMock> cb_mock_up;
    cb_mock_up.reset(new HttpMock("/NGSI10/updateContext"));
    boost::shared_ptr<HttpMock> cb_mock_query;
    cb_mock_query.reset(new HttpMock("/NGSI10/queryContext"));

    start_cbmock(cb_mock_up,cb_mock_query);
    */
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  // Setting responses for http_mocks
  // queryContext should not find any attributes:
  std::string mockResponseError;
  mockResponseError.assign("{\"errorCode\": {\"code\" : \"404\",");
  mockResponseError.append("\"reasonPhrase\" : \"No context element found\"}}");
  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/queryContext", 404,
      mockResponseError);  // First response, no contextElements found

  std::string mockResponse;  // This response lacks P1.
  mockResponse.assign("{\"contextResponses\": [");
  mockResponse.append("{ \"contextElement\":{");
  mockResponse.append("\"attributes\": [{");
  mockResponse.append("\"name\":\"config_time\",\"type\":\"string\",");
  mockResponse.append("\"value\":\"88\"");  // ATTENTION TO THIS: this has to
                                            // come in the response.
  mockResponse.append("},{");
  mockResponse.append("\"name\":\"posicion\",\"type\":\"string\",");
  mockResponse.append("\"value\":\"44\"");
  mockResponse.append("}],");
  mockResponse.append("\"id\":\"8934075379000039321\",");
  mockResponse.append("\"isPattern\":\"false\",\"type\":\"thing\"},");
  mockResponse.append(
      "\"statusCode\":{\"code\":\"200\",\"reasonPhrase\":\"OK\"}}]}");

  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/queryContext", 200,
      mockResponse);  // Second query.

  std::string mockResponseOK;

  std::string query =
      "cadena=#8934075379000039321,#0,GC,config_time,34,#0,GM,posicion,33.000/"
      "-3.234234,#0,P1,214,07,33f,633c,#0,K1,0$";

  query.assign(pion::algorithm::url_encode(query));

  pion::http::request_ptr http_request(
      new pion::http::request("/TestSample/tt"));
  http_request->set_method("POST");
  http_request->set_content(query);
  std::map<std::string, std::string> url_args;
  std::multimap<std::string, std::string> query_parameters;

  pion::http::response http_response;
  std::string response;
  ttService->service(http_request, url_args, query_parameters, http_response,
                     response);

  CC_Logger::getSingleton()->logDebug(
      "testFirstTimeTTAttributesWithK: CODE:  %d ",
      http_response.get_status_code());
  CC_Logger::getSingleton()->logDebug(
      "testFirstTimeTTAttributesWithK: Response:  %s ", response.c_str());

  CPPUNIT_ASSERT_EQUAL(
      std::string("#0,GC,config_time,88,$,#0,GM,posicion,$,#0,P1,$,#0,K1,0$"),
      response);
  CPPUNIT_ASSERT(http_response.get_status_code() == 200);
}

void TTTest::testParsingResponseOnBug_IDAS20197() {
  ESPLib libEsp;
  ESP_Postprocessor_TT* tt_post_ptr_ = new ESP_Postprocessor_TT();
  tt_post_ptr_->load_default_TTModules();

  std::string strBuffer("cadena=#deviceTT7,#0,GC,hum,35,$");
  CC_Logger::getSingleton()->logDebug(
      "testParsingResponseOnBug_IDAS20197: Starting...");

  int idSensor = libEsp.createSensor("../../tests/iotagent/sensortt-open.xml");

  if (idSensor > 0) {
    libEsp.startSensor(idSensor, "main");

    ESP_Input_Buffer ibuffer;
    std::map<std::string, std::string> params;
    // Test input
    params.clear();

    ibuffer.context.addInputData(strBuffer.c_str(), strBuffer.length());

    ESP_Result myResult =
        libEsp.executeRunnerFromInput(idSensor, "main", params, &ibuffer);

    CC_Logger::getSingleton()->logDebug(
        "testParsingResponseOnBug_IDAS20197 Result size: %d",
        myResult.ppresults.size());

    std::string device_id =
        myResult.findInputAttributeAsString("request", true);

    CPPUNIT_ASSERT_EQUAL(device_id, std::string("deviceTT7"));

    iota::esp::tt::DecodeTTJSON* decodeTT = new iota::esp::tt::DecodeTTJSON();
    std::string temp;

    tt_post_ptr_->execute(&myResult.attresults[0]);

    CPPUNIT_ASSERT(tt_post_ptr_->isResultValid());

    temp.assign(tt_post_ptr_->getResultData());
    CC_Logger::getSingleton()->logDebug(
        "Test testParsingResponseOnBug_IDAS20197 Before parsing JSON [%s]",
        temp.c_str());
    decodeTT->parse(temp);
    CC_Logger::getSingleton()->logDebug(
        "Test testParsingResponseOnBug_IDAS20197 decode JSON: [%s]",
        decodeTT->getPlainJSON().c_str());

    CPPUNIT_ASSERT_EQUAL(
        std::string("{\"name\":\"hum\",\"type\":\"string\",\"value\":\"35\"}"),
        decodeTT->getPlainJSON());
    delete decodeTT;
  }
  delete tt_post_ptr_;
  CPPUNIT_ASSERT(idSensor > 0);
}

void TTTest::testSearchResponsesGPS() {
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  iota::esp::TTService* ttService =
      (iota::esp::TTService*)iota::Process::get_process().get_service(
          "/TestTT/tt");

  TestSetup test_setup(get_service_name(__FUNCTION__), "/TestTT/tt");

  ESPLib esplib;
  iota::esp::tt::TTResponse* myResponse;
  iota::esp::tt::SearchResponse* seeker = new iota::esp::tt::SearchResponse();

  CC_Logger::getSingleton()->logDebug(
      "TEST: testSearchResponsesGPS starting ... ");

  int idsensor = esplib.createSensor("../../tests/iotagent/sensortt-open.xml");

  std::string response;

  std::string mockResponse;
  mockResponse.assign("{\"contextResponses\": [");
  mockResponse.append("{ \"contextElement\":{");
  mockResponse.append("\"attributes\": [{");
  mockResponse.append("\"name\":\"position\",\"type\":\"coords\",");
  mockResponse.append("\"value\":\"3.42432,-2.234234\",");
  mockResponse.append("\"metadatas\":[{");
  mockResponse.append(
      "\"name\":\"location\",\"type\":\"string\",\"value\":\"WGS84\"}]");
  mockResponse.append("}],");
  mockResponse.append("\"id\":\"8934075379000039321\",");
  mockResponse.append("\"isPattern\":\"false\",\"type\":\"thing\"},");
  mockResponse.append(
      "\"statusCode\":{\"code\":\"200\",\"reasonPhrase\":\"OK\"}}]}");

  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/queryContext", 200,
      mockResponse);

  if (idsensor > 0) {
    esplib.startSensor(idsensor, "main");

    ESP_Input_Buffer ibuffer;
    std::map<std::string, std::string> params;
    // Test input
    params.clear();

    std::string input =
        "cadena=#8934075379000039321,#0,GPS,3.42432,-2.234234,22,33,44,$";
    ibuffer.context.addInputData(input.c_str(), input.length());

    ESP_Result myResult =
        esplib.executeRunnerFromInput(idsensor, "main", params, &ibuffer);

    CC_Logger::getSingleton()->logDebug("Result size: %d",
                                        myResult.ppresults.size());

    std::string device_id = myResult.findInputAttributeAsString(
        "request",
        true);  // The keyword "request" is defined somewhere in the ESP Xml
                // file, how could I get it?

    CPPUNIT_ASSERT_EQUAL(device_id, std::string("8934075379000039321"));

    boost::property_tree::ptree pb_tree;
    ttService->get_service_by_apiKey(pb_tree, get_service_name(__FUNCTION__));

    iota::esp::tt::QueryContextWrapper* queryC =
        new iota::esp::tt::QueryContextWrapper(&pb_tree);

    response.assign(seeker->searchTTResponse(myResult.attresults, device_id,
                                             "thing", queryC));

    CPPUNIT_ASSERT_EQUAL(std::string("#0,GPS,$"), response);

    CC_Logger::getSingleton()->logDebug("TEST: testSearchResponsesGPS DONE");
    delete queryC;
  }
  delete seeker;
  esplib.stopSensor(idsensor);
  esplib.destroySensor(idsensor);

  CPPUNIT_ASSERT(idsensor > 0);
}

void TTTest::testSearchResponseOnBug_IDAS20201() {
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  iota::esp::TTService* ttService =
      (iota::esp::TTService*)iota::Process::get_process().get_service(
          "/TestTT/tt");

  TestSetup test_setup(get_service_name(__FUNCTION__), "/TestTT/tt");

  ESPLib esplib;
  iota::esp::tt::TTResponse* myResponse;
  iota::esp::tt::SearchResponse* seeker = new iota::esp::tt::SearchResponse();

  CC_Logger::getSingleton()->logDebug(
      "TEST: testSearchResponseOnBug_IDAS20201 starting ... ");

  int idsensor = esplib.createSensor("../../tests/iotagent/sensortt-open.xml");

  std::string response;

  std::string mockResponse;
  mockResponse.assign("{\"contextResponses\": [");
  mockResponse.append("{ \"contextElement\":{");
  mockResponse.append("\"attributes\": [{");
  mockResponse.append("\"name\":\"temp\",\"type\":\"string\",");
  mockResponse.append("\"value\":\"3\",");
  mockResponse.append("\"metadatas\":[{");
  mockResponse.append(
      "\"name\":\"sleeptime\",\"type\":\"string\",\"value\":\"234\"}]");
  mockResponse.append("}],");
  mockResponse.append("\"id\":\"8934075379000039321\",");
  mockResponse.append("\"isPattern\":\"false\",\"type\":\"thing\"},");
  mockResponse.append(
      "\"statusCode\":{\"code\":\"200\",\"reasonPhrase\":\"OK\"}}]}");

  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/queryContext", 200,
      mockResponse);

  if (idsensor > 0) {
    esplib.startSensor(idsensor, "main");

    ESP_Input_Buffer ibuffer;
    std::map<std::string, std::string> params;
    // Test input
    params.clear();

    std::string input =
        "cadena=#8934075379000039321,#0,GC,test,1,$,#0,B,12,23,34,45,56,67,50$"
        "wakeUP,#0,P1,1,2,3,4,5,$";
    ibuffer.context.addInputData(input.c_str(), input.length());

    ESP_Result myResult =
        esplib.executeRunnerFromInput(idsensor, "main", params, &ibuffer);

    CC_Logger::getSingleton()->logDebug("Result size: %d",
                                        myResult.ppresults.size());

    std::string device_id = myResult.findInputAttributeAsString(
        "request",
        true);  // The keyword "request" is defined somewhere in the ESP Xml
                // file, how could I get it?

    CPPUNIT_ASSERT_EQUAL(device_id, std::string("8934075379000039321"));

    boost::property_tree::ptree pb_tree;
    ttService->get_service_by_apiKey(pb_tree, get_service_name(__FUNCTION__));

    iota::esp::tt::QueryContextWrapper* queryC =
        new iota::esp::tt::QueryContextWrapper(&pb_tree);

    response.assign(seeker->searchTTResponse(myResult.attresults, device_id,
                                             "thing", queryC));

    CPPUNIT_ASSERT_EQUAL(
        std::string("#0,GC,test,1,$,#0,B,23,56,67,50$wakeUP,#0,P1,$"),
        response);

    CC_Logger::getSingleton()->logDebug(
        "TEST: testSearchResponseOnBug_IDAS20201 DONE");
    delete queryC;
  }
  esplib.stopSensor(idsensor);
  esplib.destroySensor(idsensor);
  delete seeker;
  CPPUNIT_ASSERT(idsensor > 0);
}

void TTTest::testTTCBPublisherP1BOnBug_IDAS20202() {
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  iota::esp::TTService* ttService =
      (iota::esp::TTService*)iota::Process::get_process().get_service(
          "/TestTT/tt");
  TestSetup test_setup(get_service_name(__FUNCTION__), "/TestTT/tt");
  CC_Logger::getSingleton()->logDebug(
      "testTTCBPublisherP1BOnBug_IDAS20202: starting");

  std::string entity("thing");

  std::string expected = "";

  boost::property_tree::ptree pb_tree;
  ttService->get_service_by_apiKey(pb_tree, get_service_name(__FUNCTION__));

  iota::RiotISO8601 timeInstant;  // I tried
  std::string strTime = timeInstant.toUTC().toString();

  expected.append("{\"updateAction\":\"APPEND\",\"contextElements\":");
  expected.append("[{\"id\":\"dev01\",\"type\":\"");
  expected.append(entity);
  expected.append("\",\"isPattern\":\"false\",\"attributes\":");
  expected.append("[{\"name\":\"B\",\"type\":\"compound\",\"value\":");
  expected.append(
      "[{\"name\":\"voltaje\",\"type\":\"string\",\"value\":\"12\"},");
  expected.append(
      "{\"name\":\"estado_activacion\",\"type\":\"string\",\"value\":\"23\"},");
  expected.append(
      "{\"name\":\"hay_cargador\",\"type\":\"string\",\"value\":\"34\"},");
  expected.append(
      "{\"name\":\"estado_carga\",\"type\":\"string\",\"value\":\"45\"},");
  expected.append(
      "{\"name\":\"modo_carga\",\"type\":\"string\",\"value\":\"56\"},");
  expected.append(
      "{\"name\":\"tiempo_desco_stack\",\"type\":\"string\",\"value\":\"67\"}]"
      ",");
  expected.append(
      "\"metadatas\":[{\"name\":\"sleepcondition\",\"type\":\"string\","
      "\"value\":\"wakeUP\"},");
  expected.append(
      "{\"name\":\"sleeptime\",\"type\":\"string\",\"value\":\"50\"},");
  expected.append(
      "{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"");
  expected.append(strTime);
  expected.append("\"}]},");
  expected.append(
      "{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"");
  expected.append(strTime);
  expected.append("\"}]}]}");

  std::string actual = "";

  std::string jsonTT = "";

  jsonTT.assign("{\"name\":\"B\",\"type\":\"compound\",\"value\":");
  jsonTT.append(
      "[{\"name\":\"voltaje\",\"type\":\"string\",\"value\":\"12\"},");
  jsonTT.append(
      "{\"name\":\"estado_activacion\",\"type\":\"string\",\"value\":\"23\"},");
  jsonTT.append(
      "{\"name\":\"hay_cargador\",\"type\":\"string\",\"value\":\"34\"},");
  jsonTT.append(
      "{\"name\":\"estado_carga\",\"type\":\"string\",\"value\":\"45\"},");
  jsonTT.append(
      "{\"name\":\"modo_carga\",\"type\":\"string\",\"value\":\"56\"},");
  jsonTT.append(
      "{\"name\":\"tiempo_desco_stack\",\"type\":\"string\",\"value\":\"67\"}]"
      ",");
  jsonTT.append(
      "\"metadatas\":[{\"name\":\"sleepcondition\",\"type\":\"string\","
      "\"value\":\"wakeUP\"},");
  jsonTT.append(
      "{\"name\":\"sleeptime\",\"type\":\"string\",\"value\":\"50\"}]}");

  iota::tt::TTCBPublisher* ttPublisher = new iota::tt::TTCBPublisher();
  ::iota::ContextElement cElement("dev01", "thing", "false");

  std::vector<std::string> vJsonTT;

  vJsonTT.push_back(jsonTT);

  actual.assign(ttPublisher->publishContextBroker(cElement, vJsonTT, pb_tree,
                                                  timeInstant));
  boost::trim(actual);
  boost::erase_all(actual, "\n");
  boost::erase_all(actual, "\r");

  CPPUNIT_ASSERT_EQUAL(expected, actual);
  CC_Logger::getSingleton()->logDebug(
      "testTTCBPublisherP1BOnBug_IDAS20202: DONE");
  delete ttPublisher;
}

void TTTest::testTTCBPublisherGMOnBug_IDAS20202() {
  unsigned int port = iota::Process::get_process().get_http_port();
  iota::esp::TTService* ttService =
      (iota::esp::TTService*)iota::Process::get_process().get_service(
          "/TestTT/tt");
  TestSetup test_setup(get_service_name(__FUNCTION__), "/TestTT/tt");

  CC_Logger::getSingleton()->logDebug(
      "testTTCBPublisherGMOnBug_IDAS20202: starting");

  std::string entity("thing");

  std::string expected = "";

  boost::property_tree::ptree pb_tree;
  ttService->get_service_by_apiKey(pb_tree, get_service_name(__FUNCTION__));

  iota::RiotISO8601 timeInstant;  // I tried
  std::string strTime = timeInstant.toUTC().toString();

  expected.append("{\"updateAction\":\"APPEND\",\"contextElements\":");
  expected.append("[{\"id\":\"dev01\",\"type\":\"");
  expected.append(entity);
  expected.append("\",\"isPattern\":\"false\",\"attributes\":");
  expected.append("[{\"name\":\"test\",\"type\":\"string\",\"value\":\"22\",");
  expected.append(
      "\"metadatas\":[{\"name\":\"sleepcondition\",\"type\":\"string\","
      "\"value\":\"wakeUP\"},");
  expected.append(
      "{\"name\":\"sleeptime\",\"type\":\"string\",\"value\":\"50\"},");
  expected.append(
      "{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"");
  expected.append(strTime);
  expected.append("\"}]},");
  expected.append(
      "{\"name\":\"TimeInstant\",\"type\":\"ISO8601\",\"value\":\"");
  expected.append(strTime);
  expected.append("\"}]}]}");

  std::string actual = "";

  std::string jsonTT = "";

  jsonTT.assign("{\"name\":\"test\",\"type\":\"string\",\"value\":\"22\",");
  jsonTT.append(
      "\"metadatas\":[{\"name\":\"sleepcondition\",\"type\":\"string\","
      "\"value\":\"wakeUP\"},");
  jsonTT.append(
      "{\"name\":\"sleeptime\",\"type\":\"string\",\"value\":\"50\"}]}");

  iota::tt::TTCBPublisher* ttPublisher = new iota::tt::TTCBPublisher();
  ::iota::ContextElement cElement("dev01", "thing", "false");

  std::vector<std::string> vJsonTT;

  vJsonTT.push_back(jsonTT);

  actual.assign(ttPublisher->publishContextBroker(cElement, vJsonTT, pb_tree,
                                                  timeInstant));
  boost::trim(actual);
  boost::erase_all(actual, "\n");
  boost::erase_all(actual, "\r");

  CPPUNIT_ASSERT_EQUAL(expected, actual);
  CC_Logger::getSingleton()->logDebug(
      "testTTCBPublisherGMOnBug_IDAS20202: DONE");
  delete ttPublisher;
}

void TTTest::testMissingSubAttributesBug_single_P1_Module_IDAS20245() {
  // boost::shared_ptr<HttpMock> cb_mock_up;
  // cb_mock_up.reset(new HttpMock("/NGSI10/updateContext"));
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");

  iota::esp::TTService* ttService =
      (iota::esp::TTService*)iota::Process::get_process().get_service(
          "/TestTT/tt");
  TestSetup test_setup(get_service_name(__FUNCTION__), "/TestTT/tt");

  test_setup.add_device("8934075379000039321",
                        ttService->get_protocol_data().protocol);

  // Setting responses for http_mocks
  // queryContext should not find any attributes:
  std::string mockResponseError;
  mockResponseError.assign("{\"errorCode\": {\"code\" : \"404\",");
  mockResponseError.append("\"reasonPhrase\" : \"No context element found\"}}");
  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/queryContext", 404,
      mockResponseError);  // First response, no contextElements found

  CC_Logger::getSingleton()->logDebug(
      "testMissingSubAttributesBug_single_P1_Module_IDAS20245: starting");

  std::string query = "cadena=#8934075379000039321,#0,P1,214,07,,633c";

  query.assign(pion::algorithm::url_encode(query));

  pion::http::request_ptr http_request(new pion::http::request("/TestTT/tt"));
  http_request->set_method("POST");
  http_request->set_content(query);
  std::map<std::string, std::string> url_args;
  std::multimap<std::string, std::string> query_parameters;

  pion::http::response http_response;
  std::string response;
  ttService->service(http_request, url_args, query_parameters, http_response,
                     response);

  CC_Logger::getSingleton()->logDebug(
      "testMissingSubAttributesBug_single_P1_Module_IDAS20245: CODE:  %d ",
      http_response.get_status_code());
  CC_Logger::getSingleton()->logDebug(
      "testMissingSubAttributesBug_single_P1_Module_IDAS20245: Response:  %s ",
      response.c_str());
  CPPUNIT_ASSERT(http_response.get_status_code() == 200);
  CPPUNIT_ASSERT_EQUAL(std::string(""), response);
  test_setup.delete_device("8934075379000039321", test_setup.get_service(),
                           test_setup.get_service_path());
}

void TTTest::testMissingSubAttributesBug_multiple_P1_GPS_B_Module_IDAS20245() {
  CC_Logger::getSingleton()->logDebug(
      "testMissingSubAttributesBug_multiple_P1_GPS_B_Module_IDAS20245: "
      "starting");

  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  // Setting responses for http_mocks
  // queryContext should not find any attributes:
  std::string mockResponseError;

  iota::esp::TTService* ttService =
      (iota::esp::TTService*)iota::Process::get_process().get_service(
          "/TestTT/tt");

  TestSetup test_setup(get_service_name(__FUNCTION__), "/TestTT/tt");

  test_setup.add_device("8934075379000039321",
                        ttService->get_protocol_data().protocol);

  mockResponseError.assign("{\"errorCode\": {\"code\" : \"404\",");
  mockResponseError.append("\"reasonPhrase\" : \"No context element found\"}}");

  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/queryContext", 404,
      mockResponseError);  // First response, no contextElements found

  std::string mockResponse;  // This response lacks P1.
  mockResponse.assign("{\"contextResponses\": [");
  mockResponse.append("{ \"contextElement\":{");
  mockResponse.append("\"attributes\": [{");
  mockResponse.append("\"name\":\"temp\",\"type\":\"string\",");
  mockResponse.append("\"value\":\"88\"");  // ATTENTION TO THIS: this has to
                                            // come in the response.
  mockResponse.append("}],");
  mockResponse.append("\"id\":\"8934075379000039321\",");
  mockResponse.append("\"isPattern\":\"false\",\"type\":\"thing\"},");
  mockResponse.append(
      "\"statusCode\":{\"code\":\"200\",\"reasonPhrase\":\"OK\"}}]}");

  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/queryContext", 200,
      mockResponse);  // Second query.

  std::string query =
      "cadena=#8934075379000039321,#0,GM,temp,23,$#0,GPS,11,,33,44,#0,B,12,23,"
      "34,,56,67,#0,P1,214,,33f,633c,#0,K1,0$";

  query.assign(pion::algorithm::url_encode(query));

  pion::http::request_ptr http_request(new pion::http::request("/TestTT/tt"));
  http_request->set_method("POST");
  http_request->set_content(query);
  std::map<std::string, std::string> url_args;
  std::multimap<std::string, std::string> query_parameters;

  pion::http::response http_response;
  std::string response;
  ttService->service(http_request, url_args, query_parameters, http_response,
                     response);

  CC_Logger::getSingleton()->logDebug(
      "testMissingSubAttributesBug_multiple_P1_GPS_B_Module_IDAS20245: CODE:  "
      "%d ",
      http_response.get_status_code());
  CC_Logger::getSingleton()->logDebug(
      "testMissingSubAttributesBug_multiple_P1_GPS_B_Module_IDAS20245: "
      "Response:  %s ",
      response.c_str());

  CPPUNIT_ASSERT_EQUAL(std::string("#0,GM,temp,$,#0,K1,0$"), response);
  CPPUNIT_ASSERT(http_response.get_status_code() == 200);
}

void TTTest::testNoEmptyResponsesOnBug_IDAS20303() {
  iota::esp::TTService* ttService =
      (iota::esp::TTService*)iota::Process::get_process().get_service(
          "/TestTT/tt");
  TestSetup test_setup(get_service_name(__FUNCTION__), "/TestTT/tt");
  test_setup.add_device("8934075379000039321",
                        ttService->get_protocol_data().protocol);

  std::string query;

  std::vector<std::string> v_query;

  v_query.push_back("cadena=#8934075379000039321,#0,GM,temp,,$");
  v_query.push_back("cadena=#8934075379000039321,#0,GM,,20,$");
  v_query.push_back("cadena=#8934075379000039321,#0,GC,temp,,$");
  v_query.push_back("cadena=#8934075379000039321,#0,GC,,20,$");
  v_query.push_back("cadena=#8934075379000039321,#0,P1,11,22,$");
  v_query.push_back("cadena=#8934075379000039321,#0,P1,11,22,33,,$");
  v_query.push_back("cadena=#8934075379000039321,#0,GPS,11,,$");
  v_query.push_back("cadena=#8934075379000039321,#0,B,11,22,33,,,$");
  v_query.push_back("cadena=#8934075379000039321,#0,GM,temp,,$,#0,GM,,,$");

  pion::http::request_ptr http_request(new pion::http::request("/TestTT/tt"));
  http_request->set_method("POST");

  std::map<std::string, std::string> url_args;
  std::multimap<std::string, std::string> query_parameters;

  // for (int i=0 ; i < v_query.size(); i++ ){
  for (std::vector<std::string>::const_iterator it = v_query.begin();
       it != v_query.end(); ++it) {
    query.assign(*it);
    CC_Logger::getSingleton()->logDebug(
        "testNoEmptyResponsesOnBug_IDAS20303: query:  %s ", query.c_str());

    query.assign(pion::algorithm::url_encode(query));
    http_request->set_content(query);
    pion::http::response http_response;

    std::string response;
    ttService->service(http_request, url_args, query_parameters, http_response,
                       response);

    CC_Logger::getSingleton()->logDebug(
        "testNoEmptyResponsesOnBug_IDAS20303: CODE:  %d ",
        http_response.get_status_code());
    CC_Logger::getSingleton()->logDebug(
        "testNoEmptyResponsesOnBug_IDAS20303: Response:  %s ",
        response.c_str());

    CPPUNIT_ASSERT(http_response.get_status_code() == 200);

    CPPUNIT_ASSERT_EQUAL(std::string(""), response);
  }
}

void TTTest::testErrorCodesOnBug_IDAS20303() {
  iota::esp::TTService* ttService =
      (iota::esp::TTService*)iota::Process::get_process().get_service(
          "/TestTT/tt");

  std::string query_404(
      "cadena=#89340753321,#0,GM,temp,23,$,#0,P1,23,23,44,55,$");
  std::string query_400("#8934075379000039321,#0,GM,,20,$");

  TestSetup test_setup(get_service_name(__FUNCTION__), "/TestTT/tt");
  test_setup.add_device("8934075379000039321",
                        ttService->get_protocol_data().protocol);

  pion::http::request_ptr http_request(new pion::http::request("/TestTT/tt"));
  http_request->set_method("POST");

  std::map<std::string, std::string> url_args;
  std::multimap<std::string, std::string> query_parameters;

  CC_Logger::getSingleton()->logDebug(
      "testErrorCodesOnBug_IDAS20303: query:  %s ", query_404.c_str());

  query_404.assign(pion::algorithm::url_encode(query_404));
  http_request->set_content(query_404);
  pion::http::response http_response;

  std::string response;
  ttService->service(http_request, url_args, query_parameters, http_response,
                     response);

  CC_Logger::getSingleton()->logDebug(
      "testErrorCodesOnBug_IDAS20303: CODE:  %d ",
      http_response.get_status_code());
  CC_Logger::getSingleton()->logDebug(
      "testErrorCodesOnBug_IDAS20303: Response:  %s ", response.c_str());

  CPPUNIT_ASSERT(http_response.get_status_code() == 404);

  CC_Logger::getSingleton()->logDebug(
      "testErrorCodesOnBug_IDAS20303: query:  %s ", query_400.c_str());

  query_400.assign(pion::algorithm::url_encode(query_400));
  http_request->set_content(query_400);

  ttService->service(http_request, url_args, query_parameters, http_response,
                     response);

  CC_Logger::getSingleton()->logDebug(
      "testErrorCodesOnBug_IDAS20303: CODE:  %d ",
      http_response.get_status_code());
  CC_Logger::getSingleton()->logDebug(
      "testErrorCodesOnBug_IDAS20303: Response:  %s ", response.c_str());

  CPPUNIT_ASSERT(http_response.get_status_code() == 400);
}

/**
Bug was about getting 500 error when dealing with one incorrect GM but having
others correct, like another GM, or P1
*/
void TTTest::testErrorCodesOnBug_IDAS20308() {
  CC_Logger::getSingleton()->logDebug(
      "testErrorCodesOnBug_IDAS20308: starting");

  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  iota::esp::TTService* ttService =
      (iota::esp::TTService*)iota::Process::get_process().get_service(
          "/TestTT/tt");
  TestSetup test_setup(get_service_name(__FUNCTION__), "/TestTT/tt");
  test_setup.add_device("8934075379000039321",
                        ttService->get_protocol_data().protocol);

  // Setting responses for http_mocks
  // queryContext should not find any attributes:
  std::string mockResponseError;
  mockResponseError.assign("{\"errorCode\": {\"code\" : \"404\",");
  mockResponseError.append("\"reasonPhrase\" : \"No context element found\"}}");
  // esp::TTService ttService;

  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/queryContext", 404,
      mockResponseError);  // First response, no contextElements found

  std::string mockResponse;  // This response lacks P1.
  mockResponse.assign("{\"contextResponses\": [");
  mockResponse.append("{ \"contextElement\":{");
  mockResponse.append("\"attributes\": [{");
  mockResponse.append("\"name\":\"temp\",\"type\":\"string\",");
  mockResponse.append("\"value\":\"88\"");  // ATTENTION TO THIS: this has to
                                            // come in the response.
  mockResponse.append("}],");
  mockResponse.append("\"id\":\"8934075379000039321\",");
  mockResponse.append("\"isPattern\":\"false\",\"type\":\"thing\"},");
  mockResponse.append(
      "\"statusCode\":{\"code\":\"200\",\"reasonPhrase\":\"OK\"}}]}");

  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/queryContext", 200,
      mockResponse);  // Second query.

  std::string query =
      "cadena=#8934075379000039321,#0,GM,,23,$,#0,GM,test,22,$,#0,P1,214,33,"
      "33f,633c,#0,K1,0$";

  query.assign(pion::algorithm::url_encode(query));

  pion::http::request_ptr http_request(new pion::http::request("/TestTT/tt"));
  http_request->set_method("POST");
  http_request->set_content(query);
  std::map<std::string, std::string> url_args;
  std::multimap<std::string, std::string> query_parameters;

  pion::http::response http_response;
  std::string response;
  ttService->service(http_request, url_args, query_parameters, http_response,
                     response);

  CC_Logger::getSingleton()->logDebug(
      "testErrorCodesOnBug_IDAS20308: CODE:  %d ",
      http_response.get_status_code());
  CC_Logger::getSingleton()->logDebug(
      "testErrorCodesOnBug_IDAS20308: Response:  %s ", response.c_str());

  CPPUNIT_ASSERT_EQUAL(std::string("#0,GM,test,$,#0,P1,$,#0,K1,0$"), response);
  CPPUNIT_ASSERT(http_response.get_status_code() == 200);
}

void TTTest::testNoEmpty_And_ValidResponse_OnBug_IDAS20308() {
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  iota::esp::TTService* ttService =
      (iota::esp::TTService*)iota::Process::get_process().get_service(
          "/TestTT/tt");
  TestSetup test_setup(get_service_name(__FUNCTION__), "/TestTT/tt");
  test_setup.add_device("8934075379000039321",
                        ttService->get_protocol_data().protocol);

  std::string mockResponseError;
  mockResponseError.assign("{\"errorCode\": {\"code\" : \"404\",");
  mockResponseError.append("\"reasonPhrase\" : \"No context element found\"}}");
  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/queryContext", 404,
      mockResponseError);  // First response, no contextElements found

  std::string mockResponse;  // This response lacks P1.
  mockResponse.assign("{\"contextResponses\": [");
  mockResponse.append("{ \"contextElement\":{");
  mockResponse.append("\"attributes\": [{");
  mockResponse.append("\"name\":\"temp\",\"type\":\"string\",");
  mockResponse.append("\"value\":\"88\"");
  mockResponse.append("}],");
  mockResponse.append("\"id\":\"8934075379000039321\",");
  mockResponse.append("\"isPattern\":\"false\",\"type\":\"thing\"},");
  mockResponse.append(
      "\"statusCode\":{\"code\":\"200\",\"reasonPhrase\":\"OK\"}}]}");

  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/queryContext", 200,
      mockResponse);  // Second query.

  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/queryContext", 404,
      mockResponseError);
  cb_mock->set_response(
      "/mock/" + test_setup.get_service() + "/NGSI10/queryContext", 200,
      mockResponse);

  std::string query;

  std::vector<std::string> v_query;

  v_query.push_back("cadena=#8934075379000039321,#0,GM,temp,33,$,#0,GM,,22,$");
  v_query.push_back(
      "cadena=#8934075379000039321,#0,GM,test,,$,#0,GM,temp,55,$");

  pion::http::request_ptr http_request(new pion::http::request("/TestTT/tt"));
  http_request->set_method("POST");

  std::map<std::string, std::string> url_args;
  std::multimap<std::string, std::string> query_parameters;

  // for (int i=0 ; i < v_query.size(); i++ ){
  for (std::vector<std::string>::const_iterator it = v_query.begin();
       it != v_query.end(); ++it) {
    query.assign(*it);
    CC_Logger::getSingleton()->logDebug(
        "testNoEmptyResponsesOnBug_IDAS20303: query:  %s ", query.c_str());

    query.assign(pion::algorithm::url_encode(query));
    http_request->set_content(query);
    pion::http::response http_response;

    std::string response;
    ttService->service(http_request, url_args, query_parameters, http_response,
                       response);

    CC_Logger::getSingleton()->logDebug(
        "testNoEmptyResponsesOnBug_IDAS20303: CODE:  %d ",
        http_response.get_status_code());
    CC_Logger::getSingleton()->logDebug(
        "testNoEmptyResponsesOnBug_IDAS20303: Response:  %s ",
        response.c_str());

    CPPUNIT_ASSERT(http_response.get_status_code() == 200);

    CPPUNIT_ASSERT_EQUAL(std::string("#0,GM,temp,$"), response);
  }
}

void TTTest::testError_when_empty_response_from_CB() {
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* cb_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  iota::esp::TTService* ttService =
      (iota::esp::TTService*)iota::Process::get_process().get_service(
          "/TestTT/tt");
  TestSetup test_setup(get_service_name(__FUNCTION__), "/TestTT/tt");

  test_setup.add_device("8934075379000039321",
                        ttService->get_protocol_data().protocol);
  std::string query;

  query.assign("cadena=#8934075379000039321,#0,GM,temp,33,$,#0,GM,,22,$");

  pion::http::request_ptr http_request(new pion::http::request("/TestTT/tt"));
  http_request->set_method("POST");

  std::map<std::string, std::string> url_args;
  std::multimap<std::string, std::string> query_parameters;

  CC_Logger::getSingleton()->logDebug(
      "testError_when_empty_response_from_CB: query:  %s ", query.c_str());

  query.assign(pion::algorithm::url_encode(query));
  http_request->set_content(query);
  pion::http::response http_response;

  std::string response;
  ttService->service(http_request, url_args, query_parameters, http_response,
                     response);

  CC_Logger::getSingleton()->logDebug(
      "testError_when_empty_response_from_CB: CODE:  %d ",
      http_response.get_status_code());
  CC_Logger::getSingleton()->logDebug(
      "testError_when_empty_response_from_CB: Response:  %s ",
      response.c_str());

  CPPUNIT_ASSERT(http_response.get_status_code() == 500);
}
// TODO
/*
void TTTest::start_cbmock(boost::shared_ptr<HttpMock>& cb_mock) {
  cb_mock->init();
  std::string mock_port = boost::lexical_cast<std::string>(cb_mock->get_port());
  std::cout << "mock with port:" << mock_port << std::endl;

  iota::Configurator::release();
  iota::Configurator* my_instance = iota::Configurator::instance();

  std::stringstream ss;
  ss << "{ \"ngsi_url\": {"
     << " \"updateContext\": \"/NGSI10/updateContext\","
     << " \"registerContext\": \"/NGSI9/registerContext\","
     << " \"queryContext\": \"/NGSI10/queryContext\""
     << "},"
     << "\"timeout\": 1,"
     << "\"dir_log\": \"/tmp/\","
     << "\"timezones\": \"/etc/iot/date_time_zonespec.csv\","
     << "\"schema_path\": \"../../schema\","
     << "\"storage\": {"
     << "\"host\": \"127.0.0.1\","
     << "\"type\": \"file\","
     << "\"port\": \"27017\","
     << "\"dbname\": \"iot\","
     << "\"file\": \"../../tests/iotagent/devices.json\""
     << "},"
     << "\"resources\":[{\"resource\": \"/iot/tt\","
     << " \"options\": {\"FileName\": \"IoTAgent-tt\" },"
     << " \"services\":[ {"
     << "\"apikey\": \"\","
     << "\"service\": \"serviceTT\","
     << "\"service_path\": \"/subservice\","
     << "\"token\": \"token2\","
     << "\"cbroker\": \"http://127.0.0.1:" << mock_port << "\", "
     << "\"entity_type\": \"thing\""
     << "} ] } ] }";
  //my_instance->update_conf(ss);
  std::string err = my_instance->read_file(ss);
  std::cout << "GET CONF " << my_instance->getAll() << std::endl;
  if (!err.empty()) {
    std::cout << "start_cbmock:" << err << std::endl;
    std::cout << "start_cbmock_data:" << ss.str() << std::endl;
  }
}

void TTTest::start_cbmock(boost::shared_ptr<HttpMock>& cb_mock_up,
                          boost::shared_ptr<HttpMock>& cb_mock_query) {
  cb_mock_up->init();
  std::string mock_port_up = boost::lexical_cast<std::string>
                             (cb_mock_up->get_port());
  cb_mock_query->init();

  std::string mock_port_query = boost::lexical_cast<std::string>
                                (cb_mock_query->get_port());
  std::cout << "mock with port Query:" << mock_port_query << std::endl;
  std::cout << "mock with port Updatecontext:" << mock_port_up << std::endl;

  iota::Configurator::release();
  iota::Configurator* my_instance = iota::Configurator::instance();

  std::stringstream ss;
  ss << "{ \"ngsi_url\": {"
     << " \"updateContext\": \":"<< mock_port_up << "/NGSI10/updateContext\","
     << " \"registerContext\": \"/NGSI9/registerContext\","
     << " \"queryContext\": \":"<< mock_port_query<< "/NGSI10/queryContext\""
     << "},"
     << "\"timeout\": 1,"
     << "\"dir_log\": \"/tmp/\","
     << "\"timezones\": \"/etc/iot/date_time_zonespec.csv\","
     << "\"schema_path\": \"../../schema\","
     << "\"storage\": {"
     << "\"host\": \"127.0.0.1\","
     << "\"type\": \"file\","
     << "\"port\": \"27017\","
     << "\"dbname\": \"iot\","
     << "\"file\": \"../../tests/iotagent/devices.json\""
     << "},"
     << "\"resources\":[{\"resource\": \"/iot/tt\","
     << " \"options\": {\"FileName\": \"IoTAgent-tt\" },"
     << " \"services\":[ {"
     << "\"apikey\": \"\","
     << "\"service\": \"serviceTT\","
     << "\"service_path\": \"/subservice\","
     << "\"token\": \"token2\","
     << "\"cbroker\": \"http://127.0.0.1\", "
     << "\"entity_type\": \"thing\""
     << "} ] } ] }";
  //my_instance->update_conf(ss);
  std::string err = my_instance->read_file(ss);
  std::cout << "GET CONF " << my_instance->getAll() << std::endl;
  if (!err.empty()) {
    std::cout << "start_cbmock:" << err << std::endl;
    std::cout << "start_cbmock_data:" << ss.str() << std::endl;
  }
}
*/
