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
#ifndef SRC_UTIL_IOTA_EXCEPTION_H_
#define SRC_UTIL_IOTA_EXCEPTION_H_
#include <stdexcept>
namespace iota {
class IotaException : public std::runtime_error {
 public:
  IotaException(const std::string& reason, const std::string& details,
                unsigned int status_code)
      : std::runtime_error(details.c_str()),
        _reason(reason),
        _status(status_code){};
  virtual ~IotaException() throw(){};
  std::string reason() { return _reason; };
  unsigned int status() { return _status; };
  virtual const char* what() const throw() {
    return std::runtime_error::what();
  };

 private:
  std::string _reason;
  unsigned int _status;
};
}
#endif
