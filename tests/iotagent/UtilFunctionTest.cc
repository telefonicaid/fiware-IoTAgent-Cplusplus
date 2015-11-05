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
#include "UtilFunctionTest.h"
#include "rest/rest_functions.h"
#include "rest/iot_statistic.h"
#include "rest/iot_duration.h"
#include "rest/iot_stat_value.h"
#include "util/FuncUtil.h"
#include "util/iota_exception.h"

#include "util/cache.h"
#include "util/command_cache.h"
#include "util/command.h"
#include "services/admin_service.h"
#include <boost/bind.hpp>
#include <limits.h>

#include "util/csv_reader.h"

#include "util/iot_url.h"
#include "util/alarm.h"
#include "util/arguments.h"
#include <boost/property_tree/ptree.hpp>
#include "util/json_parser.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/detail/json_parser_read.hpp>
#include "util/json_parser_write.hpp"
#include <stdexcept>

CPPUNIT_TEST_SUITE_REGISTRATION(UtilFunctionTest);
namespace iota {
std::string URL_BASE = "/iot";
std::string logger("main");
}
iota::AdminService* AdminService_ptr;

void UtilFunctionTest::testFormatPattern() {
  std::string url;
  std::map<std::string, std::string> filters;
  std::string url_regex;
  std::vector<std::string> url_args;

  // Empty url
  iota::format_pattern(url, filters, url_regex, url_args);
  CPPUNIT_ASSERT((url_regex.size() != 0));

  // Url no pattern
  url_regex.clear();
  std::string url_with_no_pattern("/adm/services/test/test1");
  iota::format_pattern(url_with_no_pattern, filters, url_regex, url_args);
  CPPUNIT_ASSERT(url_regex.size() != 0);
  CPPUNIT_ASSERT(url_regex[0] == '.');

  // Url with pattern
  url_regex.clear();
  std::string url_pattern("/adm/services/<service>/assets");
  std::string regex(".+[[:space:]]+/adm/services/([^/]+)/assets");
  iota::format_pattern(url_pattern, filters, url_regex, url_args);
  CPPUNIT_ASSERT(url_regex.compare(regex) == 0);
  CPPUNIT_ASSERT(url_args.size() == 1);
  CPPUNIT_ASSERT(url_args.at(0).compare("service") == 0);
}

void UtilFunctionTest::testRegexPattern() {
  std::string url("/adm/services/TestService/assets");
  std::map<std::string, std::string> filters;
  std::string url_regex;
  std::vector<std::string> url_args;
  std::map<std::string, std::string> url_placeholders;
  url_regex.clear();
  std::string url_pattern("/adm/services/<service>/assets");
  std::string regex(".+[[:space:]]+/adm/services/([^/]+)/assets");
  iota::format_pattern(url_pattern, filters, url_regex, url_args);
  // Para test
  url.insert(0, "POST ");
  // No
  // regex.insert(0, "GET[[:space:]]+");
  // Si
  // regex.insert(0, "POST[[:space:]]+");
  // regex.insert(0, "GET|POST[[:space:]]+");
  // regex.insert(0, ".+[[:space:]]+");

  bool res =
      iota::restResourceParse(url_regex, url_args, url, url_placeholders);
  CPPUNIT_ASSERT(url_placeholders["service"].compare("TestService") == 0);
  CPPUNIT_ASSERT(res);

  url.assign("/adm/services/TestService/assets/TestAsset");
  url_pattern.assign("/adm/services/<service>/assets/<asset>");
  regex.assign(".+[[:space:]]+/adm/services/([^/]+)/assets/([^/]+)");
  url_args.clear();
  url_placeholders.clear();
  url_regex.clear();
  iota::format_pattern(url_pattern, filters, url_regex, url_args);
  url.insert(0, "POST ");
  iota::restResourceParse(url_regex, url_args, url, url_placeholders);
  CPPUNIT_ASSERT(url_placeholders["service"].compare("TestService") == 0);
  CPPUNIT_ASSERT(url_placeholders["asset"].compare("TestAsset") == 0);
  CPPUNIT_ASSERT(res);

  url.assign("/adm/services/TestService/assets/TestAsset");
  url_pattern.assign("/adm/services/TestService/assets/TestAsset");
  regex.assign(".+[[:space:]]+/adm/services/TestService/assets/TestAsset");
  url_args.clear();
  url_placeholders.clear();
  url_regex.clear();
  iota::format_pattern(url_pattern, filters, url_regex, url_args);
  url.insert(0, "POST ");
  res = iota::restResourceParse(regex, url_args, url, url_placeholders);
  CPPUNIT_ASSERT(url_placeholders.size() == 0);
  CPPUNIT_ASSERT(res);

  url.assign("/adm/services/TestService/p1");
  url_pattern.assign("/adm/services/<protocol>/p1");
  // regex.assign(".+[[:space:]]+/adm/services/TestService/assets/TestAsset");
  regex.assign("POST[[:space:]]+/adm/services/([^/]+)/assets/TestAsset");
  url_args.clear();
  url_placeholders.clear();
  url_regex.clear();
  filters["method"] = "POST";
  //(POST|GET[[:space:]]+.+/)<([a-zA-Z0-9_-]+)>
  iota::format_pattern(url_pattern, filters, url_regex, url_args);
  url.insert(0, "POST ");

  res = iota::restResourceParse(url_regex, url_args, url, url_placeholders);
  CPPUNIT_ASSERT(url_placeholders.size() == 1);
  CPPUNIT_ASSERT(res);

  url.assign("/adm/services/TestService/assets/TestAsset");
  url_pattern.assign("/adm/services/Service/assets/TestAsset");
  regex.assign(".+[[:space:]]+/adm/services/Service/assets/TestAsset");
  url_args.clear();
  url_placeholders.clear();
  url_regex.clear();
  iota::format_pattern(url_pattern, filters, url_regex, url_args);
  url.insert(0, "POST ");
  res = iota::restResourceParse(regex, url_args, url, url_placeholders);
  CPPUNIT_ASSERT(url_placeholders.size() == 0);
  CPPUNIT_ASSERT(!res);

  // Create regex for pep
  url_regex.clear();
  url.assign("/iot/ngsi/<protocol>/updateContext");
  filters["method"] = "POST";
  iota::format_pattern(url, filters, url_regex, url_args);
}

