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
#include <iostream>
#include <cppunit/TestResult.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TextTestProgressListener.h>
#include "RestFunctionTest.h"
#include "services/admin_service.h"

namespace iota {
std::string URL_BASE = "/iot";
std::string logger("main");
}
iota::AdminService* AdminService_ptr;
int main(int argc, char* argv[]) {
  //CppUnit::TestResult controller;
  //CppUnit::TestResultCollector result;
  //controller.addListener( &result );
  //CppUnit::TextTestProgressListener progress;
  //controller.addListener( &progress );
  CppUnit::TextUi::TestRunner runner;
  runner.addTest(RestFunctionTest::suite());
  runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(),
                      std::cout));
  bool s = runner.run();
  return s ? 0 : 1;
}
