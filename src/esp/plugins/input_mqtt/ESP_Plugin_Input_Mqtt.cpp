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
#include "input_mqtt/ESP_Plugin_Input_Mqtt.h"
#include "ESP_XmlUtils.h"
#include "ESP_StringUtils.h"
#include "CC_Logger.h"
#include "ESP_StringUtils.h"
#include <pthread.h>

#ifdef USE_MQTT
#include "input_mqtt/ESP_MosquittoImpl.h"
#endif  // USE_MQTT

ESP_Plugin_Input_Mqtt* ESP_Plugin_Input_Mqtt::instance = NULL;

ESP_MqttWrapper::ESP_MqttWrapper(IMosquitto* mqtt, int port, std::string host,
                                 std::string topicPub, std::string topicSub,
                                 std::string user, std::string passwd) {
  this->port = port;
  this->host = host;
  this->topicPublish = topicPub;
  this->topicSubscribe = topicSub;

  this->mqttObj = mqtt;

  (topicPub.compare("") == 0) ? canPublish = false : canPublish = true;
  (topicSub.compare("") == 0) ? canSubscribe = false : canSubscribe = true;

  status = StatusType::MQTT_CONNECTING;

  if (user.length() > 0) {
    // username_pw_set(user.c_str(),passwd.c_str());
    mqttObj->mqttSetPassword(user.c_str(), passwd.c_str());
  }
}

void ESP_MqttWrapper::doConnectHost() {
  int res;

  CC_Logger::getSingleton()->logDebug(
      "doConnectHost : status %s",
      status == StatusType::MQTT_CONNECTING
          ? "Connecting"
          : (status == StatusType::MQTT_DISCONNECTED ? "Disconnected"
                                                     : "Connected"));

  if (status == StatusType::MQTT_CONNECTING) {
    CC_Logger::getSingleton()->logDebug(
        "LIB MQTT trying connection to host %s:%d.... ", host.c_str(), port);
    // status = StatusType::MQTT_CONNECTING;

    // res = connect(host.c_str(),port,60);
    res = mqttObj->mqttConnect(host.c_str(), port, 60);
  } else if (status == StatusType::MQTT_DISCONNECTED) {
    // res = reconnect();
    CC_Logger::getSingleton()->logDebug("Trying to reconnect");
    res = mqttObj
              ->mqttReconnect();  // it should jump to on_connection if succeeds
  }

  // loop(100,1);
  mqttObj->mqttLoop(100, 1);

  if (MOSQ_ERR_SUCCESS == res) {
    CC_Logger::getSingleton()->logDebug(
        "LIB MQTT connecting to host %s:%d.... DONE", host.c_str(), port);

    doSubscribe();
  }
}

int ESP_MqttWrapper::readFromChannel() {
  int rc = 0;

  // This will force messages to be pulled or pushed in MQTT layer

  rc = mqttObj->mqttLoop(0, 1);

  // Problem: what should be the reconnecting policy? always try to reconnect?
  if (rc != MOSQ_ERR_SUCCESS) {
    doConnectHost();
    SLEEP(1000);
  }

  return rc;
}

char* ESP_MqttWrapper::getFirstMQTTMsg(int* len) {
  return mqttBuffer.getFirstMqttMessage(len);
}

void ESP_MqttWrapper::doneWithFirstMQTTMsg() {
  mqttBuffer.deleteFirstMessage();
}

// Do I need to synchronize the read of bytes that happens "asynchronously" and
// the retrieval of
// the buffer?
int ESP_MqttWrapper::getBuffer(char* buffer, int lenToCopy) {
  return mqttBuffer.getSerialized(buffer, lenToCopy);
}

