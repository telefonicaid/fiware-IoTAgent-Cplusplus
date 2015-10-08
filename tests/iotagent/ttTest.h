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
#ifndef SRC_TESTS_IOTAGENT_TTTEST_H_
#define SRC_TESTS_IOTAGENT_TTTEST_H_

#include <cppunit/extensions/HelperMacros.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../mocks/http_mock.h"
#include "services/admin_service.h"
#include "../mocks/http_mock.h"

#include <boost/property_tree/ptree.hpp>
#include "util/iota_logger.h"
#include "MockIotaMqttService.h"
#include "TDA.h"
#include "thinkingthings/SearchResponse.h"
#include "util/TTCBPublisher.h"
#include "thinkingthings/TTService.h"
#include "../mocks/util_functions.h"

class TTTest : public CPPUNIT_NS::TestFixture {
  CPPUNIT_TEST_SUITE(TTTest);
  CPPUNIT_TEST(testTTResponses);
  CPPUNIT_TEST(testSearchResponses);
  CPPUNIT_TEST(testTTCBPublisher);
  CPPUNIT_TEST(testTTResponsesP1);
  CPPUNIT_TEST(testTTResponsesP1NoTResponse);
  CPPUNIT_TEST(testTTResponsesB);
  CPPUNIT_TEST(testDecodeTTJSONS);
  CPPUNIT_TEST(testDecodeTTJSONsError);
  CPPUNIT_TEST(testFirstTimeTTAttributes);
  CPPUNIT_TEST(testFirstTimeTTAttributesWithK);
  CPPUNIT_TEST(testParsingResponseOnBug_IDAS20197);
  CPPUNIT_TEST(testSearchResponsesGPS);
  CPPUNIT_TEST(testSearchResponseOnBug_IDAS20201);
  CPPUNIT_TEST(testTTCBPublisherP1BOnBug_IDAS20202);
  CPPUNIT_TEST(testTTCBPublisherGMOnBug_IDAS20202);

  CPPUNIT_TEST(
      testMissingSubAttributesBug_single_P1_Module_IDAS20245);  // awaiting fix
                                                                // for a bug
  CPPUNIT_TEST(
      testMissingSubAttributesBug_multiple_P1_GPS_B_Module_IDAS20245);  // awaiting
                                                                        // fix
                                                                        // for a
                                                                        // bug

  CPPUNIT_TEST(testNoEmptyResponsesOnBug_IDAS20303);            // it works
  CPPUNIT_TEST(testErrorCodesOnBug_IDAS20303);                  // it works
  CPPUNIT_TEST(testErrorCodesOnBug_IDAS20308);                  // it works
  CPPUNIT_TEST(testNoEmpty_And_ValidResponse_OnBug_IDAS20308);  // awaiting fix
  CPPUNIT_TEST(testError_when_empty_response_from_CB);          // awaiting fix.

  CPPUNIT_TEST_SUITE_END();

 public:
  TTTest();
  virtual ~TTTest();
  void setUp();
  void tearDown();

 protected:
  void testTTResponses();
  void testSearchResponses();
  void testTTCBPublisher();
  void testTTResponsesP1();
  void testTTResponsesP1NoTResponse();
  void testTTResponsesB();
  void testDecodeTTJSONS();
  void testDecodeTTJSONsError();
  void testFirstTimeTTAttributes();
  void testFirstTimeTTAttributesWithK();
  void testExistingTTAttributes();
  void testParsingResponseOnBug_IDAS20197();
  void testSearchResponsesGPS();
  void testSearchResponseOnBug_IDAS20201();

  void testTTCBPublisherP1BOnBug_IDAS20202();
  void testTTCBPublisherGMOnBug_IDAS20202();

  void testMissingSubAttributesBug_single_P1_Module_IDAS20245();
  void testMissingSubAttributesBug_multiple_P1_GPS_B_Module_IDAS20245();

  void testNoEmptyResponsesOnBug_IDAS20303();

  void testErrorCodesOnBug_IDAS20303();

  void testErrorCodesOnBug_IDAS20308();

  void testNoEmpty_And_ValidResponse_OnBug_IDAS20308();

  void testError_when_empty_response_from_CB();

 private:
};

#endif /* TTTEST_H */
