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
#include "h/TDAParserTests.h"
#include "ESPLib.h"
#include "CC_Logger.h"
#include "input_buffer/ESP_Plugin_Input_Buffer.h"

CPPUNIT_TEST_SUITE_REGISTRATION(TDAParserTests);

void TDAParserTests::setUp() {
  CC_Logger::getSingleton()->logDebug("TDAParserTests UP");
}

void TDAParserTests::tearDown() {
  CC_Logger::getSingleton()->logDebug("TDAParserTests DOWN");
}

void TDAParserTests::testParseBinary() {
  ESPLib esplib;
  //sensor->registerDataCallback(NULL, ESPLib::sensorDataCallback);
  //sensor->registerResultCallback(NULL, ESPLib::sensorResultCallback);
  int sensorid = esplib.createSensor("test_sensor.xml");
  if (sensorid >= 0) {
    CC_ParamsType params;
    ESP_Input_Base* input = NULL;
    params.clear();
    char* buffer = (char*)"data00";
    std::vector<ESP_Attribute> result = esplib.executeRunnerFromBuffer(sensorid,
                                        "binary",params,buffer,strlen(buffer)).ppresults; // 00 in ascii == 12336 as int
    esplib.destroySensor(sensorid);
    CPPUNIT_ASSERT(result.size() == 1);

    if (result.size() == 1) {
      ESP_Attribute value = result.at(0);
      std::string valuestr = value.getValueAsString();
      CPPUNIT_ASSERT(valuestr == "value:12336\n");
    }
  }

  CPPUNIT_ASSERT(sensorid >=0);
}

void TDAParserTests::testParseASCII() {
  ESPLib esplib;
  //sensor->registerDataCallback(NULL, ESPLib::sensorDataCallback);
  //sensor->registerResultCallback(NULL, ESPLib::sensorResultCallback);
  int sensorid = esplib.createSensor("test_sensor.xml");
  if (sensorid >= 0) {
    CC_ParamsType params;
    ESP_Input_Base* input = NULL;
    params.clear();
    char* buffer = (char*) "data88";
    std::vector<ESP_Attribute> result = esplib.executeRunnerFromBuffer(sensorid,
                                        "ascii",params,buffer,strlen(buffer)).ppresults;
    esplib.destroySensor(sensorid);
    CPPUNIT_ASSERT(result.size() == 1);

    if (result.size() == 1) {
      ESP_Attribute value = result.at(0);
      std::string valuestr = value.getValueAsString();
      CPPUNIT_ASSERT(valuestr == "value:88\n");
    }
  }

  CPPUNIT_ASSERT(sensorid >=0);
}

void TDAParserTests::testParseLoop() {
  ESPLib esplib;
  //sensor->registerDataCallback(NULL, ESPLib::sensorDataCallback);
  //sensor->registerResultCallback(NULL, ESPLib::sensorResultCallback);
  int sensorid = esplib.createSensor("test_sensor.xml");
  if (sensorid >= 0) {
    CC_ParamsType params;
    ESP_Input_Base* input = NULL;
    params.clear();
    char* buffer = (char*)"data55#80#";
    std::vector<ESP_Attribute> result = esplib.executeRunnerFromBuffer(sensorid,
                                        "loop",params,buffer,strlen(buffer)).ppresults;
    esplib.destroySensor(sensorid);
    CPPUNIT_ASSERT(result.size() == 1);

    if (result.size() == 1) {
      ESP_Attribute value = result.at(0);
      std::string valuestr = value.getValueAsString();
      CPPUNIT_ASSERT(valuestr == "value:80\n");
    }
  }

  CPPUNIT_ASSERT(sensorid >=0);
}

