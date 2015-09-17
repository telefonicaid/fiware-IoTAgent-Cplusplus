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

#include "operation_processor.h"
#include <boost/foreach.hpp>
#include <boost/exception/diagnostic_information.hpp>
iota::ModbusOperationProcessor::ModbusOperationProcessor(
  std::stringstream& json_operations) {
  read(json_operations);
};

void iota::ModbusOperationProcessor::read(std::stringstream& json_operations) {

  boost::property_tree::ptree pt_operations;
  try {
    boost::property_tree::read_json(json_operations, pt_operations);
    BOOST_FOREACH(boost::property_tree::ptree::value_type& v,
                  pt_operations.get_child("operations")) {
      std::string op = v.second.get<std::string>("operation");
      _operations.insert(std::pair<std::string, boost::property_tree::ptree>(op,
                         v.second));
      std::vector<std::string> labels;
      try {
        BOOST_FOREACH(boost::property_tree::ptree::value_type& v_p,
                      v.second.get_child("positions")) {
          labels.push_back(v_p.second.data());
        }
        if (labels.size() > 0) {
          _position_map.insert(std::pair<std::string, std::vector<std::string> >(op,
                               labels));
          labels.clear();
        }
      }
      catch (boost::exception& e) {
        // No labels
      }
    }
  }
  catch (boost::exception& e) {
    // TODO to IotaException
    std::cout << boost::diagnostic_information(e) << std::endl;
  }
}

void iota::ModbusOperationProcessor::read_operations(std::string modbus_operation_file) {
  std::stringstream ss;
  std::ifstream f;;
  f.open(modbus_operation_file.c_str(), std::ios::binary);
  if (f.good()) {
    f.rdbuf();
    std::stringstream ss;
    ss << f.rdbuf();
    read(ss);
  }
  else {
    std::cout <<  "does not exists " << modbus_operation_file << std::endl;
  }
  f.close();

};

boost::property_tree::ptree& iota::ModbusOperationProcessor::get_operation(
  std::string operation) {
  return _operations[operation];
};

std::vector<std::string>& iota::ModbusOperationProcessor::get_mapped_labels(
  std::string operation) {
  return _position_map[operation];
};

