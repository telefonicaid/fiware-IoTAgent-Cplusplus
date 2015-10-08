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
#include "TTBufferTests.h"
#include "CC_Logger.h"

/*
New tests adapted to change on the Postprocessor_PP output
*/
CPPUNIT_TEST_SUITE_REGISTRATION(TTBufferTests);

void TTBufferTests::setUp() {
  postProcessorTT = new ESP_Postprocessor_TT();
  postProcessorTT->load_default_TTModules();

  decodeTT = new iota::esp::tt::DecodeTTJSON();
}

void TTBufferTests::tearDown() {
  CC_Logger::getSingleton()->logDebug(
      "CLEAN UP: deleting created objects COMPLETE");
  delete postp;
  delete decodeTT;
  if (postProcessorTT != NULL) {
    delete postProcessorTT;
  }
}

void TTBufferTests::testParsingTTOpenBuffer() {
  ESPLib esplib;

  CC_Logger::getSingleton()->logDebug(
      "TEST: testParsingTTOpenBuffer starting ... ");

  int idsensor = esplib.createSensor("../../tests/iotagent/sensortt-open.xml");

  std::string expected1(
      "{\"name\":\"t\",\"type\":\"string\",\"value\":\"34\"}");
  std::string expected2(
      "{\"name\":\"posicion\",\"type\":\"string\",\"value\":\"33.000/"
      "-3.234234\"}");
  std::string expected3("{\"name\":\"P1\",\"type\":\"compound\",\"value\":");
  expected3.append("[{\"name\":\"mcc\",\"type\":\"string\",\"value\":\"214\"}");
  expected3.append(",{\"name\":\"mnc\",\"type\":\"string\",\"value\":\"07\"}");
  expected3.append(
      ",{\"name\":\"cell-id\",\"type\":\"string\",\"value\":\"33f\"},");
  expected3.append(
      "{\"name\":\"lac\",\"type\":\"string\",\"value\":\"633c\"}]}");

  std::vector<std::string> v_expected_JSON;
  v_expected_JSON.push_back(expected1);
  v_expected_JSON.push_back(expected2);
  v_expected_JSON.push_back(expected3);

  if (idsensor > 0) {
    esplib.startSensor(idsensor, "main");

    ESP_Input_Buffer ibuffer;
    std::map<std::string, std::string> params;
    // Test input
    params.clear();

    std::string input =
        "cadena=#8934075379000039321,#0,GM,t,34,#0,GM,posicion,33.000/"
        "-3.234234,#0,P1,214,07,33f,633c";

    ibuffer.context.addInputData(input.c_str(), input.length());

    std::vector<CC_AttributesType> result =
        esplib.executeRunnerFromInput(idsensor, "main", params, &ibuffer)
            .attresults;

    CC_Logger::getSingleton()->logDebug("Result size: %d", result.size());

    CPPUNIT_ASSERT(result.size() == 4);

    std::string temp;
    // After resolution of bug IDAS-20245, postprocessor-tt is not included in
    // ESP pipeline anymore
    int i = 0;
    for (int j = 1; j < result.size(); j++) {
      // Note that the first position is the "request" field, so it has to be
      // ignored in this instance
      postProcessorTT->execute(&result[j]);
      temp.assign(postProcessorTT->getResultData());
      decodeTT->parse(temp);
      CPPUNIT_ASSERT_EQUAL(v_expected_JSON[i++], decodeTT->getProcessedJSON());
    }

    CC_Logger::getSingleton()->logDebug("TEST: testParsingTTOpenBuffer DONE");
  }
  CPPUNIT_ASSERT(idsensor > 0);
}

void TTBufferTests::testPostProcessorTTGM() {
  CC_Logger::getSingleton()->logDebug("TEST: testPostProcessorTTGM STARTING");

  CC_AttributesType mapAttr;
  ESP_Attribute esp_busid;

  std::string busid("0");
  std::string module("GM");
  std::string param1("t");
  std::string param2("23");

  std::string expected("{");
  expected.append("\"name\":\"");
  expected.append(param1);
  expected.append("\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(param2);
  expected.append("\"}");

  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGM Initializating postprocessor");
  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));
  CC_Logger::getSingleton()->logDebug("EXPECTED %s", expected.c_str());

  CPPUNIT_ASSERT(postProcessorTT->isResultValid());

  std::string actual(postProcessorTT->getResultData());
  decodeTT->parse(actual);
  CPPUNIT_ASSERT_EQUAL(expected, decodeTT->getProcessedJSON());
  // delete decodeObject;
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGM postprocessor executed");
}

