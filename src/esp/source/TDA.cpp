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
#include "TDA.h"
#include "ESP_XmlUtils.h"
#include "ESP_StringUtils.h"
#include "CC_Logger.h"

/* Plugins Definitions */
#include "parser_basic/ESP_Plugin_Parser_Basic.h"
#ifdef USE_INPUTFILE
#include "input_file/ESP_Plugin_Input_File.h"
#endif  // USE_INPUTFILE
#ifdef USE_INPUTIP
#include "input_ip/ESP_Plugin_Input_IP.h"
#endif  // USE_INPUTIP

#ifdef USE_MQTT
#include "input_mqtt/ESP_Plugin_Input_Mqtt.h"
#endif  // USE_MQTT

#include "input_buffer/ESP_Plugin_Input_Buffer.h"
#include "output_file/ESP_Plugin_Output_File.h"

#ifdef USE_IDAS
#include "output_sbc/ESP_Plugin_Output_SBC.h"
#endif  // USE_IDAS
#ifdef USE_LUA
#include "preprocessor_lua/ESP_Plugin_Preprocessor_Lua.h"
#endif  // USE_LUA
#include "postprocessor_sensorml/ESP_Plugin_Postprocessor_Sensorml.h"
#include "postprocessor_ul20/ESP_Plugin_Postprocessor_UL20.h"
#include "postprocessor_text/ESP_Plugin_Postprocessor_Text.h"

#ifdef USE_IOT  // not using IDAS means enabling JSON and IOT but JSON
                // postprocessor could be used outside IOT (in theory)
#include "output_iot/ESP_Plugin_Output_IoT.h"
#endif  // USE_IOT

#ifdef USE_JSON
#include "postprocessor_json/ESP_Plugin_Postprocessor_json.h"
#endif  // USE_JSON

#ifdef USE_TTOPEN
#include "postprocessor_tt/ESP_Plugin_Postprocessor_TT.h"
#endif  // USE_TTOPEN

// ------------------------
// CONTEXT
// ------------------------
ESP_Context::ESP_Context() {
  input = NULL;
  output = NULL;
  close = false;
  clear();
}

ESP_Context::~ESP_Context() { clear(); }

void ESP_Context::clear() {
  if (input != NULL) {
    free(input);
    input = NULL;
  }
  if (output != NULL) {
    free(output);
    output = NULL;
  }
  input_size = 0;
  output_size = 0;
  index_start = 0;
  index_end = 0;
}

void ESP_Context::addParams(CC_ParamsType params) {
  for (CC_ParamsType::iterator it = params.begin(); it != params.end(); it++) {
    ESP_Attribute attribute;
    attribute.setValue(it->first, it->second.c_str(), it->second.length(),
                       ESP_DataType::ESP_DataType_STRING,
                       ESP_DataCode::ESP_DataCode_STRING);
    this->temp.insert(CC_AttributesPair(it->first, attribute));
  }
}

int ESP_Context::getInputSize() { return input_size; }

int ESP_Context::getOutputSize() { return output_size; }

char* ESP_Context::getInputData() { return input; }

char* ESP_Context::getOutputData() { return output; }

void ESP_Context::addInputData(const char* data, int len) {
  if (len > 0) {
    input_size += len;
    input = (char*)realloc(input, input_size);
    memcpy(input + input_size - len, data, len);
  }
}

int ESP_Context::getAvailableInputData() { return input_size - index_end; }

int ESP_Context::getAvailableOutputData() { return output_size; }

bool ESP_Context::inputDataIsAvailable(int bytes) {
  if (getAvailableInputData() >= bytes) {
    return true;
  } else {
    return false;
  }
}

bool ESP_Context::readPInputData(int bytes, const char*& result) {
  if (bytes > 0 && inputDataIsAvailable(bytes)) {
    result = &this->input[index_end];
    index_end += bytes;
    return true;
  } else {
    return false;
  }
}

bool ESP_Context::validateInputData(int offset) {
  // Offset Validating
  if (offset != 0) {
    index_end = offset;
  }

  // Move and Copy
  input_size -= index_end;
  memcpy(input, input + index_end, input_size);
  input = (char*)realloc(input, input_size);
  index_end = index_start;
  return true;
}

// *****************************
bool ESP_Context::addResultData()
// Adds results ignoring temp attributes starting with underscore _
// *****************************
{
  CC_AttributesType finalValues;
  for (CC_AttributesType::iterator it = temp.begin(); it != temp.end(); it++) {
    if (!ESP_StringUtils::startsWith(it->second.getName(), "_")) {
      finalValues.insert(CC_AttributesPair(it->first, it->second));
    }
  }
  if (finalValues.size() > 0) {
    results.push_back(finalValues);
    return true;
  } else {
    return false;
  }
}

void ESP_Context::saveIndex() { this->index_end_saved = this->index_end; }

void ESP_Context::restoreIndex() { this->index_end = this->index_end_saved; }

// TODO
int ESP_Context::searchAttributeValue(ESP_Attribute* attribute) {
  return searchAttributeValueInRange(attribute, 0, input_size - index_end);
}

int ESP_Context::searchAttributeValueInRange(ESP_Attribute* attribute,
                                             int offset, int size) {
  int datasize = attribute->_datasize;

  for (int i = index_end + offset; i < index_end + size; i++) {
    if (i >= 0 && input_size - i >= datasize) {
      if (memcmp(input + i, attribute->_value, datasize) == 0) {
        return i;
      }
    }
  }
  return -1;
}

int ESP_Context::getSizeFromIndex(int index) { return index - index_end; }

void ESP_Context::addOutputData(const char* data, int len) {
  output = (char*)realloc(output, output_size + len);
  memcpy(output + output_size, data, len);
  output_size = output_size + len;
}

void ESP_Context::removeOutputData(int nbytes) {
  memmove(output, output + nbytes, output_size - nbytes);
  output = (char*)realloc(output, output_size - nbytes);
  output_size = output_size - nbytes;
}

// ------------------------
// BASE
// ------------------------
bool ESP_Base::checkParams() { return true; }

// ------------------------
// COMMAND
// ------------------------
ESP_Command::ESP_Command() {
  header = NULL;
  body = NULL;
  footer = NULL;
  clear();
}

