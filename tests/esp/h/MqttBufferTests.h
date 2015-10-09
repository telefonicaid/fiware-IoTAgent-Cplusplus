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
#ifndef MQTTBUFFERTESTS_H
#define MQTTBUFFERTESTS_H

#include "input_mqtt/ESP_MqttBuffer.h"
#include <cppunit/extensions/HelperMacros.h>

class MqttBufferTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(MqttBufferTests);

  CPPUNIT_TEST(testGettingFirstMessage);
  CPPUNIT_TEST(testEmptyMqttBuffer);
  CPPUNIT_TEST(testRetrieveSmallerThanBuffer);
  CPPUNIT_TEST(testRetrieveGreaterThanBuffer);
  CPPUNIT_TEST(testRetrieveZeroBytes);
  // CPPUNIT_TEST (testRotate);

  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

 protected:
  void testGettingFirstMessage();
  void testEmptyMqttBuffer();
  void testRetrieveSmallerThanBuffer();
  void testRetrieveGreaterThanBuffer();
  void testRetrieveZeroBytes();

 private:
  ESP_MqttBuffer myBuf;

  bool compareBuffers(const char* src, int srclen, const char* dst, int dstlen);
  void initTestData(ESP_MqttBuffer& myBuf, int numMsgs);

  ESP_MqttMsg* createMqttMsg(const char* topic, const char* payload,
                             int payloadLen);
};

#endif  // MQTTBUFFERTESTS_H
