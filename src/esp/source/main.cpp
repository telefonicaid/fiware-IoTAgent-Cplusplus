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
#include <iostream>
#include <stdio.h>
#include "ESPLib.h"
#include "ESP_StringUtils.h"
#include "CC_Logger.h"
#include "input_buffer/ESP_Plugin_Input_Buffer.h"

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
//TEST
#include "ESP_SBC_Command.h"

std::string make_daytime_string2() {
  using namespace std; // For time_t, time and ctime;
  time_t now = time(0);
  return ctime(&now);
}

void* tempThread(void* param) {
  // RunOnce Test
  /*
  ESPLib* esplib = (ESPLib*)param;
  char buffer[1024] = "tst/id1234/temperature/50";
  int length = strlen(buffer);
  std::vector<ESP_Attribute> result = esplib->executeRunner(1, std::map<std::string,std::string>(), "main",buffer,length);
  for (size_t i=0;i<result.size();i++)
  {
      std::string temp = result.at(i).getStringValue();
      printf("%s\n",temp.c_str());
  }
  */
  return 0;
}

void resultCallback(void* userData, ESP_Runner* runner) {
  ESPLib* esplib = (ESPLib*)userData;
  ESP_Attribute* sensortype = NULL,*apikey = NULL,*sensorid = NULL, *cmd=NULL;

  for (std::vector<CC_AttributesType>::iterator it =
         runner->context.results.begin(); it != runner->context.results.end(); it++) {
    CC_Logger::getSingleton()->logDebug("New Measure:");
    for (CC_AttributesType::iterator it2 = it->begin(); it2 != it->end(); it2++) {
      CC_Logger::getSingleton()->logDebug("	Attribute Name: %s Value: %s",
                                          it2->second.getName().c_str(), it2->second.getValueAsString().c_str());

      // Get sensorid,apikey,getcmd
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
  }

  // GetCMD
  if (sensortype != NULL && sensorid != NULL && apikey != NULL &&
      sensortype->getValueAsString() == "cmdget") {
    // Get all Commands
    std::vector<std::string> cmdPtes;
    cmdPtes.push_back("CMD1");

    for (size_t i=0; i<cmdPtes.size(); i++) {
      std::string cmdText = "cmdid|1234567890";

      std::map<std::string,std::string> params;
      params.insert(std::pair<std::string,std::string>("apikey",
                    apikey->getValueAsString()));
      params.insert(std::pair<std::string,std::string>("sensorid",
                    sensorid->getValueAsString()));
      params.insert(std::pair<std::string,std::string>("cmdname","setdatetime"));
      params.insert(std::pair<std::string,std::string>("cmdparams",cmdText));
      std::vector<ESP_Attribute> result = esplib->executeRunnerFromRunner(
                                            runner->sensor->_id,"sendcmd",params,runner).ppresults;
    }
  }
  return;
}

/* ****************
 SENSOR SIMULATOR
****************** */
void sensorCallback(void* userData, ESP_Runner* runner) {
  ESPLib* esplib = (ESPLib*)userData;

  ESP_Attribute* apikey = NULL;
  ESP_Attribute* sensorid = NULL;
  ESP_Attribute* type = NULL;
  ESP_Attribute* cmdname = NULL;
  ESP_Attribute* cmdid = NULL;

  for (std::vector<CC_AttributesType>::iterator it =
         runner->context.results.begin(); it != runner->context.results.end(); it++)
    for (CC_AttributesType::iterator it2 = it->begin(); it2 != it->end(); it2++) {
      printf("Attribute Name: %s Value: %s\n",it2->second.getName().c_str(),
             it2->second.getValueAsString().c_str());

      //Get Values
      if (it2->second.getName() == "apikey") {
        apikey = &it2->second;
      }
      if (it2->second.getName() == "sensorid") {
        sensorid = &it2->second;
      }
      if (it2->second.getName() == "type") {
        type = &it2->second;
      }
      if (it2->second.getName() == "cmdname") {
        cmdname = &it2->second;
      }
      if (it2->second.getName() == "cmdid") {
        cmdid = &it2->second;
      }
    }

  // Command
  if (apikey!= NULL && sensorid != NULL && type != NULL && cmdid != NULL &&
      cmdname != NULL && type->getValueAsString() == "cmd") {
    CC_ParamsType params;

    // cmdname
    params.insert(std::pair<std::string, std::string>("cmdname",
                  cmdname->getValueAsString()));

    // cmdid
    std::string cmdparams = "cmdid|" + cmdid->getValueAsString() + "#";
    params.insert(std::pair<std::string, std::string>("cmdparams",cmdparams));

    // Send Response
    esplib->executeRunnerFromRunner(runner->sensor->_id,"cmdexe",params,runner);
  }
  return;
}

void* sensorThread(void* userData) {
  ESPLib* esplib = (ESPLib*)userData;

  int idsensor = esplib->createSensor("sensortt.xml");
  esplib->registerResultCallback(idsensor, esplib, sensorCallback);
  esplib->startSensor(idsensor,"main");

  // Wait for Sensor to Connect
  SLEEP(1000);

  while (idsensor >= 0) {
    CC_ParamsType params;
    ESP_Input_Base* input = NULL;

    // Send temperature
    /*
    params.clear();
    std::string value = ESP_StringUtils::intToString(10 + rand()%100);
    params.insert(std::pair<std::string, std::string>("_temperature",value));
    esplib->setParams(idsensor,params);
    input = esplib->sensors[idsensor]->getInputFromName("mqttwritter");
    esplib->executeRunnerFromInput(idsensor,"measuresend",params,input);

    // Ask for commands
    params.clear();
    input = esplib->sensors[idsensor]->getInputFromName("mqttwritter");
    esplib->executeRunnerFromInput(idsensor,"cmdget",params,input);
    */

    SLEEP(3000);
  }

  return NULL;
}

/* ****************
    MAIN
****************** */
int RunStandAlone() {
  ESPLib esplib;

  printf("    ESP Running as Stand Alone\n");
  CC_Logger::getSingleton()->setDirPath("logs");

  // ESP TEST
  CC_Logger::getSingleton()->logDebug("START");
  //char buffer[256] = {0};
  //printf("CWD: %s \n",getcwd(buffer,256));

  int idsensor = esplib.createSensor("sensortt.xml");
  esplib.registerResultCallback(idsensor, &esplib, resultCallback);
  esplib.startSensor(idsensor,"main");

  ESP_Input_Buffer ibuffer;
  std::map<std::string,std::string> params;

  // Test input
  params.clear();
  std::string input =
    "cadena=#8934075379000039321,#0,P1,214,07,33f,633c,#0,K1,900$,#2,T1,23.68,-1$None,#2,LU,116.57,-1$None,#";
  ibuffer.context.addInputData(input.c_str(),input.length());
  ESP_Result result = esplib.executeRunnerFromInput(idsensor,"main",params,
                      &ibuffer);

  /*
  // Test output
  params.clear();
  params.insert(std::pair<std::string,std::string>("_module","P1"));
  params.insert(std::pair<std::string,std::string>("busid","0"));
  params.insert(std::pair<std::string,std::string>("A","125"));
  params.insert(std::pair<std::string,std::string>("B","66"));
  params.insert(std::pair<std::string,std::string>("sleeptime","-1"));
  params.insert(std::pair<std::string,std::string>("sleepcondition","None"));

  esplib.executeRunnerFromInput(idsensor,"ttoutput",params,&ibuffer);
  */

  getwchar();
  esplib.stopSensor(idsensor);
  esplib.destroySensor(idsensor);

  return 0;
}

int RunSimulator() {
  ESPLib esplib;
  printf("    ESP Running as Simulator\n");
  CC_Logger::getSingleton()->setDirPath("logs");

  pthread_t thread;
  pthread_create(&thread,NULL,sensorThread,&esplib);

  getwchar();
  return 0;
}

int RunTests(int argc, char* argv[]) {
  testing::GTEST_FLAG(throw_on_failure) = true;
  testing::InitGoogleMock(&argc, argv);

  CppUnit::Test* suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;

  runner.addTest( suite );

  bool wasSuccessful = runner.run();

  return wasSuccessful ? 0 : 1;

}

void print_usage() {
  printf("ESP Plugin Stand Alone\n");
  printf("    Usage: there are three modes of execution:\n");
  printf("    * Stand Alone, No parameters: ESP \n");
  printf("    * Run Tests: ESP -t\n");
  printf("    * Run as Simulator: ESP -s\n");
  printf("    \n");
  printf("    Note: ESP cannot run as simulator and tests simultaneosly\n");
}

int main(int argc, char* argv[]) {
  int nRes = 1;

  if (argc == 3) {
    print_usage();
    exit(0);
  }


  if (argc > 1) {
    if (0 == strcmp(argv[1],"-s")) {
      nRes = RunSimulator();
    } else if (0 == strcmp(argv[1],"-t")) {
      nRes = RunTests(argc,argv);
    } else {
      print_usage();
      nRes = 0;
    }
  } else {
    nRes = RunStandAlone();
  }

  exit(nRes);
}
