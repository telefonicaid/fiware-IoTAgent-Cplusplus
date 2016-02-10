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

#include "modbus.h"
#include "rest/process.h"
#include "util/FuncUtil.h"
#include <iostream>

/* Table of CRC values for high-order byte */
static const unsigned char table_crc_hi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40};

/* Table of CRC values for low-order byte */
static const unsigned char table_crc_lo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
    0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
    0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
    0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
    0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
    0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
    0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
    0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
    0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
    0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
    0x41, 0x81, 0x80, 0x40};

iota::Modbus::Modbus(unsigned char slave_addr,
                     iota::Modbus::FunctionCode function_code,
                     unsigned short address_data, unsigned short number_of,
                     std::string app_operation)
    : _slave_address(slave_addr),
      _function_code(function_code),
      _address_data(address_data),
      _number_of_or_value(number_of),
      _app_operation(app_operation),
      _finished(false),
      _completed(false),
      _time_instant(-1),
      _confirmed(true),
      _need_be_confirmed(false),
      m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  _modbus_frame.push_back(_slave_address);
  _modbus_frame.push_back(_function_code);
  _modbus_frame.push_back((_address_data >> 8) & 0xFF);
  _modbus_frame.push_back(_address_data & 0xFF);
  _modbus_frame.push_back((_number_of_or_value >> 8) & 0xFF);
  _modbus_frame.push_back(_number_of_or_value & 0xFF);
  unsigned short crc_frame = crc(_modbus_frame);
  _modbus_frame.push_back((crc_frame >> 8) & 0xFF);
  _modbus_frame.push_back(crc_frame & 0xFF);
}

iota::Modbus::Modbus(unsigned char slave_addr,
                     iota::Modbus::FunctionCode function_code,
                     unsigned short address_data, std::vector<unsigned short> values,
                     std::string app_operation, bool confirmed)
    : _slave_address(slave_addr),
      _function_code(function_code),
      _address_data(address_data),
      _values_to_write(values),
      _app_operation(app_operation),
      _finished(false),
      _completed(false),
      _time_instant(-1),
      _confirmed(!confirmed),
      _need_be_confirmed(confirmed),
      _number_of_or_value(0),
      m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  _modbus_frame.push_back(_slave_address);
  _modbus_frame.push_back(_function_code);
  _modbus_frame.push_back((_address_data >> 8) & 0xFF);
  _modbus_frame.push_back(_address_data & 0xFF);
  _modbus_frame.push_back((values.size() >> 8) & 0xFF);
  _modbus_frame.push_back(values.size());
  _modbus_frame.push_back((values.size() * 2) & 0xFF);
  for (int i = 0; i < values.size(); i++) {
    _modbus_frame.push_back((values.at(i) >> 8) & 0xFF);
    _modbus_frame.push_back(values.at(i) & 0xFF);
  }
  unsigned short crc_frame = crc(_modbus_frame);
  _modbus_frame.push_back((crc_frame >> 8) & 0xFF);
  _modbus_frame.push_back(crc_frame & 0xFF);
}

// As master only responses
/*
iota::Modbus::Modbus(std::vector<unsigned char>& mb_msg) {
  receive_modbus_frame(mb_msg);
}
*/
void iota::Modbus::set_time_instant(std::time_t timestamp) {
  _time_instant = timestamp;
}

std::time_t iota::Modbus::get_time_instant() { return _time_instant; }

