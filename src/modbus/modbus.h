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
#ifndef SRC_MODBUS_MODBUS_H_
#define SRC_MODBUS_MODBUS_H_

#include "util/iota_logger.h"
#include <string>
#include <vector>
#include <map>
#include "operation_processor.h"

namespace iota {

class Modbus {
 public:
  typedef enum {
    // READ_COIL_STATUS = 0x01,
    // READ_INPUT_STATUS = 0x02,
    READ_HOLDING_REGISTERS = 0x03,
    // READ_INPUT_REGISTERS = 0x04,
    // FORCE_SINGLE_COIL = 0x05,
    PRESET_SINGLE_REGISTER = 0x06,
    PRESET_MULTIPLE_REGISTER = 0x010
  } FunctionCode;
  Modbus(){};
  Modbus(unsigned char slave_addr, iota::Modbus::FunctionCode function_code,
         unsigned short address_data, unsigned short number_of_or_value,
         std::string app_operation);

  // This constructor is for writting multiple registers
  Modbus(unsigned char slave_addr, iota::Modbus::FunctionCode function_code,
         unsigned short address_data, std::vector<unsigned short> values,
         std::string app_operation, bool confirmed);
  // Modbus(std::vector<unsigned char>& mb_msg);
  virtual ~Modbus(){};
  std::vector<unsigned char>& get_modbus_frame();
  std::vector<unsigned char>& get_modbus_frame_response();
  unsigned char get_slave_address() { return _slave_address; };
  FunctionCode get_function_code() { return _function_code; };
  unsigned short get_address_data() { return _address_data; };
  unsigned short get_number_of_or_value() { return _number_of_or_value; };
  bool receive_modbus_frame(const std::vector<unsigned char>& mb_msg);
  std::map<unsigned short, unsigned short> get_values();

  std::map<std::string, std::string> get_mapped_values(
      std::vector<iota::FloatPosition>& mapped_fields);
  unsigned short crc(std::vector<unsigned char>& frame);
  std::string get_operation() { return _app_operation; };

  bool finished() { return _finished; };
  void finish() { _finished = true; };

  // Manage no complete frames
  bool completed() { return _completed;};

  // Check if frame need confirmation.
  bool need_be_confirmed() { return _need_be_confirmed;};

  void set_time_instant(std::time_t timestamp);
  std::time_t get_time_instant();

 protected:
 private:
  std::string _app_operation;
  std::vector<unsigned char> _modbus_frame;
  //Response when no complete
  std::vector<unsigned char> _modbus_frame_response;
  unsigned char _slave_address;
  iota::Modbus::FunctionCode _function_code;
  unsigned short _address_data;
  unsigned short _number_of_or_value;
  std::vector<unsigned short> _values_to_write;
  std::map<unsigned short, unsigned short> _values;
  bool _finished;
  bool _completed;

  // In some implementations, MULTIPLE_REGISTER are confirmed with echo
  bool _confirmed;
  bool _need_be_confirmed;

  // To store timestamp
  std::time_t _time_instant;

  // Logger
  pion::logger m_logger;

  // Check received crc
  bool check_crc(const std::vector<unsigned char>& frame);

  // Check if frame is completed
  bool check_completed(const std::vector<unsigned char>& mb_msg);
};
}
#endif
