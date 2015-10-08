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
#ifndef MOCKPLUGININPUTMQTT_H
#define MOCKPLUGININPUTMQTT_H

#include "TDA.h"
#include "gmock/gmock.h"

class MockPluginInputMqtt : public ESP_Input_Base {
 public:
  virtual ~MockPluginInputMqtt() {}

  MOCK_METHOD0(openServer, int());  // Inits id
  MOCK_METHOD0(acceptServer, int());
  MOCK_METHOD0(stopServer, bool());   // Stops accept
  MOCK_METHOD0(closeServer, bool());  // Free Resources

  MOCK_METHOD0(openClient, int());          // Inits id
  MOCK_METHOD1(stopClient, bool(int id));   // Stops client
  MOCK_METHOD1(closeClient, bool(int id));  // Free Resources
  MOCK_METHOD3(readClient, int(int id, char* buffer, int len));
  MOCK_METHOD3(writeClient, int(int id, char* buffer, int len));
  MOCK_METHOD1(parseCustomElement, void(TiXmlElement* element));

 protected:
 private:
};

#endif  // MOCKPLUGININPUTMQTT_H
