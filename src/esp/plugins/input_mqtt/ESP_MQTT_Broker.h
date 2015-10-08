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
#ifndef ESP_MQTT_BROKER_H
#define ESP_MQTT_BROKER_H

#include <stdlib.h>
#include "sched.h"

#ifndef WIN32
#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#endif  // WIN32

/*
* This class is a wrapper for controlling MQTT Broker as a separate process.
This code will work ONLY in Linux systems. We might want to implement this for
Windows
also.
The right order to call the methods is:
1. LaunchBroker, which will fork and call mosquitto broker binary.
2. WaitForBroker, a method that will execute a "wait for pid" and block the
THREAD
where it's running on. So with this, we wait for the child process to terminate,
either
successfully or abnormally (in fact, this can be whenever ports are in used or
similar
so it's not really an abnormal situation). The idea is that if mosquitto broker
can't run,
then the runner thread will just close (but this is controlled by the calling
thread).
3. Finally, stopBroker, to force the stop of the broker. This will send a
SIGTERM to the
mosquitto broker process.
*/
class ESP_MQTT_Broker {
 public:
  ESP_MQTT_Broker();
  int launchBroker(const char* exectuable, const char* pathToConfig);
  int stopBroker();
  void waitForBroker();
  virtual ~ESP_MQTT_Broker();

 protected:
 private:
  pid_t pidBroker;
};

#endif  // ESP_MQTT_BROKER_H
