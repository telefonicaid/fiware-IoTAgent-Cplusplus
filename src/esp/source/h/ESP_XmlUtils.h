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
#ifndef __ESP_XMLUTILS_H__
#define __ESP_XMLUTILS_H__

#include <string>
#include <vector>
#include "tinyxml.h"

class ESP_XmlUtils {
 public:
  static int queryIntValue(TiXmlElement* element, const char* attribute);
  static float queryFloatValue(TiXmlElement* element, const char* attribute);
  static std::string queryStringValue(TiXmlElement* element,
                                      const char* attribute);
  static bool queryBoolValue(TiXmlElement* element, const char* attribute);
  static std::string parseString(std::string str);

  // TAG UTILS
  static int getTagSub(TiXmlElement* element);
  static int getTagType(TiXmlElement* element);
  static int getTagOperation(TiXmlElement* element);
  static int getTagMode(TiXmlElement* element);

  struct ESP_TagType {
    enum ESP_TagTypeEnum {
      ESP_TagType_UNKNOWN,
      ESP_TagType_PARSER,
      ESP_TagType_CONSTRAINT,
      ESP_TagType_VALUE,
      ESP_TagType_COMPOUNDVALUE,
      ESP_TagType_CONDITION,
      ESP_TagType_OPTION,
      ESP_TagType_STREAM,
      ESP_TagType_LENGTH,
      ESP_TagType_DATASTREAM,
      ESP_TagType_CHECKSUM,
      ESP_TagType_ASCII,
      ESP_TagType_REFERENCE,
      ESP_TagType_CONSTANT,
      ESP_TagType_LOOP,
      ESP_TagType_CLEAR,
      ESP_TagType_SEND,
      ESP_TagType_SLEEP
    };
  };

  struct ESP_TagOperation {
    enum ESP_TagOperationEnum {
      ESP_TagOperation_UNKNOWN,
      ESP_TagOperation_SUM,
      ESP_TagOperation_MINUS,
      ESP_TagOperation_MUL,
      ESP_TagOperation_DIV,
      ESP_TagOperation_MOD,
      ESP_TagOperation_SATURATE,
      ESP_TagOperation_TOKENIZE
    };
  };

  struct ESP_TagOptionOperation {
    enum ESP_TagOptionOperationEnum {
      ESP_TagOperation_UNKNOWN,
      ESP_TagOperation_EQUAL,
      ESP_TagOperation_SMALLER,
      ESP_TagOperation_GREATER,
      ESP_TagOperation_SMALLER_EQUAL,
      ESP_TagOperation_GREATER_EQUAL,
      ESP_TagOperation_NOT_EQUAL
    };
  };

  struct ESP_TagModeFlags {
    enum ESP_TagModeFlags_Enum {
      TAGMODE_FLAGS_UNKNOWN = 0,
      TAGMODE_FLAGS_PARSER = 1,
      TAGMODE_FLAGS_CALIB = 2,
      TAGMODE_FLAGS_PARSER_CALIB = 3
    };
  };
};

#endif