void UtilFunctionTest::testStatistic() {
  iota::IoTStatistic stat("name");

  CPPUNIT_ASSERT(stat.get_name().compare("name") == 0);
  stat.add("traffic", 1);
  stat.add("traffic", 2);
  stat.add("traffic", 3);
  iota::IoTStatistic::iot_accumulator_ptr t = stat["traffic"];
  std::cout << boost::accumulators::count(*t) << std::endl;
  CPPUNIT_ASSERT(boost::accumulators::count(*stat["traffic"]) == 3);
  CPPUNIT_ASSERT(boost::accumulators::sum(*stat["traffic"]) == 6);
  CPPUNIT_ASSERT(boost::accumulators::max(*stat["traffic"]) == 3);
  CPPUNIT_ASSERT(boost::accumulators::min(*stat["traffic"]) == 1);
  CPPUNIT_ASSERT(boost::accumulators::mean(*stat["traffic"]) == 2);
  stat.add("traffic", 4);
  CPPUNIT_ASSERT(boost::accumulators::count(*stat["traffic"]) == 4);
  // New accumulator for durations
  iota::IoTStatistic::iot_accumulator_ptr acc = stat["duration"];
  stat.add("duration", 12.8);
  stat.add("duration", 2.2);
  CPPUNIT_ASSERT(boost::accumulators::count(*stat["duration"]) == 2);
  CPPUNIT_ASSERT(boost::accumulators::sum(*stat["duration"]) == 15.0);
  CPPUNIT_ASSERT(boost::accumulators::max(*stat["duration"]) == 12.8);
  CPPUNIT_ASSERT(boost::accumulators::min(*stat["duration"]) == 2.2);
  CPPUNIT_ASSERT(boost::accumulators::mean(*stat["duration"]) == 7.5);

  iota::Duration* d = new iota::Duration(acc);
  delete d;
  CPPUNIT_ASSERT(boost::accumulators::count(*stat["duration"]) == 3);

  iota::IoTStatistic stat_1("iot");
  iota::IoTValue* v = new iota::IoTValue(stat_1["stat1"], 10);
  delete v;
  CPPUNIT_ASSERT(boost::accumulators::count(*stat_1["stat1"]) == 1);
  CPPUNIT_ASSERT(boost::accumulators::max(*stat_1["stat1"]) == 10);
  std::map<long,
           std::map<std::string, iota::IoTStatistic::iot_accumulator_ptr> > a =
      stat_1.get_counters();
  CPPUNIT_ASSERT(a.size() == 1);

  stat_1.add("stat1", 2);
  CPPUNIT_ASSERT(boost::accumulators::count(*stat_1["stat1"]) == 2);
  CPPUNIT_ASSERT(boost::accumulators::max(*stat_1["stat1"]) == 10);
  CPPUNIT_ASSERT(boost::accumulators::mean(*stat_1["stat1"]) == 6);
  stat_1.add("stat2", 5);
  CPPUNIT_ASSERT(boost::accumulators::count(*stat_1["stat2"]) == 1);
}

void UtilFunctionTest::testFilter() {}

void UtilFunctionTest::testUuid() {
  std::string uuid_1 = iota::riot_uuid("/iot");
  std::string uuid_2 = iota::riot_uuid("/iot");
  CPPUNIT_ASSERT_MESSAGE("Different uuid ", uuid_1.compare(uuid_2) != 0);
}

