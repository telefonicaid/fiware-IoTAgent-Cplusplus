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
#ifndef IMOSQUITTO_H
#define IMOSQUITTO_H

#include "IMqttCallback.h"

class IMosquitto {
 public:
  IMosquitto(){};
  virtual ~IMosquitto(){};

  virtual void setCallBack(IMqttCallback* callback) = 0;
  virtual int mqttConnect(const char* host, int port = 1883,
                          int keepalive = 60) = 0;
  virtual int mqttLoop(int timeout = -1, int max_packets = 1) = 0;
  virtual int mqttPublish(int* mid, const char* topic, int payloadlen = 0,
                          const void* payload = NULL, int qos = 0,
                          bool retain = false) = 0;
  virtual int mqttSubscribe(int* mid, const char* sub, int qos = 0) = 0;
  virtual int mqttUnsubscribe(int* mid, const char* sub) = 0;
  virtual int mqttDisconnect() = 0;
  virtual int mqttReconnect() = 0;
  virtual int mqttSetPassword(const char* username,
                              const char* password = NULL) = 0;

  virtual int mqttGetVersion() { return 0; }

  // more and more
};

#endif  // IMOSQUITTO_H
