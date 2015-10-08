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
#ifndef MQTTMESSAGESTESTS_H
#define MQTTMESSAGESTESTS_H

#include <cppunit/extensions/HelperMacros.h>
#include "MockPluginInputMqtt.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

/**
This class will test what ESPLib will do with some Specific MQTT messages.
That is to say, when we receive messages with certain topic, ESP plugin will
have to process them and do something consequently.
**/
class MqttMessagesTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(MqttMessagesTests);

  CPPUNIT_TEST(testBasicMqttMeasure);
  CPPUNIT_TEST(testMultiMeasure);
  CPPUNIT_TEST(testRequestingCommands);
  CPPUNIT_TEST(testSendingResponseToCommand);
  CPPUNIT_TEST(testUL20Postprocessor);
  CPPUNIT_TEST(testUL20Error);
  CPPUNIT_TEST(testUL20andMultiMeasures);
  CPPUNIT_TEST(testUL20bypassConfiguredButSingleMeasure);
  CPPUNIT_TEST(testUL20MultiMeasuresForSingleMeasureConfiguration);
  CPPUNIT_TEST(testUL20SingleWithDifferentConditions);
  CPPUNIT_TEST(testUL20MultiMeasureWithDifferentConditions);
  CPPUNIT_TEST(testSensorMLWithUL20PostProcessor);

  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

 protected:
  void testBasicMqttMeasure();

  void testMultiMeasure();

  void testSendingResponseToCommand();
  void testRequestingCommands();

  void testUL20Postprocessor();
  void testUL20Error();
  void testUL20andMultiMeasures();
  void testUL20bypassConfiguredButSingleMeasure();
  void testUL20MultiMeasuresForSingleMeasureConfiguration();
  void testUL20SingleWithDifferentConditions();
  void testUL20MultiMeasureWithDifferentConditions();  // <- modify this one

  void testSendToSBC();

  void testSensorMLWithUL20PostProcessor();

  void testUL20NewTopicForMulti();

 private:
  MockPluginInputMqtt* pluginInput;

  int stubReadClient(int id, char* buffer, int len);

  std::string loadSMLFromFile(const char* filename);

  std::string mqttTestMessage;  // This string represents the message that is
                                // going to be "received" by the mock.
};

#endif  // MQTTMESSAGESTESTS_H
