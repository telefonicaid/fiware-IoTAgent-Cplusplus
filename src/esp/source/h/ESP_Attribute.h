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
#ifndef __ESP_ATTRIBUTE_H__
#define __ESP_ATTRIBUTE_H__

#include <string>
#include <map>

#define ESP_PARSER_MAX_VALUE_SIZE 102400  // TODO, DYNAMIC SIZE

class ESP_Attribute;
typedef std::map<std::string, ESP_Attribute> CC_AttributesType;
typedef std::pair<std::string, ESP_Attribute> CC_AttributesPair;

// -----------------------------------------------------------------------------
/**< Basic Types >*/
// -----------------------------------------------------------------------------
struct ESP_DataType {
  enum ESP_DataTypeEnum {
    ESP_DataType_UNKNOWN,
    ESP_DataType_INT,
    ESP_DataType_FLOAT,
    ESP_DataType_LONG,
    ESP_DataType_DOUBLE,
    ESP_DataType_STRING,
  };
};

// -----------------------------------------------------------------------------
/**< Extended Basic Types >*/
// -----------------------------------------------------------------------------
struct ESP_DataCode {
  enum ESP_DataCodeEnum {
    ESP_DataCode_UNKNOWN = 0,
    ESP_DataCode_MSB = 1,
    ESP_DataCode_LSB = 2,
    ESP_DataCode_STRING = 3,
    ESP_DataCode_STRINGDELIM = 4

    /*
    // SubTypes
    ESP_DataType_INTLSB = 11,
    ESP_DataType_INTMSB = 12,
    //ESP_DataType_INTSTRING = 13,
    ESP_DataType_INTPRINTABLEHEX = 14,
    ESP_DataType_INTBCD = 15,
    ESP_DataType_INTBIT = 16,
    ESP_DataType_INTASCII = 17,
    ESP_DataType_FLOATLSB = 21,
    ESP_DataType_FLOATMSB = 22,
    //ESP_DataType_FLOATSTRING = 23,
    ESP_DataType_FLOATASCII = 27,
    ESP_DataType_LONGLSB = 31,
    ESP_DataType_LONGMSB = 32,
    //ESP_DataType_LONGSTRING = 33,
    ESP_DataType_LONGASCII = 37,
    ESP_DataType_DOUBLELSB = 41,
    ESP_DataType_DOUBLEMSB = 42,
    //ESP_DataType_DOUBLESTRING = 43,
    ESP_DataType_DOUBLEASCII = 47,
    ESP_DataType_STRINGINT = 51,
    ESP_DataType_STRINGFLOAT = 52,
    ESP_DataType_STRINGLONG = 53,
    ESP_DataType_STRINGDOUBLE = 54,
    ESP_DataType_STRINGASCII = 57,
    ESP_DataType_STRINGBCD = 58,
    ESP_DataType_INTSTRINGTOKEN = 61,
    ESP_DataType_FLOATSTRINGTOKEN = 63,
    ESP_DataType_JPEGIMAGE = 81
    */
  };
};

/**
 * @brief Attributes of Frame
 */
class ESP_Attribute {
 public:
  // -------------------------------------------------------------------------
  // [public-attributes]
  // -------------------------------------------------------------------------
  std::string _name;
  void* _value;
  int _datasize;        // size
  int _datatype;        // basic type
  int _datacode;        // codification of type
  std::string tempStr;  // To return string values

  // -------------------------------------------------------------------------
  // [public-methods]
  // -------------------------------------------------------------------------
  ESP_Attribute();

  ~ESP_Attribute();

  ESP_Attribute(const ESP_Attribute& pa);

  ESP_Attribute& operator=(const ESP_Attribute& pa);

  // Clear Last Value
  void clearValue();

  // Set Values
  void setValue(std::string name, const char* value, int datasize, int datatype,
                int datacode);
  // void setValue(std::string name, void *value, int datasize, int datatype,
  // int source, int dest);

  /**
   *  @brief Binary set Bit N
   */
  void setBitN(int* result, int bitpos, int value);

  /**
   *  @brief Binary get Bit N
   */
  int getBitN(int result, int bitpos);

  int getIntValue();
  float getFloatValue();
  long long getLongValue();
  double getDoubleValue();
  std::string getStringValue();
  std::string getValueAsString();
  std::string getName();

  static int compareValue(ESP_Attribute* pa1, ESP_Attribute* pa2);
  static int getDataTypeFromString(std::string type);
  static int getDataCodeFromString(std::string type);
  static ESP_Attribute* searchAttributeRefByName(CC_AttributesType* attributes,
                                                 std::string name);
};

#endif
