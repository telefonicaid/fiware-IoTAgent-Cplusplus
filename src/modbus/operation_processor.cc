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
    BOOST_FOREACH (boost::property_tree::ptree::value_type& v,
                   pt_operations.get_child("operations")) {
      std::string op = v.second.get<std::string>("operation");
      _operations.insert(
          std::pair<std::string, boost::property_tree::ptree>(op, v.second));
      std::vector<std::string> labels;
      try {
        BOOST_FOREACH (boost::property_tree::ptree::value_type& v_p,
                       v.second.get_child("positions")) {
          labels.push_back(v_p.second.data());
        }
        if (labels.size() > 0) {
          _position_map.insert(
              std::pair<std::string, std::vector<std::string> >(op, labels));
          labels.clear();
        }
      } catch (boost::exception& e) {
        // No labels
      }
    }
  } catch (boost::exception& e) {
    // TODO to IotaException
    std::cout << boost::diagnostic_information(e) << std::endl;
  }
}

void iota::ModbusOperationProcessor::read_commands(
    std::stringstream& json_commands) {
  boost::property_tree::ptree pt_commands;
  try {
    boost::property_tree::read_json(json_commands, pt_commands);
    BOOST_FOREACH (boost::property_tree::ptree::value_type& v,
                   pt_commands.get_child("commands")) {
      std::string op = v.second.get<std::string>("name");
      _commands.insert(
          std::pair<std::string, boost::property_tree::ptree>(op, v.second));
      std::map<int, std::string> names_map;
      try {
        BOOST_FOREACH (boost::property_tree::ptree::value_type& v_p,
                       v.second.get_child("parameters")) {
          int address = v_p.second.get<int>("address");
          std::string name = v_p.second.get<std::string>("name");
          names_map.insert(std::pair<int, std::string>(address, name));
        }
        if (names_map.size() > 0) {
          _position_map.insert(
              std::pair<std::string, std::map<int, std::string> >(op,
                                                                  names_map));
          names_map.clear();
        }
      } catch (boost::exception& e) {
        // No names
      }
    }
  } catch (boost::exception& e) {
    // TODO to IotaException
    std::cout << boost::diagnostic_information(e) << std::endl;
  }
}

void iota::ModbusOperationProcessor::read_operations(
    std::string modbus_operation_file) {
  std::stringstream ss;
  std::ifstream f;

  f.open(modbus_operation_file.c_str(), std::ios::binary);
  if (f.good()) {
    f.rdbuf();
    std::stringstream ss;
    ss << f.rdbuf();
    read(ss);
    read_commands(ss);

  } else {
    std::cout << "does not exists " << modbus_operation_file << std::endl;
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

boost::property_tree::ptree& iota::ModbusOperationProcessor::get_command(
    std::string command) {
  return _commands[command];
}

std::vector<std::string> iota::ModbusOperationProcessor::get_mapped_parameters(
    std::string command) {
  std::vector<std::string> ordered_parameters;
  // Try
  std::map<int, std::string> names_map = _ordered_parameters_map[command];
  std::map<int, string>::iterator itr;
  for (itr = names_map.begin(); itr != names_map.end(); itr++) {
    ordered_parameters.push_back(itr->second);
  }
  return ordered_parameters;
}

int iota::ModbusOperationProcessor::get_base_address(std::string command) {
  std::map<int, std::string> names_map = _ordered_parameters_map[command];
  std::map<int, string>::iterator itr;
  // Try
  itr = names_map.begin();
  // if not null

  return itr->first;
}