void UtilFunctionTest::testHeaders() {
  // Fiware-Service
  std::string more_length;
  for (int i = 0; i < 52; i++) {
    more_length.append("a");
  }
  try {
    iota::check_fiware_service_name(more_length);
  } catch (iota::IotaException& e) {
    CPPUNIT_ASSERT_MESSAGE("Expected code 400", e.status());
  }

  // Alfanum and _
  std::string noalpha("no-valid");
  try {
    iota::check_fiware_service_name(noalpha);
  } catch (iota::IotaException& e) {
    CPPUNIT_ASSERT_MESSAGE("Expected code 400", e.status());
  }

  std::string upper("Aa124");
  try {
    iota::check_fiware_service_name(upper);
  } catch (iota::IotaException& e) {
    CPPUNIT_ASSERT_MESSAGE("Expected code 400", e.status());
  }
  std::string f("_123abc_");
  iota::check_fiware_service_name(f);
  std::string good("1234_abc");
  iota::check_fiware_service_name(good);

  // Fiware-ServicePath
  try {
    iota::check_fiware_service_path_name(good);
  } catch (iota::IotaException& e) {
    CPPUNIT_ASSERT_MESSAGE("Expected code 400", e.status());
  }
  std::string fsp_length("/");
  for (int i = 0; i < 51; i++) {
    fsp_length.append("a");
  }
  try {
    iota::check_fiware_service_path_name(fsp_length);
  } catch (iota::IotaException& e) {
    CPPUNIT_ASSERT_MESSAGE("Expected code 400", e.status());
  }
  std::string e;
  std::string w("/#");
  iota::check_fiware_service_path_name(e);
  iota::check_fiware_service_path_name(w);
}

////////  CACHE

boost::shared_ptr<iota::Device> get_f(boost::shared_ptr<iota::Device> item) {
  boost::shared_ptr<iota::Device> m;
  if (item->_name != "Agus") {
    m.reset(new iota::Device(item->_name, "Gonzalez"));
  }
  return m;
}

std::string now_str() {
  // Get current time from the clock, using microseconds resolution
  const boost::posix_time::ptime now =
      boost::posix_time::microsec_clock::local_time();

  // Get the time offset in current day
  const boost::posix_time::time_duration td = now.time_of_day();

  const long hours = td.hours();
  const long minutes = td.minutes();
  const long seconds = td.seconds();
  const long milliseconds = td.total_milliseconds() -
                            ((hours * 3600 + minutes * 60 + seconds) * 1000);

  char buf[40];
  sprintf(buf, "%02ld:%02ld:%02ld.%03ld", hours, minutes, seconds,
          milliseconds);

  return buf;
}

boost::shared_ptr<iota::Command> timeout_f(
    boost::shared_ptr<iota::Command> item) {
  std::cout << "timeout_f:" << now_str() << std::endl;
  return item;
}

void UtilFunctionTest::testLRU() {
  std::cout << "testLRU" << std::endl;
  iota::Cache mru_cache_1(3, true);
  boost::shared_ptr<iota::Device> item1(new iota::Device("Agus", "Gonzalez"));
  mru_cache_1.insert(item1);
  const boost::shared_ptr<iota::Device> v1 = mru_cache_1.get(item1);
  CPPUNIT_ASSERT(v1->_name == "Agus");

  // Not found
  boost::shared_ptr<iota::Device> item2(new iota::Device("Luis", "Gonzalez"));
  CPPUNIT_ASSERT(mru_cache_1.get(item2).get() == NULL);

  {
    boost::shared_ptr<iota::Device> item3(new iota::Device("Juna", "Gonzalez"));
    mru_cache_1.insert(item3);
    CPPUNIT_ASSERT(mru_cache_1.size() == 2);
    const boost::shared_ptr<iota::Device> vx = mru_cache_1.get(item3);
    CPPUNIT_ASSERT(vx->_name == "Juna");
  }

  boost::shared_ptr<iota::Device> item_3(new iota::Device("Juna", "Gonzalez"));
  const boost::shared_ptr<iota::Device> v3 = mru_cache_1.get(item_3);
  CPPUNIT_ASSERT(mru_cache_1.size() == 2);

  // Set function to insert new element
  mru_cache_1.set_function(boost::bind(get_f, _1));
  mru_cache_1.get(item2);
  CPPUNIT_ASSERT(mru_cache_1.size() == 3);
  const boost::shared_ptr<iota::Device> v4 = mru_cache_1.get(item2);
  CPPUNIT_ASSERT(v4->_name == "Luis");

  // Duplicated
  mru_cache_1.insert(item2);
  CPPUNIT_ASSERT(mru_cache_1.size() == 3);

  // Capacity
  boost::shared_ptr<iota::Device> item_4(new iota::Device("Max", "Gonzalez"));
  mru_cache_1.insert(item_4);
  CPPUNIT_ASSERT(mru_cache_1.size() == 3);

  // item1 not found in cache
  boost::shared_ptr<iota::Device> nf = mru_cache_1.get(item1);
  CPPUNIT_ASSERT(nf.get() == NULL);
  // Remove item
  mru_cache_1.remove(*item2);
  CPPUNIT_ASSERT(mru_cache_1.size() == 2);
}

