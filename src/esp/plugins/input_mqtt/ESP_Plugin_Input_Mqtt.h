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
#ifndef ESP_PLUGIN_INPUT_MQTT_H
#define ESP_PLUGIN_INPUT_MQTT_H

#include <TDA.h>
#include "input_mqtt/IMqttCallback.h"
#include "input_mqtt/IMosquitto.h"

#include "input_mqtt/ESP_MqttBuffer.h"
#include "input_mqtt/ESP_MQTT_Broker.h"
#include <pthread.h>
#define TYPE_PUBLISHER "publisher"
#define TYPE_SUBSCRIBER "subscriber"

using namespace mosqpp;
#define ESP_TYPE_MQTT "mqtt"
#define ESP_TYPE_MQTT_BROKER "mqttbroker"

/**
* This class will contain common operations with Mosquitto lib, like init,
cleanup, etc...
*/
class ESP_MqttWrapper : public IMqttCallback {
 private:
  bool canPublish;
  bool canSubscribe;
  ESP_MqttBuffer mqttBuffer;
  IMosquitto* mqttObj;

 public:
  int port;
  std::string host;
  std::string topicPublish;    // default publish topic
  std::string topicSubscribe;  // default subscribe

  int status;

  struct StatusType {
    enum StatusTypeEnum { MQTT_CONNECTED, MQTT_DISCONNECTED, MQTT_CONNECTING };
  };

  ESP_MqttWrapper(IMosquitto* mqtt, int port, std::string host,
                  std::string topicPub, std::string topicSub, std::string user,
                  std::string passwd);
  ~ESP_MqttWrapper();

  void doConnectHost();
  void doDisconnect();
  void doSubscribe();
  void stopSubscription();

  int publishMaskTopic(const char* buffer, int len);

  int readFromChannel();  // this is needed on any read
  int getBuffer(char* buffer, int lenToCopy);

  char* getFirstMQTTMsg(int* len);
  void doneWithFirstMQTTMsg();

  bool canIPublish() { return canPublish; };
  bool canISubscribe() { return canSubscribe; };

  // Callbacks methods
  void on_connect(int rc);
  void on_disconnect(int rc);
  void on_publish(int mid);
  void on_message(const struct mosquitto_message* message);
  void on_subscribe(int mid, int qos_count, const int* granted_qos);
  void on_unsubscribe(int mid);
  void on_log(int level, const char* str);
  void on_error();

 protected:
};

class ESP_Plugin_Input_Mqtt : public ESP_Plugin_Input_Base {
 private:
  static ESP_Plugin_Input_Mqtt* instance;
  std::string uniqueName;
  int id;

  int nBrokers;
  pthread_mutex_t mutexMqtt;

  ESP_MqttWrapper* mqttWrap;

  ESP_Plugin_Input_Mqtt();
  // This is to avoid undesirable copies
  ESP_Plugin_Input_Mqtt(ESP_Plugin_Input_Mqtt const&);
  void operator=(ESP_Plugin_Input_Mqtt const&);

  std::map<int, ESP_MqttWrapper*> mapMqttClient;

  std::map<int, ESP_MQTT_Broker*> mapMqttBroker;

  ESP_MqttWrapper* findMqtt(int id);

  bool isBrokerRunning(int port);

  // New for testing
  std::map<std::string, IMosquitto*> mapMosquitto;
  bool isThereRefMosquitto();

  IMosquitto* getAndRemoveIMosquitto(std::string inputName);

  pthread_mutex_t mutexIMosquitto;

 public:
  // New for testing.
  void setMosquitto(IMosquitto* refIMosquitto, std::string inputName);

  ~ESP_Plugin_Input_Mqtt();

  ESP_MQTT_Broker* findBroker(int port);

  static ESP_Plugin_Input_Base* getSingleton();
  static ESP_Plugin_Input_Mqtt* getInstance();
  ESP_Input_Base* createInput(TiXmlElement* element);
  int initMQTT(int port, std::string host, std::string topicPub,
               std::string topicSub, std::string user, std::string passwd,
               bool cleanSession, std::string inputName);

  int initBroker(int port, const char* path, const char* pathToConfig);
  void stopBroker(int port);

  int doSubscribe(ESP_MqttWrapper* mqtt);
  bool stopMQTT(int idClient);
  void stopSubscription(ESP_MqttWrapper* mqtt);

  int publish(int idClient, const char* msg, int len);
  int readfromSubscription(int id, char* buffer, int len);

  int getNewId() { return id++; };
  int removeClientFromMap(int idClient);

  int removeBrokerFromMap(int port);

  char* getFirstMQTTMsg(int id, int* length);
  void removeFirstMQTTMsg(int id);
};

class ESP_MqttClient {
 public:
  ESP_Context datacontext;
};

class ESP_Input_MqttBroker : public ESP_Input_Base {
 public:
  std::string host;          // host to be connected to broker
  int port;                  // port where broker is listening to (1883)
  std::string pathToBroker;  // mosquitto broker exec.
  std::string pathToConfig;  // path to mosquitto.conf file.
  ESP_Input_MqttBroker();

  int openServer();
  int acceptServer();
  bool stopServer();
  bool closeServer();

  int openClient();
  bool stopClient(int id);
  bool closeClient(int id);
  int readClient(int id, char* buffer, int len);
  int writeClient(int id, char* buffer, int len);
  void parseCustomElement(TiXmlElement* element);
};

class ESP_Input_Mqtt : public ESP_Input_Base {
 public:
  bool _keepRunning;
  std::string host;            // host to be connected to broker
  int port;                    // port where broker is listening to (1883)
  std::string topicPublish;    // topic for publishing messages.
  std::string topicSubscribe;  // topic for subscribing messages.

  std::string user;
  std::string passwd;

  int clientid;
  std::map<int, ESP_MqttClient*> clients;

  int type;  // Not sure if this makes sense here, as there will be
             // Publishers/Subscribers classes?

  bool cleanSession;  // false to tell broker we want anything that is available
                      // for us.

  ESP_Input_Mqtt();

  struct MqttType {
    enum MqttTypeEnum { MQTT_PUBLISHER, MQTT_SUBSCRIBER };
  };

  int openServer();
  int acceptServer();
  bool stopServer();
  bool closeServer();

  int openClient();
  bool stopClient(int id);
  bool closeClient(int id);
  int readClient(int id, char* buffer, int len);
  int writeClient(int id, char* buffer, int len);
  void parseCustomElement(TiXmlElement* element);
};

#endif  // ESP_PLUGIN_INPUT_MQTT_H