void TTBufferTests::testPostProcessorTTGCwithSleepCondAndSleepTime() {
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGCwithSleepCondAndSleepTime STARTING");

  CC_AttributesType mapAttr;
  ESP_Attribute esp_busid;

  std::string busid("0");
  std::string module("GC");
  std::string param1("polling_rate");
  std::string param2("5000");

  std::string sleeptime("100");
  std::string sleepcond("Wakeup");

  std::string expected("{");
  expected.append("\"name\":\"_TTcurrent_");
  expected.append(param1);
  expected.append("\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(param2);
  expected.append("\",");
  expected.append("\"metadatas\":[{");
  expected.append("\"name\":\"sleepcondition\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(sleepcond);
  expected.append("\"},{");
  expected.append("\"name\":\"sleeptime\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(sleeptime);
  expected.append("\"}]}");

  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_sleepcond;
  esp_sleepcond.setValue("sleepcondition", sleepcond.c_str(),
                         sleepcond.length(), ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_sleeptime;
  esp_sleeptime.setValue("sleeptime", sleeptime.c_str(), sleeptime.length(),
                         ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));
  mapAttr.insert(CC_AttributesPair("sleepcondition", esp_sleepcond));
  mapAttr.insert(CC_AttributesPair("sleeptime", esp_sleeptime));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGCwithSleepCondAndSleepTime Initializating "
      "postprocessor");
  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));
  CC_Logger::getSingleton()->logDebug("EXPECTED %s", expected.c_str());

  CPPUNIT_ASSERT(postProcessorTT->isResultValid());
  std::string actual(postProcessorTT->getResultData());
  decodeTT->parse(actual);
  CC_Logger::getSingleton()->logDebug("ACTUAL %s",
                                      decodeTT->getProcessedJSON().c_str());
  CPPUNIT_ASSERT_EQUAL(expected, decodeTT->getProcessedJSON());
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGCwithSleepCondAndSleepTime postprocessor "
      "executed");
}

void TTBufferTests::testPostProcessorTTGCwithSleepCond() {
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGCwithSleepCond  STARTING");

  CC_AttributesType mapAttr;
  ESP_Attribute esp_busid;

  std::string busid("0");
  std::string module("GC");
  std::string param1("polling_rate");
  std::string param2("5000");

  std::string sleeptime("100");
  std::string sleepcond("Wakeup");

  std::string expected("{");
  expected.append("\"name\":\"_TTcurrent_");
  expected.append(param1);
  expected.append("\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(param2);
  expected.append("\",");
  expected.append("\"metadatas\":[{");
  expected.append("\"name\":\"sleepcondition\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(sleepcond);
  expected.append("\"}]}");

  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_sleepcond;
  esp_sleepcond.setValue("sleepcondition", sleepcond.c_str(),
                         sleepcond.length(), ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));
  mapAttr.insert(CC_AttributesPair("sleepcondition", esp_sleepcond));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGCwithSleepCond Initializating postprocessor");
  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));
  CC_Logger::getSingleton()->logDebug("EXPECTED %s", expected.c_str());
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGCwithSleepCond postprocessor executed");
  CPPUNIT_ASSERT(postProcessorTT->isResultValid());
  std::string actual(postProcessorTT->getResultData());
  decodeTT->parse(actual);
  CPPUNIT_ASSERT_EQUAL(expected, decodeTT->getProcessedJSON());
}

