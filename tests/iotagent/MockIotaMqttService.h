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
#ifndef MOCKCBPUBLISHER_H
#define MOCKCBPUBLISHER_H

#include "gmock/gmock.h"
#include "mqtt/IotaMqttService.h"

class MockIotaMqttService : public iota::esp::ngsi::IotaMqttService {
 public:
  virtual ~MockIotaMqttService(){};

  MOCK_METHOD3(doPublishCB,
               std::string(std::string& apikey, std::string& device,
                           std::string& json));

  MOCK_METHOD3(doPublishMultiCB,
               std::string(std::string& apikey, std::string& device,
                           std::vector<std::string>& v_json));

  MOCK_METHOD2(doRequestCommands,
               void(std::string& apikey, std::string& device));

  MOCK_METHOD3(processCommandResponse,
               void(std::string& apikey, std::string& idDevice,
                    std::string& payload));

 private:
};

#endif  // MOCKCBPUBLISHER_H
