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
#ifndef __ESP_PLUGIN_POSTPROCESSOR_TEXT_H__
#define __ESP_PLUGIN_POSTPROCESSOR_TEXT_H__
#include <TDA.h>
#include <string>
#include <stdlib.h>

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< PLUGIN POSTPROCESSOR BASIC >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
class ESP_Plugin_Postprocessor_Text : public ESP_Plugin_Postprocessor_Base {
 private:
  static ESP_Plugin_Postprocessor_Text* instance;

 public:
  ESP_Postprocessor_Base* createPostprocessor(TiXmlElement* element);
  static ESP_Plugin_Postprocessor_Text* getSingleton();
};

class ESP_Postprocessor_Text : public ESP_Postprocessor_Base {
 private:
 public:
  std::string result;

  ESP_Postprocessor_Text();
  ~ESP_Postprocessor_Text();

  // Methods
  const char* getResultData();
  int getResultSize();
  bool isResultValid();

  // Override
  bool initialize();
  bool execute(CC_AttributesType* result);
  bool terminate();
  void parseCustomElement(TiXmlElement* element);
};

#endif
