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
#include "input_buffer/ESP_Plugin_Input_Buffer.h"
#include "ESP_XmlUtils.h"

ESP_Plugin_Input_Buffer* ESP_Plugin_Input_Buffer::instance = NULL;

/* ---------------------- */
/* PLUGIN INPUT BUFFER      */
/* ---------------------- */
ESP_Plugin_Input_Buffer::ESP_Plugin_Input_Buffer() { id = 1; }

ESP_Plugin_Input_Base* ESP_Plugin_Input_Buffer::getSingleton() {
  if (ESP_Plugin_Input_Buffer::instance == NULL) {
    ESP_Plugin_Input_Buffer::instance = new ESP_Plugin_Input_Buffer();
  }
  return ESP_Plugin_Input_Buffer::instance;
}

ESP_Plugin_Input_Buffer* ESP_Plugin_Input_Buffer::getInstance() {
  return (ESP_Plugin_Input_Buffer*)ESP_Plugin_Input_Buffer::getSingleton();
}

ESP_Input_Base* ESP_Plugin_Input_Buffer::createInput(TiXmlElement* element) {
  std::string type = ESP_XmlUtils::queryStringValue(element, "type");
  std::string name = ESP_XmlUtils::queryStringValue(element, "name");

  ESP_Input_Base* result = NULL;
  if (type == "buffer") {
    result = new ESP_Input_Buffer();
  }

  // Assigns
  if (result != NULL) {
    result->_type = type;
    result->_name = name;
  }

  return result;
}

int ESP_Plugin_Input_Buffer::getID() { return id++; }

/* ---------------------- */
/* INPUT BUFFER           */
/* ---------------------- */

ESP_Input_Buffer::ESP_Input_Buffer() {}

void ESP_Input_Buffer::parseCustomElement(TiXmlElement* element) {
  this->_name = ESP_XmlUtils::queryStringValue(element, "name");
}

int ESP_Input_Buffer::openServer() { return -1; }

int ESP_Input_Buffer::acceptServer() { return -1; }

bool ESP_Input_Buffer::stopServer() { return false; }

bool ESP_Input_Buffer::closeServer() { return false; }

int ESP_Input_Buffer::openClient() {
  return ESP_Plugin_Input_Buffer::getInstance()->getID();
}

bool ESP_Input_Buffer::stopClient(int id) { return true; }

bool ESP_Input_Buffer::closeClient(int id) { return true; }

int ESP_Input_Buffer::readClient(int id, char* buffer, int len) {
  const char* result = NULL;
  int readsize = len > context.getAvailableInputData()
                     ? context.getAvailableInputData()
                     : len;

  if (context.readPInputData(readsize, result)) {
    memcpy(buffer, result, readsize);
  }
  return readsize;
}

int ESP_Input_Buffer::writeClient(int id, char* buffer, int len) {
  context.addOutputData(buffer, len);
  return len;
}

void ESP_Input_Buffer::createInputData(char* buffer, int len) {
  context.addInputData(buffer, len);
}