ESP_Command::~ESP_Command() { clear(); }

void ESP_Command::clear() {
  index = 0;
  if (header != NULL) {
    delete header;
  }
  if (body != NULL) {
    delete body;
  }
  if (footer != NULL) {
    delete footer;
  }
  header = NULL;
  body = NULL;
  footer = NULL;
}

void ESP_Command::reset() {
  this->index = 0;
  this->header->reset();
  this->body->reset();
  this->footer->reset();
}

int ESP_Command::run(ESP_Context* context) {
  int result = ESP_Tag_Base::EXECUTE_RESULT_IDLE;

  switch (index) {
    case ESP_Command::ESP_COMMAND_INDEX::HEADER: {
      result = header->run(context);
      break;
    }
    case ESP_Command::ESP_COMMAND_INDEX::BODY: {
      result = body->run(context);

      // New Data
      if (result == ESP_Tag_Base::EXECUTE_RESULT_OK) {
        // Streaming
        if (body->_loop) {
          index = ESP_Command::ESP_COMMAND_INDEX::BODY;  // Go Body
          body->reset();
        } else {
          index++;  // Go Next
        }

        context->validateInputData();
        context->addResultData();
        context->temp.clear();
        return result;  // Return after every new result
      }
      break;
    }
    case ESP_Command::ESP_COMMAND_INDEX::FOOTER: {
      result = footer->run(context);
      break;
    }
    case 3: {  // Finished
      result = ESP_Tag_Base::EXECUTE_RESULT_IDLE;
      break;
    }
  }

  // In Progress
  switch (result) {
    case ESP_Tag_Base::EXECUTE_RESULT_OK:
      // Tag Finished
      index++;
      return run(context);
      break;

    case ESP_Tag_Base::EXECUTE_RESULT_ERROR:
      context->validateInputData(1);  // Advance 1 byte
      context->temp.clear();
      this->reset();

      // Advance Context and restart process from body
      index = ESP_Command::ESP_COMMAND_INDEX::BODY;
      return run(context);
      break;

    case ESP_Tag_Base::EXECUTE_RESULT_IDLE:
      break;
  }

  return result;
}

// ---------------------------------------------------------------------------
// COMMAND TAG
// ---------------------------------------------------------------------------
ESP_Tag_Base::ESP_Tag_Base() {
  reset();
  this->_byteoffset = 0;
  this->_datasize = 0;
  this->_datatype = 0;
  this->_datacode = 0;
  parent = NULL;
  _loop = false;
}

ESP_Tag_Base::~ESP_Tag_Base() {
  // Delete children
  for (std::vector<ESP_Tag_Base*>::iterator it = subtags.begin();
       it != subtags.end(); it++) {
    delete (*it);
  }
}

void ESP_Tag_Base::reset() {
  index = 0;
  _parsed = false;
  _repeat = false;
  for (std::vector<ESP_Tag_Base*>::iterator it = subtags.begin();
       it != subtags.end(); it++) {
    (*it)->reset();
  }
}

void ESP_Tag_Base::parseElement(TiXmlElement* element) {
  // printf("Parsing Base Element\n");
  this->_name = ESP_XmlUtils::queryStringValue(element, "name");
  this->_byteoffset = ESP_XmlUtils::queryIntValue(element, "byteoffset");
  this->_datasize = ESP_XmlUtils::queryIntValue(element, "datasize");
  this->_datatype = ESP_Attribute::getDataTypeFromString(
      ESP_XmlUtils::queryStringValue(element, "datatype"));
  this->_datacode = ESP_Attribute::getDataCodeFromString(
      ESP_XmlUtils::queryStringValue(element, "datacode"));
  this->_loop = ESP_XmlUtils::queryBoolValue(element, "loop");
  this->_valueref = ESP_XmlUtils::queryStringValue(element, "valueref");

  // Default Values
  if (_datasize == 0) {
    _datasize = _byteoffset;
  }

  // Try Set Value
  const char* valueint = element->Attribute("valueint");
  const char* valuefloat = element->Attribute("valuefloat");
  const char* valuestring = element->Attribute("valuestring");
  const char* valuehex = element->Attribute("valuehex");
  if (valueint != NULL) {
    this->_value.setValue(_name, valueint, sizeof(int),
                          ESP_DataType::ESP_DataType_INT,
                          ESP_DataCode::ESP_DataCode_STRING);
    this->_datasize = this->_value._datasize;
  } else if (valuefloat != NULL) {
    this->_value.setValue(_name, valuefloat, sizeof(float),
                          ESP_DataType::ESP_DataType_FLOAT,
                          ESP_DataCode::ESP_DataCode_STRING);
    this->_datasize = this->_value._datasize;
  } else if (valuestring != NULL) {
    this->_value.setValue(_name, valuestring, strlen(valuestring),
                          ESP_DataType::ESP_DataType_STRING,
                          ESP_DataCode::ESP_DataCode_STRING);
    this->_datasize = this->_value._datasize;
  } else if (valuehex != NULL) {
    this->_value.setValue(_name, valuestring, strlen(valuestring),
                          ESP_DataType::ESP_DataType_STRING,
                          ESP_DataCode::ESP_DataCode_STRING);
    this->_datasize = this->_value._datasize;
  }

  parseCustomElement(element);
}

void ESP_Tag_Base::parseCustomElement(TiXmlElement* element) {}

ESP_Tag_Base* ESP_Tag_Base::searchParent(ESP_Tag_Base* node, std::string type) {
  while (node->parent != NULL) {
    if (node->parent->_type == "loop") {
      return node->parent;
    }
    node = node->parent;
  }

  return NULL;
}

ESP_Tag_Base* ESP_Tag_Base::getNextTag() {
  for (std::vector<ESP_Tag_Base*>::iterator it = subtags.begin();
       it != subtags.end(); it++) {
    if (!(*it)->_parsed) {
      return (*it);
    }
  }
  return NULL;
}

