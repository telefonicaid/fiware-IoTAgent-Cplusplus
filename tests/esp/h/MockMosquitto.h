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
#ifndef MOCKMOSQUITTO_H
#define MOCKMOSQUITTO_H

#include "input_mqtt/IMosquitto.h"
#include "input_mqtt/IMqttCallback.h"
#include "gmock/gmock.h"

class MockMosquitto : public IMosquitto {
 public:
  IMqttCallback* callBack;

  virtual ~MockMosquitto() {}

  // Not sure if needed here like this or just mocked up but never use

  void setCallBack(IMqttCallback* callback) { this->callBack = callback; };
  MOCK_METHOD3(mqttConnect, int(const char* host, int port, int keepalive));

  MOCK_METHOD2(mqttLoop, int(int timeout, int max_packets));

  MOCK_METHOD6(mqttPublish, int(int* mid, const char* topic, int payloadlen,
                                const void* payload, int qos, bool retain));

  MOCK_METHOD3(mqttSubscribe, int(int* mid, const char* sub, int qos));

  MOCK_METHOD2(mqttUnsubscribe, int(int* mid, const char* sub));
  MOCK_METHOD0(mqttDisconnect, int());
  MOCK_METHOD0(mqttReconnect, int());
  MOCK_METHOD2(mqttSetPassword,
               int(const char* username, const char* password));

 protected:
 private:
};

#endif  // MOCKMOSQUITTO_H