bool iota::Modbus::receive_modbus_frame(
    const std::vector<unsigned char>& mb_msg) {
  // Reading
  bool frame_ok = true;

  std::string str_frame = iota::str_to_hex(mb_msg);
  IOTA_LOG_DEBUG(m_logger, str_frame);
  std::string error_frame;
  if (_modbus_frame.size() == 0) {
    // No request
    frame_ok = false;
    error_frame = "Empty modbus frame";

  } else {
    try {

        IOTA_LOG_DEBUG(m_logger, "Checking if completed");
        _completed = check_completed(mb_msg);
        if (_completed) {
          _finished = true;

          // Check CRC in completed frame
          if (_modbus_frame_response.at(0) != _slave_address ||
              ((iota::Modbus::FunctionCode)_modbus_frame_response.at(1)) != _function_code ||
              !check_crc(_modbus_frame_response)) {
            frame_ok = false;
            error_frame = "Bad modbus frame";
          } else if (_function_code == iota::Modbus::READ_HOLDING_REGISTERS) {

            unsigned short bytes = _modbus_frame_response.at(2);
            unsigned short num_reg = 0;
            int i = 3;
            IOTA_LOG_DEBUG(
                m_logger, "Reading " + boost::lexical_cast<std::string>(bytes) +
                              " bytes from position " +
                              boost::lexical_cast<std::string>(_address_data));
            // Additional check
            // Last byte must be before CRC
            if (_modbus_frame_response.size() - 2 != bytes + 3) {
              IOTA_LOG_INFO(m_logger, "Bad frame?");
              error_frame = "Modbus read holding registers incomplete";
              frame_ok = false;
            }
            while (frame_ok == true && i < bytes + 3) {
              unsigned short value = (_modbus_frame_response.at(i) << 8) | _modbus_frame_response.at(i + 1);
              ++num_reg;
              IOTA_LOG_DEBUG(m_logger,
                             "Read value [" + boost::lexical_cast<std::string>(
                                                  _address_data + num_reg) +
                                     "] "
                                 << boost::lexical_cast<std::string>(value));
              _values.insert(std::pair<unsigned short, unsigned short>(
                  _address_data + num_reg, value));
              i += 2;
            }
            IOTA_LOG_DEBUG(m_logger,
                           "Read " + boost::lexical_cast<std::string>(num_reg) +
                               " registers");

          } else if (_function_code == iota::Modbus::PRESET_SINGLE_REGISTER) {
            unsigned short addr_response = (_modbus_frame_response.at(2) << 8) | _modbus_frame_response.at(3);
            unsigned short value = (_modbus_frame_response.at(4) << 8) | _modbus_frame_response.at(5);
            if (addr_response != _address_data ||
                value != _number_of_or_value) {
              frame_ok = false;
              error_frame =
                  "Modbus preset single register response is not request echo";
            }
          } else if (_function_code == iota::Modbus::PRESET_MULTIPLE_REGISTER) {
            if (!_confirmed) {
              frame_ok = false;
              error_frame =
                  "Modbus preset multiple register response is not confirmed with request echo";
            } else {
              unsigned short addr_response = (_modbus_frame_response.at(2) << 8) | _modbus_frame_response.at(3);
              unsigned short values = (_modbus_frame_response.at(4) << 8) | _modbus_frame_response.at(5);
              if (addr_response != _address_data ||
                  values != _values_to_write.size() ||
                  _modbus_frame_response.size() != 8) {
                frame_ok = false;
                error_frame =
                      "Modbus preset multiple register response is a bad frame";
              }
            }
          }
        }
      //}
    } catch (std::exception& e) {
      // Bad frame
      frame_ok = false;
      error_frame = "Modbus internal error ";
      error_frame.append(e.what());
    }
  }
  if (!frame_ok) {
    _finished = true;
    _completed = true;
  }
  if (!error_frame.empty()) {
    error_frame += " ";
    error_frame += str_frame;
    IOTA_LOG_ERROR(m_logger, error_frame);
  }
  return frame_ok;
}

std::vector<unsigned char>& iota::Modbus::get_modbus_frame() {
  return _modbus_frame;
}

std::vector<unsigned char>& iota::Modbus::get_modbus_frame_response() {
  return _modbus_frame_response;
}

bool iota::Modbus::check_crc(const std::vector<unsigned char>& frame) {
  bool c_ok = true;
  std::vector<unsigned char> f(frame.begin(), frame.end() - 2);
  unsigned short received_crc =
      (frame.at(frame.size() - 2) << 8) | frame.at(frame.size() - 1);
  unsigned short calculated_crc = crc(f);
  if (received_crc != calculated_crc) {
    c_ok = false;
  }
  return c_ok;
}

