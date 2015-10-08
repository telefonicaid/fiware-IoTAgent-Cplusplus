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
#ifndef SRC_TESTS_IOTAGENT_TCPTEST_H_
#define SRC_TESTS_IOTAGENT_TCPTEST_H_
#include "rest/tcp_service.h"
#include <cppunit/extensions/HelperMacros.h>
class TestService {
 public:
  TestService(){};
  virtual ~TestService(){};
  void handle_data(pion::tcp::connection_ptr& conn,
                   const std::vector<unsigned char>& r,
                   std::vector<unsigned char>& b_read,
                   const boost::system::error_code& e) {
    if (b_read.size() != 0) {
      std::cout << "HANDLE DATA with data" << std::endl;
      std::string res("GOODBYE");
      std::vector<unsigned char> v(res.begin(), res.end());
      s_->send(conn, v);
      std::cout << "Closing" << std::endl;
      s_->close_connection(conn);

    } else {
      std::cout << "HANDLE DATA with no data" << std::endl;
      std::string res("GOODBYE");
      std::vector<unsigned char> v(res.begin(), res.end());
      s_->send(conn, v);
      std::cout << "Closing" << std::endl;
      s_->close_connection(conn);
      // s_->read(conn);
    }
  }
  void add_tcp_service(boost::shared_ptr<iota::TcpService> s) { s_ = s; };

 protected:
 private:
  boost::shared_ptr<iota::TcpService> s_;
};
class TcpTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TcpTest);
  CPPUNIT_TEST(testConnection);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp(){};
  void tearDown(){};

 protected:
  void testConnection();
};
#endif
