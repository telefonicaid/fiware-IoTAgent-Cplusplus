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
#include "h/TimeZoneTests.h"
#include <time.h>

CPPUNIT_TEST_SUITE_REGISTRATION(TimeZoneTests);

void TimeZoneTests::setUp() {
  mapSML = new ESP_SML_Map();
  xmlElement = new
  TiXmlElement("<samplingtime mode=\"auto\"  frame=\"urn:x-ogc:def:trs:IDAS:1.0:ISO8601\" />");
  smlTime = new ESP_SML_SamplingTime(xmlElement,mapSML);
}

void TimeZoneTests::tearDown() {
  delete mapSML;
  delete xmlElement;
  delete smlTime;
}

void TimeZoneTests::testTimeStampInUTC() {
  mapSML->addTimestampSML(smlTime);
  mapSML->computeTimestamp();
  time_t localSecs;

  time(&localSecs);
  struct tm* tm_local = localtime(&localSecs);
  int offset = tm_local->tm_gmtoff /
               3600; //offset is in seconds from GMT to localtime.
  int localHour = tm_local->tm_hour;

  std::string strInput = std::string(smlTime->getText()->Value());
  std::string strHour = "00";
  size_t iIndex = strInput.rfind("T");
  //strValue has to have something like: %04d-%02d-%02dT%02d:%02d:%02dZ so between T and after two bytes is the hour.
  if (iIndex != std::string::npos && strInput.length() >= 2) {
    strHour = strInput.substr(iIndex+1, 2);
  }
  int testHour = atoi(strHour.c_str());

  int expectedUTCHour = localHour - offset; //offset can be negative, of course.


  CPPUNIT_ASSERT(testHour == expectedUTCHour);
}
