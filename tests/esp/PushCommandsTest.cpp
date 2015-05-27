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
#include "h/PushCommandsTest.h"
#include "h/MockMosquitto.h"
#include "CC_Logger.h"
#include "ESPLib.h"

using ::testing::Return;
using ::testing::NotNull;
using ::testing::StrEq;
using ::testing::_;
using ::testing::Invoke;


CPPUNIT_TEST_SUITE_REGISTRATION(PushCommandsTest);


void PushCommandsTest::setUp() {
  mosquittoPub = new MockMosquitto();
  mosquittoSub = new MockMosquitto();


  std::string user = "admin";
  std::string passwd = "1234";

  //Order matters.
  EXPECT_CALL(*mosquittoSub,mqttSetPassword(StrEq(user.c_str()),
              StrEq(passwd.c_str()))).WillRepeatedly(Return(0));
  EXPECT_CALL(*mosquittoPub,mqttSetPassword(StrEq(user.c_str()),
              StrEq(passwd.c_str()))).WillRepeatedly(Return(0));

  //Basic and common expected calls.
  EXPECT_CALL(*mosquittoPub,mqttConnect(_,_,_)).WillRepeatedly(Invoke(this,
      &PushCommandsTest::stubConnectPub));
  EXPECT_CALL(*mosquittoPub,mqttLoop(_,_)).WillRepeatedly(Return(0));


  EXPECT_CALL(*mosquittoSub,mqttConnect(_,_,_)).WillRepeatedly(Invoke(this,
      &PushCommandsTest::stubConnectSub));
  EXPECT_CALL(*mosquittoSub,mqttLoop(_,_)).WillRepeatedly(Return(0));



  ESP_Plugin_Input_Mqtt::getInstance()->setMosquitto(mosquittoPub,"mqttwriter");
  ESP_Plugin_Input_Mqtt::getInstance()->setMosquitto(mosquittoSub,"mqttrunner");

  mqttMsg.topic = NULL;
  mqttMsg.payload = NULL;
  mqttMsg.payloadlen = 0;


}


void PushCommandsTest::tearDown() {


}


void resultCallbackTest(void* userData, ESP_Runner* runner) {

}


void PushCommandsTest::testSendPushCommand() {
  ESPLib esplib;
  CC_Logger::getSingleton()->logDebug("Test Send Push Command ... starting");

  //Expectations:
  //There will be two mqttWrappers, but:
  //1. Only one MQTTWrapper will subscribe to '#' while the other won't as topic is empty.

  EXPECT_CALL(*mosquittoSub,mqttSubscribe(_,_,_)).WillOnce(Invoke(this,
      &PushCommandsTest::stubSubscribeSilent));

  //INPUT
  std::string apikey ="1234";
  std::string iddevice ="id001";
  std::string cmdname ="TEST";
  std::string cmdParams = "cmdid|1234";

  //EXPECTED
  std::string expectedTopic ="1234/id001/cmd/TEST";
  std::string expectedPayload="cmdid|1234";

  EXPECT_CALL(*mosquittoPub,mqttPublish(_,_,_,_,_,_)).WillOnce(Invoke(this,
      &PushCommandsTest::stubPublish));

  EXPECT_CALL(*mosquittoSub,mqttUnsubscribe(_,_)).WillOnce(Return(0));
  EXPECT_CALL(*mosquittoSub,mqttDisconnect()).WillOnce(Return(0));
  EXPECT_CALL(*mosquittoPub,mqttDisconnect()).WillOnce(Return(0));

  int sensorid = esplib.createSensor("sensormqtt.xml");
  if (sensorid >= 0) {
    esplib.registerResultCallback(sensorid, &esplib, resultCallbackTest);
    esplib.startSensor(sensorid,"main");

    std::map<std::string,std::string> cparams;
    cparams.insert(std::pair<std::string,std::string>("apikey",apikey));
    cparams.insert(std::pair<std::string,std::string>("sensorid",iddevice));
    cparams.insert(std::pair<std::string,std::string>("cmdname",cmdname));
    cparams.insert(std::pair<std::string,std::string>("cmdparams",cmdParams));

    ESP_Input_Base* input =
      esplib.sensors[sensorid]->getInputFromName("mqttwriter");
    SLEEP(100); //Wait some time for inputs to settle
    std::vector<ESP_Attribute> result = esplib.executeRunnerFromInput(sensorid,
                                        "sendcmd",cparams,input).ppresults;

    SLEEP(10);
    esplib.stopSensor(sensorid);
  }

  CPPUNIT_ASSERT(sensorid >=0); //check if sensor was created properly.
  CPPUNIT_ASSERT(mqttMsg.topic != NULL);

  std::string actualTopic = std::string(mqttMsg.topic,strlen(mqttMsg.topic));
  //Checking expected message.
  CPPUNIT_ASSERT_EQUAL(expectedTopic,actualTopic);
  CPPUNIT_ASSERT_EQUAL(expectedPayload,std::string((const char*)mqttMsg.payload,
                       mqttMsg.payloadlen));

  CC_Logger::getSingleton()->logDebug("Test Send Push Command ... DONE");
}

int PushCommandsTest::stubSubscribeSilent(int* mid, const char* sub, int qos) {

  *mid = 1;
  CC_Logger::getSingleton()->logDebug("StubSubscribeSilent: %d %s %qos",*mid,sub,
                                      qos);
  CPPUNIT_ASSERT(strcmp(sub,"#")== 0);
  return 0;
}

//Callbacks are required.
int PushCommandsTest::stubConnectPub(const char* host, int port,
                                     int keepalive) {
  mosquittoPub->callBack->on_connect(1);
  return 0;
}

//Callbacks are required.
int PushCommandsTest::stubConnectSub(const char* host, int port,
                                     int keepalive) {
  mosquittoSub->callBack->on_connect(1);
  return 0;
}

//This method will be invoked by the mock.
int PushCommandsTest::stubPublish(int* mid,const char* topic,int payloadLen,
                                  const void* payload,int qos,bool retain) {
  CC_Logger::getSingleton()->logDebug("stubPublish invoked: checking parameters");

  mqttMsg.mid = 2;
  *mid = 0;

  mqttMsg.topic = (char*) malloc(strlen(topic)+1);
  strcpy(mqttMsg.topic,topic);
  mqttMsg.payload = (void*) malloc(payloadLen);
  memcpy(mqttMsg.payload,payload,payloadLen);
  mqttMsg.payloadlen = payloadLen;

  mqttMsg.qos = qos;
  mqttMsg.retain = retain;
  return 0;
}