void TTBufferTests::testPostProcessorTTGCwithSleepTime() {
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGCwithSleepTime  STARTING");

  CC_AttributesType mapAttr;
  ESP_Attribute esp_busid;

  std::string busid("0");
  std::string module("GC");
  std::string param1("polling_rate");
  std::string param2("5000");

  std::string sleeptime("100");
  std::string sleepcond("Wakeup");

  std::string expected("{");
  expected.append("\"name\":\"_TTcurrent_");
  expected.append(param1);
  expected.append("\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(param2);
  expected.append("\",");
  expected.append("\"metadatas\":[{");
  expected.append("\"name\":\"sleeptime\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(sleeptime);
  expected.append("\"}]}");

  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_sleeptime;
  esp_sleeptime.setValue("sleeptime", sleeptime.c_str(), sleeptime.length(),
                         ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));
  mapAttr.insert(CC_AttributesPair("sleeptime", esp_sleeptime));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGCwithSleepTime Initializating postprocessor");
  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));
  CC_Logger::getSingleton()->logDebug("EXPECTED %s", expected.c_str());
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGCwithSleepTime postprocessor executed");
  CPPUNIT_ASSERT(postProcessorTT->isResultValid());
  std::string actual(postProcessorTT->getResultData());
  decodeTT->parse(actual);
  CPPUNIT_ASSERT_EQUAL(expected, decodeTT->getProcessedJSON());
}

void TTBufferTests::testPostProcessorTTGMwithSleepCondAndSleepTime() {
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGMwithSleepCondAndSleepTime  STARTING");

  CC_AttributesType mapAttr;
  ESP_Attribute esp_busid;

  std::string busid("0");
  std::string module("GM");
  std::string param1("temp");
  std::string param2("50");

  std::string sleeptime("100");
  std::string sleepcond("Wakeup");

  std::string expected("{");
  expected.append("\"name\":\"");
  expected.append(param1);
  expected.append("\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(param2);
  expected.append("\",");
  expected.append("\"metadatas\":[{");
  expected.append("\"name\":\"sleepcondition\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(sleepcond);
  expected.append("\"},{");
  expected.append("\"name\":\"sleeptime\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(sleeptime);
  expected.append("\"}]}");

  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_sleepcond;
  esp_sleepcond.setValue("sleepcondition", sleepcond.c_str(),
                         sleepcond.length(), ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_sleeptime;
  esp_sleeptime.setValue("sleeptime", sleeptime.c_str(), sleeptime.length(),
                         ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));
  mapAttr.insert(CC_AttributesPair("sleepcondition", esp_sleepcond));
  mapAttr.insert(CC_AttributesPair("sleeptime", esp_sleeptime));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGMwithSleepCondAndSleepTime Initializating "
      "postprocessor");
  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));
  CC_Logger::getSingleton()->logDebug("EXPECTED %s", expected.c_str());
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGMwithSleepCondAndSleepTime postprocessor "
      "executed");
  CPPUNIT_ASSERT(postProcessorTT->isResultValid());
  std::string actual(postProcessorTT->getResultData());

  decodeTT->parse(actual);
  CPPUNIT_ASSERT_EQUAL(expected, decodeTT->getProcessedJSON());
}

