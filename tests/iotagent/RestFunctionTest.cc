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
#include "RestFunctionTest.h"
#include "rest/rest_functions.h"
#include "rest/iot_statistic.h"
#include "rest/iot_duration.h"
#include "rest/iot_stat_value.h"
#include "util/FuncUtil.h"
#include "util/iota_exception.h"

CPPUNIT_TEST_SUITE_REGISTRATION(RestFunctionTest);

void RestFunctionTest::testFormatPattern() {

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

void RestFunctionTest::testRegexPattern() {

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
  //regex.insert(0, "GET[[:space:]]+");
  //Si
  //regex.insert(0, "POST[[:space:]]+");
  //regex.insert(0, "GET|POST[[:space:]]+");
  //regex.insert(0, ".+[[:space:]]+");

  bool res = iota::restResourceParse(url_regex, url_args, url, url_placeholders);
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

  url.assign("/adm/services/TestService/assets/TestAsset");
  url_pattern.assign("/adm/services/TestService/assets/TestAsset");
  regex.assign(".+[[:space:]]+/adm/services/TestService/assets/TestAsset");
  url_args.clear();
  url_placeholders.clear();
  url_regex.clear();
  filters["method"] = "POST|GET";
  iota::format_pattern(url_pattern, filters, url_regex, url_args);
  url.insert(0, "POST ");
  res = iota::restResourceParse(regex, url_args, url, url_placeholders);
  CPPUNIT_ASSERT(url_placeholders.size() == 0);
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
}

void RestFunctionTest::testStatistic() {
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
  std::map<long, std::map<std::string, iota::IoTStatistic::iot_accumulator_ptr> >
  a =
    stat_1.get_counters();
  CPPUNIT_ASSERT(a.size() == 1);

  stat_1.add("stat1", 2);
  CPPUNIT_ASSERT(boost::accumulators::count(*stat_1["stat1"]) == 2);
  CPPUNIT_ASSERT(boost::accumulators::max(*stat_1["stat1"]) == 10);
  CPPUNIT_ASSERT(boost::accumulators::mean(*stat_1["stat1"]) == 6);
  stat_1.add("stat2", 5);
  CPPUNIT_ASSERT(boost::accumulators::count(*stat_1["stat2"]) == 1);
  stat_1.reset();
  a = stat_1.get_counters();
  CPPUNIT_ASSERT_MESSAGE("Reset stat 0 counters", a.size() == 0);
}

void RestFunctionTest::testFilter() {

}

void RestFunctionTest::testUuid() {
  std::string uuid_1 = iota::riot_uuid("/iot");
  std::string uuid_2 = iota::riot_uuid("/iot");
  CPPUNIT_ASSERT_MESSAGE("Different uuid ", uuid_1.compare(uuid_2) != 0);
}

void RestFunctionTest::testHeaders() {
  // Fiware-Service
  std::string more_length;
  for (int i = 0; i < 52; i++) {
    more_length.append("a");
  }
  try {
    iota::check_fiware_service_name(more_length);
  }
  catch(iota::IotaException& e) {
    CPPUNIT_ASSERT_MESSAGE("Expected code 400", e.status());
  }

  //Alfanum and _
  std::string noalpha("no-valid");
  try {
    iota::check_fiware_service_name(noalpha);
  }
  catch(iota::IotaException& e) {
    CPPUNIT_ASSERT_MESSAGE("Expected code 400", e.status());
  }

   std::string upper("Aa124");
  try {
    iota::check_fiware_service_name(upper);
  }
  catch(iota::IotaException& e) {
    CPPUNIT_ASSERT_MESSAGE("Expected code 400", e.status());
  }
  std::string f("_123abc_");
  iota::check_fiware_service_name(f);
  std::string good("1234_abc");
  iota::check_fiware_service_name(good);

  // Fiware-ServicePath
   try {
    iota::check_fiware_service_path_name(good);
  }
  catch(iota::IotaException& e) {
    CPPUNIT_ASSERT_MESSAGE("Expected code 400", e.status());
  }
  std::string fsp_length("/");
  for (int i = 0; i < 51; i++) {
    fsp_length.append("a");
  }
   try {
    iota::check_fiware_service_path_name(fsp_length);
  }
  catch(iota::IotaException& e) {
    CPPUNIT_ASSERT_MESSAGE("Expected code 400", e.status());
  }
  std::string e;
  std::string w("/#");
  iota::check_fiware_service_path_name(e);
  iota::check_fiware_service_path_name(w);
}
