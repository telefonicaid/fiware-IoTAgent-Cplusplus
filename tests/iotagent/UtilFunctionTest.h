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
#ifndef SRC_TESTS_IOTAGENT_UTILFUNCTIONTEST_H_
#define SRC_TESTS_IOTAGENT_UTILFUNCTIONTEST_H_

#include <cppunit/extensions/HelperMacros.h>

class UtilFunctionTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(UtilFunctionTest);

  // rest
  CPPUNIT_TEST(testFormatPattern);
  CPPUNIT_TEST(testRegexPattern);
  CPPUNIT_TEST(testStatistic);
  CPPUNIT_TEST(testFilter);
  CPPUNIT_TEST(testUuid);
  CPPUNIT_TEST(testHeaders);

  // cache
  CPPUNIT_TEST(testLRU);
  CPPUNIT_TEST(testMRU);
  CPPUNIT_TEST(testCacheClock);
  CPPUNIT_TEST(testByEntity);
  CPPUNIT_TEST(testCommand);
  CPPUNIT_TEST(testCommandTimeout);

  // csv
  CPPUNIT_TEST(testCsv);

  // url
  CPPUNIT_TEST(testUrl);
  CPPUNIT_TEST(testAlarm);
  CPPUNIT_TEST(testPtree2String);

  // forbidden characters
  CPPUNIT_TEST(testForbiddenCharacters);

  // command line
  CPPUNIT_TEST(testCommandLine);

  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp(){};
  void tearDown(){};

 protected:
  // util
  void testFormatPattern();
  void testRegexPattern();
  void testStatistic();
  void testFilter();
  void testUuid();
  void testHeaders();

  // cache
  void testLRU();
  void testMRU();
  void testCacheClock();
  void testByEntity();
  void testCommand();
  void testCommandTimeout();

  // csv
  void testCsv();

  // url
  void testUrl();
  void testAlarm();
  void testPtree2String();

  // Forbiden characters
  void testForbiddenCharacters();

  // command line
  /**
  Feature: command line parameter -h to show help for parameters
options:

[-m]                manager
[–ipv4]             ipv4
[–ipv6]             ipv6
[-i IP]             ip
[-p PORT]           port
[-u URL_BASE]       url base
[-n IOTAGENT_NAME]  name
[-d PLUGINS_DIR]    directory
[-o OPTION=VALUE]   option
[-v LOG_LEVEL]      verbose
-id                identifier

-h                  help


  Scenario: no parameters in command line

  Scenario: not all necesary parametes

  Scenario: iotagent  -h or iotagent --help
  **/
  void testCommandLine();
};
#endif
