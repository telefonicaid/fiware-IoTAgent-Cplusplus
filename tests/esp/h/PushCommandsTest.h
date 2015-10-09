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
#ifndef PUSHCOMMANDSTEST_H
#define PUSHCOMMANDSTEST_H

#include "MockMosquitto.h"
#include "input_mqtt/ESP_Plugin_Input_Mqtt.h"
#include <cppunit/extensions/HelperMacros.h>

class PushCommandsTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(PushCommandsTest);
  CPPUNIT_TEST(testSendPushCommand);

  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

 protected:
  void testSendPushCommand();

 private:
  // Stubs:
  int stubPublish(int* mid, const char* topic, int payloadlen,
                  const void* payload, int qos, bool retain);

  int stubSubscribeSilent(int* mid, const char* sub, int qos);

  int stubConnectPub(const char* host, int port, int keepalive);
  int stubConnectSub(const char* host, int port, int keepalive);

  MockMosquitto* mosquittoPub;
  MockMosquitto* mosquittoSub;

  struct mosquitto_message mqttMsg;
};

#endif  // PUSHCOMMANDSTEST_H
