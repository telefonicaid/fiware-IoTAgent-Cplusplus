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
#ifndef SRC_TESTS_IOTAGENT_MODBUSTEST_H_
#define SRC_TESTS_IOTAGENT_MODBUSTEST_H_

#include <cppunit/extensions/HelperMacros.h>

class ModbusTest : public CPPUNIT_NS::TestFixture {
  CPPUNIT_TEST_SUITE(ModbusTest);
  CPPUNIT_TEST(testBuildFrame);
  CPPUNIT_TEST(testDecodeFrame);
  CPPUNIT_TEST(testProcessor);
  CPPUNIT_TEST(testProcessorFile);
  CPPUNIT_TEST(testCrc);
  CPPUNIT_TEST(testProcessorCommandsFile);
  CPPUNIT_TEST(testAllCommandsConfigFile);
  CPPUNIT_TEST(testOperationsWithFloat);
  CPPUNIT_TEST(testCheckFactor);
  CPPUNIT_TEST(testConversionDecimals);
  CPPUNIT_TEST(testCommandsToOperations);

  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

 protected:
  void testBuildFrame();
  void testDecodeFrame();
  void testProcessor();
  void testProcessorFile();
  void testCrc();

  void testProcessorCommandsFile();

  void testAllCommandsConfigFile();

  void testOperationsWithFloat();

  void testCheckFactor();

  void testConversionDecimals();

  void testCommandsToOperations();
};

#endif
