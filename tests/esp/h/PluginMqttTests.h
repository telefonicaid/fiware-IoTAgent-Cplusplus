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
#ifndef PLUGINMQTTTESTS_H
#define PLUGINMQTTTESTS_H
#include "TDA.h"
#include "../../../src/iotagent/esp/plugins/input_mqtt/ESP_Plugin_Input_Mqtt.h"
#include <cppunit/extensions/HelperMacros.h>

#define SERVER "server"
#define PUBLISH "apikey/sensorid/type"
#define SUBSCRIBE "#"
#define HOST "localhost"
#define PORT "1883"
#define USER "admin"
#define PASSWORD "1234"
#define NAME "mqttrunner"
#define CLEAN_SESSION "true"
#define PATH_MQTT "mosquitto"
#define CONFIG_MQTT "mosquitto.conf"

class PluginMqttTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(PluginMqttTests);

  CPPUNIT_TEST(testParseAllParameters);
  CPPUNIT_TEST(testParseMinimumParameters);
  CPPUNIT_TEST(testParseAllBroker);

  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

 protected:
  void testParseAllParameters();
  void testParseMinimumParameters();

  void testParseAllBroker();

  // void testBrokerOrServerMQTTLogic();
  // void testLaunchAndStopMQTT();

 private:
  TiXmlElement *xmlTestBasic, *xmlTestFull;
  ESP_Input_Mqtt pluginMqtt;
  ESP_Input_MqttBroker pluginBroker;
  ESP_MqttWrapper* mqttWrapper;
};

#endif  // PLUGINMQTTTESTS_H