void UtilFunctionTest::testMRU() {
  std::cout << "testMRU" << std::endl;
  iota::Cache mru_cache_1(3, false);
  boost::shared_ptr<iota::Device> item1(new iota::Device("Anna", "Gonzalez"));
  mru_cache_1.insert(item1);
  const boost::shared_ptr<iota::Device> v1 = mru_cache_1.get(item1);
  CPPUNIT_ASSERT(v1->_name == "Anna");

  // Not found
  boost::shared_ptr<iota::Device> item2(new iota::Device("Luis", "Gonzalez"));
  CPPUNIT_ASSERT(mru_cache_1.get(item2).get() == NULL);

  {
    boost::shared_ptr<iota::Device> item3(new iota::Device("Juna", "Gonzalez"));
    mru_cache_1.insert(item3);
    CPPUNIT_ASSERT(mru_cache_1.size() == 2);
    const boost::shared_ptr<iota::Device> vx = mru_cache_1.get(item3);
    CPPUNIT_ASSERT(vx->_name == "Juna");
  }

  boost::shared_ptr<iota::Device> item_3(new iota::Device("Juna", "Gonzalez"));
  const boost::shared_ptr<iota::Device> v3 = mru_cache_1.get(item_3);
  CPPUNIT_ASSERT(mru_cache_1.size() == 2);

  // Set function to insert new element
  mru_cache_1.set_function(boost::bind(get_f, _1));
  mru_cache_1.get(item2);
  CPPUNIT_ASSERT(mru_cache_1.size() == 3);
  const boost::shared_ptr<iota::Device> v4 = mru_cache_1.get(item2);
  CPPUNIT_ASSERT(v4->_name == "Luis");

  // Duplicated
  mru_cache_1.insert(item2);
  CPPUNIT_ASSERT(mru_cache_1.size() == 3);

  // Capacity
  boost::shared_ptr<iota::Device> item_4(new iota::Device("Agus", "Gonzalez"));
  mru_cache_1.insert(item_4);
  CPPUNIT_ASSERT(mru_cache_1.size() == 3);

  // item4 not found in cache
  boost::shared_ptr<iota::Device> nf = mru_cache_1.get(item_4);
  CPPUNIT_ASSERT(nf.get() == NULL);
  // Remove item
  mru_cache_1.remove(*item2);
  CPPUNIT_ASSERT(mru_cache_1.size() == 2);
}

// This test is based on timers.
void UtilFunctionTest::testCacheClock() {
  std::cout << "testCacheClock" << std::endl;
  iota::Cache mru_cache_1(3, true, 2);
  boost::asio::io_service io;
  boost::asio::deadline_timer t(io);
  t.expires_from_now(boost::posix_time::seconds(3));

  boost::shared_ptr<iota::Device> item1(new iota::Device("Test", "Gonzalez"));
  boost::shared_ptr<iota::Device> item2(new iota::Device("Test1", "Gonzalez"));
  mru_cache_1.insert(item1);
  mru_cache_1.insert(item2);
  boost::shared_ptr<iota::Device> v = mru_cache_1.get(item1);
  CPPUNIT_ASSERT(v->_name == "Test");
  CPPUNIT_ASSERT(mru_cache_1.size() == 2);
  t.wait();
  CPPUNIT_ASSERT(mru_cache_1.size() == 0);

  // Duplicated restart timer
  mru_cache_1.insert(v);
  sleep(1);
  CPPUNIT_ASSERT(mru_cache_1.size() == 1);
  boost::shared_ptr<iota::Device> v1 = mru_cache_1.get(v);
  t.expires_from_now(boost::posix_time::seconds(3));
  t.wait();
  CPPUNIT_ASSERT(mru_cache_1.size() == 0);
  iota::Cache mru_cache_2(1, true, 2);
  //boost::shared_ptr<boost::asio::io_service> io2 = mru_cache_2.get_io_service();
  boost::shared_ptr<iota::Device> item3(new iota::Device("Cancel", "Gonzalez"));
  boost::shared_ptr<iota::Device> item4(new iota::Device("Test3", "Gonzalez"));
  mru_cache_2.insert(item3);
  mru_cache_2.insert(item4);
  t.expires_from_now(boost::posix_time::seconds(3));
  t.wait();
  CPPUNIT_ASSERT(mru_cache_2.size() == 0);

  iota::Cache mru_cache_3(1, false, 2);
  t.expires_from_now(boost::posix_time::seconds(3));
  mru_cache_3.insert(item3);
  mru_cache_3.insert(item4);
  t.wait();
}

