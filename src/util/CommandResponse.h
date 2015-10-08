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
#ifndef SRC_UTIL_COMMANDRESPONSE_H_
#define SRC_UTIL_COMMANDRESPONSE_H_

#include <string>

namespace iota {

class CommandResponse {
 public:
  // Constructor

  CommandResponse() {}

  // Destructor
  ~CommandResponse() {}

  // Command Identifier
  void idCommand(std::string id) { _id_command = id; }
  std::string idCommand() { return _id_command; }

  // Response
  std::string getCommandResponse() { return _commandResponse; }
  void setCommandResponse(std::string response) { _commandResponse = response; }

 protected:
 private:
  // Response
  std::string _commandResponse;

  // Command Identifier
  std::string _id_command;
};
}

#endif
