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
#ifndef ESP_SBC_COMMAND_H
#define ESP_SBC_COMMAND_H

#include <map>
#include <string>
#include <tinyxml.h>

#define XML_PAID_COMMAND "paid:command"

class ESP_SBC_Command {
 public:
  ESP_SBC_Command(std::string inputSensor);
  virtual ~ESP_SBC_Command();
  std::string getCommandName();
  std::string getCommandID();
  std::string getDestination();
  std::string getUL20output();

  std::map<std::string, std::string> getParamMap();
  bool isParseCorrect();

 protected:
 private:
  bool parseCorrect;
  TiXmlDocument doc;
  std::string outputUL20;
  std::string name;
  std::string dest;
  std::string cmd_id;
  void parseParams(TiXmlElement* element);
  std::map<std::string, std::string> mapParams;
};

#endif  // ESP_SBC_COMMAND_H
