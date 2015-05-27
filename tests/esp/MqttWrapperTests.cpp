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
#include "h/MqttWrapperTests.h"
#include "h/MockMosquitto.h"
#include "CC_Logger.h"

using ::testing::Return;
using ::testing::NotNull;
using ::testing::StrEq;




CPPUNIT_TEST_SUITE_REGISTRATION(MqttWrapperTests);

void MqttWrapperTests::setUp() {
  mosquitto = new MockMosquitto();
  std::string user = "user";
  std::string passwd = "passwd";
  EXPECT_CALL(*mosquitto,mqttSetPassword(StrEq(user),
                                         StrEq(passwd))).Times(1).WillOnce(Return(0));
  mqttWrapper = new ESP_MqttWrapper(mosquitto,1883,"localhost","api/id/key","#",
                                    user,passwd);
  mqttMsg.topic = NULL;
  mqttMsg.payload = NULL;
}

void MqttWrapperTests::tearDown() {
  delete mqttWrapper;
  //Deletion of mosquitto is done within ~ESP_MqttWrapper.
}



void MqttWrapperTests::testConnect() {

  EXPECT_CALL(*mosquitto,mqttConnect(_,_,_)).WillOnce(Invoke(this,
      &MqttWrapperTests::stubConnect));
  EXPECT_CALL(*mosquitto,mqttLoop(100,1)).Times(1).WillOnce(Return(0));
  EXPECT_CALL(*mosquitto,mqttSubscribe(_,_,_)).Times(1).WillOnce(Return(0));
  //Let the test begin...
  CC_Logger::getSingleton()->logDebug("Testing MQTT connect ...");
  mqttWrapper->doConnectHost();
  CPPUNIT_ASSERT(ESP_MqttWrapper::StatusType::MQTT_CONNECTED==
                 mqttWrapper->status);
  CC_Logger::getSingleton()->logDebug("Testing MQTT connect ... DONE");
}



void MqttWrapperTests::testReceiveMsg() {
  mqttWrapper->status = ESP_MqttWrapper::StatusType::MQTT_CONNECTED;
  EXPECT_CALL(*mosquitto,mqttLoop(0,1)).Times(1).WillOnce(Invoke(this,
      &MqttWrapperTests::stubLoopToOnMessage));

  mqttWrapper->readFromChannel();

  int nLen = 0;
  char* msg = 0;
  char expected[] = "1234/id001/temperature//33";
  char actual[27] = {};

  CC_Logger::getSingleton()->logDebug("Testing MQTT Receive Msg ...");
  msg = mqttWrapper->getFirstMQTTMsg(&nLen);
  CC_Logger::getSingleton()->logDebug("Testing MQTT Receive Msg ... COMPARING BUFFERS");
  memcpy(actual,msg,nLen);

  std::string strExpected = std::string(expected,strlen(expected));
  std::string strActual = std::string(actual,strlen(actual));
  CPPUNIT_ASSERT(strlen(expected) == nLen);
  CPPUNIT_ASSERT_EQUAL(strExpected,strActual);
  mqttWrapper->doneWithFirstMQTTMsg();
  CC_Logger::getSingleton()->logDebug("Testing MQTT Receive Msg ... DONE");
}

void MqttWrapperTests::testPublishMsg() {
  CC_Logger::getSingleton()->logDebug("Testing MQTT PublishMask Msg ...");
  char buffer[] = "1234/id001/temperature//33";
  std::string expectedTopic ="1234/id001/temperature";
  char expectedPaylad[] = "33";

  mqttWrapper->status = ESP_MqttWrapper::StatusType::MQTT_CONNECTED;
  int mid = 0;

  EXPECT_CALL(*mosquitto,mqttPublish(_,_,_,_,_,_)).Times(1).WillOnce(Invoke(this,
      &MqttWrapperTests::stubPublish));//el bueno
  EXPECT_CALL(*mosquitto,mqttLoop(100,1)).Times(1).WillOnce(Return(0));

  int res = mqttWrapper->publishMaskTopic(buffer,strlen(buffer));

  CC_Logger::getSingleton()->logDebug("Testing MQTT PublishMask Msg: Topic: %s",
                                      mqttMsg.topic);
  CPPUNIT_ASSERT(res == strlen(buffer));

  std::string actualTopic = std::string(mqttMsg.topic,strlen(mqttMsg.topic));
  CPPUNIT_ASSERT_EQUAL(expectedTopic,actualTopic);
  CPPUNIT_ASSERT_EQUAL(std::string("33"),std::string((const char*)mqttMsg.payload,
                       mqttMsg.payloadlen));

  CC_Logger::getSingleton()->logDebug("Testing MQTT PublishMask Msg ... DONE");
  free(mqttMsg.payload);
  free(mqttMsg.topic);

}