void TTBufferTests::testPostProcessorTTP1() {
  CC_Logger::getSingleton()->logDebug("TEST: testPostProcessorTTP1  STARTING");

  CC_AttributesType mapAttr;
  ESP_Attribute esp_busid;

  std::string busid("0");
  std::string module("P1");
  std::string param1("23");
  std::string param2("45");
  std::string param3("55");
  std::string param4("77");
  std::string param5("99");

  std::string values("");
  std::string sleeptime("100");
  std::string sleepcond("Wakeup");

  std::string expected("{");

  expected.append("\"name\":\"");
  expected.append(module);
  expected.append("\",");
  expected.append("\"type\":\"compound\",");
  expected.append("\"value\":[{");

  expected.append("\"name\":\"mcc\",\"type\":\"string\",\"value\":\"");
  expected.append(param1);
  expected.append("\"},{");

  expected.append("\"name\":\"mnc\",\"type\":\"string\",\"value\":\"");
  expected.append(param2);
  expected.append("\"},{");
  expected.append("\"name\":\"cell-id\",\"type\":\"string\",\"value\":\"");
  expected.append(param3);
  expected.append("\"},{");
  expected.append("\"name\":\"lac\",\"type\":\"string\",\"value\":\"");
  expected.append(param4);
  expected.append("\"},{");
  expected.append("\"name\":\"dbm\",\"type\":\"string\",\"value\":\"");
  expected.append(param5);
  expected.append("\"");
  expected.append("}],");
  expected.append("\"metadatas\":[{");
  expected.append("\"name\":\"sleepcondition\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(sleepcond);
  expected.append("\"},{");
  expected.append("\"name\":\"sleeptime\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(sleeptime);
  expected.append("\"}]}");

  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param3;
  esp_param3.setValue("param3", param3.c_str(), param3.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param4;
  esp_param4.setValue("param4", param4.c_str(), param4.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param5;
  esp_param5.setValue("param5", param5.c_str(), param5.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);

  ESP_Attribute esp_sleepcond;
  esp_sleepcond.setValue("sleepcondition", sleepcond.c_str(),
                         sleepcond.length(), ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_sleeptime;
  esp_sleeptime.setValue("sleeptime", sleeptime.c_str(), sleeptime.length(),
                         ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));
  mapAttr.insert(CC_AttributesPair("param_3", esp_param3));
  mapAttr.insert(CC_AttributesPair("param_4", esp_param4));
  mapAttr.insert(CC_AttributesPair("param_5", esp_param5));
  mapAttr.insert(CC_AttributesPair("sleepcondition", esp_sleepcond));
  mapAttr.insert(CC_AttributesPair("sleeptime", esp_sleeptime));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTP1 Initializating postprocessor");
  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));
  CC_Logger::getSingleton()->logDebug("EXPECTED %s", expected.c_str());

  CPPUNIT_ASSERT(postProcessorTT->isResultValid());
  std::string actual(postProcessorTT->getResultData());
  decodeTT->parse(actual);
  CC_Logger::getSingleton()->logDebug("ACTUAL %s",
                                      decodeTT->getProcessedJSON().c_str());
  CPPUNIT_ASSERT_EQUAL(expected, decodeTT->getProcessedJSON());
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTP1 postprocessor executed");
}

void TTBufferTests::testPostProcessorTTGPS() {
  CC_Logger::getSingleton()->logDebug("TEST: testPostProcessorTTGPS  STARTING");

  CC_AttributesType mapAttr;
  ESP_Attribute esp_busid;

  std::string busid("0");
  std::string module("GPS");
  std::string param1("40.418889");
  std::string param2("-3.691944");
  std::string sleeptime("100");
  std::string sleepcond("Wakeup");

  std::string expected("{");

  /*
  {
        "name": "position",
        "type": "coords",
        "value": "40.418889, -3.691944",
        "metadatas": [
        {
          "name": "location",
          "type": "string",
          "value": "WGS84"
        },
        {
          "name" : "sleepcondition",
          ...
        }

        ]
  }

  */
  expected.append("\"name\":\"position\",");
  expected.append("\"type\":\"coords\",");
  expected.append("\"value\":\"40.418889,-3.691944\",");
  expected.append("\"metadatas\":[{");
  expected.append("\"name\":\"location\",\"type\":\"string\",");
  expected.append("\"value\":\"WGS84\"},{");
  expected.append("\"name\":\"sleepcondition\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(sleepcond);
  expected.append("\"},{");
  expected.append("\"name\":\"sleeptime\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(sleeptime);
  expected.append("\"}]}");

  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);

  ESP_Attribute esp_sleepcond;
  esp_sleepcond.setValue("sleepcondition", sleepcond.c_str(),
                         sleepcond.length(), ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_sleeptime;
  esp_sleeptime.setValue("sleeptime", sleeptime.c_str(), sleeptime.length(),
                         ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));
  mapAttr.insert(CC_AttributesPair("sleepcondition", esp_sleepcond));
  mapAttr.insert(CC_AttributesPair("sleeptime", esp_sleeptime));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGPS Initializating postprocessor");
  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));
  CC_Logger::getSingleton()->logDebug("EXPECTED %s", expected.c_str());

  CPPUNIT_ASSERT(postProcessorTT->isResultValid());
  std::string actual(postProcessorTT->getResultData());
  decodeTT->parse(actual);
  CPPUNIT_ASSERT_EQUAL(expected, decodeTT->getProcessedJSON());
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGPS postprocessor executed");
}

void TTBufferTests::testPostProcessorTTGPSNoSleepCondOrTime() {
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGPSNoSleepCondOrTime  STARTING");

  CC_AttributesType mapAttr;
  ESP_Attribute esp_busid;

  std::string busid("0");
  std::string module("GPS");
  std::string param1("40.418889");
  std::string param2("-3.691944");

  /*
  {
        "name": "position",
        "type": "coords",
        "value": "40.418889, -3.691944",
        "metadatas": [
        {
          "name": "location",
          "type": "string",
          "value": "WGS84"
        },
        {
          "name" : "sleepcondition",
          ...
        }

        ]
  }

  */
  std::string expected("{");
  expected.append("\"name\":\"position\",");
  expected.append("\"type\":\"coords\",");
  expected.append("\"value\":\"40.418889,-3.691944\",");
  expected.append("\"metadatas\":[{");
  expected.append("\"name\":\"location\",\"type\":\"string\",");
  expected.append("\"value\":\"WGS84\"}]}");

  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGPSNoSleepCondOrTime Initializating "
      "postprocessor");
  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));
  CC_Logger::getSingleton()->logDebug("EXPECTED %s", expected.c_str());

  CPPUNIT_ASSERT(postProcessorTT->isResultValid());
  std::string actual(postProcessorTT->getResultData());
  decodeTT->parse(actual);
  CPPUNIT_ASSERT_EQUAL(expected, decodeTT->getProcessedJSON());
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGPSNoSleepCondOrTime postprocessor executed");
}

