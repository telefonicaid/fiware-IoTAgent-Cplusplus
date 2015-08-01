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

std::string get_device(std::string device_id, std::string protocol, bool endpoint) {
  const iota::JsonValue& devices = iota::DevicesFile::instance()->getDevicesObject();
  std::string device_json;
  bool has_endpoint = false;
  for (rapidjson::SizeType i = 0; i < devices.Size(); i++) {
    if (device_id.compare(devices[i]["device_id"].GetString()) == 0) {
      if (devices[i].HasMember("endpoint")) {
        has_endpoint = true;
      }
      rapidjson::StringBuffer buffer;
      rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
      devices[i].Accept(writer);
      std::cout << buffer.GetString() << std::endl;
      device_json.assign(buffer.GetString());
      break;
    }
  }


  std::string prot("\"protocol\": \"");
  prot.append(protocol);
  prot.append("\",");
  device_json.insert(1, prot);
  if (endpoint && !has_endpoint) {
    std::string port = boost::lexical_cast<std::string>(iota::Process::get_process().get_http_port());
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
  return device_json;
}

int add_device(std::string device_json, std::string service) {
  std::string service_path("/");
  service_path.append(service);
  pion::http::response http_response;
  std::string response;
  return iota::Process::get_process().get_admin_service()->post_device_json(service, service_path, device_json, http_response, response, "1234");
}

int delete_device(std::string device_id, std::string service) {
  std::string service_path("/");
  service_path.append(service);
  pion::http::response http_response;
  std::string response;
  return iota::Process::get_process().get_admin_service()->delete_device_json(service, service_path, device_id, http_response, response, "1234", "");
}

std::string get_service_name(std::string name) {
 std::string srv_name(name);
 std::transform(srv_name.begin(), srv_name.end(), srv_name.begin(), ::tolower);
 return srv_name;
}