bool iota::Modbus::check_completed(const std::vector<unsigned char>& frame) {
  bool completed = false;
  _modbus_frame_response.insert(_modbus_frame_response.end(), frame.begin(), frame.end());
  std::string str_frame = iota::str_to_hex(_modbus_frame_response);
  IOTA_LOG_DEBUG(m_logger, "Checking if completed " + str_frame);
  if (_function_code == iota::Modbus::READ_HOLDING_REGISTERS) {
    if (_modbus_frame_response.size() >= 3) {
      unsigned short bytes = _modbus_frame_response.at(2);
      if (_modbus_frame_response.size() - 2 == bytes + 3) {
        completed = true;
      }
    }
  }
  else if (_function_code == iota::Modbus::PRESET_SINGLE_REGISTER) {
    // Echo request, same size
    if (_modbus_frame_response.size() == _modbus_frame.size()) {
      completed = true;
    }
  }
  else if (_function_code == iota::Modbus::PRESET_MULTIPLE_REGISTER) {

    // response may be length fixed.
    if (!_need_be_confirmed || (_need_be_confirmed && _confirmed)) {
      if (_modbus_frame_response.size() >= 8) {
        completed = true;
      }
    }
    if (_need_be_confirmed && !_confirmed) {
      IOTA_LOG_DEBUG(m_logger, "Frame need be confirmed " + str_frame);
      if (_modbus_frame_response.size() == _modbus_frame.size()) {
        if (std::equal(_modbus_frame_response.begin(),
                       _modbus_frame_response.end(),
                       _modbus_frame.begin())) {
          _confirmed = true;
          _modbus_frame_response.clear();
        } else {
          // This frame does not receive echo. This operation is completed.
          completed = true;
        }
      } else if (_modbus_frame_response.size() > _modbus_frame.size()) {
        // This frame does not receive echo. This operation is completed.
        completed = true;
      }
    }
  }

  IOTA_LOG_DEBUG(m_logger, "Modbus frame completed/confirmed? " + boost::lexical_cast<std::string>(completed));
  return completed;
}

unsigned short iota::Modbus::crc(std::vector<unsigned char>& frame) {
  unsigned char crc_hi = 0xFF; /* high CRC byte initialized */
  unsigned char crc_lo = 0xFF; /* low CRC byte initialized */
  unsigned int i;              /* will index into CRC lookup */
  int buffer_length = frame.size();
  unsigned int pos = 0;
  /* pass through message buffer */
  while (pos < buffer_length) {
    i = crc_hi ^ frame[pos]; /* calculate the CRC  */
    crc_hi = crc_lo ^ table_crc_hi[i];
    crc_lo = table_crc_lo[i];
    pos++;
  }

  return (crc_hi << 8 | crc_lo);
}

std::map<unsigned short, unsigned short> iota::Modbus::get_values() {
  return _values;
}

std::map<std::string, std::string> iota::Modbus::get_mapped_values(
    std::vector<iota::FloatPosition>& mapped_fields) {
  // Original values are map but unsigned short and ordered.
  std::map<std::string, std::string> mapped_values;

  std::map<unsigned short, unsigned short>::iterator it_values =
      _values.begin();
  if (mapped_fields.size() == 0) {
    // Set labels to key in values map
    while (it_values != _values.end()) {
      mapped_values.insert(std::pair<std::string, std::string>(
          boost::lexical_cast<std::string>(it_values->first),
          boost::lexical_cast<std::string>(it_values->second)));
      ++it_values;
    }
  } else if (mapped_fields.size() > 0 &&
             mapped_fields.size() == _values.size()) {
    int i = 0;
    for (i = 0; i < mapped_fields.size(); i++) {
      std::stringstream ss;
      ss << it_values->first;
      std::string str_field = ss.str();
      if (!mapped_fields.at(i).name.empty()) {
        str_field = mapped_fields.at(i).name;
        double factor = mapped_fields.at(i).factor;

        std::stringstream ss_value;
        double temp = factor * it_values->second;
        ss_value << std::fixed
                 << std::setprecision(mapped_fields.at(i).precision) << temp;

        mapped_values.insert(
            std::pair<std::string, std::string>(str_field, ss_value.str()));
      }
      ++it_values;
    }
  }
  return mapped_values;
}