void TTBufferTests::testPostProcessorTTGPSSleepCondOrTimeEmpty() {
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGPSSleepCondOrTimeEmpty  STARTING");

  CC_AttributesType mapAttr;
  ESP_Attribute esp_busid;

  std::string busid("0");
  std::string module("GPS");
  std::string param1("40.418889");
  std::string param2("-3.691944");
  std::string sleeptime("");
  std::string sleepcond("");

  std::string expected("{");

  /*
  {
        "name": "position",
        "type": "coords",
        "value": "40.418889, -3.691944",
        "metadatas": [
        {
          "name": "location",
          "type": "string",
          "value": "WGS84"
        },
        {
          "name" : "sleepcondition",
          ...
        }

        ]
  }

  */
  expected.append("\"name\":\"position\",");
  expected.append("\"type\":\"coords\",");
  expected.append("\"value\":\"40.418889,-3.691944\",");
  expected.append("\"metadatas\":[{");
  expected.append("\"name\":\"location\",\"type\":\"string\",");
  expected.append("\"value\":\"WGS84\"}]}");

  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);

  ESP_Attribute esp_sleepcond;
  esp_sleepcond.setValue("sleepcondition", sleepcond.c_str(),
                         sleepcond.length(), ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_sleeptime;
  esp_sleeptime.setValue("sleeptime", sleeptime.c_str(), sleeptime.length(),
                         ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));
  mapAttr.insert(CC_AttributesPair("sleepcondition", esp_sleepcond));
  mapAttr.insert(CC_AttributesPair("sleeptime", esp_sleeptime));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGPSSleepCondOrTimeEmpty Initializating "
      "postprocessor");
  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));
  CC_Logger::getSingleton()->logDebug("EXPECTED %s", expected.c_str());

  CPPUNIT_ASSERT(postProcessorTT->isResultValid());
  std::string actual(postProcessorTT->getResultData());
  decodeTT->parse(actual);
  CPPUNIT_ASSERT_EQUAL(expected, decodeTT->getProcessedJSON());
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTGPSSleepCondOrTimeEmpty postprocessor "
      "executed");
}