int ESP_MqttWrapper::publishMaskTopic(const char* buffer, int len) {
  // If connected
  if (status == StatusType::MQTT_CONNECTED) {
    int idMsg = -1;
    std::string str = std::string(buffer, len);
    std::string topic, payload;

    CC_Logger::getSingleton()->logDebug("MQTT Publish: %s",
                                        std::string(buffer, len).c_str());

    // Split
    std::string token = "//";
    size_t tpos = str.find(token);
    if (tpos != std::string::npos) {
      topic = std::string(buffer, tpos);
      payload = std::string(buffer + tpos + token.length(),
                            len - tpos - token.length());
    }

    // Publish

    int res = mqttObj->mqttPublish(&idMsg, topic.c_str(), payload.length(),
                                   payload.c_str(), 0, false);

    mqttObj->mqttLoop(100, 1);

    if (res == MOSQ_ERR_SUCCESS) {
      return len;
    }
  }
  return 0;
}

ESP_MqttWrapper::~ESP_MqttWrapper() {
  /*
  CC_Logger::getSingleton()->logDebug("LIB MQTT stopping.... ");
  mosqpp::lib_cleanup();
  CC_Logger::getSingleton()->logDebug("LIB MQTT stopping.... DONE");
  */
  if (NULL != mqttObj) {
    delete mqttObj;  // It seems there's no better place to delete IMosquitto
                     // object.
    mqttObj = NULL;
  }
}

void ESP_MqttWrapper::on_connect(int rc) {
  CC_Logger::getSingleton()->logDebug(
      "Connection established with %s:%d rc=%d OK", host.c_str(), port, rc);

  CC_Logger::getSingleton()->logDebug("LIB MOSQUITTO %d",
                                      mqttObj->mqttGetVersion());
  this->status = StatusType::MQTT_CONNECTED;
}

void ESP_MqttWrapper::on_disconnect(int rc) {
  if (status == StatusType::MQTT_CONNECTED) {
    // Endpoint disconnection.
    CC_Logger::getSingleton()->logDebug(
        "Connection terminated abnormally RC:%d", rc);
    // status = StatusType::MQTT_CONNECTING;
    status = StatusType::MQTT_DISCONNECTED;
  }
  // status = StatusType::MQTT_DISCONNECTED;
}

void ESP_MqttWrapper::on_publish(int mid) {
  CC_Logger::getSingleton()->logDebug("Message Successfully Published: %d",
                                      mid);
}

void ESP_MqttWrapper::on_message(const struct mosquitto_message* message) {
  CC_Logger::getSingleton()->logDebug("Message received.... ");
  if (message != NULL) {
    CC_Logger::getSingleton()->logDebug("MSG: ID: %d", message->mid);
    CC_Logger::getSingleton()->logDebug("MSG: Payload: %s",
                                        (const char*)message->payload);
    CC_Logger::getSingleton()->logDebug("MSG: topic: %s",
                                        (const char*)message->topic);

    mqttBuffer.addMsg(new ESP_MqttMsg(message));
  } else {
    CC_Logger::getSingleton()->logError(
        "Message could not be retrieved... ERROR");
  }
}

void ESP_MqttWrapper::on_subscribe(int mid, int qos_count,
                                   const int* granted_qos) {
  CC_Logger::getSingleton()->logDebug("Subscription done: %d granted QOS: %d",
                                      mid, *granted_qos);
}

void ESP_MqttWrapper::on_unsubscribe(int mid) {
  CC_Logger::getSingleton()->logDebug("Unsubcribed from: %d", mid);
}

void ESP_MqttWrapper::on_log(int level, const char* str) {
  // CC_Logger::getSingleton()->logDebug(str);
}

void ESP_MqttWrapper::on_error() {
  CC_Logger::getSingleton()->logError("MQTT General Error.");
}

void ESP_MqttWrapper::doDisconnect() {
  if (status == StatusType::MQTT_CONNECTED) {
    // disconnect();
    mqttObj->mqttDisconnect();
    CC_Logger::getSingleton()->logDebug("MQTT: disconnecting from host.");
  }
  status = StatusType::MQTT_DISCONNECTED;  // intentional disconnection
}
/* ---------------------- */
/* MQTT CLIENT            */
/* ---------------------- */

