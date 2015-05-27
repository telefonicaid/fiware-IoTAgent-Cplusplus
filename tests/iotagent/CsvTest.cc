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
#include "CsvTest.h"
#include "util/csv_reader.h"
#include "services/admin_service.h"

CPPUNIT_TEST_SUITE_REGISTRATION(CsvTest);
namespace iota {
std::string logger("main");
std::string URL_BASE("/iot");
}
iota::AdminService* AdminService_ptr;


void CsvTest::testCsv() {
  std::string csv_ok("item1,item2, item3\nvalue1, value2, value3");
  std::vector< std::map<std::string, std::string> > data;
  csv_reader(csv_ok, data);
  CPPUNIT_ASSERT(data.size() == 1);
  CPPUNIT_ASSERT(data[0]["item1"].compare("value1") == 0);
  data.clear();
  std::string csv_ok_empty_value("item1,item2, item3\nvalue1,, value3");
  csv_reader(csv_ok_empty_value, data);
  CPPUNIT_ASSERT(data.size() == 1);
  CPPUNIT_ASSERT(data[0]["item1"].compare("value1") == 0);
  CPPUNIT_ASSERT(data[0]["item2"].empty());

  std::string csv_nok("item1,item2, item3\nvalue1,value3");
  data.clear();
  csv_reader(csv_nok, data);
  CPPUNIT_ASSERT(data.size() == 0);
}



