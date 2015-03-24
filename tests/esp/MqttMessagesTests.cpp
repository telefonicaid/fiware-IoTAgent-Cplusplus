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
#include "CC_Logger.h"
#include "h/MqttMessagesTests.h"
#include "ESPLib.h"

using ::testing::Return;
using ::testing::_;
using ::testing::Invoke;
using ::testing::StrEq;

CPPUNIT_TEST_SUITE_REGISTRATION(MqttMessagesTests);

//Method called when a cmdget mqtt message is received
//it also makes the CPPUNIT_ASSERT call
void resultTestCallback(void* userData, ESP_Runner* runner) {
  //ESPLib *esplib = (ESPLib*)userData;
  CC_Logger::getSingleton()->logDebug("resultTestCallback INVOKED ...");

  for (std::vector<CC_AttributesType>::iterator it =
         runner->context.results.begin(); it != runner->context.results.end(); it++) {
    ESP_Attribute* sensortype = NULL,*apikey = NULL,*sensorid = NULL, *cmd=NULL;

    for (CC_AttributesType::iterator it2 = it->begin(); it2 != it->end(); it2++) {
      printf("Attribute Name: %s Value: %s\n",it2->second.getName().c_str(),
             it2->second.getValueAsString().c_str());

      if (it2->second.getName() == "apikey") {
        apikey = &it2->second;
      }
      if (it2->second.getName() == "sensorid") {
        sensorid = &it2->second;
      }
      if (it2->second.getName() == "cmd") {
        cmd = &it2->second;
      }
      if (it2->second.getName() == "type") {
        sensortype = &it2->second;
      }

    }
    CC_Logger::getSingleton()->logDebug("cmdget received? ...");
    CPPUNIT_ASSERT(sensortype->getValueAsString() == "cmdget");
    CC_Logger::getSingleton()->logDebug("cmdget received? ... YES");
  }
}

void MqttMessagesTests::setUp() {
  pluginInput = new MockPluginInputMqtt();
}

void MqttMessagesTests::tearDown() {
  delete pluginInput;
}

//it will load a valid XML and use a mock to simulate
//the reception of a MQTT message.
//ASSUMPTION: timestamp are fixed within SensorML created by postprocessors.
void MqttMessagesTests::testBasicMqttMeasure() {
  ESPLib libESP;
  //Loading of actual SensorMQTT XML file.
  int sensorid = libESP.createSensor("sensormqtt_fixed_timestamp.xml");
  CC_Logger::getSingleton()->logDebug("Test testBasicMqttMeasure ...");
  mqttTestMessage = "1234/id001/temperature//33";

  std::string sensorML = loadSMLFromFile("temperature_33.sml");


  if (sensorid >= 0) {
    CC_ParamsType params;
    params.clear();

    //Setting up expectations
    EXPECT_CALL(*pluginInput,openClient()).WillOnce(Return(0));
    EXPECT_CALL(*pluginInput,readClient(_,_,_)).WillOnce(Invoke(this,
        &MqttMessagesTests::stubReadClient)).WillRepeatedly(Return(0));
    EXPECT_CALL(*pluginInput,closeClient(_)).WillOnce(Return(true));

    std::vector<ESP_Attribute> result = libESP.executeRunnerFromInput(sensorid,
                                        "main",params,pluginInput).ppresults;
    libESP.destroySensor(sensorid);

    CPPUNIT_ASSERT(result.size() == 1);

    if (result.size() == 1) {
      ESP_Attribute value = result.at(0);
      std::string valuestr = value.getValueAsString();
      CC_Logger::getSingleton()->logDebug("EXPECTED: %s",sensorML.c_str());
      CPPUNIT_ASSERT(valuestr == sensorML);

    }
  }

  CPPUNIT_ASSERT(sensorid >=0);

}