void ESP_Tag_Base::setChildrenParsed(bool parent, bool status) {
  if (parent) {
    this->_parsed = status;
  }

  for (std::vector<ESP_Tag_Base*>::iterator it = subtags.begin();
       it != subtags.end(); it++) {
    (*it)->setChildrenParsed(true, status);
  }
}

/**
* Iterate Tree
**/
int ESP_Tag_Base::run(ESP_Context* context) {
  if (!_parsed) {
    int result = execute(context);
    if (result == ESP_Tag_Base::EXECUTE_RESULT_OK) {
      // Set Parsed
      _parsed = true;
      while (getNextTag()) {
        result = run(context);
        if (result != EXECUTE_RESULT_OK) {
          break;
        }
      }

      // Repeat Tag After evaluating Children
      if (_repeat) {
        setChildrenParsed(true, false);
      }
    }
    return result;
  } else {
    // Parse Next's
    int result = ESP_Tag_Base::EXECUTE_RESULT_OK;
    ESP_Tag_Base* next = getNextTag();
    while (next != NULL && result == ESP_Tag_Base::EXECUTE_RESULT_OK) {
      result = next->run(context);
      next = getNextTag();
    }
    return result;
  }
}

void ESP_Tag_Header::parseCustomElement(TiXmlElement* element) {
  // printf("Parsing Header Element\n");
}

int ESP_Tag_Header::execute(ESP_Context* context) {
  // printf("Executing Header Element\n");
  return EXECUTE_RESULT_OK;
}

void ESP_Tag_Body::parseCustomElement(TiXmlElement* element) {
  // printf("Parsing Body Element\n");
}

int ESP_Tag_Body::execute(ESP_Context* context) {
  // printf("Executing Body Element\n");
  return EXECUTE_RESULT_OK;
}

void ESP_Tag_Footer::parseCustomElement(TiXmlElement* element) {
  // printf("Parsing Footer Element\n");
}

int ESP_Tag_Footer::execute(ESP_Context* context) {
  // printf("Executing Footer Element\n");
  return EXECUTE_RESULT_OK;
}

// ------------------------
// SENSOR
// ------------------------
/*
    Constructor
*/
ESP_Sensor::ESP_Sensor() {
  _id = 0;
  _type = 0;
  readyToRun = false;
  dataCallbackUserData = NULL;
  resultCallbackUserData = NULL;
  dataCallback = NULL;
  resultCallback = NULL;
}

/*
    Destructor
*/
ESP_Sensor::~ESP_Sensor() {
  // Destroy Elements
  for (unsigned int i = 0; i < inputs.size(); i++) {
    delete inputs[i];
  }
  for (unsigned int i = 0; i < outputs.size(); i++) {
    CC_Logger::getSingleton()->logDebug("DELETE Sensor: outputs[%d] : %s", i,
                                        outputs[i]->_name.c_str());
    delete outputs[i];
  }
  for (unsigned int i = 0; i < preprocessors.size(); i++) {
    delete preprocessors[i];
  }
  for (unsigned int i = 0; i < postprocessors.size(); i++) {
    delete postprocessors[i];
  }
  for (unsigned int i = 0; i < serverRunners.size(); i++) {
    delete serverRunners[i];
  }
  for (unsigned int i = 0; i < clientRunners.size(); i++) {
    delete clientRunners[i];
  }

  commands.clear();

  pthread_mutex_destroy(&mutexMapClient);
  pthread_mutex_destroy(&mutexMapServer);
  CC_Logger::getSingleton()->logDebug("Deleting Sensor DONE");
}

/*
    Register Callback
*/
void ESP_Sensor::registerDataCallback(
    void* userData, void (*cb)(void* userData, const char* buffer, int nread)) {
  this->dataCallbackUserData = userData;
  this->dataCallback = cb;
}

void ESP_Sensor::registerResultCallback(void* userData,
                                        void (*cb)(void* userData,
                                                   ESP_Runner* runner)) {
  this->resultCallbackUserData = userData;
  this->resultCallback = cb;
}

void ESP_Sensor::runDataCallback(const char* buffer, int nread) {
  if (this->dataCallback != NULL) {
    this->dataCallback(dataCallbackUserData, buffer, nread);
  }
}

void ESP_Sensor::runResultCallback(ESP_Runner* runner) {
  if (this->resultCallback != NULL) {
    this->resultCallback(resultCallbackUserData, runner);
  }
}

/*
    Load from File
*/
bool ESP_Sensor::loadFromFile(std::string fileName) {
  doc.Clear();
  bool ok = false;

  // Load
  ok = doc.LoadFile(fileName.c_str());

  if (ok) {
    // Parse
    ok = parseRootElement(doc.RootElement());
    readyToRun = true;
  } else {
    CC_Logger::getSingleton()->logError("Error parsing Xml: %s\n",
                                        doc.ErrorDesc());
    readyToRun = false;
  }

  return ok;
}

bool ESP_Sensor::parseRootElement(TiXmlElement* rootElement) {
  bool ok = false;

  if (rootElement != NULL) {
    // Load Plugins
    parsePlugins(rootElement);
    parsePreprocessors(rootElement);
    parsePostprocessors(rootElement);

    parseInputs(rootElement);
    parseOutputs(rootElement);
    parseCommands(rootElement);

    // Assign processors
    updateInputPreprocessors();
    updateOutputPostprocessors();

    ok = true;
  }

  return ok;
}

/*
    Update InputPreprocessors
*/
bool ESP_Sensor::updateInputPreprocessors() {
  for (std::vector<ESP_Input_Base*>::iterator it = inputs.begin();
       it != inputs.end(); it++) {
    for (std::vector<std::string>::iterator it2 = (*it)->_preprocessors.begin();
         it2 != (*it)->_preprocessors.end(); it2++) {
      for (std::vector<ESP_Preprocessor_Base*>::iterator it3 =
               preprocessors.begin();
           it3 != preprocessors.end(); it3++) {
        if (it2->compare((*it3)->_name) == 0) {
          (*it)->preprocessors.push_back(*it3);
        }
      }
    }
  }
  return true;
}

