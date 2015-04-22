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
#include "tcpTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION(TcpTest);
namespace iota {
std::string logger("main");
}


void TcpTest::testConnection() {
  TestService test_service;
  boost::asio::ip::address address = boost::asio::ip::address::from_string("127.0.0.1");
  boost::asio::ip::tcp::endpoint e(address, 9999);
  iota::TcpService tcp_service(e);
  tcp_service.register_handler("TEST", boost::bind(&TestService::handle_data, &test_service, _1, _2, _3));
  tcp_service.start();
  pion::single_service_scheduler sch;
  pion::tcp::connection tcp_conn(sch.get_io_service());
  boost::system::error_code error_code;
  error_code = tcp_conn.connect(boost::asio::ip::address::from_string("127.0.0.1"), 9999);
  std::cout << error_code.message() << std::endl;
  tcp_conn.write(boost::asio::buffer("HELLO"), error_code);
  sleep(5);
  tcp_conn.set_lifecycle(pion::tcp::connection::LIFECYCLE_CLOSE);
  tcp_conn.finish();


}