void TDAParserTests::testParseConditionClear() {
  ESPLib esplib;
  //sensor->registerDataCallback(NULL, ESPLib::sensorDataCallback);
  //sensor->registerResultCallback(NULL, ESPLib::sensorResultCallback);
  int sensorid = esplib.createSensor("test_sensor.xml");
  if (sensorid >= 0) {
    CC_ParamsType params;
    ESP_Input_Base* input = NULL;
    params.clear();
    char* buffer = (char*)"data#5";
    std::vector<ESP_Attribute> result = esplib.executeRunnerFromBuffer(sensorid,
                                        "conditionclear",params,buffer,strlen(buffer)).ppresults;
    esplib.destroySensor(sensorid);
    CPPUNIT_ASSERT(result.size() == 1);

    if (result.size() == 1) {
      ESP_Attribute value = result.at(0);
      std::string valuestr = value.getValueAsString();
      CPPUNIT_ASSERT(valuestr == "value:5\n");
    }
  }

  CPPUNIT_ASSERT(sensorid >=0);
}

void TDAParserTests::testParseWrite() {
  ESPLib esplib;
  //sensor->registerDataCallback(NULL, ESPLib::sensorDataCallback);
  //sensor->registerResultCallback(NULL, ESPLib::sensorResultCallback);
  int sensorid = esplib.createSensor("test_sensor.xml");
  if (sensorid >= 0) {
    CC_ParamsType params;
    ESP_Input_Buffer* input = new ESP_Input_Buffer();
    int inputid = input->openClient();
    input->createInputData((char*)"data",4);
    std::vector<ESP_Attribute> result = esplib.executeRunnerFromInput(sensorid,
                                        "write",params,input).ppresults;
    esplib.destroySensor(sensorid);
    CPPUNIT_ASSERT(result.size() == 1);

    if (result.size() == 1) {
      CPPUNIT_ASSERT(input->context.getOutputSize() == 1);
    }
    delete input;
  }

  CPPUNIT_ASSERT(sensorid >=0);
}

void TDAParserTests::testConditionDefault() {
  ESPLib esplib;
  //sensor->registerDataCallback(NULL, ESPLib::sensorDataCallback);
  //sensor->registerResultCallback(NULL, ESPLib::sensorResultCallback);
  int sensorid = esplib.createSensor("test_sensor.xml");
  CC_Logger::getSingleton()->logDebug("TDAParserTests : testing condition default operation...");
  if (sensorid >= 0) {
    CC_ParamsType params;

    params.clear();
    char* buffer = (char*)"1234-apikey/id-device01/t//23";//MQTT message

    std::vector<ESP_Attribute> result = esplib.executeRunnerFromBuffer(sensorid,
                                        "conditiondefault",params,buffer,strlen(buffer)).ppresults;
    esplib.destroySensor(sensorid);

    //apikey == 1234-apikey
    //sensorid == id-device01
    //type == t
    //payload == 23
    ESP_Attribute value = result.at(0);

    CPPUNIT_ASSERT_EQUAL(
      std::string("apikey:1234-apikey\npayload:23\nsensorid:id-device01\ntype:t\n"),
      value.getValueAsString());

    CC_Logger::getSingleton()->logDebug("TDAParserTests : testing condition default operation... DONE");
  }
  CPPUNIT_ASSERT(sensorid >=0);
}



void TDAParserTests::testSwitch() {
  ESPLib esplib;
  //sensor->registerDataCallback(NULL, ESPLib::sensorDataCallback);
  //sensor->registerResultCallback(NULL, ESPLib::sensorResultCallback);
  int sensorid = esplib.createSensor("test_sensor.xml");
  CC_Logger::getSingleton()->logDebug("TDAParserTests : testing switch operation...");
  if (sensorid >= 0) {
    CC_ParamsType params;

    params.clear();
    char* buffer = (char*)"1234-apikey/id-device01/t//23";//MQTT message

    std::vector<ESP_Attribute> result = esplib.executeRunnerFromBuffer(sensorid,
                                        "switch",params,buffer,strlen(buffer)).ppresults;
    esplib.destroySensor(sensorid);
    CPPUNIT_ASSERT(result.size() == 1);

    //apikey == 1234-apikey
    //sensorid == id-device01
    //type == t
    //payload == 23
    ESP_Attribute value;
    if (result.size() == 1) {
      value = result.at(0);
      std::string valuestr = value.getValueAsString();
      CPPUNIT_ASSERT(valuestr == "value:23\n");
    }

    CC_Logger::getSingleton()->logDebug("TDAParserTests : testing switch operation... DONE");

  }
  CPPUNIT_ASSERT(sensorid >=0);
}
