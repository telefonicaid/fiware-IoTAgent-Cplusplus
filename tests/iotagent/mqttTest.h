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
#ifndef SRC_TESTS_IOTAGENT_MQTTTEST_H_
#define SRC_TESTS_IOTAGENT_MQTTTEST_H_

#include <cppunit/extensions/HelperMacros.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../mocks/http_mock.h"
#include "services/admin_service.h"
#include "../mocks/http_mock.h"

#include <boost/property_tree/ptree.hpp>
#include "util/iota_logger.h"
#include "mqtt/IotaMqttService.h"
#include "mqtt/IotaMqttServiceImpl.h"
#include "../esp/h/MockMosquitto.h"
#include "mqtt/MqttService.h"
#include "MockIotaMqttService.h"

#include "../esp/h/MockPluginOutput.h"
#include "../esp/h/MockPluginInputMqtt.h"

class MqttTest : public CPPUNIT_NS::TestFixture {
  CPPUNIT_TEST_SUITE(MqttTest);

    CPPUNIT_TEST(testCBPublisher);
    CPPUNIT_TEST(testCBPublisherMissingApikey);
    CPPUNIT_TEST(testCBPublisherMissingIDdevice);
    CPPUNIT_TEST(testReceivedMqtt);
    CPPUNIT_TEST(testBadXMLConfigOutputIoT);
    CPPUNIT_TEST(testLongAliasesNotWorking);
    CPPUNIT_TEST(testBadEntityType);
    CPPUNIT_TEST(testNotInitCBPublisher);
    CPPUNIT_TEST(testMultipleMeasures);

    CPPUNIT_TEST(testExtractingCmdId);

    CPPUNIT_TEST(testPushCommandExecution);
    CPPUNIT_TEST(testPushCommandResponse);
    CPPUNIT_TEST(testPostprocessorJSON_IoTOutput_cmd);

    CPPUNIT_TEST(testLocationContextBroker);

    CPPUNIT_TEST(testCommandsBody_BUG);
    CPPUNIT_TEST(testCommandsWithJSONFormat);

  CPPUNIT_TEST(testSendAllRegistrationsWithCommands);

  CPPUNIT_TEST(testEmptyDeviceEmptyApikey_BUG_DM1069);
  CPPUNIT_TEST(testNotProvisionedApikey);

  CPPUNIT_TEST_SUITE_END();

 public:
  MqttTest();
  virtual ~MqttTest();
  void setUp();
  void tearDown();

 protected:
  void testCBPublisher();
  void testCBPublisherMissingApikey();
  void testCBPublisherMissingIDdevice();
  void testReceivedMqtt();

  void testLongAliasesNotWorking();
  void testBadXMLConfigOutputIoT();
  void testBadEntityType();

  void testNotInitCBPublisher();

  void testMultipleMeasures();

  void testExtractingCmdId();

  void testPushCommandResponse();
  void testPushCommandExecution();

  void testPostprocessorJSON_IoTOutput_cmd();

  void testLocationContextBroker();

  void testCommandsBody_BUG();

  void testCommandsWithJSONFormat();

  void testSendAllRegistrationsWithCommands();

  void testEmptyDeviceEmptyApikey_BUG_DM1069();

  void testNotProvisionedApikey();

 private:
  std::string mqtt_alias;
  std::string mqtt_payload;
  std::string mqtt_apikey;
  std::string mqtt_device;

  std::string input_payload;

  std::string marshalled_Mqtt;

  iota::Configurator* conf;
  iota::esp::ngsi::IotaMqttServiceImpl* cbPublish;
  MockMosquitto* mockMosquitto;
  MockMosquitto* mockMosquittoPub;
  MockIotaMqttService* mockPublisher;
  iota::esp::MqttService* mqttService;

  void defineExpectationsMqttt();

  void defineExpectationsMqttNoIncomingMsg();

  struct mosquitto_message mqttMsg;

  int stubLoopToOnMessage(int, int);
  int stubReadClient(int, char*, int);
  bool stubExecute(CC_AttributesType* attributes,
                   ESP_Postprocessor_Base* postprocessor,
                   std::map<std::string, void*> userData);

  int stubConnect(const char* host, int port, int keepalive);

  // void start_cbmock(boost::shared_ptr<HttpMock>& cb_mock,const std::string&
  // type = "mongodb");

  void defineExpectationsMqttPublisher();

  int stubConnectPub(const char* host, int port, int keepalive);
  int stubPublish(int* mid, const char* topic, int payloadLen,
                  const void* payload, int qos, bool retain);

  int stubPublishPush(int* mid, const char* topic, int payloadLen,
                      const void* payload, int qos, bool retain);

  int stubPublishPayloadFormat(int* mid, const char* topic, int payloadLen,
                               const void* payload, int qos, bool retain);
};

#endif /* MQTTTEST_H */
