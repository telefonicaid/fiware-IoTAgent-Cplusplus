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
#include "cbCommTest.h"

int main(int argc, char* argv[]) {
  // Logger
  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;

  // Http Server and admin service
  iota::Process& process = iota::Process::initialize("/TestCBCommunication", 3);
  iota::Configurator* conf =
      iota::Configurator::initialize("../../tests/iotagent/config_mongo.json");
  pion::http::plugin_server_ptr http_server = process.add_http_server("", "");
  iota::AdminService* adm = new iota::AdminService();
  process.set_admin_service(adm);

  // Mock
  MockService* mock = new MockService();
  process.add_service("/mock", mock);
  adm->add_service("/mock", mock);
  process.start();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(cbCommTest::suite());
  runner.setOutputter(
      new CppUnit::CompilerOutputter(&runner.result(), std::cerr));
  bool s = runner.run();
  std::cout << "FIN" << std::endl;
  process.shutdown();
  return s ? 0 : 1;
}
