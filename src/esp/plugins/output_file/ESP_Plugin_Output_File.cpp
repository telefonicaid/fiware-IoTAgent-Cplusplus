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
#include "output_file/ESP_Plugin_Output_File.h"
#include "ESP_XmlUtils.h"

ESP_Plugin_Output_File* ESP_Plugin_Output_File::instance = NULL;

/* ---------------------- */
/* PLUGIN Output FILE      */
/* ---------------------- */
ESP_Plugin_Output_File::ESP_Plugin_Output_File() { id = 0; }

ESP_Plugin_Output_Base* ESP_Plugin_Output_File::getSingleton() {
  if (ESP_Plugin_Output_File::instance == NULL) {
    ESP_Plugin_Output_File::instance = new ESP_Plugin_Output_File();
  }
  return ESP_Plugin_Output_File::instance;
}

ESP_Plugin_Output_File* ESP_Plugin_Output_File::getInstance() {
  return (ESP_Plugin_Output_File*)ESP_Plugin_Output_File::getSingleton();
}

ESP_Output_Base* ESP_Plugin_Output_File::createOutput(TiXmlElement* element) {
  std::string type = ESP_XmlUtils::queryStringValue(element, "type");
  std::string name = ESP_XmlUtils::queryStringValue(element, "name");

  ESP_Output_Base* result = NULL;
  if (type == "file") {
    result = new ESP_Output_File();
  }

  // Assigns
  if (result != NULL) {
    result->_type = type;
    result->_name = name;
  }

  return result;
}

bool ESP_Plugin_Output_File::writeToFile(std::string filename,
                                         std::string result) {
  FILE* f = fopen(filename.c_str(), "a+");
  fprintf(f, "%s\n", result.c_str());
  fclose(f);
  return true;
}

// --------------- //
// OUTPUT FILE
// --------------- //
void ESP_Output_File::parseCustomElement(TiXmlElement* element) {
  this->_name = ESP_XmlUtils::queryStringValue(element, "name");
}

bool ESP_Output_File::execute(CC_AttributesType* attributes,
                              ESP_Postprocessor_Base* postprocessor,
                              std::map<std::string, void*> userData) {
  if (postprocessor->isResultValid()) {
    ESP_Plugin_Output_File::getInstance()->writeToFile(
        this->_name, std::string(postprocessor->getResultData(),
                                 postprocessor->getResultSize()));
    return true;
  }
  return false;
}
