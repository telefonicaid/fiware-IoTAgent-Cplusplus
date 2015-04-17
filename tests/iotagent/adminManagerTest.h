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
#ifndef SRC_TESTS_ADMIN_MANAGERTEST_H_
#define	SRC_TESTS_ADMIN_MANAGERTEST_H_

#include <cppunit/extensions/HelperMacros.h>



#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../mocks/http_mock.h"
#include "services/admin_service.h"
#include "../mocks/http_mock.h"

#include <boost/property_tree/ptree.hpp>
#include <pion/logger.hpp>
#include <pion/logger.hpp>
#include "manager/admin_mgmt_service.h"
#include "services/admin_service.h"

#include "util/service_collection.h"



class AdminManagerTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(AdminManagerTest);

    CPPUNIT_TEST(testDeviceToBeAdded);
    //CPPUNIT_TEST(testGetEndpointsFromDevices);
    //CPPUNIT_TEST(testAddDevicesToEndpoints);
    CPPUNIT_TEST(testGetDevices);
    //CPPUNIT_TEST(testMultiplePostsWithResponse);
    CPPUNIT_TEST_SUITE_END();

public:
    AdminManagerTest();
    virtual ~AdminManagerTest();
    void setUp();
    void tearDown();

protected:

  void testDeviceToBeAdded();
  void testGetEndpointsFromDevices();
  void testAddDevicesToEndpoints();
  void testGetDevices();
  void testMultiplePostsWithResponse();

private:
    void cleanDB();
    iota::AdminService* adm;
    pion::http::plugin_server_ptr wserver;
    pion::one_to_one_scheduler scheduler;



};

#endif	/* _H */

