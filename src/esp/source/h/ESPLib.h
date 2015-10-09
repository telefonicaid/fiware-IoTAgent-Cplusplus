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
#ifndef __ESPLIB_H__
#define __ESPLIB_H__

#include <time.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>

class ESP_Sensor;
class ESP_Runner;
class ESP_Context;
class ESP_Input_Base;
class ESP_Attribute;
class ESP_Result;

class ESPLib {
 private:
  int idsensor;

 public:
  ESPLib();
  virtual ~ESPLib();
  pthread_mutex_t mutexSensors;
  std::map<int, ESP_Sensor*> sensors;

  // Sensor
  int createSensor(std::string xmlFile);
  bool destroySensor(int id);
  bool setParams(int id, std::map<std::string, std::string> params);
  bool setUserData(int id, std::map<std::string, void*> userData);
  bool startSensor(int id, std::string command);
  bool stopSensor(int id);
  void setLoggerPath(std::string path);

  // RunOnce
  ESP_Result executeRunner(ESP_Sensor* sensor, std::string command,
                           std::map<std::string, std::string> params,
                           ESP_Input_Base* input, int inputid);
  ESP_Result executeRunnerFromBuffer(int id, std::string command,
                                     std::map<std::string, std::string> params,
                                     char* buffer, int length);
  ESP_Result executeRunnerFromRunner(int id, std::string command,
                                     std::map<std::string, std::string> params,
                                     ESP_Runner* baserunner);
  ESP_Result executeRunnerFromInput(int id, std::string command,
                                    std::map<std::string, std::string> params,
                                    ESP_Input_Base* baseinput);

  // Callbacks
  void registerDataCallback(int id, void* userData,
                            void (*cb)(void* userData, const char* buffer,
                                       int nread));
  void registerResultCallback(int id, void* userData,
                              void (*cb)(void* userData, ESP_Runner* runner));

  // Default Callbacks
  static void sensorDataCallback(void* userData, const char* buffer, int nread);
  static void sensorResultCallback(void* userData, ESP_Runner* runner);
};

#endif
