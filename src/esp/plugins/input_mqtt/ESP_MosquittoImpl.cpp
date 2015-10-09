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
#include "input_mqtt/ESP_MosquittoImpl.h"

using namespace mosqpp;

ESP_MosquittoImpl::ESP_MosquittoImpl(const char* id, bool cleanSession)
    : mosquittopp(id, cleanSession) {}

int ESP_MosquittoImpl::mqttGetVersion() { return LIBMOSQUITTO_VERSION_NUMBER; }

ESP_MosquittoImpl::~ESP_MosquittoImpl() {
  // dtor
}

void ESP_MosquittoImpl::setCallBack(IMqttCallback* callback) {
  this->mqttCallBack = callback;
}

int ESP_MosquittoImpl::mqttConnect(const char* host, int port, int keepalive) {
  return connect(host, port, keepalive);
}

int ESP_MosquittoImpl::mqttLoop(int timeout, int max_packets) {
  return loop(timeout, max_packets);
}

int ESP_MosquittoImpl::mqttPublish(int* mid, const char* topic, int payloadlen,
                                   const void* payload, int qos, bool retain) {
  return publish(mid, topic, payloadlen, payload, qos, retain);
}

int ESP_MosquittoImpl::mqttSubscribe(int* mid, const char* sub, int qos) {
  return subscribe(mid, sub, qos);
}

int ESP_MosquittoImpl::mqttUnsubscribe(int* mid, const char* sub) {
  return unsubscribe(mid, sub);
}

int ESP_MosquittoImpl::mqttDisconnect() { return disconnect(); }

int ESP_MosquittoImpl::mqttReconnect() { return reconnect(); }

int ESP_MosquittoImpl::mqttSetPassword(const char* username,
                                       const char* password) {
  return username_pw_set(username, password);
}

// Methods of mosquittopp
void ESP_MosquittoImpl::on_connect(int rc) { mqttCallBack->on_connect(rc); }

void ESP_MosquittoImpl::on_disconnect(int rc) {
  mqttCallBack->on_disconnect(rc);
}

void ESP_MosquittoImpl::on_publish(int mid) { mqttCallBack->on_publish(mid); }

void ESP_MosquittoImpl::on_message(const struct mosquitto_message* message) {
  mqttCallBack->on_message(message);
}

void ESP_MosquittoImpl::on_subscribe(int mid, int qos_count,
                                     const int* granted_qos) {
  mqttCallBack->on_subscribe(mid, qos_count, granted_qos);
}

void ESP_MosquittoImpl::on_unsubscribe(int mid) {
  mqttCallBack->on_unsubscribe(mid);
}

void ESP_MosquittoImpl::on_error() { mqttCallBack->on_error(); }
