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
#ifndef SRC_TESTS_IOTAGENT_SAMPLETEST_H_
#define SRC_TESTS_IOTAGENT_SAMPLETEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include "services/admin_service.h"
#include "services/ngsi_service.h"
#include "sample_test/test_service.h"
#include "../mocks/http_mock.h"
#include "baseTest.h"

class SampleTest : public CPPUNIT_NS::TestFixture, public BaseTest {
  CPPUNIT_TEST_SUITE(SampleTest);

  CPPUNIT_TEST(testNormalPOST);
  CPPUNIT_TEST(testCommandNormalPOST);
  CPPUNIT_TEST(testPUSHCommand);
  CPPUNIT_TEST(testPollingCommand);

  // SimplePlugingTest
  CPPUNIT_TEST(testFilter);
  CPPUNIT_TEST(testGetDevice);

  CPPUNIT_TEST(testRegisterIoTA);
  CPPUNIT_TEST(testProvisionProtocolCommands);


  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  SampleTest();
  ~SampleTest();

 private:
  /** function toi fill data to cb_mock, it is not a test */
  /*
  void start_cbmock(boost::shared_ptr<HttpMock>& cb_mock,
                    const std::string& type = "file");
                    */

  void testNormalPOST();

  void testCommandNormalPOST();
  void testPUSHCommand();
  void testPollingCommand();

  void testFilter();
  void testGetDevice();
  void testRegisterIoTA();

  void testProvisionProtocolCommands();
  /*
      pion::http::plugin_server_ptr wserver;
      pion::one_to_one_scheduler scheduler;
      iota::TestService* spserv_auth;
      */
};

#endif /* SAMPLETEST_H */
