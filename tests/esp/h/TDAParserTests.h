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
#ifndef __TDAPARSERTESTS_H__
#define __TDAPARSERTESTS_H__

#include <cppunit/extensions/HelperMacros.h>

class TDAParserTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TDAParserTests);

  CPPUNIT_TEST(testParseBinary);
  CPPUNIT_TEST(testParseASCII);
  CPPUNIT_TEST(testParseLoop);
  CPPUNIT_TEST(testParseConditionClear);
  CPPUNIT_TEST(testParseWrite);
  CPPUNIT_TEST(testConditionDefault);
  CPPUNIT_TEST(testSwitch);

  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

 protected:
  void testParseBinary();
  void testParseASCII();
  void testParseLoop();
  void testParseConditionClear();
  void testParseWrite();

  void testConditionDefault();
  void testSwitch();

 private:
};

#endif  // MQTTBUFFERTESTS_H
