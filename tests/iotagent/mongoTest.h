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
#ifndef SRC_TESTS_IOTAGENT_MONGO_TEST_H_
#define SRC_TESTS_IOTAGENT_MONGO_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

class MongoTest : public CPPUNIT_NS::TestFixture {
  CPPUNIT_TEST_SUITE(MongoTest);

  CPPUNIT_TEST(testGenericCollection);
  CPPUNIT_TEST(testCommandCollection);
  CPPUNIT_TEST(testDeviceCollection);

  // to test testReplica  you need a replicaSet in mongo
  // and configure it in tests/iotagent/config_mongo_replica.json
  CPPUNIT_TEST(testReplica);
  CPPUNIT_TEST(testMongoAlone);
  CPPUNIT_TEST(testException);

  CPPUNIT_TEST(testProtocolCollection);
  CPPUNIT_TEST(testServiceMgmtCollection);

  CPPUNIT_TEST(testNoMongo);
  CPPUNIT_TEST(testArray);


  CPPUNIT_TEST_SUITE_END();

 public:
  void tearDown();
  void setUp();

 private:

  std::string testjoinCommands(const std::string &obj1,
                               const std::string &obj2);

  void testGenericCollection();
  void testCommandCollection();
  void testDeviceCollection();
  void testProtocolCollection();
  void testServiceMgmtCollection();

  void testNoMongo();
  void testReplica();
  void testMongoAlone();
  void testException();

  void testArray();

  static void workerFunc();
};

#endif /* UL20TEST_H */