//This will test what happens when multi is used as last topic, in other words,
//when we are receiving measures in  UL2.0 protocol format within the payload
void MqttMessagesTests::testMultiMeasure() {
  //Expected behaviour is: we receive a message with several measures,
  //and we produce individual SensorML responses.

  ESPLib libESP;
  //Loading of actual SensorMQTT XML file.
  int sensorid = libESP.createSensor("sensormqtt_fixed_timestamp.xml");
  CC_Logger::getSingleton()->logDebug("Test testMultiMeasure ...");
  mqttTestMessage =
    "1234/id001/multi//temperature|33#location|29/29/29#"; //sending juts two measures.

  //Expected SensorML outputs.
  std::string sensorML_temp = loadSMLFromFile("temperature_33.sml");
  std::string sensorML_loc = loadSMLFromFile("location_29-29.sml");

  if (sensorid >= 0) {
    CC_ParamsType params;
    params.clear();
    //Setting up expectations
    EXPECT_CALL(*pluginInput,openClient()).WillOnce(Return(0));
    EXPECT_CALL(*pluginInput,readClient(_,_,_)).WillOnce(Invoke(this,
        &MqttMessagesTests::stubReadClient)).WillRepeatedly(Return(0));
    EXPECT_CALL(*pluginInput,closeClient(_)).WillOnce(Return(true));

    std::vector<ESP_Attribute> result = libESP.executeRunnerFromInput(sensorid,
                                        "main",params,pluginInput).ppresults;
    libESP.destroySensor(sensorid);

    CPPUNIT_ASSERT(result.size() == 2); //checking there are two results

    //Checking first result.
    CC_Logger::getSingleton()->logDebug("Test  testMultiMeasure: checking first output...");
    ESP_Attribute value = result.at(0);
    std::string valuestr = value.getValueAsString();
    CC_Logger::getSingleton()->logDebug("EXPECTED: %s",sensorML_temp.c_str());
    CPPUNIT_ASSERT(valuestr == sensorML_temp);

    CC_Logger::getSingleton()->logDebug("Test  testMultiMeasure: checking second output...");
    value = result.at(1);
    valuestr = value.getValueAsString();
    CC_Logger::getSingleton()->logDebug("EXPECTED: %s",sensorML_loc.c_str());
    CPPUNIT_ASSERT(valuestr == sensorML_loc);


  }

  CPPUNIT_ASSERT(sensorid >=
                 0);//it will fail if files weren't parsed for instance.
  CC_Logger::getSingleton()->logDebug("Test  testMultiMeasure: ... DONE");
}

//This tests that ESP Plugin invokes get_all_commands on IDAS Service after
//receiving a "cmdget" mqtt message.
void MqttMessagesTests::testRequestingCommands() {

  ESPLib libESP;

  int sensorid = libESP.createSensor("sensormqtt_fixed_timestamp.xml");

  CC_Logger::getSingleton()->logDebug("Test testRequestingCommands ...");
  mqttTestMessage = "1234/id001/cmdget//0"; //requesting a CMD by pulling
  CC_Logger::getSingleton()->logDebug("Test testRequestingCommands ... registering callback: sensorid %d",
                                      sensorid);
  libESP.registerResultCallback(sensorid,&libESP, resultTestCallback);

  if (sensorid >= 0) {
    CC_ParamsType params;
    params.clear();
    //Setting up expectations
    EXPECT_CALL(*pluginInput,openClient()).WillOnce(Return(0));
    EXPECT_CALL(*pluginInput,readClient(_,_,_)).WillOnce(Invoke(this,
        &MqttMessagesTests::stubReadClient)).WillRepeatedly(Return(0));
    EXPECT_CALL(*pluginInput,closeClient(_)).WillOnce(Return(true));

    std::vector<ESP_Attribute> result = libESP.executeRunnerFromInput(sensorid,
                                        "main",params,pluginInput).ppresults;
    libESP.destroySensor(sensorid);
    //Assertion is done within resultTestCallback.
  }
  //NOTE: there shouldn't be any CPPUNIT_ASSERT call in here, but in the resultTestCallback method so it's the best way
  //to test that the callback was actually invoked.
  CC_Logger::getSingleton()->logDebug("Test testRequestingCommands ... DONE");
}

