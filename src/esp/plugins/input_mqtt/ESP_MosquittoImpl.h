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
#ifndef ESP_MOSQUITTOIMPL_H
#define ESP_MOSQUITTOIMPL_H

#include "mosquittopp.h"
#include "IMosquitto.h"

class ESP_MosquittoImpl : public IMosquitto, mosqpp::mosquittopp {
 public:
  ESP_MosquittoImpl(const char* id, bool cleanSession);
  virtual ~ESP_MosquittoImpl();

  void setCallBack(IMqttCallback* callback);
  int mqttConnect(const char* host, int port = 1883, int keepalive = 60);
  int mqttLoop(int timeout = -1, int max_packets = 1);
  int mqttPublish(int* mid, const char* topic, int payloadlen = 0,
                  const void* payload = NULL, int qos = 0, bool retain = false);
  int mqttSubscribe(int* mid, const char* sub, int qos = 0);
  int mqttUnsubscribe(int* mid, const char* sub);
  int mqttDisconnect();
  int mqttReconnect();
  int mqttSetPassword(const char* username, const char* password = NULL);

  int mqttGetVersion();

  // Methods of mosquittopp
  void on_connect(int rc);
  void on_disconnect(int rc);
  void on_publish(int mid);
  void on_message(const struct mosquitto_message* message);
  void on_subscribe(int mid, int qos_count, const int* granted_qos);
  void on_unsubscribe(int mid);
  void on_error();

 protected:
 private:
  IMqttCallback* mqttCallBack;
};

#endif  // ESP_MOSQUITTOIMPL_H
