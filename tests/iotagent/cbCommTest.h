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
#ifndef SRC_TESTS_IOTAGENT_CBCOMMTEST_H_
#define SRC_TESTS_IOTAGENT_CBCOMMTEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include "services/admin_service.h"
#include "../mocks/http_mock.h"

class cbCommTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(cbCommTest);

  CPPUNIT_TEST(testSend);
  CPPUNIT_TEST(testAsyncSend);
  CPPUNIT_TEST(testSynchSendTimeout);
  CPPUNIT_TEST(testAlarm);
  CPPUNIT_TEST(testAsynchSendTimeout);

  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp(){};
  void tearDown(){};

 protected:
  void testSend();
  void testAsyncSend();
  void testSynchSendTimeout();
  void testAsynchSendTimeout();
  void testAlarm();

 private:
};
#endif
