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
#include "input_mqtt/ESP_MqttBuffer.h"

ESP_MqttBuffer::~ESP_MqttBuffer() {
  // dtor
}

/**
It has to change number of bytes can be read
*/
void ESP_MqttBuffer::addMsg(ESP_MqttMsg* msg) {
  qMqttMsg.push(msg);
  unreadBytes += msg->getTotalLength();
}

/**
It has to change number of bytes can be read
*/
void ESP_MqttBuffer::removeMsg() {
  // delete top message,
  delete (qMqttMsg.front());
  // then remove it from the queue
  qMqttMsg.pop();
}

char* ESP_MqttBuffer::getFirstMqttMessage(int* length) {
  if (qMqttMsg.empty()) {
    *length = 0;
    return NULL;
  } else {
    return qMqttMsg.front()->getBufferToBeCopied(length);
  }
}

/**
This method should be used over complete MQTT messages, so it's not valid
for partial read.
*/
void ESP_MqttBuffer::deleteFirstMessage() {
  int nMsgLen;
  qMqttMsg.front()->getBufferToBeCopied(&nMsgLen);
  // overall byte count is decreased by exactly the bytes of the first message.
  decreaseUnreadBytes(nMsgLen);
  removeMsg();
}

/**
* int lenRead: bytes to be Read, if bigger than unreadBytes, latter will be
* returned instead.
* buffOutput was already allocated
*
**/
int ESP_MqttBuffer::getSerialized(char* buffOutput, int lenRead) {
  int nBuffLen = 0;

  int nMsgLen = 0;

  // Need to check if there are more elements
  // although unreadBytes = 0 means there are no more messages.
  if (qMqttMsg.empty()) {
    return nBuffLen;
  }

  char* auxBuff = qMqttMsg.front()->getBufferToBeCopied(&nMsgLen);

  // Copy first message
  // nMsgLen is the actual length of bytes to be copied, same result as calling
  // getRemainingLength()
  // but it might be greater than lenRead, in that case, it means there will be
  // some bytes
  // left to be read within the Mqtt message.
  nBuffLen = (nMsgLen > lenRead) ? lenRead : nMsgLen;
  memcpy(buffOutput, auxBuff, nBuffLen);

  // As soon as bytes are copied, unreadBytes is decreased.
  decreaseUnreadBytes(nBuffLen);

  // CASE OUTPUT BUFFER IS SMALLER THAN MESSAGE'S (NOT all bytes are read).
  // In case the output buffer is smaller than message to be read.
  if (nBuffLen < nMsgLen) {
    // I need to update offset for next read.
    // so only left over bytes will be read next time.
    qMqttMsg.front()->moveOffset(nBuffLen);
    // I need to also update unreadBytes.
    return nBuffLen;
  }

  // otherwise, message has been fully read, so we remove it from the queue
  // pointers are freed up
  removeMsg();

  return nBuffLen;
}

ESP_MqttMsg::ESP_MqttMsg(const struct mosquitto_message* msg) {
  readOffset = 0;
  // Serializing piece of code
  int topicLen = strlen(msg->topic);
  length = msg->payloadlen + topicLen +
           strlen(ESP_MQTT_DELIM_END);  // extra tail chars at the end of topic
  buffSerialized = (char*)malloc(length);

  char* pRef = buffSerialized;

  if (buffSerialized != NULL) {
    memcpy(buffSerialized, msg->topic, topicLen);
    buffSerialized +=
        topicLen;  // pointer moved to the next available position for payload

    memcpy(buffSerialized, ESP_MQTT_DELIM_END, strlen(ESP_MQTT_DELIM_END));
    buffSerialized += strlen(ESP_MQTT_DELIM_END);

    memcpy(buffSerialized, msg->payload, msg->payloadlen);

    buffSerialized = pRef;  // this was pointing to original position.
  } else {
    length = -1;
  }
}

ESP_MqttMsg::~ESP_MqttMsg() { free(buffSerialized); }

int ESP_MqttMsg::getTotalLength() { return length; }

int ESP_MqttMsg::getRemainingLength() { return length - readOffset; }

char* ESP_MqttMsg::getBufferToBeCopied(int* len) {
  char* temp = buffSerialized + readOffset;
  *len = getRemainingLength();
  return temp;
}

void ESP_MqttMsg::moveOffset(int nbytes) { readOffset += nbytes; }
