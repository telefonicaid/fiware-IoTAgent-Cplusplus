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
#include "input_mqtt/ESP_MQTT_Broker.h"
#include <iostream>
#include "CC_Logger.h"
#include <errno.h>

ESP_MQTT_Broker::ESP_MQTT_Broker() {
  // ctor
  pidBroker = -1;
}

ESP_MQTT_Broker::~ESP_MQTT_Broker() {
  // dtor
}

int ESP_MQTT_Broker::launchBroker(const char* executable,
                                  const char* pathToConfig) {
#ifdef WIN32
  CC_Logger::getSingleton()->logError(
      "Broker can't be launched as a process in Windows");
  return 0;
#else
  pidBroker = fork();

  if (pidBroker < 0) {
    CC_Logger::getSingleton()->logError("FORK Failed");
    return -1;
  } else if (pidBroker == 0) {
    char buffer[256] = {0};
    getcwd(buffer, 256);
    std::string arguments1 = "-c";

    /* child args */
    char* arg[4];

    arg[0] = (char*)malloc(strlen(executable) + 1);
    strcpy(arg[0], executable);
    arg[1] = (char*)malloc(arguments1.length() + 1);
    strcpy(arg[1], arguments1.c_str());

    arg[2] = (char*)malloc(strlen(pathToConfig) + 1);
    strcpy(arg[2], pathToConfig);

    arg[3] = 0;

    /* this is the child */
    prctl(PR_SET_PDEATHSIG, SIGTERM);
    execv(executable, arg);

    free(arg[0]);
    free(arg[1]);
    free(arg[2]);
    CC_Logger::getSingleton()->logError("FATAL Error: broker couldn't start");
    std::cerr << "ERROR: broker could not start, child exiting\n";
    exit(-1);
  }

  CC_Logger::getSingleton()->logDebug("Child process : %d", pidBroker);
  // parent process
  return 1;  // all good
#endif  // WIN32
}

void ESP_MQTT_Broker::waitForBroker() {
#ifndef WIN32
  int res;
  if (pidBroker < 0) {
    return;
  }
  CC_Logger::getSingleton()->logDebug("WAITING FOR BROKER TO TERMINATE");
  wait(&res);
  CC_Logger::getSingleton()->logDebug("BROKER TERMINATED");
#endif  // WIN32
}

int ESP_MQTT_Broker::stopBroker() {
#ifdef WIN32
  return 0;
#else
  if (pidBroker > 0) {
    CC_Logger::getSingleton()->logDebug("Stopping child process");
    kill(pidBroker, SIGTERM);

    return 1;
  }
  return 0;
#endif  // WIN32
}
