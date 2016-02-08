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
#ifndef SRC_TESTS_IOTAGENT_ADMINTEST_H_
#define SRC_TESTS_IOTAGENT_ADMINTEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include "services/admin_service.h"

#include "../mocks/http_mock.h"
#include "../mocks/util_functions.h"

class TestPlugin : public iota::RestHandle {
 public:
  TestPlugin(){};
  virtual ~TestPlugin(){};
  virtual iota::ProtocolData get_protocol_data() {
    iota::ProtocolData protocol_data;
    protocol_data.description = "Ultra Light Propietary Protocol";
    protocol_data.protocol = "PDI-IoTA-UltraLight";
    return protocol_data;
  };
};

class AdminTest : public CPPUNIT_NS::TestFixture {
  CPPUNIT_TEST_SUITE(AdminTest);
  CPPUNIT_TEST(testGetConf);
  CPPUNIT_TEST(testGetAgents);
  CPPUNIT_TEST(testTimezones);
  CPPUNIT_TEST(testPostConf);
  CPPUNIT_TEST(testValidationSchema);
  CPPUNIT_TEST(testPostBadContentType);
  CPPUNIT_TEST(testPostDevice);
  CPPUNIT_TEST(testPostService);
  CPPUNIT_TEST(testPostService2);
  CPPUNIT_TEST(testNoRestApiService);

  CPPUNIT_TEST(testBADPostDevice);

  CPPUNIT_TEST(testPtreeWrite);
  CPPUNIT_TEST(testAttributeService);

  CPPUNIT_TEST(testAbout);
  CPPUNIT_TEST(testcheck_device_protocol);

  CPPUNIT_TEST(testBADConfigurator);
  CPPUNIT_TEST(testConfigurator);
  CPPUNIT_TEST(testNgsiOperation);
  CPPUNIT_TEST(testHttpMessages);
  CPPUNIT_TEST(testConversionMap);
  CPPUNIT_TEST(testAuthInfo);
  CPPUNIT_TEST(testConfiguratorMongo);
  CPPUNIT_TEST(testForbiddenCharacters);
  // This test is not needed. White space is a forbidden character.
  //CPPUNIT_TEST(testSpaceURI);
  CPPUNIT_TEST(testPostDeviceSameEntity);
  CPPUNIT_TEST(testRetriesRegisterManager);
  CPPUNIT_TEST(testPostServiceWrongResource);

  CPPUNIT_TEST_SUITE_END();

 public:
  AdminTest();
  virtual ~AdminTest();
  void setUp();
  void tearDown();

  static const std::string HOST;
  static const std::string CONTENT_JSON;

  static const std::string URI_DEVICE;
  // POST
  static const std::string POST_DEVICE;
  static const std::string BAD_PROTOCOL_POST_DEVICE;
  static const std::string BAD_POST_DEVICE;
  static const std::string BAD_POST_DEVICE2;
  static const std::string BAD_POST_DEVICE3;
  static const std::string BAD_POST_DEVICE_NO_DEVICE_ID;
  static const int POST_RESPONSE_CODE;
  static const std::string POST_RESPONSE;
  static const std::string POST_DEVICE_NO_DEVICE_ID;
  static const int POST_RESPONSE_CODE_NO_DEVICE_ID;
  static const std::string POST_RESPONSE_NO_DEVICE_ID;

  // GET ALL empty
  static const std::string GET_EMPTY_RESPONSE_DEVICES;
  static const int GET_RESPONSE_CODE;
  static const int GET_RESPONSE_CODE_NOT_FOUND;

  // PUT
  static const std::string PUT_DEVICE;
  static const int PUT_RESPONSE_CODE;
  static const std::string PUT_RESPONSE;

  // DELETE
  static const int DELETE_RESPONSE_CODE;
  static const std::string DELETE_RESPONSE;

  // GET elto
  static const std::string GET_DEVICE_RESPONSE;

  static const std::string URI_SERVICE;
  static const std::string URI_SERVICE2;
  // POST
  static const std::string POST_SERVICE;
  static const std::string POST_SERVICE2;
  static const std::string PUT_SERVICE_WITH_ATTRIBUTES;
  static const std::string POST_SERVICE_WITH_ATTRIBUTES;
  static const std::string POST_SERVICE_TO_DELETE_FIELDS;
  static const std::string PUT_SERVICE_TO_DELETE_FIELDS;

  static const std::string BAD_POST_SERVICE1;
  static const std::string BAD_POST_SERVICE2;

  static const std::string POST_SERVICE_WRONG_RESOURCE;
  static const std::string POST_SERVICE_INVALID_RESOURCE;
  static const std::string POST_SERVICE_INVALID_APIKEY;

  // GET ALL empty
  static const std::string GET_EMPTY_RESPONSE_SERVICES;
  // PUT
  static const std::string PUT_SERVICE;
  // GET elto
  static const std::string GET_SERVICE_RESPONSE;

  // PROTOCOL
  static const std::string URI_PROTOCOLS;
  static const std::string POST_PROTOCOLS1;
  static const std::string POST_PROTOCOLS2;
  static const std::string POST_PROTOCOLS3;
  static const std::string POST_PROTOCOLS4;
  static const std::string GET_PROTOCOLS_RESPONSE;

  // SERVICE_MANAGEMENT
  static const std::string URI_SERVICES_MANAGEMET;
  static const std::string POST_SERVICE_MANAGEMENT1;
  static const std::string POST_SERVICE_MANAGEMENT2;
  static const std::string GET_SERVICE_MANAGEMENT_RESPONSE;

  // DEVICE_MANAGEMENT
  static const std::string URI_DEVICES_MANAGEMEMT;
  static const std::string POST_DEVICE_MANAGEMENT1;
  static const std::string GET_DEVICE_MANAGEMENT_RESPONSE;

  int http_test(const std::string& uri, const std::string& method,
                const std::string& service, const std::string& service_path,
                const std::string& content_type, const std::string& body,
                const std::map<std::string, std::string>& headers,
                const std::string& query_string, std::string& response);

 private:
  void testValidationSchema();
  std::string validationSchema(const std::string& json_str,
                               const std::string& json_schema);

  void testGetConf();
  void testPostConf();
  void testGetAgents();
  void testReload();
  void testTimezones();
  void testCsvProvision();
  void testNoRestApiService();

  void testPostBadContentType();

  /**
     * @name    testPostDevice
     * @brief   information from one command, recieved in updateConext
     *
     *  @class iota::DeviceApi
     *  @test  testPostDevice
     *  explicacion del text
     */
  void testPostDevice();
  void testPostService();
  void testPostService2();
  void testPtreeWrite();

  void testBADPostDevice();
  void testAttributeService();

  void testAbout();
  void testcheck_device_protocol();

  void testBADConfigurator();
  void testConfigurator();
  void testNgsiOperation();
  void testHttpMessages();
  void testConversionMap();
  void testAuthInfo();
  void testConfiguratorMongo();
  void testForbiddenCharacters();

  void testSpaceURI();

  void testRetriesRegisterManager();
  void testPostDeviceSameEntity();

  void testPostServiceWrongResource();

  pion::http::plugin_server_ptr wserver;
  pion::one_to_one_scheduler scheduler;
  TestPlugin plugin;
};

#endif