/* ---------------------- */
/* PLUGIN INPUT MQTT      */
/* ---------------------- */
ESP_Plugin_Input_Mqtt::ESP_Plugin_Input_Mqtt() {
  id = 0;

  uniqueName = ESP_StringUtils::intToString(rand() % 10000);
  nBrokers = 0;
  mqttWrap = NULL;
  CC_Logger::getSingleton()->logDebug("LIB MQTT initializing.... ");
  mosqpp::lib_init();
  CC_Logger::getSingleton()->logDebug("LIB MQTT initializing.... DONE");

  pthread_mutex_init(&mutexMqtt, NULL);
}

ESP_Plugin_Input_Mqtt::~ESP_Plugin_Input_Mqtt() { mosqpp::lib_cleanup(); }

ESP_Plugin_Input_Base* ESP_Plugin_Input_Mqtt::getSingleton() {
  if (ESP_Plugin_Input_Mqtt::instance == NULL) {
    ESP_Plugin_Input_Mqtt::instance = new ESP_Plugin_Input_Mqtt();
  }
  return ESP_Plugin_Input_Mqtt::instance;
}

ESP_Plugin_Input_Mqtt* ESP_Plugin_Input_Mqtt::getInstance() {
  return (ESP_Plugin_Input_Mqtt*)ESP_Plugin_Input_Mqtt::getSingleton();
}

/**
*
*/
ESP_Input_Base* ESP_Plugin_Input_Mqtt::createInput(TiXmlElement* element) {
  std::string type = ESP_XmlUtils::queryStringValue(element, "type");
  std::string name = ESP_XmlUtils::queryStringValue(element, "name");
  // CC_Logger::getSingleton()->logDebug("Type : %s",type.c_str());

  ESP_Input_Base* result = NULL;
  if (type.compare(ESP_TYPE_MQTT) == 0) {
    result = new ESP_Input_Mqtt();
  } else if (type.compare(ESP_TYPE_MQTT_BROKER) == 0) {
    result = new ESP_Input_MqttBroker();
  }

  // Assigns
  if (result != NULL) {
    result->_type = type;
    result->_name = name;
  }

  // CC_Logger::getSingleton()->logError("Type not supported by this plugin:
  // %s",type.c_str());
  return result;
}

void ESP_Plugin_Input_Mqtt::setMosquitto(IMosquitto* mosquitto,
                                         std::string inputName) {
  pthread_mutex_lock(&mutexMqtt);

  mapMosquitto.insert(
      std::pair<std::string, IMosquitto*>(inputName, mosquitto));
  CC_Logger::getSingleton()->logDebug("SetMosquitto: %s new size: %d [%p]",
                                      inputName.c_str(), mapMosquitto.size(),
                                      mosquitto);
  pthread_mutex_unlock(&mutexMqtt);
}

bool ESP_Plugin_Input_Mqtt::isThereRefMosquitto() {
  bool ret = false;
  pthread_mutex_lock(&mutexMqtt);
  ret = mapMosquitto.size() > 0;
  CC_Logger::getSingleton()->logDebug(
      "ESP_Plugin_Input_MQTT: checking map size  : %d", mapMosquitto.size());

  pthread_mutex_unlock(&mutexMqtt);

  return ret;
}

IMosquitto* ESP_Plugin_Input_Mqtt::getAndRemoveIMosquitto(
    std::string inputName) {
  if (isThereRefMosquitto()) {
    pthread_mutex_lock(&mutexMqtt);
    //
    IMosquitto* obj = 0x0;
    std::map<std::string, IMosquitto*>::iterator it =
        mapMosquitto.find(inputName);

    if (it != mapMosquitto.end()) {
      obj = it->second;
      mapMosquitto.erase(it);
      CC_Logger::getSingleton()->logDebug(
          "ESP_Plugin_Input_MQTT: Replacing Mosquitto implementor %s map size "
          "after : %d",
          inputName.c_str(), mapMosquitto.size());
    }
    //
    pthread_mutex_unlock(&mutexMqtt);
    return obj;
  }
  return NULL;
}

