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
#include <cppunit/XmlOutputter.h>
#include "adminManagerTest.h"
#include "../mocks/http_mock.h"

int main(int argc, char* argv[]) {
  // Logger
  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;

  // Http server
  iota::Process& process = iota::Process::initialize("/TestManager", 12);
  iota::Configurator* conf =
      iota::Configurator::initialize("../../tests/iotagent/config_mongo.json");
  pion::http::plugin_server_ptr http_server =
      process.add_http_server("", "127.0.0.1:7070");

  // Manager is admin service
  iota::AdminManagerService* admMgm = new iota::AdminManagerService();
  admMgm->set_timeout(20);
  process.set_admin_service(admMgm);

  // Admin service for agent
  iota::AdminService* adm = new iota::AdminService();
  http_server->add_service("/iotagent", adm);

  // Plugin
  iota::UL20Service* ul20_service = new iota::UL20Service();
  adm->add_service("/iot/d", ul20_service);

  // Mock
  MockService* mock = new MockService();
  // Add to http server
  process.add_service("/mock", mock);
  // Add to adm (no function start in mock)
  admMgm->add_service("/mock", mock);
  process.start();

  testing::GTEST_FLAG(throw_on_failure) = true;
  testing::InitGoogleMock(&argc, argv);

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(AdminManagerTest::suite());
  runner.setOutputter(
      new CppUnit::CompilerOutputter(&runner.result(), std::cerr));
  bool s = runner.run();
  std::cout << "Shutdown server" << std::endl;
  process.shutdown();
  return s ? 0 : 1;
}
