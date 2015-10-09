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
#ifndef SRC_TESTS_IOTAGENT_TTBUFFERTESTS_H_
#define SRC_TESTS_IOTAGENT_TTBUFFERTESTS_H_

#include <cppunit/extensions/HelperMacros.h>
#include "ESPLib.h"
#include "input_buffer/ESP_Plugin_Input_Buffer.h"
#include "postprocessor_tt/ESP_Plugin_Postprocessor_TT.h"
#include "thinkingthings/DecodeTTJSON.h"

class TTBufferTests : public CPPUNIT_NS::TestFixture {
  CPPUNIT_TEST_SUITE(TTBufferTests);

  CPPUNIT_TEST(testPostProcessorTTGM);
  CPPUNIT_TEST(testPostProcessorTTGCwithSleepCondAndSleepTime);
  CPPUNIT_TEST(testParsingTTOpenBuffer);
  CPPUNIT_TEST(testPostProcessorTTGMwithSleepCondAndSleepTime);
  CPPUNIT_TEST(testPostProcessorTTGCwithSleepCond);
  CPPUNIT_TEST(testPostProcessorTTGCwithSleepTime);
  CPPUNIT_TEST(testPostProcessorTTP1);
  CPPUNIT_TEST(testPostProcessorTTGPS);
  CPPUNIT_TEST(testPostProcessorTTGPSNoSleepCondOrTime);
  CPPUNIT_TEST(testPostProcessorTTGPSSleepCondOrTimeEmpty);
  CPPUNIT_TEST(testPostProcessorWrongSleepCondTimeOnBug_IDAS20215);
  CPPUNIT_TEST(testPostProcessorReverseWrongSleepCondTimeOnBug_IDAS20215);
  CPPUNIT_TEST(testPostProcessorMissingAttributes);
  CPPUNIT_TEST(testPostProcessorTTB);

  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

 protected:
  void testParsingTTOpenBuffer();
  void testPostProcessorTTGM();
  void testPostProcessorTTGMwithSleepCondAndSleepTime();

  void testPostProcessorTTGCwithSleepCondAndSleepTime();
  void testPostProcessorTTGCwithSleepCond();
  void testPostProcessorTTGCwithSleepTime();

  void testPostProcessorTTP1();
  void testPostProcessorTTGPS();
  void testPostProcessorTTGPSNoSleepCondOrTime();
  void testPostProcessorTTGPSSleepCondOrTimeEmpty();

  void testPostProcessorWrongSleepCondTimeOnBug_IDAS20215();
  void testPostProcessorReverseWrongSleepCondTimeOnBug_IDAS20215();

  void testPostProcessorMissingAttributes();
  void testPostProcessorTTB();

 private:
  TiXmlElement* postp;
  iota::esp::tt::DecodeTTJSON* decodeTT;
  ESP_Postprocessor_TT* postProcessorTT;

  bool compareBuffers(const char* src, int srclen, const char* dst, int dstlen);
};

#endif  // TTBufferTests_H
