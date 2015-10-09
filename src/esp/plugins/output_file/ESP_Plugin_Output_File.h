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
#ifndef __ESP_PLUGIN_OUTPUT_FILE_H__
#define __ESP_PLUGIN_OUTPUT_FILE_H__
#include <TDA.h>

/* //////////////////  */
/* PLUGIN OUTPUT FILE   */
/* //////////////////  */
class ESP_Plugin_Output_File : public ESP_Plugin_Output_Base {
 private:
  static ESP_Plugin_Output_File* instance;
  int id;
  std::map<int, FILE*> files;
  ESP_Plugin_Output_File();

 public:
  static ESP_Plugin_Output_Base* getSingleton();
  static ESP_Plugin_Output_File* getInstance();
  ESP_Output_Base* createOutput(TiXmlElement* element);

  // Methods
  bool writeToFile(std::string filename, std::string result);
};

class ESP_Output_File : public ESP_Output_Base {
 public:
  std::string _name;

  bool execute(CC_AttributesType* attributes,
               ESP_Postprocessor_Base* postprocessor,
               std::map<std::string, void*> userData);
  void parseCustomElement(TiXmlElement* element);
};

#endif
