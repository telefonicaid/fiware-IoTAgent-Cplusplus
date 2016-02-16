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
#include "util/alarm.h"
#include "rest/types.h"
#include <boost/foreach.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include "util/FuncUtil.h"

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
      std::vector<iota::FloatPosition> labels_with_factor;
      try {
        BOOST_FOREACH (boost::property_tree::ptree::value_type& v_p,
                       v.second.get_child("positions")) {
          iota::FloatPosition position;

          position.name = v_p.second.get<std::string>("name", "");
          if (!position.name.empty()) {
            position.factor = v_p.second.get<float>("factor", 1);

            std::string precision = v_p.second.get<std::string>("factor", "1");

            position.precision = (short)iota::number_of_decimals(precision);

          } else {
            position.name = v_p.second.data();
            position.factor = 1;
            position.precision = 0;
          }

          labels_with_factor.push_back(position);
        }
        if (labels_with_factor.size() > 0) {
          _position_map.insert(
              std::pair<std::string, std::vector<iota::FloatPosition> >(
                  op, labels_with_factor));
          labels_with_factor.clear();
        }
      } catch (boost::exception& e) {
        // No labels
      }
    }
    try {
      _config = pt_operations.get_child("config");
    } catch (boost::exception& e) {
    }

  } catch (std::exception& e) {
    // TODO to IotaException
    iota::Alarm::error(iota::types::ALARM_CODE_BAD_CONFIGURATION, "Modbus operations", "Modbus configuration", e.what());
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
      iota::ParamsMap names_map;
      try {
        BOOST_FOREACH (boost::property_tree::ptree::value_type& v_p,
                       v.second.get_child("parameters")) {
          int address = v_p.second.get<int>("address");

          iota::CommandParameter read_param;

          read_param.name = v_p.second.get<std::string>("name");
          read_param.num_positions =
              v_p.second.get<unsigned short>("positions", 1);
          read_param.base_address = address;
          read_param.type = v_p.second.get<std::string>("type", "numeric");

          names_map.insert(
              std::pair<int, iota::CommandParameter>(address, read_param));
        }
        if (names_map.size() > 0) {
          _ordered_parameters_map.insert(
              std::pair<std::string, iota::ParamsMap>(op, names_map));
          names_map.clear();
        }
      } catch (boost::exception& e) {
        // No names
      }

      add_command_as_operation(op);
    }
  } catch (std::exception& e) {
    // TODO to IotaException
    iota::Alarm::error(iota::types::ALARM_CODE_BAD_CONFIGURATION, "Modbus commands", "Modbus configuration", e.what());
  }
}

void iota::ModbusOperationProcessor::read_operations(
    std::string modbus_operation_file) {
  std::stringstream ss;
  std::ifstream f;

  f.open(modbus_operation_file.c_str(), std::ios::binary);
  if (f.good()) {
    f.rdbuf();
    std::stringstream ss, ss1;
    ss << f.rdbuf();
    read(ss);
    f.seekg(0);
    ss1 << f.rdbuf();
    read_commands(ss1);

  } else {
    std::cout << "does not exists " << modbus_operation_file << std::endl;
  }
  f.close();
};

boost::property_tree::ptree& iota::ModbusOperationProcessor::get_operation(
    std::string operation) {
  return _operations[operation];
};

boost::property_tree::ptree& iota::ModbusOperationProcessor::get_config(){
  return _config;
};

std::vector<iota::FloatPosition>&
iota::ModbusOperationProcessor::get_mapped_labels(std::string operation) {
  return _position_map[operation];
};

boost::property_tree::ptree& iota::ModbusOperationProcessor::get_command(
    std::string command) {
  return _commands[command];
}

std::vector<iota::CommandParameter>
iota::ModbusOperationProcessor::get_mapped_parameters(std::string command) {
  std::vector<iota::CommandParameter> ordered_parameters;
  // Try
  try {
    iota::ParamsMap names_map = _ordered_parameters_map[command];
    iota::ParamsMap::iterator itr;

    for (itr = names_map.begin(); itr != names_map.end(); itr++) {
      ordered_parameters.push_back(itr->second);
    }

  } catch (std::exception& e) {
  }

  return ordered_parameters;
}

