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

class AdminTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(AdminTest);

    CPPUNIT_TEST(testGetConf);
    CPPUNIT_TEST(testGetAgents);
    CPPUNIT_TEST(testTimezones);
    //CPPUNIT_TEST(testCsvProvision);
    CPPUNIT_TEST(testPostConf);
    //TODO se comenta para que no este cam,biando el ficherode conf todo el rato CPPUNIT_TEST(testReload);

    CPPUNIT_TEST(testPostBadContentType);

    CPPUNIT_TEST(testPostDevice);
    CPPUNIT_TEST(testPostService);
    CPPUNIT_TEST(testPostService2);
    CPPUNIT_TEST(testNoRestApiService);

    CPPUNIT_TEST(testBADPostDevice);

    CPPUNIT_TEST(testPtreeWrite);

    CPPUNIT_TEST(testAbout);
    CPPUNIT_TEST_SUITE_END();

  public:
    AdminTest();
    virtual ~AdminTest();
    void setUp();
    void tearDown();

    static const std::string HOST;
    static const std::string CONTENT_JSON;

    static const std::string URI_DEVICE;
    //POST
    static const std::string POST_DEVICE;
    static const std::string BAD_POST_DEVICE;
    static const std::string BAD_POST_DEVICE2;
     static const std::string BAD_POST_DEVICE3;
    static const int POST_RESPONSE_CODE;
    static const std::string POST_RESPONSE;
    static const std::string POST_DEVICE_NO_DEVICE_ID;
    static const int POST_RESPONSE_CODE_NO_DEVICE_ID;
    static const std::string POST_RESPONSE_NO_DEVICE_ID;

    //GET ALL empty
    static const std::string GET_EMPTY_RESPONSE_DEVICES;
    static const int GET_RESPONSE_CODE;
    static const int GET_RESPONSE_CODE_NOT_FOUND;

    //PUT
    static const std::string PUT_DEVICE;
    static const int PUT_RESPONSE_CODE;
    static const std::string PUT_RESPONSE;

    //DELETE
    static const int DELETE_RESPONSE_CODE;
    static const std::string DELETE_RESPONSE;

    //GET elto
    static const std::string GET_DEVICE_RESPONSE;

    static const std::string URI_SERVICE;
    static const std::string URI_SERVICE2;
    //POST
    static const std::string POST_SERVICE;
    static const std::string POST_SERVICE2;
    static const std::string PUT_SERVICE_WITH_ATTRIBUTES;
    static const std::string POST_SERVICE_WITH_ATTRIBUTES;
    static const std::string BAD_POST_SERVICE1;
    static const std::string BAD_POST_SERVICE2;

    //GET ALL empty
    static const std::string GET_EMPTY_RESPONSE_SERVICES;
    //PUT
    static const std::string PUT_SERVICE;
    //GET elto
    static const std::string GET_SERVICE_RESPONSE;



    int http_test(const std::string& uri,
                  const std::string& method,
                  const std::string& service,
                  const std::string& service_path,
                  const std::string& content_type,
                  const std::string& body,
                  const std::map<std::string,std::string>& headers,
                  const std::string& query_string,
                  std::string& response);

  private:

    void testGetConf();
    void testPostConf();
    void testGetAgents();
    void testReload();
    void testTimezones();
    void testCsvProvision();
    void testNoRestApiService();

    void  testPostBadContentType();

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

    void testAbout();

    iota::AdminService* adm;
    pion::http::plugin_server_ptr wserver;
    pion::one_to_one_scheduler scheduler;
};

#endif