/*
    Update OutputPostprocessors
*/
bool ESP_Sensor::updateOutputPostprocessors() {
  for (std::vector<ESP_Output_Base*>::iterator it = outputs.begin();
       it != outputs.end(); it++) {
    for (std::vector<std::string>::iterator it2 =
             (*it)->_postprocessors.begin();
         it2 != (*it)->_postprocessors.end(); it2++) {
      for (std::vector<ESP_Postprocessor_Base*>::iterator it3 =
               postprocessors.begin();
           it3 != postprocessors.end(); it3++) {
        if (it2->compare((*it3)->_name) == 0) {
          (*it)->postprocessors.push_back(*it3);
        }
      }
    }
  }
  return true;
}

/*
    Plugin Parsers
*/
bool ESP_Sensor::parsePlugins(TiXmlElement* element) {
  if (element != NULL) {
    TiXmlElement* pluginsElement =
        element->FirstChildElement(XML_SECTION_PLUGINS);
    if (pluginsElement) {
      for (TiXmlElement* ele = pluginsElement->FirstChildElement(); ele != NULL;
           ele = ele->NextSiblingElement()) {
        const char* value = ele->Value();
        const char* name = ele->Attribute("name");

        // Add Parser Plugins
        if (strcmp(value, "pluginparser") == 0) {
          if (strcmp(name, "binaryparser") == 0) {
            addParserPlugin(ESP_Plugin_Parser_Basic::getSingleton());
          }
        }
        // Add Input Plugins
        else if (strcmp(value, "plugininput") == 0) {
#ifdef USE_INPUTFILE
          if (strcmp(name, "file") == 0) {
            addInputPlugin(ESP_Plugin_Input_File::getSingleton());
          } else
#endif  // USE_INPUTFILE
#ifdef USE_INPUTIP
              if (strcmp(name, "ip") == 0) {
            addInputPlugin(ESP_Plugin_Input_IP::getSingleton());
          } else
#endif  // USE_INPUTIP
#ifdef USE_MQTT
              if (strcmp(name, "mqtt") == 0) {
            addInputPlugin(ESP_Plugin_Input_Mqtt::getSingleton());
          } else
#endif  // USE_MQTT
              if (strcmp(name, "buffer") == 0) {
            addInputPlugin(ESP_Plugin_Input_Buffer::getSingleton());
          }
        }
        // Add Output Plugins
        else if (strcmp(value, "pluginoutput") == 0) {
          if (strcmp(name, "file") == 0) {
            addOutputPlugin(ESP_Plugin_Output_File::getSingleton());
          }
#ifdef USE_IDAS
          else if (strcmp(name, "sbc") == 0) {
            addOutputPlugin(ESP_Plugin_Output_SBC::getSingleton());
          }
#endif  // USE_IDAS

#ifdef USE_IOT
          if (strcmp(name, "iot") == 0) {
            addOutputPlugin(ESP_Plugin_Output_IoT::getSingleton());
          }
#endif  // USE_IOT
        }
        // Add Preprocessor Plugins
        else if (strcmp(value, "pluginpreprocessor") == 0) {
#ifdef USE_LUA
          if (strcmp(name, "lua") == 0) {
            addPreprocessorPlugin(ESP_Plugin_Preprocessor_Lua::getSingleton());
          }
#endif  // USE_LUA
        }
        // Add Postprocessor Plugins
        else if (strcmp(value, "pluginpostprocessor") == 0) {
          if (strcmp(name, "sensorml") == 0) {
            addPostprocessorPlugin(
                ESP_Plugin_Postprocessor_Sensorml::getSingleton());
          } else if (strcmp(name, "text") == 0) {
            addPostprocessorPlugin(
                ESP_Plugin_Postprocessor_Text::getSingleton());
          } else if (strcmp(name, "ul20") == 0) {
            addPostprocessorPlugin(
                ESP_Plugin_Postprocessor_UL20::getSingleton());
          }
#ifdef USE_JSON
          else if (strcmp(name, "json") == 0) {
            addPostprocessorPlugin(
                ESP_Plugin_Postprocessor_json::getSingleton());
          }
#endif  // USE_JSON

#ifdef USE_TTOPEN
          else if (strcmp(name, "ttopen") == 0) {
            addPostprocessorPlugin(ESP_Plugin_Postprocessor_TT::getSingleton());
          }
#endif  // USE_TTOPEN
        }
      }
    }
  }

  return true;
}

bool ESP_Sensor::parseInputs(TiXmlElement* element) {
  if (element != NULL) {
    TiXmlElement* pluginsElement =
        element->FirstChildElement(XML_SECTION_INPUTS);
    if (pluginsElement) {
      for (TiXmlElement* ele = pluginsElement->FirstChildElement(); ele != NULL;
           ele = ele->NextSiblingElement()) {
        ESP_Input_Base* input = createInputFromPlugins(ele);

        if (input != NULL) {
          input->parseElement(ele);
          if (input->checkParams()) {
            inputs.push_back(input);
          }
        }
      }
    }
  }
  return true;
}

bool ESP_Sensor::parseOutputs(TiXmlElement* element) {
  if (element != NULL) {
    TiXmlElement* pluginsElement =
        element->FirstChildElement(XML_SECTION_OUTPUTS);
    if (pluginsElement) {
      for (TiXmlElement* ele = pluginsElement->FirstChildElement(); ele != NULL;
           ele = ele->NextSiblingElement()) {
        ESP_Output_Base* output = createOutputFromPlugins(ele);

        if (output != NULL) {
          output->parseElement(ele);
          if (output->checkParams()) {
            outputs.push_back(output);
          }
        }
      }
    }
  }
  return true;
}

bool ESP_Sensor::parsePreprocessors(TiXmlElement* element) {
  if (element != NULL) {
    TiXmlElement* pluginsElement =
        element->FirstChildElement(XML_SECTION_PREPROCESSORS);
    if (pluginsElement) {
      for (TiXmlElement* ele = pluginsElement->FirstChildElement(); ele != NULL;
           ele = ele->NextSiblingElement()) {
        ESP_Preprocessor_Base* preprocessor =
            createPreprocessorFromPlugins(ele);

        if (preprocessor != NULL) {
          preprocessor->parseElement(ele);
          if (preprocessor->checkParams()) {
            preprocessors.push_back(preprocessor);
          }
        }
      }
    }
  }
  return true;
}