/**
  @brief The bug appears when a measure with either sleepcond or sleeptime is
  processed and next another without them
  comes in, the previous value is stored and returned on the recent measure,
  which is wrong.
*/
void TTBufferTests::testPostProcessorWrongSleepCondTimeOnBug_IDAS20215() {
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorWrongSleepCondTimeOnBug_IDAS20215 Starting...");

  CC_AttributesType mapAttr;
  ESP_Attribute esp_busid;

  std::string busid("0");
  std::string module("GM");
  std::string param1("temp");
  std::string param2("50");

  std::string sleeptime("100");
  std::string sleepcond("Wakeup");

  std::string expected("{");
  expected.append("\"name\":\"");
  expected.append(param1);
  expected.append("\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(param2);
  expected.append("\",");
  expected.append("\"metadatas\":[{");
  expected.append("\"name\":\"sleepcondition\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(sleepcond);
  expected.append("\"},{");
  expected.append("\"name\":\"sleeptime\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(sleeptime);
  expected.append("\"}]}");

  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_sleepcond;
  esp_sleepcond.setValue("sleepcondition", sleepcond.c_str(),
                         sleepcond.length(), ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_sleeptime;
  esp_sleeptime.setValue("sleeptime", sleeptime.c_str(), sleeptime.length(),
                         ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));
  mapAttr.insert(CC_AttributesPair("sleepcondition", esp_sleepcond));
  mapAttr.insert(CC_AttributesPair("sleeptime", esp_sleeptime));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorWrongSleepCondTimeOnBug_IDAS20215 Initializating "
      "postprocessor");
  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));
  CC_Logger::getSingleton()->logDebug("EXPECTED %s", expected.c_str());
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorWrongSleepCondTimeOnBug_IDAS20215 postprocessor "
      "executed FIRST TIME");
  CPPUNIT_ASSERT(postProcessorTT->isResultValid());
  std::string actual(postProcessorTT->getResultData());

  decodeTT->parse(actual);
  CPPUNIT_ASSERT_EQUAL(expected, decodeTT->getProcessedJSON());

  mapAttr.clear();
  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorWrongSleepCondTimeOnBug_IDAS20215 Initializating "
      "postprocessor SECOND TIME");
  expected.assign("{");
  expected.append("\"name\":\"");
  expected.append(param1);
  expected.append("\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(param2);
  expected.append("\"");
  expected.append("}");
  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));
  CC_Logger::getSingleton()->logDebug("EXPECTED %s", expected.c_str());
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorWrongSleepCondTimeOnBug_IDAS20215 postprocessor "
      "executed SECOND TIME");
  CPPUNIT_ASSERT(postProcessorTT->isResultValid());
  actual.assign(postProcessorTT->getResultData());

  decodeTT->parse(actual);
  CPPUNIT_ASSERT_EQUAL(expected, decodeTT->getProcessedJSON());
}

/**
  @brief Same bug (sleepcond and sleeptime) are retained from previous measure,
  but reverse direction, first measure without
  sleepcond and sleeptime and next with those values.
*/
void TTBufferTests::
    testPostProcessorReverseWrongSleepCondTimeOnBug_IDAS20215() {
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorReverseWrongSleepCondTimeOnBug_IDAS20215 "
      "Starting...");

  CC_AttributesType mapAttr;
  ESP_Attribute esp_busid;

  std::string busid("0");
  std::string module("GM");
  std::string param1("temp");
  std::string param2("50");

  std::string sleeptime("100");
  std::string sleepcond("Wakeup");

  std::string expected("{");

  expected.append("\"name\":\"");
  expected.append(param1);
  expected.append("\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(param2);
  expected.append("\"");
  expected.append("}");
  postProcessorTT->initialize();

  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_sleepcond;
  esp_sleepcond.setValue("sleepcondition", sleepcond.c_str(),
                         sleepcond.length(), ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_sleeptime;
  esp_sleeptime.setValue("sleeptime", sleeptime.c_str(), sleeptime.length(),
                         ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorReverseWrongSleepCondTimeOnBug_IDAS20215 "
      "Initializating postprocessor");
  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));
  CC_Logger::getSingleton()->logDebug("EXPECTED %s", expected.c_str());
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorReverseWrongSleepCondTimeOnBug_IDAS20215 "
      "postprocessor executed FIRST TIME");
  CPPUNIT_ASSERT(postProcessorTT->isResultValid());
  std::string actual(postProcessorTT->getResultData());

  decodeTT->parse(actual);
  CPPUNIT_ASSERT_EQUAL(expected, decodeTT->getProcessedJSON());

  mapAttr.clear();
  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));
  mapAttr.insert(CC_AttributesPair("sleepcondition", esp_sleepcond));
  mapAttr.insert(CC_AttributesPair("sleeptime", esp_sleeptime));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorReverseWrongSleepCondTimeOnBug_IDAS20215 "
      "Initializating postprocessor SECOND TIME");
  expected.assign("{");
  expected.append("\"name\":\"");
  expected.append(param1);
  expected.append("\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(param2);
  expected.append("\",");
  expected.append("\"metadatas\":[{");
  expected.append("\"name\":\"sleepcondition\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(sleepcond);
  expected.append("\"},{");
  expected.append("\"name\":\"sleeptime\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(sleeptime);
  expected.append("\"}]}");

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));
  CC_Logger::getSingleton()->logDebug("EXPECTED %s", expected.c_str());
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorReverseWrongSleepCondTimeOnBug_IDAS20215 "
      "postprocessor executed SECOND TIME");
  CPPUNIT_ASSERT(postProcessorTT->isResultValid());
  actual.assign(postProcessorTT->getResultData());

  decodeTT->parse(actual);
  CPPUNIT_ASSERT_EQUAL(expected, decodeTT->getProcessedJSON());
}

