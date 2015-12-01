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

#include "util_functions.h"
#include "services/admin_service.h"
#include "util/common.h"
#include "util/dev_file.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <algorithm>

std::map<std::string, std::string> TestSetup::_DEVICES_UT_;
std::string TestSetup::get_device(std::string device_id, std::string protocol,
                                  bool endpoint) {
  std::string device_json;
  if (TestSetup::_DEVICES_UT_.size() == 0) {
    std::cout << "Construyendo _DEVICES_UT_" << std::endl;
    const iota::JsonValue& devices =
        iota::DevicesFile::instance()->getDevicesObject();
    for (rapidjson::SizeType i = 0; i < devices.Size(); i++) {
      rapidjson::StringBuffer buffer;
      rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
      devices[i].Accept(writer);
      device_json.assign(buffer.GetString());
      std::string key_device = devices[i]["device_id"].GetString();
      TestSetup::_DEVICES_UT_.insert(
          std::pair<std::string, std::string>(key_device, device_json));
    }
  }
  device_json.clear();
  std::map<std::string, std::string>::iterator it =
      TestSetup::_DEVICES_UT_.find(device_id);
  if (it != TestSetup::_DEVICES_UT_.end()) {
    std::cout << "Encontrado " << device_id << std::endl;
    device_json = it->second;
  }
  std::cout << "Dev " << device_id << "JSON " << device_json << std::endl;
  if (!device_json.empty()) {
    std::size_t has_endpoint = device_json.find("\"protocol\": \"");
    std::string prot("\"protocol\": \"");
    prot.append(protocol);
    prot.append("\",");
    device_json.insert(1, prot);
    if (endpoint && has_endpoint == std::string::npos) {
      std::string port = boost::lexical_cast<std::string>(
          iota::Process::get_process().get_http_port());
      std::string endpoint("\"endpoint\": \"");
      endpoint.append("http://127.0.0.1:");
      endpoint.append(port);
      endpoint.append("/mock/");
      endpoint.append(device_id);
      endpoint.append("\",");
      device_json.insert(1, endpoint);
    }
    device_json.insert(0, "{\"devices\": [");
    device_json.append("]}");
  }
  return device_json;
}

int TestSetup::add_device(std::string device_id, std::string protocol,
                          bool endpoint, std::string service,
                          std::string service_path) {
  pion::http::response http_response;
  std::string response;
  std::string device_json = get_device(device_id, protocol, endpoint);
  if (service.empty()) {
    service = _service;
  }
  if (service_path.empty()) {
    service_path = _service_path;
  }
  _devices.push_back(device_id);
  return iota::Process::get_process().get_admin_service()->post_device_json(
      service, service_path, device_json, http_response, response, "1234");
}

int TestSetup::delete_device(std::string device_id, std::string service,
                             std::string service_path) {
  pion::http::response http_response;
  std::string response;
  if (service.empty()) {
    service = _service;
  }
  if (service_path.empty()) {
    service_path = _service_path;
  }
  return iota::Process::get_process().get_admin_service()->delete_device_json(
      service, service_path, device_id, http_response, response, "1234", "");
}

std::string get_service_name(std::string name) {
  std::string srv_name(name);
  std::transform(srv_name.begin(), srv_name.end(), srv_name.begin(), ::tolower);
  return srv_name;
}

TestSetup::TestSetup(std::string service, std::string resource,
                     std::string my_post_service) {
  _apikey = service;
  std::transform(service.begin(), service.end(), service.begin(), ::tolower);
  std::string service_path("/");
  service_path.append(service);
  _service = service;
  _service_path = service_path;
  _resource = resource;

  unsigned int port = iota::Process::get_process().get_http_port();
  std::string cbroker_url("http://127.0.0.1:");
  cbroker_url.append(boost::lexical_cast<std::string>(port));
  cbroker_url.append("/mock/" + _service);

  std::string post_service(
      "{\"services\": [{"
      "\"apikey\": \"" +
      _apikey + "\",\"token\": \"" + _apikey +
      "\","
      "\"cbroker\": \"" +
      cbroker_url + "\",\"entity_type\": \"" + _service +
      "\",\"resource\": \"" + resource + "\"}]}");
  if (!my_post_service.empty()) {
    post_service = my_post_service;
  }

  boost::shared_ptr<iota::ServiceCollection> col(new iota::ServiceCollection());
  pion::http::response http_r;
  std::string r;
  ((iota::AdminService*)iota::Process::get_process().get_service(
       iota::Process::get_url_base()))
      ->delete_service_json(col, service, service_path, service, _apikey,
                            _resource, true, http_r, r, _service, _service);
  ((iota::AdminService*)iota::Process::get_process().get_service(
       iota::Process::get_url_base()))
      ->post_service_json(col, service, service_path, post_service, http_r, r,
                          _apikey, _service);
}