bool ESP_Sensor::parsePostprocessors(TiXmlElement* element) {
  if (element != NULL) {
    TiXmlElement* pluginsElement =
        element->FirstChildElement(XML_SECTION_POSTPROCESSORS);
    if (pluginsElement) {
      for (TiXmlElement* ele = pluginsElement->FirstChildElement(); ele != NULL;
           ele = ele->NextSiblingElement()) {
        ESP_Postprocessor_Base* postprocessor =
            createPostprocessorFromPlugins(ele);

        if (postprocessor != NULL) {
          postprocessor->parseElement(ele);
          if (postprocessor->checkParams()) {
            postprocessors.push_back(postprocessor);
          }
        }
      }
    }
  }
  return true;
}

/*
    Parse All Commands
*/
bool ESP_Sensor::parseCommands(TiXmlElement* element) {
  if (element != NULL) {
    TiXmlElement* commandsElement =
        element->FirstChildElement(XML_SECTION_COMMANDS);
    if (commandsElement != NULL) {
      for (TiXmlElement* i = commandsElement->FirstChildElement(); i != NULL;
           i = i->NextSiblingElement()) {
        std::string name = ESP_XmlUtils::queryStringValue(i, "name");
        commands.insert(std::pair<std::string, TiXmlElement*>(name, i));
      }
    }
  }

  return true;
}

ESP_Command* ESP_Sensor::createCommand(std::string name) {
  ESP_Command* result = NULL;
  std::map<std::string, TiXmlElement*>::iterator it = commands.find(name);
  if (it != commands.end()) {
    result = new ESP_Command();
    result->name = ESP_XmlUtils::queryStringValue(it->second, "name");
    result->roe = ESP_XmlUtils::queryBoolValue(it->second, "roe");

    // DefaultParams
    TiXmlElement* defaultparams = it->second->FirstChildElement("params");
    if (defaultparams != NULL) {
      for (TiXmlElement* i = defaultparams->FirstChildElement(); i != NULL;
           i = i->NextSiblingElement()) {
        std::string pname = ESP_XmlUtils::queryStringValue(i, "name");
        std::string pvalue = ESP_XmlUtils::queryStringValue(i, "value");
        // Do not Overwrite
        if (result->params.find(pname) == result->params.end()) {
          result->params.insert(
              std::pair<std::string, std::string>(pname, pvalue));
        }
      }
    }

    // Parser
    result->header = new ESP_Tag_Header();
    result->body = new ESP_Tag_Body();
    result->footer = new ESP_Tag_Footer();
    TiXmlElement* parser = it->second->FirstChildElement("parser");
    if (parser != NULL) {
      for (TiXmlElement* i = parser->FirstChildElement(); i != NULL;
           i = i->NextSiblingElement()) {
        std::string type = ESP_XmlUtils::queryStringValue(i, "type");
        if (type == "header") {
          createCommandDetails(result->header, i);
        }
        if (type == "body") {
          createCommandDetails(result->body, i);
        }
        if (type == "footer") {
          createCommandDetails(result->footer, i);
        }
      }
    }
  }
  return result;
}

/*
    Parse One Command attributes and children
*/
bool ESP_Sensor::createCommandDetails(ESP_Tag_Base* commandtag,
                                      TiXmlElement* element) {
  if (element != NULL) {
    // Parse Attributes
    commandtag->parseElement(element);

    // Parse Children
    for (TiXmlElement* i = element->FirstChildElement(); i != NULL;
         i = i->NextSiblingElement()) {
      ESP_Tag_Base* commandsubparser = createParserFromPlugins(i);
      if (commandsubparser != NULL) {
        commandsubparser->parent = commandtag;
        commandtag->subtags.push_back(commandsubparser);
        createCommandDetails(commandsubparser, i);
      }
    }
  }

  return true;
}

/*
    API
*/
std::vector<std::string> ESP_Sensor::getCommands() {
  std::vector<std::string> result;
  for (std::map<std::string, TiXmlElement*>::iterator it = commands.begin();
       it != commands.end(); it++) {
    result.push_back(it->first);
  }
  return result;
}

void ESP_Sensor::setUserData(std::map<std::string, void*> userData) {
  this->userData = userData;
}

ESP_Input_Base* ESP_Sensor::getInputFromName(std::string name) {
  for (std::vector<ESP_Input_Base*>::iterator it = inputs.begin();
       it != inputs.end(); it++) {
    if ((*it)->_name == name) {
      return (*it);
    }
  }
  return NULL;
}

/*
    Plugins
*/
void ESP_Sensor::addInputPlugin(ESP_Plugin_Input_Base* plugin) {
  input_plugins.push_back(plugin);
}

void ESP_Sensor::addOutputPlugin(ESP_Plugin_Output_Base* plugin) {
  output_plugins.push_back(plugin);
}

void ESP_Sensor::addPreprocessorPlugin(ESP_Plugin_Preprocessor_Base* plugin) {
  preprocessor_plugins.push_back(plugin);
}

void ESP_Sensor::addPostprocessorPlugin(ESP_Plugin_Postprocessor_Base* plugin) {
  postprocessor_plugins.push_back(plugin);
}

void ESP_Sensor::addParserPlugin(ESP_Plugin_Parser_Base* plugin) {
  parser_plugins.push_back(plugin);
}

ESP_Tag_Base* ESP_Sensor::createParserFromPlugins(TiXmlElement* element) {
  ESP_Tag_Base* result = NULL;
  for (unsigned int i = 0; i < parser_plugins.size(); i++) {
    result = parser_plugins[i]->createParser(element);
    if (result != NULL) {
      return result;
    }
  }

  return result;
}

ESP_Input_Base* ESP_Sensor::createInputFromPlugins(TiXmlElement* element) {
  ESP_Input_Base* result = NULL;
  for (unsigned int i = 0; i < input_plugins.size(); i++) {
    result = input_plugins[i]->createInput(element);
    if (result != NULL) {
      return result;
    }
  }

  return result;
}

