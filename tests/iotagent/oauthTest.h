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
#ifndef SRC_TESTS_IOTAGENT_OAUTHTEST_H_
#define SRC_TESTS_IOTAGENT_OAUTHTEST_H_

#include <cppunit/extensions/HelperMacros.h>

class OAuthTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(OAuthTest);

  CPPUNIT_TEST(testIdentity);
  CPPUNIT_TEST(testGetTokenTrust);
  CPPUNIT_TEST(testValidateToken);
  CPPUNIT_TEST(testGetSubservice);
  CPPUNIT_TEST(testGetUserRoles);
  CPPUNIT_TEST(testAccessControl);
  CPPUNIT_TEST(testValidateAsync);
  CPPUNIT_TEST(testActions);
  CPPUNIT_TEST(test502);

  CPPUNIT_TEST_SUITE_END();

 public:
  OAuthTest();
  void setUp();
  void tearDown();

 protected:
  void testIdentity();
  void testGetTokenTrust();
  void testValidateToken();
  void testGetSubservice();
  void testGetUserRoles();
  void testAccessControl();
  void testValidateAsync();
  void testActions();
  void test502();
};
#endif