int ESP_Plugin_Input_Mqtt::initMQTT(int port, std::string host,
                                    std::string topicPub, std::string topicSub,
                                    std::string user, std::string passwd,
                                    bool cleanSession, std::string inputName) {
  int idMqtt = getNewId();

  std::string finalID =
      uniqueName + std::string("-") + ESP_StringUtils::intToString(idMqtt);

  IMosquitto* refIMosquitto = NULL;
  if (!isThereRefMosquitto()) {
#ifdef USE_MQTT
    refIMosquitto = new ESP_MosquittoImpl(
        finalID.c_str(),
        cleanSession);  // this object will be destroyed whitin ~ESP_MqttWrapper
#else
    return -1;  // abort the creation of the service. If MQTT lib is not used,
                // nothing else should be allowed to be done.
#endif  // USE_MQTT
  } else {
    refIMosquitto = getAndRemoveIMosquitto(inputName);
    CC_Logger::getSingleton()->logDebug(
        "InitMQTT: getting mosquitto from map %p", refIMosquitto);
    if (refIMosquitto == NULL) return -1;
  }

  ESP_MqttWrapper* mqttWrap = new ESP_MqttWrapper(
      refIMosquitto, port, host, topicPub, topicSub, user, passwd);
  refIMosquitto->setCallBack(mqttWrap);

  // Attempt to connect,
  mqttWrap->doConnectHost();

  mapMqttClient.insert(std::pair<int, ESP_MqttWrapper*>(idMqtt, mqttWrap));

  // It'll check if there's something to subscribe to.
  return idMqtt;
}

int ESP_Plugin_Input_Mqtt::publish(int idClient, const char* buffer, int len) {
  // CC_Logger::getSingleton()->logDebug("Plugin Input MQTT: just about to
  // publish %d bytes",len);
  if (NULL != buffer) {
    ESP_MqttWrapper* mqttObj;

    mqttObj = findMqtt(idClient);

    if (mqttObj) {
      int res = mqttObj->publishMaskTopic(buffer, len);
      if (res > 0) {
        CC_Logger::getSingleton()->logDebug(
            "Plugin Input MQTT:  bytes published: %d.... ", res);
      }
      return res;
    } else {
      CC_Logger::getSingleton()->logError(
          "Plugin Input MQTT: no object found to publish");
      return 0;
    }
    // return (mqttObj)? mqttObj->publishDefaultTopic(buffer,len):0;
  }
  return 0;
}

void ESP_MqttWrapper::doSubscribe() {
  int id = -1;  // used internally

  if (!canISubscribe()) {
    return;  // Nothing to subscribe to
  }

  CC_Logger::getSingleton()->logDebug("Lib MQTT: subscribing to topic %s ... ",
                                      topicSubscribe.c_str());

  // int res = subscribe(&id,topicSubscribe.c_str(),0);
  int res = mqttObj->mqttSubscribe(&id, topicSubscribe.c_str(), 0);

  if (MOSQ_ERR_SUCCESS == res) {
    CC_Logger::getSingleton()->logDebug(
        "Lib MQTT: subscribing to topic %s ... DONE ID: %d",
        topicSubscribe.c_str(), id);

  } else {
    CC_Logger::getSingleton()->logError(
        "Lib MQTT: subscribing to topic %s .... ERROR", topicSubscribe.c_str());
  }
}

void ESP_MqttWrapper::stopSubscription() {
  if (status != StatusType::MQTT_CONNECTED) {
    return;
  }

  if (canISubscribe()) {
    CC_Logger::getSingleton()->logDebug(
        "Lib MQTT: unsubscribing of topic %s .... ", topicSubscribe.c_str());
    int id = 0;
    // int res = unsubscribe(&id,topicSubscribe.c_str());
    int res = mqttObj->mqttUnsubscribe(&id, topicSubscribe.c_str());
    if (MOSQ_ERR_SUCCESS == res) {
      CC_Logger::getSingleton()->logDebug(
          "Lib MQTT: unsubscribing of topic %s .... DONE",
          topicSubscribe.c_str());
    } else {
      CC_Logger::getSingleton()->logError(
          "Lib MQTT: unsubscribing of topic %s .... ERROR (disconnecting "
          "anyway)",
          topicSubscribe.c_str());
    }
  }
}

