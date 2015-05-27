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
#ifndef SRC_TESTS_IOTAGENT_APIKEYTEST_H_
#define SRC_TESTS_IOTAGENT_APIKEYTEST_H_

#include <cppunit/extensions/HelperMacros.h>

class ApiKeyTest: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(ApiKeyTest);
    CPPUNIT_TEST(testBADConfigurator);
    CPPUNIT_TEST(testConfigurator);
    CPPUNIT_TEST(testNgsiOperation);
    CPPUNIT_TEST(testHttpMessages);
    CPPUNIT_TEST(testConversionMap);
    CPPUNIT_TEST(testAuthInfo);
    CPPUNIT_TEST(testConfiguratorMongo);

    CPPUNIT_TEST_SUITE_END();
  public:
    void setUp() {};
    void tearDown() {};
  protected:
    void testBADConfigurator();
    void testConfigurator();
    void testNgsiOperation();
    void testHttpMessages();
    void testConversionMap();
    void testAuthInfo();
    void testConfiguratorMongo();
};
#endif
