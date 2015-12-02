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

#ifndef TESTS_IOTAGENT_UTIL_TEST_H
#define TESTS_IOTAGENT_UTIL_TEST_H

#include <string>
#include <map>
#include <vector>

std::string get_service_name(std::string name);

class TestSetup {
 public:
  TestSetup(std::string service, std::string resource,
            std::string my_post_service = std::string());
  TestSetup(std::string service, std::string resource, bool empty_apikey);
  TestSetup(std::string service, std::string resource, bool empty_apikey, std::string entity_type);
  ~TestSetup();
  std::string get_service() { return _service; };
  void set_service_path(std::string service_path) {
    _service_path = service_path;
  };
  std::string get_service_path() { return _service_path; };
  void set_apikey(std::string apikey) { _apikey = apikey; };
  std::string get_apikey() { return _apikey; };
  std::string get_device(std::string device_id, std::string protocol,
                         bool endpoint = false);
  int add_device(std::string device_id, std::string protocol,
                 bool endpoint = false, std::string service = std::string(),
                 std::string service_path = std::string());
  int delete_device(std::string device_id, std::string service = std::string(),
                    std::string service_path = std::string());
  static std::map<std::string, std::string> _DEVICES_UT_;

 private:
  std::string _service;
  std::string _service_path;
  std::string _apikey;
  std::string _resource;
  std::vector<std::string> _devices;
};
#endif
