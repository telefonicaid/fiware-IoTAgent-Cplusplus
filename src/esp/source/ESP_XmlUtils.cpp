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
#include "ESP_XmlUtils.h"

#include <string.h>
#include <vector>
#include "ESP_StringUtils.h"

int ESP_XmlUtils::queryIntValue(TiXmlElement* element, const char* attribute) {
  const char* value = element->Attribute(attribute);
  if (value) {
    return atoi(value);
  } else {
    return 0;
  }
}

float ESP_XmlUtils::queryFloatValue(TiXmlElement* element,
                                    const char* attribute) {
  const char* value = element->Attribute(attribute);
  if (value) {
    return (float)atof(value);
  } else {
    return 0;
  }
}

std::string ESP_XmlUtils::queryStringValue(TiXmlElement* element,
                                           const char* attribute) {
  const char* value = element->Attribute(attribute);
  if (value) {
    return std::string(value);
  } else {
    return std::string("");
  }
}

bool ESP_XmlUtils::queryBoolValue(TiXmlElement* element,
                                  const char* attribute) {
  const char* value = element->Attribute(attribute);

  if (value && ESP_StringUtils::strToLower(value).compare("true") == 0) {
    return true;
  } else {
    return false;
  }
}

std::string ESP_XmlUtils::parseString(std::string str) {
  // str = CC_StringUtils::condense(str); // it's not necessary, tinyxml
  // condenses the string value
  str = ESP_StringUtils::replaceString(str, "\\n ", "\n");
  str = ESP_StringUtils::replaceString(str, "\\n", "\n");
  return str;
}

/*!
    get Sub value as Int
*/
int ESP_XmlUtils::getTagSub(TiXmlElement* element) {
  // Tag Element
  char* tag = (char*)element->Value();
  if (tag != NULL) {
    if ((strcmp(tag, "sub") == 0)) {
      return 1;
    }
  }
  return 0;
}

/*!
    get Type as Enum
*/
int ESP_XmlUtils::getTagType(TiXmlElement* element) {
  // Tag Element
  char* type = (char*)element->Attribute("type");
  if (type != NULL) {
    if ((strcmp(type, "constraint") == 0)) {
      return ESP_TagType::ESP_TagType_CONSTRAINT;
    } else if ((strcmp(type, "compoundvalue") == 0)) {
      return ESP_TagType::ESP_TagType_COMPOUNDVALUE;
    } else if ((strcmp(type, "value") == 0)) {
      return ESP_TagType::ESP_TagType_VALUE;
    } else if ((strcmp(type, "condition") == 0)) {
      return ESP_TagType::ESP_TagType_CONDITION;
    } else if ((strcmp(type, "option") == 0)) {
      return ESP_TagType::ESP_TagType_OPTION;
    } else if ((strcmp(type, "stream") == 0)) {
      return ESP_TagType::ESP_TagType_STREAM;
    } else if ((strcmp(type, "checksum") == 0)) {
      return ESP_TagType::ESP_TagType_CHECKSUM;
    } else if ((strcmp(type, "ascii") == 0)) {
      return ESP_TagType::ESP_TagType_ASCII;
    } else if ((strcmp(type, "reference") == 0)) {
      return ESP_TagType::ESP_TagType_REFERENCE;
    } else if ((strcmp(type, "constant") == 0)) {
      return ESP_TagType::ESP_TagType_CONSTANT;
    } else if ((strcmp(type, "loop") == 0)) {
      return ESP_TagType::ESP_TagType_LOOP;
    } else if ((strcmp(type, "clear") == 0)) {
      return ESP_TagType::ESP_TagType_CLEAR;
    } else if ((strcmp(type, "send") == 0)) {
      return ESP_TagType::ESP_TagType_SEND;
    } else if ((strcmp(type, "sleep") == 0)) {
      return ESP_TagType::ESP_TagType_SLEEP;
    }
  }
  return ESP_TagType::ESP_TagType_UNKNOWN;
}

/*!
    Get Operation value as Enum
*/
int ESP_XmlUtils::getTagOperation(TiXmlElement* element) {
  // Tag Element
  char* type = (char*)element->Attribute("operation");
  if (type != NULL) {
    if ((strcmp(type, "+") == 0)) {
      return ESP_TagOperation::ESP_TagOperation_SUM;
    } else if ((strcmp(type, "-") == 0)) {
      return ESP_TagOperation::ESP_TagOperation_MINUS;
    } else if ((strcmp(type, "*") == 0)) {
      return ESP_TagOperation::ESP_TagOperation_MUL;
    } else if ((strcmp(type, "/") == 0)) {
      return ESP_TagOperation::ESP_TagOperation_DIV;
    } else if ((strcmp(type, "%") == 0)) {
      return ESP_TagOperation::ESP_TagOperation_MOD;
    } else if ((strcmp(type, "saturate") == 0)) {
      return ESP_TagOperation::ESP_TagOperation_SATURATE;
    } else if ((strcmp(type, "==") == 0)) {
      return ESP_TagOptionOperation::ESP_TagOperation_EQUAL;
    } else if ((strcmp(type, "<") == 0)) {
      return ESP_TagOptionOperation::ESP_TagOperation_SMALLER;
    } else if ((strcmp(type, "<=") == 0)) {
      return ESP_TagOptionOperation::ESP_TagOperation_SMALLER_EQUAL;
    } else if ((strcmp(type, ">") == 0)) {
      return ESP_TagOptionOperation::ESP_TagOperation_GREATER;
    } else if ((strcmp(type, ">=") == 0)) {
      return ESP_TagOptionOperation::ESP_TagOperation_GREATER_EQUAL;
    } else if (ESP_StringUtils::isIntegerNumber(type)) {
      return atoi(type);
    }
  }
  return 1;
}

int ESP_XmlUtils::getTagMode(TiXmlElement* element) {
  char* value = (char*)element->Value();
  if (value != NULL) {
    std::string strValue(value);
    if (strValue.compare("parser") == 0) {
      return ESP_XmlUtils::ESP_TagModeFlags::TAGMODE_FLAGS_PARSER;
    } else if (strValue.compare("calib") == 0) {
      return ESP_XmlUtils::ESP_TagModeFlags::TAGMODE_FLAGS_CALIB;
    }
  }
  return ESP_XmlUtils::ESP_TagModeFlags::TAGMODE_FLAGS_UNKNOWN;
}