//This is for sending a response to a command coming from the device.
void MqttMessagesTests::testSendingResponseToCommand() {
  ESPLib libESP;

  int sensorid = libESP.createSensor("sensormqtt_fixed_timestamp.xml");

  CC_Logger::getSingleton()->logDebug("Test testSendingResponseToCommand ...");
  mqttTestMessage =
    "1234/id001/cmdexe/TEST//cmdid|123456789"; //response to a cmd.

  std::string sensorML = loadSMLFromFile("cmdresponse.sml");

  if (sensorid >= 0) {
    CC_ParamsType params;
    params.clear();
    //Setting up expectations
    EXPECT_CALL(*pluginInput,openClient()).WillOnce(Return(0));
    EXPECT_CALL(*pluginInput,readClient(_,_,_)).WillOnce(Invoke(this,
        &MqttMessagesTests::stubReadClient)).WillRepeatedly(Return(0));
    EXPECT_CALL(*pluginInput,closeClient(_)).WillOnce(Return(true));

    std::vector<ESP_Attribute> result = libESP.executeRunnerFromInput(sensorid,
                                        "main",params,pluginInput).ppresults;
    libESP.destroySensor(sensorid);

    if (result.size() == 1) {
      ESP_Attribute value = result.at(0);
      std::string valuestr = value.getValueAsString();
      CC_Logger::getSingleton()->logDebug("EXPECTED: %s",sensorML.c_str());
      CPPUNIT_ASSERT(valuestr == sensorML);

    }

  }
  CPPUNIT_ASSERT(sensorid >=0);
  CC_Logger::getSingleton()->logDebug("Test testSendingResponseToCommand ... DONE");
}


void MqttMessagesTests::testUL20Postprocessor() {
  ESPLib esplib;
  //sensor->registerDataCallback(NULL, ESPLib::sensorDataCallback);
  //sensor->registerResultCallback(NULL, ESPLib::sensorResultCallback);
  int sensorid = esplib.createSensor("test_sensor_ul20.xml");
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing UL2.0 postprocessor...");
  std::string expectedUL20 = "t|23";
  if (sensorid >= 0) {
    CC_ParamsType params;

    params.clear();
    char* buffer = (char*) "1234-apikey/id-device01/t//23";//MQTT message

    std::vector<ESP_Attribute> result = esplib.executeRunnerFromBuffer(sensorid,
                                        "conditiondefault",params,buffer,strlen(buffer)).ppresults;
    esplib.destroySensor(sensorid);

    ESP_Attribute value = result.at(1); //assuming there are two outputs.
    CPPUNIT_ASSERT_EQUAL(expectedUL20,value.getValueAsString());

  }
  CPPUNIT_ASSERT(sensorid >=0);
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing UL2.0 postprocessor... DONE");

}

void MqttMessagesTests::testUL20Error() {

  /**
  Test modified at 29-10-2014 in order to allow empty messages to be
  passed across Context Broker. This means that if a MQTT message arrives, even if
  there is no payload, an output to the destination will be still produced
  */
  ESPLib esplib;
  int sensorid = esplib.createSensor("test_sensor_ul20.xml");
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing UL2.0 empty message...");
  std::string expected ="t|";
  if (sensorid >= 0) {
    CC_ParamsType params;

    params.clear();
    char* buffer = (char*) "1234-apikey/id-device01/t//";//MQTT message, incomplete

    std::vector<ESP_Attribute> result = esplib.executeRunnerFromBuffer(sensorid,
                                        "conditiondefault",params,buffer,strlen(buffer)).ppresults;
    esplib.destroySensor(sensorid);

    //ESP_Attribute value = result.at(0); //assuming there are two outputs.

    CC_Logger::getSingleton()->logDebug("Output: Length %d",result.size());
    CPPUNIT_ASSERT(result.size()==3);
    ESP_Attribute value = result.at(1);
    CC_Logger::getSingleton()->logDebug("Output test:  %s",
                                        value.getValueAsString().c_str());
    CPPUNIT_ASSERT_EQUAL(expected,value.getValueAsString());

  }
  CPPUNIT_ASSERT(sensorid >=0);
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing UL2.0 empty message... DONE");
}

/**
This test will process /multi/ topics with inputs in UL2.0 format
and instead of using SensorML as output, it will just use UL2.0
It requires a UL20 postprocessor with tag "bypass" properly configured.
*/
void MqttMessagesTests::testUL20andMultiMeasures() {

  ESPLib esplib;
  //sensor->registerDataCallback(NULL, ESPLib::sensorDataCallback);
  //sensor->registerResultCallback(NULL, ESPLib::sensorResultCallback);
  int sensorid = esplib.createSensor("test_sensor_ul20.xml");
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing UL2.0 multi measures...");
  std::string expectedUL20 = "t|23#p|33#loc|33.234/23.33";
  if (sensorid >= 0) {
    CC_ParamsType params;

    params.clear();
    char* buffer = (char*)
                   "1234-apikey/id-device01/multi//t|23#p|33#loc|33.234/23.33";//MQTT message, three measures including location

    std::vector<ESP_Attribute> result = esplib.executeRunnerFromBuffer(sensorid,
                                        "mqtt-multiul20",params,buffer,strlen(buffer)).ppresults;
    esplib.destroySensor(sensorid);



    CC_Logger::getSingleton()->logDebug("Output: Length %d",result.size());
    CPPUNIT_ASSERT(result.size()==3);
    ESP_Attribute value = result.at(2);
    CC_Logger::getSingleton()->logDebug("Output test:  %s",
                                        value.getValueAsString().c_str());
    CPPUNIT_ASSERT_EQUAL(expectedUL20,value.getValueAsString());


  }
  CPPUNIT_ASSERT(sensorid >=0);
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing UL2.0 multi measures... DONE");
}

