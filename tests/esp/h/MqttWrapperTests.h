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
#ifndef MQTTWRAPPERTESTS_H
#define MQTTWRAPPERTESTS_H

#include "MockMosquitto.h"
#include "input_mqtt/ESP_Plugin_Input_Mqtt.h"

#include <cppunit/extensions/HelperMacros.h>

using testing::Invoke;
using testing::_;

class MqttWrapperTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(MqttWrapperTests);

  CPPUNIT_TEST(testConnect);
  CPPUNIT_TEST(testReceiveMsg);
  CPPUNIT_TEST(testPublishMsg);
  CPPUNIT_TEST(testConnectAndReconnect);

  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

 protected:
  void testConnect();
  void testReceiveMsg();
  void testPublishMsg();
  void testConnectAndReconnect();

 private:
  // In order to use Invoke I need to provide a method with same signature as
  // mocked called method.
  int stubConnect(const char* host, int port, int keepalive);

  // This will trigger a call to on_message.
  int stubLoopToOnMessage(int, int);

  // This will store the mqtt message into the structure so it's available for
  // later checking.
  int stubPublish(int* mid, const char* topic, int payloadlen,
                  const void* payload, int qos, bool retain);

  int stubToReConnect();

  MockMosquitto* mosquitto;
  ESP_MqttWrapper* mqttWrapper;

  struct mosquitto_message mqttMsg;
};

#endif  // MQTTWRAPPERTESTS_H
