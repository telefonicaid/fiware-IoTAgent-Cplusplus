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
#include "ESPLib.h"
#include "TDA.h"
#include "CC_Logger.h"
#include "input_buffer/ESP_Plugin_Input_Buffer.h"

ESPLib::ESPLib() {
  idsensor = 0;
  pthread_mutex_init(&mutexSensors, NULL);
}

ESPLib::~ESPLib() { pthread_mutex_destroy(&mutexSensors); }

int ESPLib::createSensor(std::string xmlFile) {
  CC_Logger::getSingleton()->logDebug("Loading sensor File: %s",
                                      xmlFile.c_str());

  ESP_Sensor* sensor = new ESP_Sensor();
  sensor->registerDataCallback(NULL, ESPLib::sensorDataCallback);
  sensor->registerResultCallback(NULL, ESPLib::sensorResultCallback);
  bool ok = sensor->loadFromFile(xmlFile);

  CC_Logger::getSingleton()->logDebug("Loaded sensor File: %s status=%d",
                                      xmlFile.c_str(), ok);

  if (ok) {
    pthread_mutex_init(&sensor->mutexMapServer, NULL);
    pthread_mutex_init(&sensor->mutexMapClient, NULL);

    CC_Logger::getSingleton()->logDebug(
        "Acquiring lock previous sensor was:  id %d, stored sensors: %d",
        idsensor, sensors.size());
    pthread_mutex_lock(&mutexSensors);
    int id = ++idsensor;
    sensor->_id = id;
    sensors.insert(std::pair<int, ESP_Sensor*>(id, sensor));
    pthread_mutex_unlock(&mutexSensors);

    return id;
  } else {
    return -1;
  }
}

bool ESPLib::destroySensor(int id) {
  pthread_mutex_lock(&mutexSensors);
  std::map<int, ESP_Sensor*>::iterator it = sensors.find(id);

  CC_Logger::getSingleton()->logDebug("Destroying sensor [%d]", id);
  if (it != sensors.end()) {
    // DGF: modify this also..
    // Wait for all runners to stop
    /*while (it->second->clientRunners.size() > 0 ||
    it->second->serverRunners.size() > 0)
    {
        SLEEP(15);
    }
    */
    // DGF: how do I know that servers and runners have stopped?
    // using bFinished is an option.

    // DGF: 02022015: removing this because the destructor of the sensor will
    // take care of this.
    /* if (it->second->clientRunners.size() >0) {
       it->second->clientRunners.clear();
     }

     if (it->second->serverRunners.size() >0) {
       it->second->serverRunners.clear();
     }
    */
    ESP_Sensor* sensor_to_remove = it->second;
    sensors.erase(it);

    delete sensor_to_remove;
    sensor_to_remove = NULL;

    pthread_mutex_unlock(&mutexSensors);
    return true;
  }
  pthread_mutex_unlock(&mutexSensors);
  return true;
}

bool ESPLib::setParams(int id, std::map<std::string, std::string> params) {
  return false;
}

bool ESPLib::setUserData(int id, std::map<std::string, void*> userData) {
  pthread_mutex_lock(&mutexSensors);
  std::map<int, ESP_Sensor*>::iterator it = sensors.find(id);
  pthread_mutex_unlock(&mutexSensors);
  if (it != sensors.end()) {
    it->second->setUserData(userData);
    return true;
  }
  return false;
}

bool ESPLib::startSensor(int id, std::string command) {
  pthread_mutex_lock(&mutexSensors);
  std::map<int, ESP_Sensor*>::iterator it = sensors.find(id);
  pthread_mutex_unlock(&mutexSensors);

  if (it != sensors.end()) {
    sensors[id]->run(command, std::map<std::string, std::string>());
    return true;
  }
  return false;
}

bool ESPLib::stopSensor(int id) {
  pthread_mutex_lock(&mutexSensors);
  std::map<int, ESP_Sensor*>::iterator it = sensors.find(id);
  pthread_mutex_unlock(&mutexSensors);
  if (it != sensors.end()) {
    sensors[id]->stop();
    return true;
  }
  return false;
}