/**
This test is for the scenario where bypass is configured for the UL20 postprocessor
however, a single measure is received, therefore, UL20 has to be constructed using
traditional single measure approach.
*/
void MqttMessagesTests::testUL20bypassConfiguredButSingleMeasure() {
  ESPLib esplib;
  //sensor->registerDataCallback(NULL, ESPLib::sensorDataCallback);
  //sensor->registerResultCallback(NULL, ESPLib::sensorResultCallback);
  int sensorid = esplib.createSensor("test_sensor_ul20.xml");
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing UL2.0 bypass with single measure...");
  std::string expectedUL20 = "t|23";
  if (sensorid >= 0) {
    CC_ParamsType params;

    params.clear();
    char* buffer = (char*) "1234-apikey/id-device01/t//23";//MQTT message.

    std::vector<ESP_Attribute> result = esplib.executeRunnerFromBuffer(sensorid,
                                        "mqtt-multiul20",params,buffer,strlen(buffer)).ppresults;
    esplib.destroySensor(sensorid);



    CC_Logger::getSingleton()->logDebug("Output: Length %d",result.size());
    CPPUNIT_ASSERT(result.size()==3);
    ESP_Attribute value = result.at(2);
    CC_Logger::getSingleton()->logDebug("Output test:  %s",
                                        value.getValueAsString().c_str());
    CPPUNIT_ASSERT_EQUAL(expectedUL20,value.getValueAsString());


  }
  CPPUNIT_ASSERT(sensorid >=0);
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing UL2.0 bypass with single measure... DONE");
}

/**
    What happens when a single mode UL20 postprocessor receives a multi measure,
    that is to say, postprocessor is not configured with bypass. (In the .xml file,
    this is the first UL20 postprocessor or the second after the Text one).

    This is an unexpected behaviour, so, if "multi" measures are expected, bypass
    MUST be included, otherwise, inconsistent behaviour happens (like this test).
*/
void MqttMessagesTests::testUL20MultiMeasuresForSingleMeasureConfiguration() {
  ESPLib esplib;
  //sensor->registerDataCallback(NULL, ESPLib::sensorDataCallback);
  //sensor->registerResultCallback(NULL, ESPLib::sensorResultCallback);
  int sensorid = esplib.createSensor("test_sensor_ul20.xml");
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing UL2.0 bypass with single measure...");
  std::string expectedUL20 = ""; //This is not a valid measure UL2.0!!!
  if (sensorid >= 0) {
    CC_ParamsType params;

    params.clear();
    char* buffer = (char*)
                   "1234-apikey/id-device01/multi//t|23#p|33#loc|33.234/23.33";//MQTT message.

    std::vector<ESP_Attribute> result = esplib.executeRunnerFromBuffer(sensorid,
                                        "mqtt-multiul20",params,buffer,strlen(buffer)).ppresults;
    esplib.destroySensor(sensorid);


    CC_Logger::getSingleton()->logDebug("Output: Length %d",result.size());
    CPPUNIT_ASSERT(result.size()==3);
    ESP_Attribute value = result.at(1);
    CC_Logger::getSingleton()->logDebug("Output test:  %s",
                                        value.getValueAsString().c_str());
    CPPUNIT_ASSERT(expectedUL20 != value.getValueAsString());


  }
  CPPUNIT_ASSERT(sensorid >=0);
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing UL2.0 bypass with single measure... DONE");
}

