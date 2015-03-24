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
#include "urlTest.h"
#include "util/iot_url.h"
#include "util/alarm.h"
#include "services/admin_service.h"
#include <boost/property_tree/ptree.hpp>
#include "util/json_parser.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/detail/json_parser_read.hpp>
#include "util/json_parser_write.hpp"
#include <stdexcept>

CPPUNIT_TEST_SUITE_REGISTRATION(UrlTest);
namespace iota {
std::string logger("main");
std::string URL_BASE("/iot");
}
iota::AdminService* AdminService_ptr;

template <typename T>
struct my_id_translator
{
    typedef T internal_type;
    typedef T external_type;

    boost::optional<T> get_value(const T &v) {
      return  v.substr(1, v.size() - 2) ;
    }

    boost::optional<T> put_value(const T &v) {
      return '"' + v +'"';
    }
};

UrlTest::UrlTest() {

}
void UrlTest::setUp() {

}
void UrlTest::tearDown() {
}

void UrlTest::testUrl() {
  // Url ok
  std::string url_ok("https://0.0.0.0:0/path?p1=v1&p2=v2");
  iota::IoTUrl iot_url(url_ok);
  CPPUNIT_ASSERT_MESSAGE("Checking ip ",
                         iot_url.getHost().compare("0.0.0.0") == 0);
  CPPUNIT_ASSERT_MESSAGE("Checking port ", iot_url.getPort() == 0);
  CPPUNIT_ASSERT_MESSAGE("Checking protocol ",
                         iot_url.getProtocol().compare("https") == 0);
  CPPUNIT_ASSERT_MESSAGE("Checking path ",
                         iot_url.getPath().compare("/path") == 0);
  CPPUNIT_ASSERT_MESSAGE("Checking parameters ",
                         iot_url.getQuery().compare("p1=v1&p2=v2") == 0);
  CPPUNIT_ASSERT_MESSAGE("Checking ssl ", iot_url.getSSL());

  // Url nok
  std::string url_nok("malformed");
  CPPUNIT_ASSERT_THROW_MESSAGE("Malformed url ", new iota::IoTUrl(url_nok),
                               std::runtime_error);
}

void UrlTest::testAlarm() {

  iota::Alarm *palarm = iota::Alarm::instance();
  CPPUNIT_ASSERT_MESSAGE("alarms clean ",palarm->size() == 0);
  iota::Alarm::error(22, "endpoint", "status", "text");

  CPPUNIT_ASSERT_MESSAGE("alarms one ",palarm->size() == 1);

  iota::Alarm::info(22, "endpoint", "status", "text");

  CPPUNIT_ASSERT_MESSAGE("alarms clean ",palarm->size() == 0);

}


void UrlTest::testPtree2String() {

  boost::property_tree::ptree ptin;
  std::string escapess=  "breakline\n\r\tescapes";

  ptin.put("cbroker", "http://127.0.0.1:8888/iotagent/ngsi90");
  ptin.put("breakline", escapess);
  ptin.put("eñes", "õñoñoÿÐ");
  ptin.put("comillas", "\"comillas\"");
  ptin.put("timeout", 10);
  ptin.put("timeoutSTR", "10");
  ptin.put("float", 10.123);
  ptin.put("floatSTR", "10.123");
  double d = 10.123456789;
  ptin.put("double", d);
  ptin.put("doubleSTR", "10.123456789");
  ptin.put("active", false);

  std::stringstream ss;
  iota::property_tree::json_parser::write_json(ss, ptin);
  std::string text = ss.str();
  std::cout  << "json write:" << text << std::endl;

  CPPUNIT_ASSERT_MESSAGE( "cbroker bad" , text.find(
        "http://127.0.0.1:8888/iotagent/ngsi90") != std::string::npos);
  CPPUNIT_ASSERT_MESSAGE("breakline bad", text.find(
          "breakline\\n\\r\\tescapes") != std::string::npos);
  CPPUNIT_ASSERT_MESSAGE("eñes bad", text.find(
        "õñoñoÿÐ") != std::string::npos);
  CPPUNIT_ASSERT_MESSAGE("comillas bad", text.find(
        "\"comillas\"") != std::string::npos);

  boost::property_tree::ptree ptout;
  std::istringstream is (text);
  boost::property_tree::read_json(is, ptout);

  std::string cbroker = ptout.get("cbroker", "");
  std::string breakline = ptout.get("breakline", "");
  std::string comillas = ptout.get("comillas", "");
  std::string enes = ptout.get("eñes", "");

  int timeout = ptout.get("timeout", 0);
  std::string timeoutSTR = ptout.get("timeoutSTR", "");
  float vac = 0.0;
  float f = ptout.get("float", vac);
  float f2 = ptout.get("floatSTR", vac);
  double va =0.0;
  double dd = ptout.get("double", va);
  double dd2 = ptout.get("doubleSTR", va);
  bool active = ptout.get("active", true);

  std::cout  << "read:" <<  cbroker << "|" << breakline
             << "|" << comillas << "|" << enes
             << "|" << timeout << "|" << timeoutSTR
             << "|"<< dd << "|"<< dd2
             << "|"<< f << "|"<< f2 << "|" << active << std::endl;
  CPPUNIT_ASSERT_MESSAGE("cbroker bad ",cbroker.compare("http://127.0.0.1:8888/iotagent/ngsi90") == 0);
  CPPUNIT_ASSERT_MESSAGE("breakline bad ",breakline.compare(escapess) == 0);
  CPPUNIT_ASSERT_MESSAGE("eñes bad ",enes.compare("õñoñoÿÐ") == 0);
  CPPUNIT_ASSERT_MESSAGE("comillas bad ",comillas.compare("\"comillas\"") == 0);
  CPPUNIT_ASSERT_MESSAGE("timeout bad ",timeout == 10);
  CPPUNIT_ASSERT_MESSAGE("timeoutSTR bad ",timeoutSTR.compare("10") == 0);
  CPPUNIT_ASSERT_MESSAGE("double2 bad ",dd2 == 10.123456789);
  CPPUNIT_ASSERT_MESSAGE("double bad ",dd == 10.123456789);
  float comparaf = 10.123;
  CPPUNIT_ASSERT_MESSAGE("float bad ",f == comparaf);
  CPPUNIT_ASSERT_MESSAGE("float2 bad ",f2 == comparaf);
  CPPUNIT_ASSERT_MESSAGE("alarms clean ",active == false);



}