int ESP_Plugin_Input_Mqtt::removeClientFromMap(int idClient) {
  std::map<int, ESP_MqttWrapper*>::iterator it = mapMqttClient.find(idClient);

  if (it != mapMqttClient.end()) {
    mapMqttClient.erase(it);
    return 1;
  } else {
    return 0;
  }
}

bool ESP_Plugin_Input_Mqtt::stopMQTT(int idClient) {
  ESP_MqttWrapper* mqttObj = findMqtt(idClient);

  if (NULL == mqttObj) {
    return false;
  }

  mqttObj->stopSubscription();

  mqttObj->doDisconnect();
  delete (mqttObj);

  removeClientFromMap(idClient);

  return true;
}

ESP_MqttWrapper* ESP_Plugin_Input_Mqtt::findMqtt(int id) {
  std::map<int, ESP_MqttWrapper*>::iterator it = mapMqttClient.find(id);
  if (it != mapMqttClient.end()) {
    return it->second;
  } else {
    return NULL;
  }
}

int ESP_Plugin_Input_Mqtt::readfromSubscription(int id, char* buffer, int len) {
  ESP_MqttWrapper* mqttObj;

  mqttObj = findMqtt(id);

  // This will just do a "loop read" on the channel (subscription topic).
  if (mqttObj != NULL) {
    mqttObj->readFromChannel();
    return mqttObj->getBuffer(buffer, len);
  } else {
    return 0;
  }
}

char* ESP_Plugin_Input_Mqtt::getFirstMQTTMsg(int id, int* length) {
  ESP_MqttWrapper* mqttObj;

  mqttObj = findMqtt(id);

  // This will just do a "loop read" on the channel (subscription topic).
  if (mqttObj != NULL) {
    mqttObj->readFromChannel();
    return mqttObj->getFirstMQTTMsg(length);
  } else {
    // Error
    *length = -1;
    return NULL;
  }
}

void ESP_Plugin_Input_Mqtt::removeFirstMQTTMsg(int id) {
  ESP_MqttWrapper* mqttObj;

  mqttObj = findMqtt(id);

  // This will just do a "loop read" on the channel (subscription topic).
  if (mqttObj != NULL) {
    mqttObj->doneWithFirstMQTTMsg();
  } else {
    CC_Logger::getSingleton()->logError("No MQTT Wrapper found for %d id", id);
  }
}

bool ESP_Plugin_Input_Mqtt::isBrokerRunning(int port) {
  // First check, if port is already in the list
  // additionally, we can check if port is in use.
  if (findBroker(port) != NULL) {
    return true;
  }
  return false;
}

int ESP_Plugin_Input_Mqtt::initBroker(int port, const char* pathBroker,
                                      const char* pathToConfig) {
  if (!isBrokerRunning(port)) {
    ESP_MQTT_Broker* mqttBroker = new ESP_MQTT_Broker();

    int res = mqttBroker->launchBroker(pathBroker, pathToConfig);
    if (res == 1) {
      nBrokers++;
      mapMqttBroker.insert(std::pair<int, ESP_MQTT_Broker*>(port, mqttBroker));
      return port;
    }
    // Broker couldn't start
    return 0;
  }
  return port;
}

void ESP_Plugin_Input_Mqtt::stopBroker(int port) {
  CC_Logger::getSingleton()->logDebug("number of running brokers: %d",
                                      nBrokers);
  nBrokers--;
  if (nBrokers == 0) {
    ESP_MQTT_Broker* mqttBroker = findBroker(port);
    if (mqttBroker != NULL) {
      mqttBroker->stopBroker();
      delete (mqttBroker);
      removeBrokerFromMap(port);
    }
  }
  if (nBrokers < 0) {
    nBrokers = 0;  // just in case it's called multiple times
  }
}

