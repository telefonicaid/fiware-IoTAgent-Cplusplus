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
#ifndef TIMEZONETESTS_H
#define TIMEZONETESTS_H
#include "../../../src/iotagent/esp/plugins/postprocessor_sensorml/ESP_SML_Classes.h"
#include <cppunit/extensions/HelperMacros.h>
#include <tinyxml.h>

class TimeZoneTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TimeZoneTests);
  CPPUNIT_TEST(testTimeStampInUTC);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

 protected:
  void testTimeStampInUTC();

 private:
  ESP_SML_Map* mapSML;
  ESP_SML_SamplingTime* smlTime;
  TiXmlElement* xmlElement;
};

#endif  // TIMEZONETESTS_H
