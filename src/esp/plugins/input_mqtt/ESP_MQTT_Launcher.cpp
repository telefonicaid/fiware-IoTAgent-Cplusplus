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
#include "ESP_MQTT_Launcher.h"

ESP_MQTT_Launcher::ESP_MQTT_Launcher() {
  // ctor
  pidBroker = -1;
}

ESP_MQTT_Launcher::~ESP_MQTT_Launcher() {
  // dtor
}

int ESP_MQTT_Launcher::launchBroker() {
#ifdef WIN32
  return 0;
#else
  pidBroker = fork();

  if (pidBroker < 0) {
    return -1;
  } else if (pidBroker == 0) {
    /* this is the child */
    prctl(PR_SET_PDEATHSIG, SIGTERM);
    execv("mosquitto", NULL);
    return -2;
  }
  // parent process
  return 1;  // all good
#endif  // WIN32
}

int ESP_MQTT_Launcher::stopBroker() {
#ifdef WIN32
  return 0;
#else
  if (pidBroker > 0) {
    kill(pidBroker, SIGTERM);

    return 1;
  }
  return 0;
#endif  // WIN32
}
