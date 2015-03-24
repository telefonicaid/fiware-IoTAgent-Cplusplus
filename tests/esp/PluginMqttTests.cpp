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
#include "h/PluginMqttTests.h"




CPPUNIT_TEST_SUITE_REGISTRATION(PluginMqttTests);

std::string to_string(int value) {

  char buffer[10] = {0};
  sprintf(buffer,"%d",value);
  return std::string(buffer);

}

void PluginMqttTests::setUp() {
  //
  xmlTestBasic = new TiXmlElement("input");
  xmlTestBasic->SetAttribute("publish",PUBLISH);
  xmlTestBasic->SetAttribute("subscribe",SUBSCRIBE);
  xmlTestBasic->SetAttribute("host",HOST);
  xmlTestBasic->SetAttribute("port",PORT);

  xmlTestFull = (TiXmlElement*)xmlTestBasic->Clone();

  xmlTestFull->SetAttribute("user",USER);
  xmlTestFull->SetAttribute("password",PASSWORD);
  xmlTestFull->SetAttribute("name",NAME);
  xmlTestFull->SetAttribute("clean-session",CLEAN_SESSION);
  xmlTestFull->SetAttribute("path",PATH_MQTT);
  xmlTestFull->SetAttribute("config",CONFIG_MQTT);


}

void PluginMqttTests::tearDown() {
  delete xmlTestBasic;
  delete xmlTestFull;
}

//testing the parse of ALL mandatory parameters for MQTT Input Plugin.
void PluginMqttTests::testParseAllParameters() {
  pluginMqtt.parseCustomElement(xmlTestFull);

  CPPUNIT_ASSERT_EQUAL(std::string(PUBLISH),pluginMqtt.topicPublish);
  CPPUNIT_ASSERT_EQUAL(std::string(SUBSCRIBE),pluginMqtt.topicSubscribe);
  CPPUNIT_ASSERT_EQUAL(std::string(HOST),pluginMqtt.host);
  CPPUNIT_ASSERT_EQUAL(std::string(PORT),to_string(pluginMqtt.port));

  CPPUNIT_ASSERT_EQUAL(std::string(USER),pluginMqtt.user);
  CPPUNIT_ASSERT_EQUAL(std::string(PASSWORD),pluginMqtt.passwd);
  //CPPUNIT_ASSERT_EQUAL(std::string (NAME),pluginMqtt._name);
  CPPUNIT_ASSERT(strcmp(CLEAN_SESSION,
                        "true")==0 ? true : false == pluginMqtt.cleanSession);


}

//At least some minimum parameters must be set, otherwise, it would fail.
void PluginMqttTests::testParseMinimumParameters() {

  pluginMqtt.parseCustomElement(xmlTestBasic);

  CPPUNIT_ASSERT_EQUAL(std::string(PUBLISH),pluginMqtt.topicPublish);
  CPPUNIT_ASSERT_EQUAL(std::string(SUBSCRIBE),pluginMqtt.topicSubscribe);
  CPPUNIT_ASSERT_EQUAL(std::string(HOST),pluginMqtt.host);
  CPPUNIT_ASSERT_EQUAL(std::string(PORT),to_string(pluginMqtt.port));

  CPPUNIT_ASSERT(true == pluginMqtt.cleanSession);

}


void PluginMqttTests::testParseAllBroker() {
  pluginBroker.parseCustomElement(xmlTestFull);

  CPPUNIT_ASSERT_EQUAL(std::string(PATH_MQTT),pluginBroker.pathToBroker);
  CPPUNIT_ASSERT_EQUAL(std::string(CONFIG_MQTT),pluginBroker.pathToConfig);

}


