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
#ifndef __ESP_PLUGIN_POSTPROCESSOR_SENSORML_H__
#define __ESP_PLUGIN_POSTPROCESSOR_SENSORML_H__
#include <TDA.h>
#include <string>
#include <stdlib.h>
#include "ESP_SML_Classes.h"

#define SML_MEASURE 1
#define SML_COMMAND 2
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< PLUGIN POSTPROCESSOR BASIC >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
class ESP_Plugin_Postprocessor_Sensorml : public ESP_Plugin_Postprocessor_Base {
 private:
  static ESP_Plugin_Postprocessor_Sensorml* instance;

 public:
  ESP_Postprocessor_Base* createPostprocessor(TiXmlElement* element);

  static ESP_Plugin_Postprocessor_Sensorml* getSingleton();
};

class ESP_Postprocessor_Sensorml : public ESP_Postprocessor_Base {
 private:
  ESP_SML_Keyword smlKeyword;
  ESP_SML_Map mapTrans;
  pthread_mutex_t mutexPost;

  int subType;

 public:
  std::string result;

  ESP_SML_Base* smlObject;
  TiXmlDocument doc;

  ESP_Postprocessor_Sensorml(std::string name, int subType = SML_MEASURE);
  ~ESP_Postprocessor_Sensorml();

  TiXmlPrinter* printer;
  TiXmlElement* xmlRes;
  TiXmlDeclaration* decl;
  // Methods
  const char* getResultData();
  int getResultSize();

  bool isResultValid();
  void preProcessAttributes(CC_AttributesType* attributes);  // Get Attributes
                                                             // and create new
                                                             // ones if needed

  // Override
  bool initialize();
  bool execute(CC_AttributesType* result);
  bool terminate();

  void parseCustomElement(TiXmlElement* element);
};

#endif