void TTBufferTests::testPostProcessorMissingAttributes() {
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorMissingAttributes Starting...");

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorMissingAttributes checking P1...");

  CC_AttributesType mapAttr;
  ESP_Attribute esp_busid;

  std::string busid("0");
  std::string module("P1");
  std::string param1("11");
  std::string param2("20");
  std::string param3("30");

  std::string sleeptime("100");
  std::string sleepcond("Wakeup");

  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);

  ESP_Attribute esp_param3;
  esp_param3.setValue("param3", param3.c_str(), param3.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);

  ESP_Attribute esp_sleepcond;
  esp_sleepcond.setValue("sleepcondition", sleepcond.c_str(),
                         sleepcond.length(), ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_sleeptime;
  esp_sleeptime.setValue("sleeptime", sleeptime.c_str(), sleeptime.length(),
                         ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));
  mapAttr.insert(CC_AttributesPair("param_3", esp_param3));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorMissingAttributes Initializating postprocessor");
  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorMissingAttributes postprocessor executed FOR P1");
  CPPUNIT_ASSERT(!postProcessorTT->isResultValid());

  postProcessorTT->terminate();

  mapAttr.clear();
  module.assign("GPS");

  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));

  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorMissingAttributes postprocessor executed FOR "
      "GPS");
  CPPUNIT_ASSERT(!postProcessorTT->isResultValid());
  postProcessorTT->terminate();
  mapAttr.clear();
  module.assign("B");

  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));
  mapAttr.insert(CC_AttributesPair("param_3", esp_param3));

  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorMissingAttributes postprocessor executed FOR B");
  CPPUNIT_ASSERT(!postProcessorTT->isResultValid());
  postProcessorTT->terminate();

  mapAttr.clear();
  module.assign("GM");

  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));

  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorMissingAttributes postprocessor executed FOR GM");
  CPPUNIT_ASSERT(!postProcessorTT->isResultValid());
  postProcessorTT->terminate();
}