//Test what happens when UL2.0 is configured along with SensorML for single measure scenario.
void MqttMessagesTests::testUL20SingleWithDifferentConditions() {
  ESPLib esplib;
  //sensor->registerDataCallback(NULL, ESPLib::sensorDataCallback);
  //sensor->registerResultCallback(NULL, ESPLib::sensorResultCallback);
  int sensorid = esplib.createSensor("sensormqtt_fixed_timestamp_ul20.xml");
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing  SensorML conditions and UL2.0 single measure...");
  std::string expectedUL20 = "t|23";
  if (sensorid >= 0) {
    CC_ParamsType params;

    params.clear();
    char* buffer = (char*) "1234-apikey/id-device01/t//23";//MQTT message.

    std::vector<ESP_Attribute> result = esplib.executeRunnerFromBuffer(sensorid,
                                        "main",params,buffer,strlen(buffer)).ppresults;
    esplib.destroySensor(sensorid);


    CC_Logger::getSingleton()->logDebug("Output: Length %d",result.size());
    CPPUNIT_ASSERT(result.size()==1);  //only UL20 postpro should pick up the result
    ESP_Attribute value = result.at(0);
    CC_Logger::getSingleton()->logDebug("Output test:  %s",
                                        value.getValueAsString().c_str());
    CPPUNIT_ASSERT_EQUAL(expectedUL20,value.getValueAsString());

  }
  CPPUNIT_ASSERT(sensorid >=0);
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing  SensorML conditions and UL2.0 single measure... DONE");

}


/**
* This test was failing prior adding "switch/option" tags. Now it should work.
 This particular scenario is, when we have a set of options, being one the default (not stating multi specifically)
 and a multi-measure comes in with UL20 in the payload. The expected result is that rest of not matching options
 are ignored and this is processed. A previous test was testing multi option on its own, so this is a step up.

*/
void MqttMessagesTests::testUL20MultiMeasureWithDifferentConditions() {
  ESPLib esplib;
  //sensor->registerDataCallback(NULL, ESPLib::sensorDataCallback);
  //sensor->registerResultCallback(NULL, ESPLib::sensorResultCallback);
  int sensorid = esplib.createSensor("test_sensor_ul20.xml");
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing  Multiple conditions with UL2.0 MULTI measure...");
  std::string expectedUL20 = "temperature|23#presence|1";
  if (sensorid >= 0) {
    CC_ParamsType params;

    params.clear();
    char* buffer = (char*)
                   "1234-apikey/id-device01/multi//temperature|23#presence|1";//MQTT message.

    std::vector<ESP_Attribute> result = esplib.executeRunnerFromBuffer(sensorid,
                                        "conditiondefault",params,buffer,strlen(buffer)).ppresults;
    esplib.destroySensor(sensorid);


    CC_Logger::getSingleton()->logDebug("Output: Length %d",result.size());
    CPPUNIT_ASSERT(result.size()==3);
    ESP_Attribute value = result.at(
                            2); //this is the postprocessor that has been properly configured for ul20 multi.
    CC_Logger::getSingleton()->logDebug("Output test:  %s",
                                        value.getValueAsString().c_str());
    CPPUNIT_ASSERT_EQUAL(expectedUL20,value.getValueAsString());


  }
  CPPUNIT_ASSERT(sensorid >=0);
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing  Multiple conditions with UL2.0 MULTI measure... DONE");

}


void MqttMessagesTests::testSensorMLWithUL20PostProcessor() {
  ESPLib esplib;
  //sensor->registerDataCallback(NULL, ESPLib::sensorDataCallback);
  //sensor->registerResultCallback(NULL, ESPLib::sensorResultCallback);
  int sensorid = esplib.createSensor("sensormqtt_fixed_timestamp_ul20.xml");
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing  SensorML matching conditions and UL2.0 SINGLE measure...");
  //std::string expectedUL20 = "t|23";
  if (sensorid >= 0) {
    CC_ParamsType params;

    params.clear();
    char* buffer = (char*) "1234-apikey/id-device01/temperature//23";//MQTT message.

    std::vector<ESP_Attribute> result = esplib.executeRunnerFromBuffer(sensorid,
                                        "main",params,buffer,strlen(buffer)).ppresults;
    esplib.destroySensor(sensorid);

    CC_Logger::getSingleton()->logDebug("Output: Length %d",result.size());
    CPPUNIT_ASSERT(result.size()==1);  //only UL20 postpro should pick up the result
    ESP_Attribute value = result.at(0);
    CC_Logger::getSingleton()->logDebug("Output test:  %s",
                                        value.getValueAsString().c_str());
    //CPPUNIT_ASSERT_EQUAL(expectedUL20,value.getValueAsString());


  }
  CPPUNIT_ASSERT(sensorid >=0);
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing  SensorML matching conditions and UL2.0 SINGLE measure... DONE");

}

