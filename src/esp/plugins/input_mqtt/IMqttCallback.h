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
#ifndef IMQTTCALLBACK_H
#define IMQTTCALLBACK_H

#include "mosquittopp.h"

class IMqttCallback {
 public:
  virtual ~IMqttCallback() {}

  virtual void on_connect(int rc) = 0;
  virtual void on_disconnect(int rc) = 0;
  virtual void on_publish(int mid) = 0;
  virtual void on_message(const struct mosquitto_message* message) = 0;
  virtual void on_subscribe(int mid, int qos_count, const int* granted_qos) = 0;
  virtual void on_unsubscribe(int mid) = 0;
  virtual void on_log(int level, const char* str) = 0;
  virtual void on_error() = 0;
};

#endif  // IMQTTCALLBACK_H
