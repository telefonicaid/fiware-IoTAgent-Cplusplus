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
#include "h/MqttBufferTests.h"
#include "CC_Logger.h"

CPPUNIT_TEST_SUITE_REGISTRATION(MqttBufferTests);

void MqttBufferTests::setUp() {
  //ctor
}

void MqttBufferTests::tearDown() {
  while (!myBuf.isQueueEmpty()) {
    myBuf.deleteFirstMessage();

  }
  CC_Logger::getSingleton()->logDebug("CLEAN UP: deleting created objects COMPLETE");

}



void MqttBufferTests::initTestData(ESP_MqttBuffer& myBuf,int numMensajes) {
  char topic[] = "Topic-X";
  char payload[] = "payloadX";

  struct mosquitto_message mqttMsg;
  mqttMsg.topic = topic;
  mqttMsg.payload = (void*) payload;
  mqttMsg.payloadlen = 8;

  //loop for copying multpile messages.
  for (int i=0; i< numMensajes; i++) {
    sprintf(payload,"payload%d",i);
    sprintf(topic,"Topic-%d",i);
    //mqttMsg has a pointer to payload, so it will
    //get the new payload, and ESP_MqttMsg will COPY
    //whatever is in the strcutre.
    myBuf.addMsg(new ESP_MqttMsg(&mqttMsg));
  }
}

ESP_MqttMsg* MqttBufferTests::createMqttMsg(const char* topic,
    const char* payload, int payloadLen) {
  struct mosquitto_message mqttMsg;
  mqttMsg.topic = (char*) topic;
  mqttMsg.payload = (void*) payload;
  mqttMsg.payloadlen = payloadLen;

  return new ESP_MqttMsg(&mqttMsg);

}

void MqttBufferTests::testRetrieveZeroBytes() {
  initTestData(myBuf,2);
  char buffer[10];
  int len = 0;
  int lenToRead = 0;
  int bytesBefore;

  bytesBefore = myBuf.getUnreadBytes();

  len = myBuf.getSerialized(buffer,lenToRead);

  CPPUNIT_ASSERT(bytesBefore == myBuf.getUnreadBytes());
  CPPUNIT_ASSERT(0 == len);

}

/*

void test_BigBuffer(char *buffer){

    ESP_MqttBuffer myBuf;
    initTestData(myBuf,5);

    CC_Logger::getSingleton()->logDebug("TEST: Reading whole buffer...");
    int len = myBuf.getSerialized(buffer,1024);
    CC_Logger::getSingleton()->logDebug("Read : %d bytes Output: %s",len,buffer);

    assert(!myBuf.isQueueEmpty());
    CC_Logger::getSingleton()->logDebug("QUEUE status: %s",(myBuf.isQueueEmpty()?"empty":"not empty"));
    CC_Logger::getSingleton()->logDebug("UNREADBYTES : %d",myBuf.getUnreadBytes());
    CC_Logger::getSingleton()->logDebug("TEST PASSED");

}
*/

void MqttBufferTests::testRetrieveSmallerThanBuffer() {
  //ESP_MqttBuffer myBuf;
  initTestData(myBuf,5);

  int lenRead = 10;

  int len = -1;
  int counter = 1;

  char buffer[1024];
  memset(buffer,0,1024);

  CC_Logger::getSingleton()->logDebug("TEST: Reading buffer in multiple attempts size: 10");
  do {
    CC_Logger::getSingleton()->logDebug("TEST: Trying to read #%d...",counter);
    len = myBuf.getSerialized(buffer,lenRead);
    CC_Logger::getSingleton()->logDebug("Read : %d bytes Output: %s",len,buffer);

    memset(buffer,0,1024);
    counter++;

  }
  while (len>0);

  CC_Logger::getSingleton()->logDebug("QUEUE status: %s",
                                      (myBuf.isQueueEmpty()?"empty":"not empty"));
  CC_Logger::getSingleton()->logDebug("UNREADBYTES : %d",myBuf.getUnreadBytes());

  CPPUNIT_ASSERT(0==myBuf.getUnreadBytes());

  //assert(0== myBuf.getUnreadBytes());

}

void MqttBufferTests::testRetrieveGreaterThanBuffer() {
  //ESP_MqttBuffer myBuf;
  initTestData(myBuf,6);

  int lenRead = 128;

  int len = -1;
  int counter = 1;

  char buffer[1024];
  memset(buffer,0,1024);

  CC_Logger::getSingleton()->logDebug("TEST: Reading buffer in multiple attempts size: %d",
                                      lenRead);
  do {
    CC_Logger::getSingleton()->logDebug("TEST: Trying to read #%d...",counter);
    len = myBuf.getSerialized(buffer,lenRead);
    CC_Logger::getSingleton()->logDebug("Read : %d bytes Output: %s",len,buffer);
    memset(buffer,0,1024);
    counter++;

  }
  while (len>0);

  assert(0== myBuf.getUnreadBytes());
  CC_Logger::getSingleton()->logDebug("Test PASSED");

}



//>>>>>>>>>>>> TEST NEW FEATURES 2-7-2014

bool MqttBufferTests::compareBuffers(const char* src,int srclen,
                                     const char* dst, int dstlen) {
  if (srclen != dstlen) {
    return false;
  }
  int i = 0;
  bool equals = true;
  while (i < srclen) {
    if (src[i] != dst[i]) {
      equals = false;
      break;
    }
    i++;
  }
  return equals;
}



void MqttBufferTests::testEmptyMqttBuffer() {
  //ESP_MqttBuffer myBuf;
  char buffer[1024];

  CC_Logger::getSingleton()->logDebug("TEST: Reading whole (empty) buffer...");
  int len = myBuf.getSerialized(buffer,1024);

  CPPUNIT_ASSERT(0 == len);

}

void MqttBufferTests::testGettingFirstMessage() {

  // ESP_MqttBuffer myBuf;
  //initTestData(myBuf,3);
  char topic[] = "topic";
  char payload[] = "payload-x";

  char wholeMsg[] ="topic//payload-x";
  //Insertar mensajes! en el buffer.
  ESP_MqttMsg* msg_1 = createMqttMsg(topic,payload,9);
  ESP_MqttMsg* msg_2 = createMqttMsg(topic,payload,9);
  ESP_MqttMsg* msg_3 = createMqttMsg(topic,payload,9);

  myBuf.addMsg(msg_1);
  myBuf.addMsg(msg_2);
  myBuf.addMsg(msg_3);

  int nLen;
  char* buffOutput;

  char* tempMqttBuf;

  //Loop:
  int i = 0;
  while (!myBuf.isQueueEmpty()) {
    // CC_Logger::getSingleton()->logDebug("TEST: getting Top message #%d",++i);
    tempMqttBuf = myBuf.getFirstMqttMessage(&nLen);
    CPPUNIT_ASSERT(nLen>0);
    if (nLen > 0) {
      buffOutput = (char*) malloc(nLen+1);
      memcpy(buffOutput,tempMqttBuf,nLen);
      //Message copied,
      myBuf.deleteFirstMessage();
      buffOutput[nLen] = 0; //for easy printing
      //   CC_Logger::getSingleton()->logDebug("TEST: message from queue: %s",buffOutput);
      //   CC_Logger::getSingleton()->logDebug("TEST :original %d obtenido %d",strlen(wholeMsg),nLen);

      CPPUNIT_ASSERT(true == compareBuffers(buffOutput,nLen,wholeMsg,
                                            strlen(wholeMsg)));
      free(buffOutput);
      buffOutput = NULL;
      nLen = 0;
    }

  }

  CPPUNIT_ASSERT(true == myBuf.isQueueEmpty());

}
