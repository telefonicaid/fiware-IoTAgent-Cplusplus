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
#ifndef ESP_MQTTBUFFER_H
#define ESP_MQTTBUFFER_H

#include "mosquittopp.h"
#include <queue>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <map>

#define ESP_MQTT_DELIM_START '$'
#define ESP_MQTT_DELIM_END "//"
// static const char MQTT_TOPIC_START = ESP_MQTT_DELIM_START;
// static std::string MQTT_TOPIC_END = ESP_MQTT_DELIM_END;

class ESP_MqttMsg {
 public:
  ESP_MqttMsg(const struct mosquitto_message* mqttMsg);
  virtual ~ESP_MqttMsg();

  int getTotalLength();
  int getRemainingLength();

  void moveOffset(int nbytes);
  char* getBufferToBeCopied(int* len);

 protected:
 private:
  // Optionally, I could store the original Mqtt message structure.
  int readOffset;  // marker to indicate where read operation will  copy
  // bytes from. Required in case of subsequent partial reads.

  char* buffSerialized;  // buffer to be read with serialized message
  int length;            // length of this buffer
};

class ESP_MqttBuffer {
 public:
  ESP_MqttBuffer() { unreadBytes = 0; };
  virtual ~ESP_MqttBuffer();

  // it returns number of bytes read.
  int getSerialized(char* buffOut, int lenRead);

  char* getFirstMqttMessage(int* length);
  void deleteFirstMessage();

  void addMsg(ESP_MqttMsg* newMsg);

  void removeMsg();

  void decreaseUnreadBytes(int bytes) { unreadBytes -= bytes; };

  int getUnreadBytes() { return unreadBytes; };
  bool isQueueEmpty() { return qMqttMsg.empty(); };

 protected:
 private:
  std::queue<ESP_MqttMsg*> qMqttMsg;

  int unreadBytes;  // this will have number of bytes can be read in one go
  // it doesn't necessarily mean that read operation has to
  // meet this number, but in case it tries to exceed it,
  // this will be the actual length of bytes returned.
  // Until first read action, this is just the length of a
  //"virtual" byte array of topics and payload all serialized.
};

#endif  // ESP_MQTTBUFFER_H