int ESP_Plugin_Input_Mqtt::removeBrokerFromMap(int port) {
  std::map<int, ESP_MQTT_Broker*>::iterator it = mapMqttBroker.find(port);

  if (it != mapMqttBroker.end()) {
    mapMqttBroker.erase(it);
    return 1;
  } else {
    return 0;
  }
}

ESP_MQTT_Broker* ESP_Plugin_Input_Mqtt::findBroker(int port) {
  std::map<int, ESP_MQTT_Broker*>::iterator it = mapMqttBroker.find(port);
  if (it != mapMqttBroker.end()) {
    return it->second;
  } else {
    return NULL;
  }
}

ESP_Input_Mqtt::ESP_Input_Mqtt() {
  port = 1883;
  clientid = 1;
  _keepRunning = true;
}

int ESP_Input_Mqtt::openServer() {
  this->_sid = ESP_Plugin_Input_Mqtt::getInstance()->initMQTT(
      port, host, topicPublish, topicSubscribe, user, passwd, cleanSession,
      _name);
  return this->_sid;
}

int ESP_Input_Mqtt::acceptServer() {
  char* data = NULL;
  int ndata = 0;
  int id = -1;

  // Wait for new message
  while (ndata == 0 && _sid >= 0 && _keepRunning) {
    data = ESP_Plugin_Input_Mqtt::getInstance()->getFirstMQTTMsg(_sid, &ndata);

    if (ndata > 0) {
      id = clientid++;
      ESP_MqttClient* client = new ESP_MqttClient();
      client->datacontext.addInputData(data, ndata);
      clients.insert(std::pair<int, ESP_MqttClient*>(id, client));
      ESP_Plugin_Input_Mqtt::getInstance()->removeFirstMQTTMsg(_sid);
    }
    SLEEP(15);
  }

  return id;
}

bool ESP_Input_Mqtt::stopServer() {
  _keepRunning = false;
  return true;
}

bool ESP_Input_Mqtt::closeServer() {
  // id == mqttwrapper id
  ESP_Plugin_Input_Mqtt::getInstance()->stopMQTT(_sid);
  _sid = -1;

  return true;
}

int ESP_Input_Mqtt::openClient() {
  if (_sid < 0) {
    return ESP_Plugin_Input_Mqtt::getInstance()->initMQTT(
        port, host, topicPublish, topicSubscribe, user, passwd, cleanSession,
        _name);
  } else {
    // Return new Client from Virtual Accept
    int id = clientid++;
    ESP_MqttClient* client = new ESP_MqttClient();
    clients.insert(std::pair<int, ESP_MqttClient*>(id, client));
    return id;
    return -1;
  }
}

bool ESP_Input_Mqtt::stopClient(int id) { return true; }

bool ESP_Input_Mqtt::closeClient(int id) {
  if (_sid < 0) {
    return ESP_Plugin_Input_Mqtt::getInstance()->stopMQTT(id);
  } else {
    // Delete Client
    pthread_mutex_lock(&mutex);
    std::map<int, ESP_MqttClient*>::iterator it = clients.find(id);
    if (it != clients.end()) {
      delete it->second;
      clients.erase(it);
    }
    pthread_mutex_unlock(&mutex);
    return true;
  }
}