void MqttMessagesTests::testUL20NewTopicForMulti() {
  ESPLib esplib;
  //sensor->registerDataCallback(NULL, ESPLib::sensorDataCallback);
  //sensor->registerResultCallback(NULL, ESPLib::sensorResultCallback);
  int sensorid = esplib.createSensor("sensormqtt.xml");
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing  UL20 with new topic for multi");
  std::string expectedUL20 = "t|23#t|34#p|1#pressure|34";
  if (sensorid >= 0) {
    CC_ParamsType params;

    params.clear();
    char* buffer = (char*)
                   "1234-apikey/id-device01/mul20//t|23#t|34#p|1#pressure|34";//MQTT message.

    std::vector<ESP_Attribute> result = esplib.executeRunnerFromBuffer(sensorid,
                                        "main",params,buffer,strlen(buffer)).ppresults;
    esplib.destroySensor(sensorid);

    CC_Logger::getSingleton()->logDebug("Output: Length %d",result.size());
    CPPUNIT_ASSERT(result.size()==1);  //only UL20 postpro should pick up the result
    ESP_Attribute value = result.at(0);
    CC_Logger::getSingleton()->logDebug("Output test:  %s",
                                        value.getValueAsString().c_str());
    CPPUNIT_ASSERT_EQUAL(expectedUL20,value.getValueAsString());


  }
  CPPUNIT_ASSERT(sensorid >=0);
  CC_Logger::getSingleton()->logDebug("MqttMessagesTests : testing  UL20 with new topic for multi ... DONE");

}

/*
void MqttMessagesTests::testSendToSBC()
{
    ESPLib libESP;
    std::string resource= "esp";
    int sensorid = libESP.createSensor("sensormqtt_fixed_timestamp_ul20.xml");

    CC_Logger::getSingleton()->logDebug("Test testSendToSBC ...");

    idasService = new MockIDASService();

    mqttTestMessage = "1234/id001/t//23"; //UL20


    std::string expected_qp = "apikey=1234&i=id001";
    std::string expected_result = "t|23";

    //Setting expectations for IDASService.

    EXPECT_CALL(*idasService,SOSReceiveMessage(StrEq("POST"),StrEq("10.1.2.3"),StrEq(resource),StrEq(expected_qp),StrEq(expected_result),_,StrEq(""))).WillOnce(Return(200));


    std::map<std::string, void*> userData;
    userData.insert(std::pair<std::string, void*>("_idas_service",(void*)&idasService));
    userData.insert(std::pair<std::string, void*>("_resource", (void*)&resource));

    libESP.setUserData(sensorid,userData);


    if (sensorid >= 0)
    {
        CC_ParamsType params;
        params.clear();
        //Setting up expectations
        EXPECT_CALL(*pluginInput,openClient()).WillOnce(Return(0));
        EXPECT_CALL(*pluginInput,readClient(_,_,_)).WillOnce(Invoke(this,&MqttMessagesTests::stubReadClient)).WillRepeatedly(Return(0));
        EXPECT_CALL(*pluginInput,closeClient(_)).WillOnce(Return(true));

        std::vector<ESP_Attribute> result = libESP.executeRunnerFromInput(sensorid,"main",params,pluginInput).ppresults;

        delete idasService;
        libESP.destroySensor(sensorid);

    }
    CPPUNIT_ASSERT(sensorid >=0);

    CC_Logger::getSingleton()->logDebug("Test testSendToSBC ... DONE");
}
*/


std::string MqttMessagesTests::loadSMLFromFile(const char* filename) {
  TiXmlDocument doc(filename);
  CPPUNIT_ASSERT(doc.LoadFile());
  TiXmlPrinter printer;
  printer.SetIndent("    ");
  doc.Accept(&printer);
  return printer.CStr();
}

int MqttMessagesTests::stubReadClient(int id,char* buffer,int len) {
  if (buffer == NULL) {
    return 0;
  }
  else {
    //COPY
    CC_Logger::getSingleton()->logDebug("STUB READ CLIENT: Copying message into buffer");
    memcpy(buffer,mqttTestMessage.c_str(),mqttTestMessage.length());
    return mqttTestMessage.length();
  }
}

