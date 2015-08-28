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
#include "baseTest.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <pion/process.hpp>
#include <pion/http/response.hpp>

#include <cppunit/extensions/HelperMacros.h>
#include <sys/time.h>

#define  DOC_MESSAGE  "##"


const std::string BaseTest::HOST("127.0.0.1");
const std::string BaseTest::CONTENT_JSON("application/json");

BaseTest::BaseTest() {}

std::string BaseTest::create_random_service(const std::string& sem){
  std::string result(sem);

  // get miliseconds from now
  struct timeval tp;
  gettimeofday(&tp, NULL);
  long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
  result.append(boost::lexical_cast<std::string>(ms));

  return result;
}


int BaseTest::delete_mongo(const std::string& table,
                 const std::string& service,
                 const std::string& service_path){

  try {
    iota::Collection table1(table);

    mongo::BSONObj p;

    if (!service.empty() && !service_path.empty()){
      p = BSON(iota::store::types::SERVICE << service <<
                            iota::store::types::SERVICE_PATH << service_path);
    }

    return table1.remove(p);
  }
  catch (std::exception e) {
    std::cout << "throw exception:" << e.what() << std::endl;
    return -1;
  }

}

int BaseTest::check_mongo(const std::string& table,
                                  const std::string& service, const std::string& service_path,
                                  const std::string& field_name, const std::string& value) {

  try {
    iota::Collection table1(table);

    mongo::BSONObj p = BSON(iota::store::types::SERVICE << service <<
                            iota::store::types::SERVICE_PATH << service_path <<
                            field_name << value);

    return table1.count(p);
  }
  catch (std::exception e) {
    std::cout << "throw exception:" << e.what() << std::endl;
    return -1;
  }

}

int BaseTest::check_mongo(const std::string& table,
                          const std::string& json) {

  try {
    iota::Collection table1(table);

    mongo::BSONObj p =  mongo::fromjson(json);

    return table1.count(p);
  }
  catch (std::exception e) {
    std::cout << "throw exception:" << e.what() << std::endl;
    return -1;
  }

}

int BaseTest::http_test(const std::string& uri,
                                const std::string& method,
                                const std::string& service,
                                const std::string& service_path,
                                const std::string& content_type,
                                const std::string& body,
                                const std::map<std::string, std::string>& headers,
                                const std::string& query_string,
                                std::string& response) {

  boost::shared_ptr<iota::HttpClient> http_client;
  pion::http::response_ptr http_response;

  pion::http::request_ptr http_request(new pion::http::request());
  http_request->set_resource(uri);

  http_request->set_method(method);
  http_request->set_content_type(content_type);
  if (!service.empty()) {
    http_request->add_header(iota::types::FIWARE_SERVICE, service);
  }
  if (!service_path.empty()) {
    http_request->add_header(iota::types::FIWARE_SERVICEPATH, service_path);
  }

  //http_request.add_header("Accept", "application/json");

  if (!query_string.empty()) {
    http_request->set_query_string(query_string);
  }

  std::map<std::string, std::string>::const_iterator iter;
  for (iter = headers.begin(); iter != headers.end(); ++iter) {
    std::cout << "header: " << iter->first << iter->second << std::endl;
    http_request->add_header(iter->first, iter->second);
  }
  if (!body.empty()) {
    http_request->set_content(body);
  }
  std::cout << "send" << std::endl;
  http_client.reset(
      new iota::HttpClient(HOST, iota::Process::get_process().get_http_port()));

  int timeout = 2;
  std::string proxy = "";
  http_response = http_client->send(http_request, timeout, proxy);

  int code_res;

  if (http_response.get() == NULL){
    code_res = -1;
    response.assign("ERROR no response from server");
  }else {
    code_res = http_response->get_status_code();
    response.assign(http_response->get_content());
  }

  return code_res;

}


void BaseTest::feature(const std::string& name,
          const std::string& idas,
          const std::string& description){

  if (!idas.empty()){
    std::cout << DOC_MESSAGE << "@" << idas << std::endl;
  }

  std::cout << DOC_MESSAGE << "Feature:" << name << std::endl;
  if (!description.empty()){
    std::cout << DOC_MESSAGE << description << std::endl;
  }
  std::cout << DOC_MESSAGE << std::endl;
}