void TTBufferTests::testPostProcessorTTB() {
  CC_AttributesType mapAttr;
  ESP_Attribute esp_busid;

  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTB Initializating postprocessor");
  std::string busid("0");
  std::string module("B");
  std::string param1("11");
  std::string param2("22");
  std::string param3("33");
  std::string param4("44");
  std::string param5("55");
  std::string param6("66");

  std::string values("");
  std::string sleeptime("100");
  std::string sleepcond("Wakeup");

  std::string expected("{");

  expected.append("\"name\":\"");
  expected.append(module);
  expected.append("\",");
  expected.append("\"type\":\"compound\",");
  expected.append("\"value\":[{");

  expected.append("\"name\":\"voltaje\",\"type\":\"string\",\"value\":\"");
  expected.append(param1);
  expected.append("\"},{");

  expected.append(
      "\"name\":\"estado_activacion\",\"type\":\"string\",\"value\":\"");
  expected.append(param2);
  expected.append("\"},{");
  expected.append("\"name\":\"hay_cargador\",\"type\":\"string\",\"value\":\"");
  expected.append(param3);
  expected.append("\"},{");
  expected.append("\"name\":\"estado_carga\",\"type\":\"string\",\"value\":\"");
  expected.append(param4);
  expected.append("\"},{");
  expected.append("\"name\":\"modo_carga\",\"type\":\"string\",\"value\":\"");
  expected.append(param5);
  expected.append("\"},{");
  expected.append(
      "\"name\":\"tiempo_desco_stack\",\"type\":\"string\",\"value\":\"");
  expected.append(param6);
  expected.append("\"");
  expected.append("}],");
  expected.append("\"metadatas\":[{");
  expected.append("\"name\":\"sleepcondition\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(sleepcond);
  expected.append("\"},{");
  expected.append("\"name\":\"sleeptime\",");
  expected.append("\"type\":\"string\",");
  expected.append("\"value\":\"");
  expected.append(sleeptime);
  expected.append("\"}]}");

  esp_busid.setValue("busid", busid.c_str(), busid.length(),
                     ESP_DataType::ESP_DataType_STRING,
                     ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_module;
  esp_module.setValue("module", module.c_str(), module.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param1;
  esp_param1.setValue("param1", param1.c_str(), param1.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param2;
  esp_param2.setValue("param2", param2.c_str(), param2.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param3;
  esp_param3.setValue("param3", param3.c_str(), param3.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param4;
  esp_param4.setValue("param4", param4.c_str(), param4.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param5;
  esp_param5.setValue("param5", param5.c_str(), param5.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_param6;
  esp_param6.setValue("param6", param6.c_str(), param6.length(),
                      ESP_DataType::ESP_DataType_STRING,
                      ESP_DataCode::ESP_DataCode_STRING);

  ESP_Attribute esp_sleepcond;
  esp_sleepcond.setValue("sleepcondition", sleepcond.c_str(),
                         sleepcond.length(), ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);
  ESP_Attribute esp_sleeptime;
  esp_sleeptime.setValue("sleeptime", sleeptime.c_str(), sleeptime.length(),
                         ESP_DataType::ESP_DataType_STRING,
                         ESP_DataCode::ESP_DataCode_STRING);

  mapAttr.insert(CC_AttributesPair("busid", esp_busid));
  mapAttr.insert(CC_AttributesPair("module", esp_module));
  mapAttr.insert(CC_AttributesPair("param_1", esp_param1));
  mapAttr.insert(CC_AttributesPair("param_2", esp_param2));
  mapAttr.insert(CC_AttributesPair("param_3", esp_param3));
  mapAttr.insert(CC_AttributesPair("param_4", esp_param4));
  mapAttr.insert(CC_AttributesPair("param_5", esp_param5));
  mapAttr.insert(CC_AttributesPair("param_6", esp_param6));
  mapAttr.insert(CC_AttributesPair("sleepcondition", esp_sleepcond));
  mapAttr.insert(CC_AttributesPair("sleeptime", esp_sleeptime));

  postProcessorTT->initialize();

  CPPUNIT_ASSERT(postProcessorTT->execute(&mapAttr));
  CC_Logger::getSingleton()->logDebug("EXPECTED %s", expected.c_str());

  CPPUNIT_ASSERT(postProcessorTT->isResultValid());
  std::string actual(postProcessorTT->getResultData());
  decodeTT->parse(actual);
  CC_Logger::getSingleton()->logDebug("ACTUAL %s",
                                      decodeTT->getProcessedJSON().c_str());
  CPPUNIT_ASSERT_EQUAL(expected, decodeTT->getProcessedJSON());
  CC_Logger::getSingleton()->logDebug(
      "TEST: testPostProcessorTTB postprocessor executed");
}

bool TTBufferTests::compareBuffers(const char* src, int srclen, const char* dst,
                                   int dstlen) {
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