ESP_Output_Base* ESP_Sensor::createOutputFromPlugins(TiXmlElement* element) {
  ESP_Output_Base* result = NULL;
  for (unsigned int i = 0; i < output_plugins.size(); i++) {
    result = output_plugins[i]->createOutput(element);
    if (result != NULL) {
      return result;
    }
  }

  return result;
}

ESP_Input_Base::ESP_Input_Base() {
  _sid = -1;
  _mode = 0;
  pthread_mutex_init(&mutex, NULL);
}

ESP_Input_Base::~ESP_Input_Base() { pthread_mutex_destroy(&mutex); }

void ESP_Input_Base::parseElement(TiXmlElement* element) {
  std::string mode = ESP_XmlUtils::queryStringValue(element, "mode");
  _mode = mode.compare("server") == 0
              ? ESP_Input_Base::InputMode::INPUT_MODE_SERVER
              : ESP_Input_Base::InputMode::INPUT_MODE_CLIENT;
  _name = ESP_XmlUtils::queryStringValue(element, "name");
  std::string prepros =
      ESP_XmlUtils::queryStringValue(element, "preprocessors");

  CC_StringTokenizer ST(prepros, ",");
  for (int i = 0; i < ST.countElements(); i++) {
    _preprocessors.push_back(ST.elementAt(i));
  }

  parseCustomElement(element);
}

ESP_Output_Base::ESP_Output_Base() {}

void ESP_Output_Base::parseElement(TiXmlElement* element) {
  std::string postpros =
      ESP_XmlUtils::queryStringValue(element, "postprocessors");

  CC_StringTokenizer ST(postpros, ",");
  for (int i = 0; i < ST.countElements(); i++) {
    _postprocessors.push_back(ST.elementAt(i));
  }
  parseCustomElement(element);
}

ESP_Preprocessor_Base* ESP_Sensor::createPreprocessorFromPlugins(
    TiXmlElement* element) {
  ESP_Preprocessor_Base* result = NULL;
  for (unsigned int i = 0; i < preprocessor_plugins.size(); i++) {
    result = preprocessor_plugins[i]->createPreprocessor(element);
    if (result != NULL) {
      return result;
    }
  }

  return result;
}

ESP_Postprocessor_Base* ESP_Sensor::createPostprocessorFromPlugins(
    TiXmlElement* element) {
  ESP_Postprocessor_Base* result = NULL;
  for (unsigned int i = 0; i < postprocessor_plugins.size(); i++) {
    result = postprocessor_plugins[i]->createPostprocessor(element);
    if (result != NULL) {
      return result;
    }
  }

  return result;
}

void ESP_Preprocessor_Base::parseElement(TiXmlElement* element) {
  _script = ESP_XmlUtils::queryStringValue(element, "script");
  _name = ESP_XmlUtils::queryStringValue(element, "name");
  parseCustomElement(element);
}

void ESP_Postprocessor_Base::parseElement(TiXmlElement* element) {
  _name = ESP_XmlUtils::queryStringValue(element, "name");
  parseCustomElement(element);
}

std::string ESP_Postprocessor_Base::getType() { return _type; }

void ESP_Sensor::run(std::string name, CC_ParamsType params) {
  if (readyToRun) {
    // Get Inputs and create Runners
    for (unsigned int i = 0; i < inputs.size(); i++) {
      if (inputs[i]->_mode == ESP_Input_Base::InputMode::INPUT_MODE_CLIENT) {
        ESP_Runner_Client* runner = NULL;
        runner = new ESP_Runner_Client(this);
        runner->input = inputs[i];
        runner->command = this->createCommand(name);
        runner->context.addParams(runner->command->params);
        runner->context.addParams(params);
        runner->run();
        clientRunners.insert(std::pair<unsigned long long, ESP_Runner_Client*>(
            (unsigned long long)runner, runner));
      } else if (inputs[i]->_mode ==
                 ESP_Input_Base::InputMode::INPUT_MODE_SERVER) {
        ESP_Runner_Server* runner = NULL;
        runner = new ESP_Runner_Server(this);
        runner->input = inputs[i];
        runner->command = this->createCommand(name);
        runner->context.addParams(runner->command->params);
        runner->context.addParams(params);
        runner->run();
        serverRunners.insert(std::pair<unsigned long long, ESP_Runner_Server*>(
            (unsigned long long)runner, runner));
      }
    }
  }
}

void ESP_Sensor::stop() {
  CC_Logger::getSingleton()->logDebug("Sensor: Stopping");
  ESP_Runner_Server* runnerTemp;
  bool deleteServers = true;
  bool deleteClients = true;
  ESP_Runner_Client* runnerClient;

  while (deleteServers) {
    pthread_mutex_lock(&mutexMapServer);
    if (serverRunners.size() > 0) {
      std::map<unsigned long long, ESP_Runner_Server*>::iterator it =
          serverRunners.begin();
      if (it != serverRunners.end()) {
        runnerTemp = (ESP_Runner_Server*)it->second;
      }
    } else {
      pthread_mutex_unlock(&mutexMapServer);
      break;
    }

    pthread_mutex_unlock(&mutexMapServer);
    for (unsigned int j = 0; j < runnerTemp->sensor->inputs.size(); j++) {
      runnerTemp->sensor->inputs[j]->stopServer();
    }
    runnerTemp->stopRunner(100);
  }

  while (true) {
    pthread_mutex_lock(&mutexMapClient);
    if (clientRunners.size() > 0) {
      std::map<unsigned long long, ESP_Runner_Client*>::iterator it =
          clientRunners.begin();
      if (it != clientRunners.end()) {
        runnerClient = (ESP_Runner_Client*)it->second;
      }

    } else {
      pthread_mutex_unlock(&mutexMapClient);
      break;
    }
    pthread_mutex_unlock(&mutexMapClient);
    runnerClient->stopRunner(100);
  }
}

/* --------------- */
/* RUNNERS         */
/* --------------- */
ESP_Runner::ESP_Runner(ESP_Sensor* sensor) {
  this->sensor = sensor;
  this->_keepRunning = true;
  this->input = NULL;
  this->command = NULL;
  this->_id = -1;
  this->runOnce = false;
  initConditions();
}

ESP_Runner::~ESP_Runner() {
  pthread_mutex_destroy(&mutexCond);
  pthread_cond_destroy(&condFinished);
}

