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
#ifndef SRC_TESTS_IOTAGENT_JSONTEST_H_
#define SRC_TESTS_IOTAGENT_JSONTEST_H_

#include <cppunit/extensions/HelperMacros.h>

class JsonTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(JsonTest);

  CPPUNIT_TEST(testConversionUpdateContext);
  CPPUNIT_TEST(testContextElement);
  CPPUNIT_TEST(testContext);
  CPPUNIT_TEST(testResponse);
  CPPUNIT_TEST(testQueryContext);
  CPPUNIT_TEST(testRegisterContext);
  CPPUNIT_TEST(testAttributeRegister);
  CPPUNIT_TEST(testEntity);
  CPPUNIT_TEST(testSubscribeContext);
  CPPUNIT_TEST(testSubscribeResponse);
  CPPUNIT_TEST(testErrors);
  CPPUNIT_TEST(testAttrCompound);
  CPPUNIT_TEST(testAttrCompoundAsObject);
  CPPUNIT_TEST(testConversion);
  CPPUNIT_TEST(testAttributeJSONCommand);
  CPPUNIT_TEST(testUpdateContextIdentifiersTooLong);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp(){};
  void tearDown(){};

 protected:
  void testConversionUpdateContext();
  void testContextElement();
  void testContext();
  void testResponse();
  void testQueryContext();
  void testRegisterContext();
  void testAttributeRegister();
  void testEntity();
  void testSubscribeContext();
  void testSubscribeResponse();
  void testErrors();
  void testAttrCompound();
  void testAttrCompoundAsObject();
  void testConversion();
  void testAttributeJSONCommand();
  void testUpdateContextIdentifiersTooLong();
};
#endif