void UtilFunctionTest::testByEntity() {
  std::cout << "testByEntity" << std::endl;
  iota::Cache mru_cache_1(3, false);
  boost::shared_ptr<iota::Device> item1(new iota::Device("Anna", "Gonzalez"));
  mru_cache_1.insert(item1);
  const boost::shared_ptr<iota::Device> v1 = mru_cache_1.get(item1);
  CPPUNIT_ASSERT(v1->_name == "Anna");

  // By entity
  // empty entity fields
  const boost::shared_ptr<iota::Device> ve1 = mru_cache_1.get_by_entity(item1);
  CPPUNIT_ASSERT(ve1.get() != NULL);

  // Complete device
  boost::shared_ptr<iota::Device> item2(new iota::Device("d1", "s1"));
  item2->_entity_name = "e";
  item2->_entity_type = "t";
  item2->_protocol = "ul20";
  mru_cache_1.insert(item2);
  CPPUNIT_ASSERT(mru_cache_1.size() == 2);

  // different device, same entity, different protocol
  boost::shared_ptr<iota::Device> item2_1(new iota::Device("d1_1", "s1"));
  item2_1->_entity_name = "e";
  item2_1->_entity_type = "t";
  item2_1->_protocol = "mqtt";
  mru_cache_1.insert(item2_1);
  CPPUNIT_ASSERT(mru_cache_1.size() == 3);

  // Find
  boost::shared_ptr<iota::Device> q2(new iota::Device("", "s1"));
  q2->_entity_name = "e";
  q2->_entity_type = "t";
  q2->_protocol = "ul20";
  boost::shared_ptr<iota::Device> vitem2 = mru_cache_1.get_by_entity(q2);
  CPPUNIT_ASSERT(vitem2->_entity_name == "e");
  CPPUNIT_ASSERT(vitem2->_name == "d1");

  boost::shared_ptr<iota::Device> q2_1(new iota::Device("", "s1"));
  q2_1->_entity_name = "e";
  q2_1->_entity_type = "t";
  q2_1->_protocol = "mqtt";
  boost::shared_ptr<iota::Device> vitem2_1 = mru_cache_1.get_by_entity(q2_1);
  CPPUNIT_ASSERT(vitem2_1->_entity_name == "e");
  CPPUNIT_ASSERT(vitem2_1->_name == "d1_1");

  // Find entity
  boost::shared_ptr<iota::Device> item3(new iota::Device("e1", "t1", "s1"));
  item3->_protocol = "ul20";
  vitem2 = mru_cache_1.get_by_entity(item3);
  CPPUNIT_ASSERT(vitem2.get() == NULL);

  boost::shared_ptr<iota::Device> item4(new iota::Device("e", "t", "s1"));
  item4->_protocol = "ul20";
  vitem2 = mru_cache_1.get_by_entity(item4);
  CPPUNIT_ASSERT(vitem2.get() != NULL);

  boost::shared_ptr<iota::Device> item5(new iota::Device("e", "t", "s1"));
  // if you forget to put protocol, no device
  boost::shared_ptr<iota::Device> vitem5 = mru_cache_1.get_by_entity(item5);
  CPPUNIT_ASSERT(vitem5.get() == NULL);
}

void UtilFunctionTest::testCommandTimeout() {
  std::cout << "START testCommandTimeout" << std::endl;

  // OJO el timeout que se usa es el del device no el definido en la cola
  iota::CommandCache command_cache_1(SHRT_MAX, true);
  std::cout << "insert:" << now_str() << std::endl;
  boost::asio::io_service io;
  boost::asio::deadline_timer t(io);
  t.expires_from_now(boost::posix_time::seconds(2));

  command_cache_1.set_timeout_function(timeout_f);
  boost::property_tree::ptree pt;
  pt.put("body", "command");
  boost::shared_ptr<iota::Command> item1(
      new iota::Command("id1", "id1", "servico1", "/", "seq1", "room1", "thing",
                        "http://uri:888//ee", 1, "", pt));

  std::cout << "insert:" << now_str() << std::endl;
  command_cache_1.insert(item1);

  std::cout << "command_cache_1 size 1" << std::endl;
  CPPUNIT_ASSERT(command_cache_1.size() == 1);
  t.wait();
  std::cout << "after wait 0" << std::endl;
  CPPUNIT_ASSERT(command_cache_1.size() == 0);

  std::cout << "END testCommandTimeout" << std::endl;
}