// TODO: Read len by len
int ESP_Input_Mqtt::readClient(int id, char* buffer, int len) {
  int ndata = -1;

  if (this->_sid < 0) {
    ndata = ESP_Plugin_Input_Mqtt::getInstance()->readfromSubscription(
        id, buffer, len);
  } else {
    ESP_MqttClient* client = NULL;

    // Get Cached Data
    pthread_mutex_lock(&mutex);
    std::map<int, ESP_MqttClient*>::iterator it = clients.find(id);
    if (it != clients.end()) {
      client = it->second;
    }
    pthread_mutex_unlock(&mutex);

    // Read Data
    if (client != NULL && client->datacontext.getAvailableInputData() > 0) {
      // Get Data
      ndata = client->datacontext.getAvailableInputData();
      if (len < ndata) {
        ndata = len;
      }
      const char* newdata = NULL;
      client->datacontext.readPInputData(ndata, newdata);
      memcpy(buffer, newdata, ndata);
      client->datacontext.validateInputData(ndata);

      /*
      // Delete if empty
      if (it->second->datacontext.getAvailableInputData() == 0)
      {
          delete it->second;
          clients.erase(it);
      }
      */
    }
  }
  return ndata;
}

int ESP_Input_Mqtt::writeClient(int id, char* buffer, int len) {
  int ndata = -1;

  if (_sid < 0) {
    ndata = ESP_Plugin_Input_Mqtt::getInstance()->publish(id, buffer, len);
  } else {
    ndata = ESP_Plugin_Input_Mqtt::getInstance()->publish(_sid, buffer, len);
  }

  return ndata;
}

/**
* Specific input values are queried from element
* topic, port, host
*/
void ESP_Input_Mqtt::parseCustomElement(TiXmlElement* element) {
  this->topicPublish = ESP_XmlUtils::queryStringValue(element, "publish");
  this->topicSubscribe = ESP_XmlUtils::queryStringValue(element, "subscribe");

  this->host = ESP_XmlUtils::queryStringValue(element, "host");
  int nPort = ESP_XmlUtils::queryIntValue(element, "port");
  if (nPort > 0) {
    port = nPort;
  }

  this->user = ESP_XmlUtils::queryStringValue(element, "user");
  this->passwd = ESP_XmlUtils::queryStringValue(element, "password");

  this->cleanSession = true;
  std::string cSession =
      ESP_XmlUtils::queryStringValue(element, "clean-session");
  if (cSession.length() > 0) {
    this->cleanSession = ESP_XmlUtils::queryBoolValue(element, "clean-session");
  }
}

ESP_Input_MqttBroker::ESP_Input_MqttBroker() {
  port = 1883;
  // Default values.
  host = "localhost";
  pathToBroker = "mosquitto";
  pathToConfig = "mosquitto.conf";
}

int ESP_Input_MqttBroker::openServer() {
  this->_sid = ESP_Plugin_Input_Mqtt::getInstance()->initBroker(
      port, pathToBroker.c_str(), pathToConfig.c_str());
  return this->_sid;
}

int ESP_Input_MqttBroker::acceptServer() {
  ESP_MQTT_Broker* mqttBroker =
      ESP_Plugin_Input_Mqtt::getInstance()->findBroker(_sid);
  if (mqttBroker != NULL) {
    mqttBroker->waitForBroker();
  }
  return -1;
}

bool ESP_Input_MqttBroker::stopServer() {
  // id == port
  ESP_Plugin_Input_Mqtt::getInstance()->stopBroker(_sid);
  return true;
}

bool ESP_Input_MqttBroker::closeServer() {
  // id == port
  ESP_Plugin_Input_Mqtt::getInstance()->stopBroker(_sid);
  return true;
}

int ESP_Input_MqttBroker::openClient() { return -1; }

bool ESP_Input_MqttBroker::stopClient(int id) { return true; }

bool ESP_Input_MqttBroker::closeClient(int id) { return false; }

int ESP_Input_MqttBroker::readClient(int id, char* buffer, int len) {
  return -1;
}

int ESP_Input_MqttBroker::writeClient(int id, char* buffer, int len) {
  return -1;
}

void ESP_Input_MqttBroker::parseCustomElement(TiXmlElement* element) {
  // Server aka Broker will also need port parameter
  int nPort = ESP_XmlUtils::queryIntValue(element, "port");
  if (nPort > 0) {
    port = nPort;
  }
  pathToBroker = ESP_XmlUtils::queryStringValue(element, "path");

  pathToConfig = ESP_XmlUtils::queryStringValue(element, "config");
}