ESP_Result ESPLib::executeRunner(ESP_Sensor* sensor, std::string command,
                                 std::map<std::string, std::string> params,
                                 ESP_Input_Base* input, int inputid) {
  ESP_Result result;
  ESP_Runner_Client* runner = new ESP_Runner_Client(sensor);

  runner->command = sensor->createCommand(command);
  if (runner->command != NULL) {
    runner->input = input;
    runner->_id = inputid;
    runner->context.addParams(runner->command->params);
    runner->context.addParams(params);
    runner->runOnce = true;
    result = ESP_Runner_Client::runLoop(runner);
    /*
    for (std::map<std::string, ESP_Attribute>::iterator it = mapResult.begin();
    it != mapResult.end(); it++)
    {
        result.push_back(it->second);
    }
    */
  }
  // DGF:
  delete runner;

  return result;
}

ESP_Result ESPLib::executeRunnerFromBuffer(
    int id, std::string command, std::map<std::string, std::string> params,
    char* buffer, int length) {
  ESP_Result result;
  pthread_mutex_lock(&mutexSensors);
  std::map<int, ESP_Sensor*>::iterator it = sensors.find(id);
  pthread_mutex_unlock(&mutexSensors);

  if (it != sensors.end()) {
    ESP_Input_Buffer* input = new ESP_Input_Buffer();
    int inputid = input->openClient();
    input->createInputData(buffer, length);
    result = executeRunner(it->second, command, params, input, inputid);
    delete input;
  }

  return result;
}

ESP_Result ESPLib::executeRunnerFromInput(
    int id, std::string command, std::map<std::string, std::string> params,
    ESP_Input_Base* baseinput) {
  ESP_Result result;
  pthread_mutex_lock(&mutexSensors);
  std::map<int, ESP_Sensor*>::iterator it = sensors.find(id);
  pthread_mutex_unlock(&mutexSensors);

  if (it != sensors.end() && baseinput != NULL) {
    result =
        executeRunner(it->second, command, params, baseinput, baseinput->_sid);
  }

  return result;
}

ESP_Result ESPLib::executeRunnerFromRunner(
    int id, std::string command, std::map<std::string, std::string> params,
    ESP_Runner* baserunner) {
  ESP_Result result;
  pthread_mutex_lock(&mutexSensors);
  std::map<int, ESP_Sensor*>::iterator it = sensors.find(id);
  pthread_mutex_unlock(&mutexSensors);

  if (it != sensors.end()) {
    ESP_Input_Base* input = baserunner->input;
    int inputid =
        baserunner->_id;  // Ignore id to open new connection (SHOULD BE -1)
    result = executeRunner(it->second, command, params, input, inputid);
  }

  return result;
}

void ESPLib::registerDataCallback(int id, void* userData,
                                  void (*cb)(void* userData, const char* buffer,
                                             int nread)) {
  pthread_mutex_lock(&mutexSensors);
  std::map<int, ESP_Sensor*>::iterator it = sensors.find(id);
  pthread_mutex_unlock(&mutexSensors);

  if (it != sensors.end()) {
    it->second->registerDataCallback(userData, cb);
  }
}

void ESPLib::registerResultCallback(int id, void* userData,
                                    void (*cb)(void* userData,
                                               ESP_Runner* runner)) {
  std::map<int, ESP_Sensor*>::iterator it = sensors.find(id);

  if (it != sensors.end()) {
    it->second->registerResultCallback(userData, cb);
  }
}

void ESPLib::sensorDataCallback(void* userData, const char* buffer, int nread) {
  return;
}

void ESPLib::sensorResultCallback(void* userData, ESP_Runner* runner) {
  // Printf
  // system("cls");
  for (std::vector<CC_AttributesType>::iterator it =
           runner->context.results.begin();
       it != runner->context.results.end(); it++)
    for (CC_AttributesType::iterator it2 = it->begin(); it2 != it->end();
         it2++) {
      printf("Sensor Data: %s=%s\n", it2->second.getName().c_str(),
             it2->second.getValueAsString().c_str());
    }
}

void ESPLib::setLoggerPath(std::string path) {
  CC_Logger::getSingleton()->setDirPath(path);
}