void UtilFunctionTest::testCommand() {
  std::cout << "START testCommand" << std::endl;
  boost::property_tree::ptree command;
  command.put("body", "texto del comando para el device");

  iota::CommandCache command_cache_1(3, false);

  boost::shared_ptr<iota::Command> item1(new iota::Command(
      "id1", "servico1", "/", "seq1", "room1", "http://uri:888//ee", command));
  item1->set_status(1);
  command_cache_1.insert(item1);
  boost::shared_ptr<iota::Command> item2(new iota::Command(
      "id2", "servico1", "/", "seq2", "room1", "http://uri:888//ee", command));
  item2->set_status(1);
  command_cache_1.insert(item2);
  boost::shared_ptr<iota::Command> item3(new iota::Command(
      "id3", "servico1", "/", "seq3", "roomm3", "http://uri:888//ee", command));
  item3->set_status(1);
  command_cache_1.insert(item3);
  boost::shared_ptr<iota::Command> item4(new iota::Command(
      "id4", "servico1", "/", "seq4", "", "http://uri:888//ee", command));
  item4->set_status(1);
  command_cache_1.insert(item4);

  std::cout << "get" << std::endl;
  const boost::shared_ptr<iota::Command> v1 = command_cache_1.get(item1);

  // By Id
  std::cout << "getById" << std::endl;
  iota::Command* cp = new iota::Command("", "servico1", "/");
  cp->set_id("id1");
  boost::shared_ptr<iota::Command> itemQid(cp);
  boost::shared_ptr<iota::Command> vitem = command_cache_1.get_by_id(itemQid);
  CPPUNIT_ASSERT(vitem.get() != NULL);
  std::cout << "getById da " << vitem->get_id() << vitem->get_entity()
            << std::endl;

  // By entity
  // empty entity fields
  std::cout << "getByEntity" << std::endl;
  boost::shared_ptr<iota::Command> itemQ(
      new iota::Command("room1", "servico1", "/"));
  itemQ->set_status(1);
  boost::shared_ptr<iota::Command> vitem2 =
      command_cache_1.get_by_entity(itemQ);
  CPPUNIT_ASSERT(vitem2.get() != NULL);
  std::cout << "getByEntity da " << vitem2->get_id() << std::endl;

  iota::CommandVect commands = command_cache_1.get_by_entityV(itemQ, 2);
  std::cout << "getByEntity da " << commands.size() << std::endl;
  iota::CommandVect::const_iterator i;
  iota::CommandPtr ptr;
  for (i = commands.begin(); i != commands.end(); ++i) {
    ptr = *i;
    std::cout << (ptr->get_id()) << ",";
  }
  std::cout << std::endl;

  // una vez que los hemos leido han cambiado de estado
  iota::CommandVect commands2 = command_cache_1.get_by_entityV(itemQ, 2);
  std::cout << "getByEntity da " << commands2.size() << std::endl;
  iota::CommandVect::const_iterator i2;
  iota::CommandPtr ptr2;
  for (i2 = commands2.begin(); i2 != commands2.end(); ++i) {
    ptr2 = *i2;
    std::cout << (ptr2->get_id()) << ",";
  }
  std::cout << std::endl;

  CPPUNIT_ASSERT(commands2.size() == 0);

  std::cout << "END testCommand" << std::endl;
}