void BaseTest::scenario(const std::string& name,
          const std::string& idas,
          const std::string& description){

  if (!idas.empty()){
    std::cout << DOC_MESSAGE << "@" << idas << std::endl;
  }

  std::cout << DOC_MESSAGE << "Scenario:" << name << std::endl;
  if (!description.empty()){
    std::cout << DOC_MESSAGE << description << std::endl;
  }
  std::cout << DOC_MESSAGE << std::endl;
}

void BaseTest::init_config(unsigned int cb_port,
                      const std::string& type,
                      const std::string& resource,
                      const std::string& manager,
                      const std::string& public_ip,
                      const std::string& identifier,
                      const std::string& proxy,
                      const std::string& service) {

  iota::Configurator::release();
  iota::Configurator* my_instance = iota::Configurator::instance();

  std::stringstream ss;
  ss <<   "{\"ngsi_url\": {"
     <<   "     \"cbroker\": \"http://127.0.0.1:" << cb_port << "/mock\", "
     <<   "     \"updateContext\": \"/NGSI10/updateContext\","
     <<   "     \"registerContext\": \"/NGSI9/registerContext\","
     <<   "     \"queryContext\": \"/NGSI10/queryContext\""
     <<   "},"
     <<   "\"timeout\": 1,"
     <<   "\"dir_log\": \"/tmp/\","
     <<   "\"timezones\": \"/etc/iot/date_time_zonespec.csv\","
     <<   "\"schema_path\": \"../../schema\","
     <<   "\"storage\": {"
     <<   "\"host\": \"127.0.0.1\","
     <<   "\"type\": \"" <<  type << "\","
     <<   "\"port\": \"27017\","
     <<   "\"dbname\": \"iotest\","
     <<   "\"file\": \"../../tests/iotagent/devices.json\""
     << "},";

  if (!manager.empty()){
    ss <<   "\"iota_manager\" : \"" << manager << "\",";
  }

  if (!public_ip.empty()){
    ss <<   "\"public_ip\" : \"" << public_ip << "\",";
  }

  if (!identifier.empty()){
    ss <<   "\"identifier\" : \"" << identifier << "\",";
  }

  if (!proxy.empty()){
    ss <<   "\"proxy\" : \"" << proxy << "\",";
  }

  if (resource.compare("/iot/d") ==0){
    ss <<  "\"resources\":[{\"resource\": \"/iot/d\","
       << "  \"options\": {\"FileName\": \"UL20Service\" },"
       <<    " \"services\":[";
  }else if (resource.compare("/iot/mqtt") ==0){
    ss <<  "\"resources\":[{\"resource\": \"/iot/mqtt\","
       << "  \"options\": {\"FileName\": \"UL20Service\" },"
       <<    " \"services\":[";
  }

  if (!service.empty()){
    std::string cb_port_string = boost::lexical_cast<std::string>(cb_port);
    ss << boost::replace_all_copy(service, "%%port%%", cb_port_string);
  }
  ss << "] } ] }";

  //my_instance->update_conf(ss);
  std::string err = my_instance->read_file(ss);
  std::cout << "GET CONF " << my_instance->getAll() << std::endl;
  if (!err.empty()) {
    std::cout << "start_cbmock:" << err << std::endl;
    std::cout << "start_cbmock_data:" << ss.str() << std::endl;
  }

  pion::logger pion_logger(PION_GET_LOGGER("main"));
  PION_LOG_SETLEVEL_DEBUG(pion_logger);
  PION_LOG_CONFIG_BASIC;
}

void BaseTest::check_last_contains(MockService* cb_mock,
          const std::string& data,
          const std::string& message,
          int wait){

  if (wait > 0){
    // for asynchronous operation it is neccesary wait time
    boost::this_thread::sleep(boost::posix_time::milliseconds(wait * 100));
  }
  std::string cb_last = cb_mock->get_last("/mock");
  std::cout << "@UT@Last simulator:" << cb_last << std::endl;
  std::cout << "Then last message received by "  <<
                   " must contain " << data << std::endl;
  IOTASSERT_MESSAGE(message,
        cb_last.find(data) != std::string::npos);
}

void BaseTest::check_last_equal(MockService* cb_mock,
          const std::string& data,
          const std::string& message,
          int wait){

  if (wait > 0){
    // for asynchronous operation it is neccesary wait time
    boost::this_thread::sleep(boost::posix_time::milliseconds(wait * 100));
  }

  std::string cb_last = cb_mock->get_last("/mock");
  std::cout << "@UT@Last simulator:" << cb_last << std::endl;
  IOTASSERT_MESSAGE(message, cb_last.compare(data) == 0);
}