TestSetup::TestSetup(std::string service, std::string resource,
                     bool empty_apikey) {
  if (empty_apikey) {
    _apikey = "";
  } else {
    _apikey = service;
  }
  std::transform(service.begin(), service.end(), service.begin(), ::tolower);
  std::string service_path("/");
  service_path.append(service);
  _service = service;
  _service_path = service_path;
  _resource = resource;

  unsigned int port = iota::Process::get_process().get_http_port();
  std::string cbroker_url("http://127.0.0.1:");
  cbroker_url.append(boost::lexical_cast<std::string>(port));
  cbroker_url.append("/mock/" + _service);

  std::string post_service(
      "{\"services\": [{"
      "\"apikey\": \"" +
      _apikey + "\",\"token\": \"" + _apikey +
      "\","
      "\"cbroker\": \"" +
      cbroker_url + "\",\"entity_type\": \"" + _service +
      "\",\"resource\": \"" + resource + "\"}]}");

  boost::shared_ptr<iota::ServiceCollection> col(new iota::ServiceCollection());
  pion::http::response http_r;
  std::string r;
  ((iota::AdminService*)iota::Process::get_process().get_service(
       iota::Process::get_url_base()))
      ->delete_service_json(col, service, service_path, service, _apikey,
                            _resource, true, http_r, r, _service, _service);
  ((iota::AdminService*)iota::Process::get_process().get_service(
       iota::Process::get_url_base()))
      ->post_service_json(col, service, service_path, post_service, http_r, r,
                          _apikey, _service);
}
TestSetup::TestSetup(std::string service, std::string resource,
                     bool empty_apikey, std::string entity_name) {
  if (empty_apikey) {
    _apikey = "";
  } else {
    _apikey = service;
  }
  std::transform(service.begin(), service.end(), service.begin(), ::tolower);
  std::string service_path("/");
  service_path.append(service);
  _service = service;
  _service_path = service_path;
  _resource = resource;

  unsigned int port = iota::Process::get_process().get_http_port();
  std::string cbroker_url("http://127.0.0.1:");
  cbroker_url.append(boost::lexical_cast<std::string>(port));
  cbroker_url.append("/mock/" + _service);

  std::string post_service(
      "{\"services\": [{"
      "\"apikey\": \"" +
      _apikey + "\",\"token\": \"" + _apikey +
      "\","
      "\"cbroker\": \"" +
      cbroker_url + "\",\"entity_type\": \"" + entity_name +
      "\",\"resource\": \"" + resource + "\"}]}");

  boost::shared_ptr<iota::ServiceCollection> col(new iota::ServiceCollection());
  pion::http::response http_r;
  std::string r;
  ((iota::AdminService*)iota::Process::get_process().get_service(
       iota::Process::get_url_base()))
      ->delete_service_json(col, service, service_path, service, _apikey,
                            _resource, true, http_r, r, _service, _service);
  ((iota::AdminService*)iota::Process::get_process().get_service(
       iota::Process::get_url_base()))
      ->post_service_json(col, service, service_path, post_service, http_r, r,
                          _apikey, _service);
}
TestSetup::~TestSetup() {
  int i = 0;
  for (i = 0; i < _devices.size(); i++) {
    delete_device(_devices[i]);
  }
  pion::http::response http_r;
  std::string r;
  boost::shared_ptr<iota::ServiceCollection> col(new iota::ServiceCollection());
  ((iota::AdminService*)iota::Process::get_process().get_service(
       iota::Process::get_url_base()))
      ->delete_service_json(col, _service, _service_path, _service, _apikey,
                            _resource, true, http_r, r, _service, _service);
}
