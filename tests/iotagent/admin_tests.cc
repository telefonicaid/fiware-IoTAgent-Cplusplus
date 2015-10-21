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
#include "rest/process.h"
#include "adminTest.h"
#include "services/admin_service.h"

#include "mongo/client/init.h"

int main(int argc, char* argv[]) {
  // Logger
  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;

  // Url base
  iota::Process& process = iota::Process::initialize("/TestAdmin", 5);
  iota::Configurator* conf =
      iota::Configurator::initialize("../../tests/iotagent/config_mongo.json");

  // Http Server and admin
  pion::http::plugin_server_ptr http_server = process.add_http_server("", "");
  iota::AdminService* adm = new iota::AdminService();
  process.set_admin_service(adm);

  // This urls are only for testing
  std::map<std::string, std::string> filters;
  adm->add_service("/TestAdmin/res", adm);
  adm->add_url(iota::ADMIN_SERVICE_AGENTS, filters,
               REST_HANDLE(&iota::AdminService::agents), adm);
  adm->add_url(iota::ADMIN_SERVICE_AGENTS + "/<agent>", filters,
               REST_HANDLE(&iota::AdminService::agent), adm);
  adm->add_url(iota::ADMIN_SERVICE_AGENTS + "/<agent>/services", filters,
               REST_HANDLE(&iota::AdminService::services), adm);
  adm->add_url(iota::ADMIN_SERVICE_AGENTS + "/<agent>/services" + "/<service>",
               filters, REST_HANDLE(&iota::AdminService::service), adm);

  // Mock
  MockService* mock = new MockService();
  // Add to http server
  http_server->add_service("/mock", mock);
  adm->add_service("/mock", mock);

  TestPlugin* plugin = new TestPlugin();
  plugin->set_resource("/TestAdmin/d");
  adm->add_service("/TestAdmin/d", plugin);
  process.start();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(AdminTest::suite());
  runner.setOutputter(
      new CppUnit::CompilerOutputter(&runner.result(), std::cerr));
  bool s = runner.run();
  process.shutdown();
  return s ? 0 : 1;
}