void ESP_Runner::initConditions() {
  // Setting up condition for notifications.
  pthread_mutex_init(&mutexCond, NULL);
  pthread_cond_init(&condFinished, NULL);
  bFinished = false;

  CC_Logger::getSingleton()->logDebug("InitConditions ...[%d] DONE", _id);
}

void ESP_Runner::signalFinish() {
  pthread_mutex_lock(&mutexCond);
  bFinished = true;
  pthread_cond_signal(&condFinished);
  pthread_mutex_unlock(&mutexCond);
  CC_Logger::getSingleton()->logDebug("SIGNALING Finished! Runner[%d]", _id);
}

void ESP_Runner::stopRunner(unsigned int msecs) {
  this->_keepRunning = false;  //

  struct timespec timeToWait;
  struct timeval now;

  CC_Logger::getSingleton()->logDebug("Stopping Runner [%d] msecs [%d]", _id,
                                      msecs);
  gettimeofday(&now, NULL);
  timeToWait.tv_sec = now.tv_sec + 5;
  timeToWait.tv_nsec = (now.tv_usec + 1000UL * msecs) * 1000UL;

  while (!bFinished) {
    // CC_Logger::getSingleton()->logDebug("Waiting for signal on Runner [%d]
    // ",_id);
    pthread_cond_timedwait(&condFinished, &mutexCond, &timeToWait);
  }
  CC_Logger::getSingleton()->logDebug("Stopping Runner [%d], signal? %s DONE",
                                      _id, bFinished ? "yes" : "no");
  // std::cout << "Thread : ["<< name << "] has finally stopped  count: " <<
  // count <<  std::endl;
}

ESP_Runner_Server::ESP_Runner_Server(ESP_Sensor* sensor) : ESP_Runner(sensor) {}

void ESP_Runner_Server::run() {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&thread, &attr, ESP_Runner_Server::runThread, this);
  // initConditions();
}

void* ESP_Runner_Server::runThread(void* userData) {
  ESP_Runner* runner = (ESP_Runner*)userData;
  ESP_Sensor* sensorRef = runner->sensor;
  CC_Logger::getSingleton()->logDebug("Run Server Thread %d", sensorRef->_id);

  // Get Input
  ESP_Input_Base* input = runner->input;

  // Open
  if (input != NULL) {
    runner->_id = input->openServer();
    if (input->_sid >= 0) {
      while (runner->_keepRunning) {
        int id = input->acceptServer();

        // New Connection
        if (id > 0) {
          // TODO: Add to clients
          ESP_Runner_Client* client = new ESP_Runner_Client(runner->sensor);
          client->_id = id;  // id is pre-assigned
          client->input = input;
          client->command =
              runner->sensor->createCommand(runner->command->name);

          pthread_mutex_lock(&sensorRef->mutexMapClient);
          runner->sensor->clientRunners.insert(
              std::pair<unsigned long long, ESP_Runner_Client*>(
                  (unsigned long long)client, client));
          pthread_mutex_unlock(&sensorRef->mutexMapClient);

          client->run();
        }
        // Error
        else if (id < 0) {
          runner->_keepRunning = false;
        }

        sched_yield();
        SLEEP(15);
        if (runner->runOnce) {
          runner->_keepRunning = false;
        }
      }

      input->closeServer();
    }
  }

  // Clear

  pthread_mutex_lock(
      &runner->sensor
           ->mutexMapServer);  // This lock is acquired at ESP_Sensor::stop()

  runner->signalFinish();

  std::map<unsigned long long, ESP_Runner_Server*>::iterator it =
      runner->sensor->serverRunners.find((unsigned long long)runner);
  if (it != sensorRef->serverRunners.end()) {
    runner->sensor->serverRunners.erase(it);
    delete runner;
  }

  pthread_mutex_unlock(&sensorRef->mutexMapServer);

  return NULL;
}

ESP_Runner_Client::ESP_Runner_Client(ESP_Sensor* sensor) : ESP_Runner(sensor) {}

void ESP_Runner_Client::run() {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&thread, &attr, ESP_Runner_Client::runThread, this);
}

void* ESP_Runner_Client::runThread(void* userData) {
  ESP_Runner_Client::runLoop(userData);
  return NULL;
}

