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
#ifndef ESP_PLUGIN_POSTPROCESSOR_TT_H
#define ESP_PLUGIN_POSTPROCESSOR_TT_H
#include <TDA.h>
#include <string>
#include <stdlib.h>
#include "TTModules.h"

#define TAG_XML_TTOPEN "ttopen"
#define TAG_XML_TTMODULE "ttmodule"

#define ATTRB_KEYWORD "keyword"
#define ATTRB_REF "ref"

#define TT_PROCESSED "processed"
#define TT_PLAIN "plain"

class ESP_Plugin_Postprocessor_TT : public ESP_Plugin_Postprocessor_Base {
 private:
  static ESP_Plugin_Postprocessor_TT* instance;
  ESP_Plugin_Postprocessor_TT();

 public:
  virtual ~ESP_Plugin_Postprocessor_TT(){};

  ESP_Postprocessor_Base* createPostprocessor(TiXmlElement* element);

  static ESP_Plugin_Postprocessor_TT* getSingleton();
};

class ESP_Postprocessor_TT : public ESP_Postprocessor_Base {
 private:
  std::map<std::string, TTModules*> modulesMap;

  std::string result;
  bool resultValid;

  std::string valueRef;
  std::string ttOpenKeyword_GM;
  std::string ttOpenKeyword_GC;

 public:
  static std::string TYPE;

  ESP_Postprocessor_TT();
  virtual ~ESP_Postprocessor_TT();

  // Methods
  const char* getResultData();
  int getResultSize();
  bool isResultValid();

  bool initialize();
  bool execute(CC_AttributesType* result);
  bool terminate();

  void parseCustomElement(TiXmlElement* element);

  bool checkParams();

  void load_default_TTModules();
};

#endif  // ESP_PLUGIN_POSTPROCESSOR_TT_H