///////CsvTest
void UtilFunctionTest::testCsv() {
  std::string csv_ok("item1,item2, item3\nvalue1, value2, value3");
  std::vector<std::map<std::string, std::string> > data;
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

///////  URL Test

template <typename T>
struct my_id_translator {
  typedef T internal_type;
  typedef T external_type;

  boost::optional<T> get_value(const T& v) { return v.substr(1, v.size() - 2); }

  boost::optional<T> put_value(const T& v) { return '"' + v + '"'; }
};

void UtilFunctionTest::testUrl() {
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

void UtilFunctionTest::testAlarm() {
  iota::Alarm* palarm = iota::Alarm::instance();
  CPPUNIT_ASSERT_MESSAGE("alarms clean ", palarm->size() == 0);
  iota::Alarm::error(22, "endpoint", "status", "text");

  CPPUNIT_ASSERT_MESSAGE("alarms one ", palarm->size() == 1);

  iota::Alarm::info(22, "endpoint", "status", "text");

  CPPUNIT_ASSERT_MESSAGE("alarms clean ", palarm->size() == 0);

  std::cout << "@UT@Perdida de medida" << std::endl;
  boost::property_tree::ptree ptin;
  ptin.put("cbroker", "http://127.0.0.1:8888/iotagent/ngsi90");
  ptin.put("service", "myservice");
  ptin.put("service_path", "myservice_path");
  ptin.put("timeout", 10);
  ptin.put("active", false);

  std::stringstream ss;
  iota::property_tree::json_parser::write_json(ss, ptin);

  iota::Alarm::error(22, "endpoint", "{updateContext:22}", ptin, "status", "text");
  CPPUNIT_ASSERT_MESSAGE("alarms one2 ", palarm->size() == 1);

  iota::Alarm::info(22, "endpoint", "status", "text");
  CPPUNIT_ASSERT_MESSAGE("alarms clean2 ", palarm->size() == 0);

}

void UtilFunctionTest::testPtree2String() {
  boost::property_tree::ptree ptin;
  std::string escapess = "breakline\n\r\tescapes";

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
  std::cout << "json write:" << text << std::endl;

  CPPUNIT_ASSERT_MESSAGE(
      "cbroker bad",
      text.find("http://127.0.0.1:8888/iotagent/ngsi90") != std::string::npos);
  CPPUNIT_ASSERT_MESSAGE(
      "breakline bad",
      text.find("breakline\\n\\r\\tescapes") != std::string::npos);
  CPPUNIT_ASSERT_MESSAGE("eñes bad", text.find("õñoñoÿÐ") != std::string::npos);
  CPPUNIT_ASSERT_MESSAGE("comillas bad",
                         text.find("\"comillas\"") != std::string::npos);

  boost::property_tree::ptree ptout;
  std::istringstream is(text);
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
  double va = 0.0;
  double dd = ptout.get("double", va);
  double dd2 = ptout.get("doubleSTR", va);
  bool active = ptout.get("active", true);

  std::cout << "read:" << cbroker << "|" << breakline << "|" << comillas << "|"
            << enes << "|" << timeout << "|" << timeoutSTR << "|" << dd << "|"
            << dd2 << "|" << f << "|" << f2 << "|" << active << std::endl;
  CPPUNIT_ASSERT_MESSAGE(
      "cbroker bad ",
      cbroker.compare("http://127.0.0.1:8888/iotagent/ngsi90") == 0);
  CPPUNIT_ASSERT_MESSAGE("breakline bad ", breakline.compare(escapess) == 0);
  CPPUNIT_ASSERT_MESSAGE("eñes bad ", enes.compare("õñoñoÿÐ") == 0);
  CPPUNIT_ASSERT_MESSAGE("comillas bad ",
                         comillas.compare("\"comillas\"") == 0);
  CPPUNIT_ASSERT_MESSAGE("timeout bad ", timeout == 10);
  CPPUNIT_ASSERT_MESSAGE("timeoutSTR bad ", timeoutSTR.compare("10") == 0);
  CPPUNIT_ASSERT_MESSAGE("double2 bad ", dd2 == 10.123456789);
  CPPUNIT_ASSERT_MESSAGE("double bad ", dd == 10.123456789);
  float comparaf = 10.123;
  CPPUNIT_ASSERT_MESSAGE("float bad ", f == comparaf);
  CPPUNIT_ASSERT_MESSAGE("float2 bad ", f2 == comparaf);
  CPPUNIT_ASSERT_MESSAGE("alarms clean ", active == false);
}

void UtilFunctionTest::testForbiddenCharacters() {
  std::string no_forbidden("This string has not forbidden characters");
  std::string with_minor("This string < use minor");
  std::string with_major("This>use major");
  std::string with_double_quotes("This ha\"s double quotes");
  std::string with_single_quotes("This doesn't single quotes");
  std::string with_equal("six=two*3");
  std::string with_semicolon("one;two;three");
  std::string with_left_parenthesis("This string (for example");
  std::string with_right_parenthesis("This string for example)");
  CPPUNIT_ASSERT_MESSAGE(
      no_forbidden,
      iota::check_forbidden_characters(iota::types::IOTA_FORBIDDEN_CHARACTERS,
                                       no_forbidden) == false);
  CPPUNIT_ASSERT_MESSAGE(with_minor, iota::check_forbidden_characters(
                                         iota::types::IOTA_FORBIDDEN_CHARACTERS,
                                         with_minor) == true);
  CPPUNIT_ASSERT_MESSAGE(with_minor, iota::check_forbidden_characters(
                                         iota::types::IOTA_FORBIDDEN_CHARACTERS,
                                         with_minor) == true);
  CPPUNIT_ASSERT_MESSAGE(
      with_double_quotes,
      iota::check_forbidden_characters(iota::types::IOTA_FORBIDDEN_CHARACTERS,
                                       with_double_quotes) == true);
  CPPUNIT_ASSERT_MESSAGE(
      with_single_quotes,
      iota::check_forbidden_characters(iota::types::IOTA_FORBIDDEN_CHARACTERS,
                                       with_single_quotes) == true);
  CPPUNIT_ASSERT_MESSAGE(
      with_semicolon,
      iota::check_forbidden_characters(iota::types::IOTA_FORBIDDEN_CHARACTERS,
                                       with_semicolon) == true);
  CPPUNIT_ASSERT_MESSAGE(
      with_left_parenthesis,
      iota::check_forbidden_characters(iota::types::IOTA_FORBIDDEN_CHARACTERS,
                                       with_left_parenthesis) == true);
  CPPUNIT_ASSERT_MESSAGE(
      with_right_parenthesis,
      iota::check_forbidden_characters(iota::types::IOTA_FORBIDDEN_CHARACTERS,
                                       with_right_parenthesis) == true);

  // Checking operator
  bool forbidden = iota::check_forbidden_characters(
      iota::types::IOTA_FORBIDDEN_CHARACTERS, no_forbidden);
  CPPUNIT_ASSERT(forbidden == false);
  forbidden =
      forbidden || iota::check_forbidden_characters(
                       iota::types::IOTA_FORBIDDEN_CHARACTERS, no_forbidden);
  CPPUNIT_ASSERT(forbidden == false);
  forbidden =
      forbidden || iota::check_forbidden_characters(
                       iota::types::IOTA_FORBIDDEN_CHARACTERS, with_minor);
  CPPUNIT_ASSERT(forbidden == true);
  forbidden =
      forbidden || iota::check_forbidden_characters(
                       iota::types::IOTA_FORBIDDEN_CHARACTERS, no_forbidden);
  CPPUNIT_ASSERT(forbidden == true);
  forbidden =
      forbidden || iota::check_forbidden_characters(
                       iota::types::IOTA_FORBIDDEN_CHARACTERS, with_minor);
  CPPUNIT_ASSERT(forbidden == true);
}

void UtilFunctionTest::testCommandLine() {
  std::cout << "@UT@START testCommandLine" << std::endl;
  iota::Arguments arguments;

  std::cout << "@UT@Scenario: no parameters in command line" << std::endl;
  int argc = 1;
  const char* argv1[] = {"iotagent"};
  std::string result = arguments.parser(argc, argv1);
  std::cout << "@UT@result1:" << result << std::endl;
  CPPUNIT_ASSERT(result.compare(iota::types::HELP_MESSAGE_ERR_CONFIG +
                                iota::types::HELP_MESSAGE_OPS) == 0);

  std::cout << "@UT@Scenario: not all necesary parameters" << std::endl;
  const char* argv2[] = {
      "iotagent"
      "param0",
      "param1", "param2"};
  result = arguments.parser(4, argv2);
  std::cout << "@UT@result2:" << result << std::endl;
  CPPUNIT_ASSERT(result.compare(iota::types::HELP_MESSAGE_ERR_PARAM +
                                iota::types::HELP_MESSAGE_OPS) == 0);
  std::cout << "service_name:" << arguments.get_service_name() << std::endl;
  std::cout << "component_name:" << arguments.get_component_name() << std::endl;

  std::cout << "@UT@Scenario: iotagent  -h " << std::endl;
  const char* argv3[] = {"iotagent", "-h"};
  result = arguments.parser(2, argv3);
  std::cout << "@UT@result3:" << result << std::endl;
  CPPUNIT_ASSERT(result.compare(iota::types::HELP_MESSAGE_OPS +
                                iota::types::HELP_MESSAGE) == 0);

  std::cout << "@UT@Scenario: iotagent  --help " << std::endl;
  const char* argv4[] = {"iotagent", "--help"};
  result = arguments.parser(2, argv4);
  std::cout << "@UT@result4:" << result << std::endl;
  CPPUNIT_ASSERT(result.compare(iota::types::HELP_MESSAGE_OPS +
                                iota::types::HELP_MESSAGE) == 0);

  std::cout << "@UT@Scenario: param not recognize " << std::endl;
  const char* argv5[] = {"iotagent", "--bomb"};
  result = arguments.parser(2, argv5);
  std::cout << "@UT@result5:" << result << std::endl;
  CPPUNIT_ASSERT(result.compare(iota::types::HELP_MESSAGE_ERR_BAD_PARAM +
                                iota::types::HELP_MESSAGE_OPS) == 0);

  std::cout << "@UT@Scenario: iotagent ok " << std::endl;
  const char* argv6[] = {"iotagent",
                         "-v",
                         "DEBUG",
                         "-n",
                         "dev",
                         "-i",
                         "127.0.0.1",
                         "-p",
                         "8080",
                         "-d",
                         "/home/develop/GH/lib/Debug",
                         "-c",
                         "/home/iotagent/config_iot.json"};
  result = arguments.parser(13, argv6);
  std::cout << "@UT@result6:" << result << std::endl;
  CPPUNIT_ASSERT(result.empty());
  CPPUNIT_ASSERT(!arguments.get_manager());

  std::cout << "@UT@Scenario: iota manager ok " << std::endl;
  const char* argv7[] = {"iotagent",
                         "-m",
                         "-6",
                         "-v",
                         "DEBUG",
                         "-n",
                         "manager",
                         "-i",
                         "192.0.3.25",
                         "-p",
                         "8081",
                         "-d",
                         "/home/develop/GH/lib/Debug",
                         "-c",
                         "/home/iotagent/config.json"};
  result = arguments.parser(15, argv7);
  std::cout << "@UT@result7:" << result << std::endl;
  CPPUNIT_ASSERT(result.empty());
  CPPUNIT_ASSERT(arguments.get_manager());

  std::cout << "@UT@Scenario: iota manager large param ok " << std::endl;
  const char* argv75[] = {"iotagent",
                          "--manager",
                          "--ipv6",
                          "--verbose",
                          "DEBUG",
                          "--name",
                          "manager",
                          "--ip",
                          "192.0.3.25",
                          "--port",
                          "8081",
                          "--plugins-dir",
                          "/home/develop/GH/lib/Debug",
                          "--config_file",
                          "/home/iotagent/config.json"};
  result = arguments.parser(15, argv75);
  std::cout << "@UT@result75:" << result << std::endl;
  CPPUNIT_ASSERT(result.empty());
  CPPUNIT_ASSERT(arguments.get_manager());
  CPPUNIT_ASSERT(arguments.get_plugin_directory().compare(
                     "/home/develop/GH/lib/Debug") == 0);

  std::cout << "@UT@Scenario: no core for few parameters " << std::endl;
  const char* argv8[] = {"iotagent", "-"};
  result = arguments.parser(2, argv6);
  std::cout << "@UT@result8:" << result << std::endl;
  CPPUNIT_ASSERT(result.compare(iota::types::HELP_MESSAGE_ERR_BAD_PARAM +
                                iota::types::HELP_MESSAGE_OPS) == 0);

  std::cout << "@UT@END testCommandLine" << std::endl;
}