void MqttWrapperTests::testConnectAndReconnect() {
  CC_Logger::getSingleton()->logDebug("Testing MQTT Reconnect ...");
  mqttWrapper->status = ESP_MqttWrapper::StatusType::MQTT_CONNECTED;

  int nLen = 0;
  char* msg = 0;
  char expected[] = "1234/id001/temperature//33";
  char actual[27] = {};

  /*
  Scenario:
   MQTT is already connected
   Try to read and loop returns an error,
   then try to connect again using reconnect, it also fails
   next attempt will succeed,
   then it reads a message from the buffer.
  */

  EXPECT_CALL(*mosquitto,mqttLoop(0,
                                  1)).Times(3).WillOnce(Return(1)).WillOnce(Return(1)).WillOnce(Invoke(this,
                                      &MqttWrapperTests::stubLoopToOnMessage));; //Returns an error
  EXPECT_CALL(*mosquitto,mqttReconnect()).Times(2).WillOnce(Return(-1)).WillOnce(
    Invoke(this,
           &MqttWrapperTests::stubToReConnect)); //first time an error, then connects;
  EXPECT_CALL(*mosquitto,mqttLoop(100,1)).Times(2).WillRepeatedly(Return(0));
  //EXPECT_CALL(*mosquitto,mqttLoop(0,1)).Times(1).//this will simulate a message being received
  EXPECT_CALL(*mosquitto,mqttSubscribe(_,_,
                                       _)).WillOnce(Return(
                                           0));//after successfully connection, it will subscribe to topics.
  int rc = -1;
  mqttWrapper->status =
    ESP_MqttWrapper::StatusType::MQTT_CONNECTING; //this is the effect of a on_disconnect call.
  int guard = 4;
  while (rc != 0) {
    rc = mqttWrapper->readFromChannel();
    CC_Logger::getSingleton()->logDebug("");

    guard--;

    if (guard == 0) {
      CPPUNIT_ASSERT(guard == 4);
    }

  }
  //a message should be read.
  CPPUNIT_ASSERT(mqttWrapper->status ==
                 ESP_MqttWrapper::StatusType::MQTT_CONNECTED);

  CC_Logger::getSingleton()->logDebug("Testing MQTT Reconnect  ...");
  msg = mqttWrapper->getFirstMQTTMsg(&nLen);
  CC_Logger::getSingleton()->logDebug("Testing MQTT Reconnect ... COMPARING BUFFERS");
  memcpy(actual,msg,nLen);

  std::string strExpected = std::string(expected,strlen(expected));
  std::string strActual = std::string(actual,strlen(actual));
  CPPUNIT_ASSERT(strlen(expected) == nLen);
  CPPUNIT_ASSERT_EQUAL(strExpected,strActual);
  //connected
  mqttWrapper->doneWithFirstMQTTMsg();
  CC_Logger::getSingleton()->logDebug("Testing MQTT Reconnect  ... DONE");

}



int MqttWrapperTests::stubToReConnect() {
  mqttWrapper->on_connect(1);
  return 0;
}


int MqttWrapperTests::stubPublish(int* mid,const char* topic,int payloadLen,
                                  const void* payload,int qos,bool retain) {

  mqttMsg.mid = 2;
  *mid = mqttMsg.mid;

  mqttMsg.topic = (char*) malloc(strlen(topic)+1);
  strcpy(mqttMsg.topic,topic);
  mqttMsg.payload = (void*) malloc(payloadLen);
  memcpy(mqttMsg.payload,payload,payloadLen);
  mqttMsg.payloadlen = payloadLen;

  mqttMsg.qos = qos;
  mqttMsg.retain = retain;
  return 0;
}

int MqttWrapperTests::stubConnect(const char* host, int port, int keepalive) {
  mqttWrapper->on_connect(1);
  return 0;
}

int MqttWrapperTests::stubLoopToOnMessage(int,int) {

  mqttMsg.mid = 2;
  mqttMsg.qos = 0;
  mqttMsg.retain = false;
  mqttMsg.topic = (char*) "1234/id001/temperature";
  mqttMsg.payload = (void*) "33";
  mqttMsg.payloadlen = 2;

  mqttWrapper->on_message(&mqttMsg);
  return 0;
}
