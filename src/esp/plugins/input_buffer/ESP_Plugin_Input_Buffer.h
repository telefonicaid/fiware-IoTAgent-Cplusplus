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
#ifndef SRC_ESP_PLUGIN_INPUT_BUFFER_H_
#define SRC_ESP_PLUGIN_INPUT_BUFFER_H_
#include <TDA.h>

/* //////////////////  */
/* PLUGIN INPUT BUFFER   */
/* //////////////////  */
class ESP_Plugin_Input_Buffer : public ESP_Plugin_Input_Base {
 private:
  static ESP_Plugin_Input_Buffer* instance;
  int id;
  std::map<int, FILE*> files;
  ESP_Plugin_Input_Buffer();

 public:
  static ESP_Plugin_Input_Base* getSingleton();
  static ESP_Plugin_Input_Buffer* getInstance();
  ESP_Input_Base* createInput(TiXmlElement* element);

  // Methods
  int getID();
};

class ESP_Input_Buffer : public ESP_Input_Base {
 public:
  std::string _name;
  ESP_Context context;

  ESP_Input_Buffer();
  int openServer();
  int acceptServer();
  bool stopServer();
  bool closeServer();

  int openClient();
  bool stopClient(int id);
  bool closeClient(int id);
  int readClient(int id, char* buffer, int len);
  int writeClient(int id, char* buffer, int len);
  void parseCustomElement(TiXmlElement* element);

  // Custom
  void createInputData(char* buffer, int len);
};

#endif