int iota::ModbusOperationProcessor::get_base_address(std::string command) {
  iota::ParamsMap names_map = _ordered_parameters_map[command];
  iota::ParamsMap::iterator itr;
  // Try
  itr = names_map.begin();

  if (itr != names_map.end()) {
    return itr->first;
  } else {
    return -1;
  }
}

std::string iota::ModbusOperationProcessor::get_protocol_commands() {
  std::string res("{\"commands\":[");
  std::string k;
  int i = 0;
  for (std::map<std::string, boost::property_tree::ptree>::iterator iter =
           _commands.begin();
       iter != _commands.end(); ++iter) {
    k = iter->first;
    if (i > 0) {
      res.append(",");
    }
    res.append("{\"name\": \"");
    res.append(k);
    res.append("\", \"type\": \"command\", \"value\": \"\"}");
    i++;
  }

  if (i == 0) {
    return "";
  } else {
    res.append("]}");
  }

  return res;
}

std::map<std::string, boost::property_tree::ptree>&
iota::ModbusOperationProcessor::get_commands() {
  return _commands;
}

void iota::ModbusOperationProcessor::add_command_as_operation(
    const std::string& command_name) {
  std::map<std::string, boost::property_tree::ptree>::iterator it_cmd =
      _commands.find(command_name);

  if (it_cmd != _commands.end()) {
    boost::property_tree::ptree& pt_command = it_cmd->second;

    boost::property_tree::ptree pt_operation;

    pt_operation.put("operation", command_name);
    pt_operation.put("modbusOperation", boost::lexical_cast<std::string>(
                                            0x03));  // Always a read operation

    std::map<std::string, iota::ParamsMap>::iterator it_params =
        _ordered_parameters_map.find(command_name);
    if (it_params != _ordered_parameters_map.end()) {
      int base_address = get_base_address(command_name);
      pt_operation.put("modbusAddress",
                       boost::lexical_cast<std::string>(base_address));

      iota::ParamsMap& params_map = it_params->second;

      int num_of_registers =
          params_map.size();  // num of registers = num of params.

      std::map<int, iota::CommandParameter>::iterator it_cmd_param;

      int num_of_positions = num_of_registers;  // Default value.
      it_cmd_param = params_map.begin();

      if (it_cmd_param != params_map.end()) {
        // ONLY one register for read operations with string/numeric with more
        // than one position as of now.
        if (it_cmd_param->second.type.compare("string") == 0 ||
            it_cmd_param->second.type.compare("numeric") == 0) {
          pt_operation.put("modbusRegisterType", it_cmd_param->second.type);
          num_of_positions = it_cmd_param->second.num_positions;
        }

      } else {
        // Complain and... throw exception?
        return;
      }

      pt_operation.put("modbusNumberOfPositions",
                       boost::lexical_cast<std::string>(num_of_positions));

      pt_operation.put("modbusNumberOfRegisters",
                       boost::lexical_cast<std::string>(num_of_registers));

      _operations.insert(std::pair<std::string, boost::property_tree::ptree>(
          command_name, pt_operation));
      std::vector<iota::FloatPosition> labels_with_factor;

      for (it_cmd_param = params_map.begin(); it_cmd_param != params_map.end();
           ++it_cmd_param) {
        iota::FloatPosition position;

        position.name = it_cmd_param->second.name;
        position.factor =
            1;  // this might be specific for commands read operations,
        // but for simplicity, let's assume the factor for command parameters is
        // applied
        // somewhere else.
        position.precision = 0;

        labels_with_factor.push_back(position);
      }
      _position_map.insert(
          std::pair<std::string, std::vector<iota::FloatPosition> >(
              command_name, labels_with_factor));
    }

  }  // Else error
}