ESP_Result ESP_Runner_Client::runLoop(void* userData) {
  char* buffer = new char[RUNNER_MAX_BUFFER];
  ESP_Result result;  // Return data
  ESP_Runner* runner = (ESP_Runner*)userData;
  ESP_Sensor* sensorRef = runner->sensor;
  CC_Logger::getSingleton()->logDebug("Run Client Thread Start");

  // Check Command
  if (runner->command == NULL) {
    CC_Logger::getSingleton()->logError("Runner Error, no Command Available");
    return result;
  }

  // Open
  if (runner->_id < 0) {
    runner->_id = runner->input->openClient();
  }

  CC_Logger::getSingleton()->logDebug("Runner Input ID: %d", runner->_id);

  // Read
  int status = ESP_Tag_Base::EXECUTE_RESULT_OK;
  while (runner->_keepRunning && runner->_id >= 0) {
    // TODO: Read until no more data
    memset(buffer, 0, RUNNER_MAX_BUFFER);
    int nread =
        runner->input->readClient(runner->_id, buffer, RUNNER_MAX_BUFFER);
    int nwrite = runner->context.getAvailableOutputData();

    // Add to Context
    if (nread > 0 || nwrite > 0 || status == ESP_Tag_Base::EXECUTE_RESULT_OK) {
      // Preprocess,Log,Callback
      if (nread > 0) {
        // Log
        // for (int i=0;i<nread;i++) CC_Logger::getSingleton()->logDebug("Read
        // Byte = %d",buffer[i]);

        // Preprocessors
        for (std::vector<ESP_Preprocessor_Base*>::iterator it =
                 runner->input->preprocessors.begin();
             it != runner->input->preprocessors.end(); it++) {
          // start
          (*it)->initialize();
          if ((*it)->execute(&runner->context)) {
            // Update buffer
            memcpy(buffer, (*it)->getResultData(), (*it)->getResultSize());
            nread = (*it)->getResultSize();
          }

          // end
          (*it)->terminate();
        }

        // Callback
        runner->sensor->runDataCallback(buffer, nread);

        // Add Data
        runner->context.addInputData(buffer, nread);
      }

      // Execute Command
      runner->context.results.clear();
      status = runner->command->run(&runner->context);

      // Reset on End
      if (runner->command->roe) {
        runner->command->reset();
        runner->context.temp.clear();
      }

      // Write
      if (runner->context.getAvailableOutputData() > 0) {
        nwrite = runner->input->writeClient(
            runner->_id, (char*)runner->context.getOutputData(),
            runner->context.getOutputSize());
        if (nwrite >= 0) {
          runner->context.removeOutputData(nwrite);
          if (nwrite > 0) {
            CC_Logger::getSingleton()->logDebug("Writing output data: %d",
                                                nwrite);
          }
        } else {
          runner->_keepRunning = false;
          CC_Logger::getSingleton()->logError("Error writing output data: %d",
                                              nwrite);
        }
      }

      // New Data
      if (runner->context.results.size() > 0) {
        // Add to Results
        if (runner->runOnce) {
          result.attresults.insert(result.attresults.begin(),
                                   runner->context.results.begin(),
                                   runner->context.results.end());
        }

        // Callback
        runner->sensor->runResultCallback(runner);

        // Iterate through results
        for (std::vector<CC_AttributesType>::iterator it =
                 runner->context.results.begin();
             it != runner->context.results.end(); it++) {
          CC_AttributesType* newattr = &(*it);
          CC_Logger::getSingleton()->logDebug("New Measure with Attributes: %d",
                                              newattr->size());

          // Results of postprocessor
          CC_AttributesType ppResult;

          // Postprocessors Update
          for (std::vector<ESP_Postprocessor_Base*>::iterator it2 =
                   runner->sensor->postprocessors.begin();
               it2 != runner->sensor->postprocessors.end(); it2++) {
            // CC_Logger::getSingleton()->logDebug("Executing postprocessor:
            // %s", (*it)->_name.c_str());
            (*it2)->initialize();
            (*it2)->execute(newattr);
            // Now copy formatted message into vResult vector.
            if ((*it2)->isResultValid()) {
              CC_Logger::getSingleton()->logDebug(
                  "Result is valid at %s postprocessor", (*it2)->_name.c_str());
              ESP_Attribute attr;
              attr.setValue("", (char*)(*it2)->getResultData(),
                            (*it2)->getResultSize(),
                            ESP_DataType::ESP_DataType_STRING,
                            ESP_DataCode::ESP_DataCode_STRING);
              ppResult.insert(CC_AttributesPair((*it2)->_name, attr));
              if (runner->runOnce) {
                result.ppresults.push_back(attr);
              }
            }
            (*it2)->terminate();
          }

          // Outputs
          CC_Logger::getSingleton()->logDebug(
              "About to checkt outputs, there are %d",
              runner->sensor->outputs.size());
          for (std::vector<ESP_Output_Base*>::iterator it2 =
                   runner->sensor->outputs.begin();
               it2 != runner->sensor->outputs.end(); it2++) {
            CC_Logger::getSingleton()->logDebug("Checking outputs: ");
            for (std::vector<ESP_Postprocessor_Base*>::iterator it3 =
                     (*it2)->postprocessors.begin();
                 it3 != (*it2)->postprocessors.end(); it3++) {
              // CC_Logger::getSingleton()->logDebug("Executing output ");
              (*it2)->execute(newattr, *it3, runner->sensor->userData);
            }
          }
        }
      }

      // Close
      if (runner->context.close) {
        runner->_keepRunning = false;
        runner->context.close = false;
      }
    }
    // Connection close
    else {
      // Read Error
      if (nread < 0) {
        if (status == ESP_Tag_Base::EXECUTE_RESULT_IDLE) {
          // No more steps
          if (runner->command->index ==
              ESP_Command::ESP_COMMAND_INDEX::FINISHED) {
            runner->_keepRunning = false;

          }
          // Try Next Step
          else {
            runner->command->index++;
            status = ESP_Tag_Base::EXECUTE_RESULT_OK;
          }
        }
      }

      // If RunOnce
      if (runner->runOnce) {
        break;
      }
    }

    sched_yield();
    SLEEP(15);
  }

  CC_Logger::getSingleton()->logDebug("Stop Client Thread ID: %d", runner->_id);

  // Close
  runner->input->closeClient(runner->_id);

  // Clear
  delete[] buffer;
  delete runner->command;

  pthread_mutex_lock(&sensorRef->mutexMapClient);  // This lock is also acquired
                                                   // at ESP_Sensor::stop()

  runner->signalFinish();  // DGF

  std::map<unsigned long long, ESP_Runner_Client*>::iterator it =
      runner->sensor->clientRunners.find((unsigned long long)runner);
  if (it != sensorRef->clientRunners.end()) {
    runner->sensor->clientRunners.erase(it);

    delete runner;
  }

  pthread_mutex_unlock(&sensorRef->mutexMapClient);

  return result;
}

ESP_PostProc_Result::ESP_PostProc_Result(char* dataIn, int nlenIn) {
  data = (char*)malloc(nlen);
  nlen = nlenIn;

  memcpy(data, dataIn, nlenIn);
}

std::string ESP_Result::findInputAttributeAsString(std::string name) {
  return findInputAttributeAsString(name, false);
}

std::string ESP_Result::findInputAttributeAsString(std::string name,
                                                   bool remove) {
  std::string result("");

  for (int i = 0; i < attresults.size(); i++) {
    CC_AttributesType::iterator itAttribute = attresults[i].find(name);
    if (itAttribute != attresults[i].end()) {
      result.assign(itAttribute->second.getValueAsString());

      if (remove) {
        attresults.erase(attresults.begin() + i);
      }

      return result;
    }
  }
  return result;
}

int ESP_Result::getOutputSize() { return ppresults.size(); }

std::string ESP_Result::getOutputResultAsString(int index) {
  if (index < ppresults.size()) {
    return ppresults[index].getValueAsString();
  }
  return "";
}
