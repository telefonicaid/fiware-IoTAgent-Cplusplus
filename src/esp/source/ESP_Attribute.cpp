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
#include "ESP_Attribute.h"
#include <stdlib.h>
#include <string.h>
#include <sstream>

ESP_Attribute::ESP_Attribute() {
  _value = NULL;
  _datatype = 0;
  _datasize = 0;
  _datacode = 0;
  _name = "";
}

ESP_Attribute::~ESP_Attribute() { clearValue(); }

ESP_Attribute::ESP_Attribute(const ESP_Attribute& pa) {
  _name = pa._name;
  _value = NULL;
  _datasize = 0;
  if (pa._datasize > 0 && pa._value != NULL) {
    _value = malloc(pa._datasize * sizeof(char));
    memcpy(_value, pa._value, pa._datasize);
  }
  _datasize = pa._datasize;
  _datatype = pa._datatype;
  _datacode = pa._datacode;
}

ESP_Attribute& ESP_Attribute::operator=(const ESP_Attribute& pa) {
  clearValue();
  _name = pa._name;
  if (pa._datasize > 0 && pa._value != NULL) {
    _value = malloc(pa._datasize * sizeof(char));
    memcpy(_value, pa._value, pa._datasize);
  }
  _datasize = pa._datasize;
  _datatype = pa._datatype;
  _datacode = pa._datacode;

  return *this;
}

void ESP_Attribute::clearValue() {
  if (this->_value != NULL) {
    free(this->_value);
    this->_value = NULL;
    this->_datasize = 0;
  }
}

void ESP_Attribute::setValue(std::string name, const char* value, int datasize,
                             int datatype, int datacode) {
  // Assign Name always
  this->_name = name;

  // Check VALUE
  if (value == NULL) {
    return;
  }

  // Initialize
  char* result = new char[ESP_PARSER_MAX_VALUE_SIZE];
  memset(result, 0, ESP_PARSER_MAX_VALUE_SIZE);

  clearValue();
  this->_datasize = datasize;
  this->_datatype = datatype;
  this->_datacode = datacode;

  switch (datatype) {
    case ESP_DataType::ESP_DataType_INT: {
      switch (datacode) {
        case ESP_DataCode::ESP_DataCode_MSB:
        case ESP_DataCode::ESP_DataCode_LSB: {
          memcpy(result, value, datasize);
          break;
        }
        case ESP_DataCode::ESP_DataCode_STRINGDELIM:
        case ESP_DataCode::ESP_DataCode_STRING: {
          int temp = atoi(std::string(value, datasize).c_str());
          memcpy(result, &temp, sizeof(int));
          break;
        }
        default: {
          memcpy(result, value, datasize);
          break;
        }
      }
      this->_datasize = sizeof(int);
      break;
    }

    // dgf: new datatype added
    case ESP_DataType::ESP_DataType_LONG: {
      switch (datacode) {
        case ESP_DataCode::ESP_DataCode_MSB:
        case ESP_DataCode::ESP_DataCode_LSB: {
          memcpy(result, value, datasize);
          break;
        }
        case ESP_DataCode::ESP_DataCode_STRINGDELIM:
        case ESP_DataCode::ESP_DataCode_STRING: {
          long long temp = 0;
          std::istringstream ss(value);
          ss >> temp;
          memcpy(result, &temp, sizeof(long long));
          break;
        }
        default: {
          memcpy(result, value, datasize);
          break;
        }
      }
      this->_datasize = sizeof(long long);
      break;
    }

    case ESP_DataType::ESP_DataType_FLOAT: {
      switch (datacode) {
        case ESP_DataCode::ESP_DataCode_MSB:
        case ESP_DataCode::ESP_DataCode_LSB: {
          memcpy(result, value, datasize);
          break;
        }
        case ESP_DataCode::ESP_DataCode_STRINGDELIM:
        case ESP_DataCode::ESP_DataCode_STRING: {
          float temp = (float)atof(std::string(value, datasize).c_str());
          memcpy(result, &temp, sizeof(float));
          break;
        }
        default: {
          memcpy(result, value, datasize);
          break;
        }
      }
      this->_datasize = sizeof(float);
      break;
    }
    case ESP_DataType::ESP_DataType_DOUBLE: {
      switch (datacode) {
        case ESP_DataCode::ESP_DataCode_MSB:
        case ESP_DataCode::ESP_DataCode_LSB: {
          memcpy(result, value, datasize);
          break;
        }
        case ESP_DataCode::ESP_DataCode_STRINGDELIM:
        case ESP_DataCode::ESP_DataCode_STRING: {
          double temp = atof(std::string(value, datasize).c_str());
          memcpy(result, &temp, sizeof(double));
          break;
        }
        default: {
          memcpy(result, value, datasize);
          break;
        }
      }
      this->_datasize = sizeof(double);
      break;
    }
    case ESP_DataType::ESP_DataType_STRING: {
      memcpy(result, value, datasize);
      // memcpy(result+datasize, "\0", 1);
      // this->_datasize++; // the \0 is there, but do not take it into account
      // for the size
      break;
    }
    default:
      break;
  }

  // Copy Final Result
  this->_value = (char*)malloc(sizeof(char) * this->_datasize);
  memcpy(this->_value, result, this->_datasize);
  delete[] result;
}

/*
void ESP_Attribute::setValue(std::string name, void *value, int datasize, int
datatype, int source, int dest)
{
    unsigned char result[ESP_PARSER_MAX_VALUE_SIZE] = {0}; // max result

    // Clear Previous value
    clearValue();

    // Set Vars
    this->name = name;

    // Basic Types, direct copy
    //if (datatype <= ESP_DataType::ESP_DataType_STRING)
    //{
    memcpy(result,value,datasize);
    //}
    // Ascii SubType
    if (datatype%10 == ESP_DataType::ESP_DataType_SUB_ASCII)
    {
        if (datatype == ESP_DataType::ESP_DataType_INTASCII)
        {
            datasize = sizeof(int);
            *(int *)result = atoi((char *)value);
            datatype = ESP_DataType::ESP_DataType_INT;
        }
        else if (datatype == ESP_DataType::ESP_DataType_FLOATASCII)
        {
            datasize = sizeof(float);
            *(float *)result = (float)atof((char *)value);
            datatype = ESP_DataType::ESP_DataType_FLOAT;
        }
        else if (datatype == ESP_DataType::ESP_DataType_LONGASCII)
        {
            datasize = sizeof(long long);
            *(long long *)result = atoll((char *)value);
            datatype = ESP_DataType::ESP_DataType_LONG;
        }
        else if (datatype == ESP_DataType::ESP_DataType_DOUBLEASCII)
        {
            datasize = sizeof(double);
            *(double *)result = atof((char *)value);
            datatype = ESP_DataType::ESP_DataType_DOUBLE;
        }
        else if (datatype == ESP_DataType::ESP_DataType_STRINGASCII)
        {
            memcpy(result,value,datasize);
        }
    }
    // Bit Type
    else if (datatype == ESP_DataType::ESP_DataType_INTBIT)
    {
        int temp = 0;
        setBitN(&temp, dest, getBitN(*(int *)value, source));
        datasize = sizeof(temp);
        memcpy(result, &temp, datasize);
    }
    // String Type
    else if (datatype == ESP_DataType::ESP_DataType_STRING || datatype/10 ==
ESP_DataType::ESP_DataType_STRING)
    {
        datasize++; // to include '\0' to parse
        if (datatype == ESP_DataType::ESP_DataType_STRINGINT)
        {
            datasize = sizeof(int);
            *(int *)result = atoi((char *)value);
            datatype = ESP_DataType::ESP_DataType_INT;
        }
        else if (datatype == ESP_DataType::ESP_DataType_STRINGFLOAT)
        {
            datasize = sizeof(float);
            *(float *)result = (float)atof((char *)value);
            datatype = ESP_DataType::ESP_DataType_FLOAT;
        }
        else if (datatype == ESP_DataType::ESP_DataType_STRINGLONG)
        {
            datasize = sizeof(long long);
            *(long long *)result = atoll((char *)value);
            datatype = ESP_DataType::ESP_DataType_LONG;
        }
        else if (datatype == ESP_DataType::ESP_DataType_STRINGDOUBLE)
        {
            datasize = sizeof(double);
            *(double *)result = atof((char *)value);
            datatype = ESP_DataType::ESP_DataType_DOUBLE;
        }
        else
        {
            datatype = ESP_DataType::ESP_DataType_STRING;
        }
    }

    this->datasize = datasize;
    this->datatype = datatype;
    if (this->datatype > 10) this->datatype = this->datatype / 10; // Get Basic
Type

    // if Value
    if(datasize > 0 && value != NULL)
    {
        this->value = (void*)malloc(datasize);
        memcpy(this->value,result,datasize);
    }
}
*/

/*
    Binary set Bit N
*/
void ESP_Attribute::setBitN(int* result, int bitpos, int value) {
  int temp = 1;
  temp <<= bitpos;
  if (value) {
    *result = *result | temp;
  } else {
    *result = *result & (~temp);
  }
}

/*
    Binary get Bit N
*/
int ESP_Attribute::getBitN(int result, int bitpos) {
  int temp = 1;
  temp <<= bitpos;
  result &= temp;
  if (result) {
    return 1;
  } else {
    return 0;
  }
}

int ESP_Attribute::compareValue(ESP_Attribute* pa1, ESP_Attribute* pa2) {
  if (pa1 != NULL && pa2 != NULL && pa1->_datatype == pa2->_datatype) {
    if (pa1->_datatype == ESP_DataType::ESP_DataType_INT) {
      return pa1->getIntValue() < pa2->getIntValue()
                 ? -1
                 : pa1->getIntValue() > pa2->getIntValue() ? 1 : 0;
    } else if (pa1->_datatype == ESP_DataType::ESP_DataType_FLOAT) {
      return pa1->getFloatValue() < pa2->getFloatValue()
                 ? -1
                 : pa1->getFloatValue() > pa2->getFloatValue() ? 1 : 0;
    } else if (pa1->_datatype == ESP_DataType::ESP_DataType_LONG) {
      return pa1->getLongValue() < pa2->getLongValue()
                 ? -1
                 : pa1->getLongValue() > pa2->getLongValue() ? 1 : 0;
    } else if (pa1->_datatype == ESP_DataType::ESP_DataType_DOUBLE) {
      return pa1->getDoubleValue() < pa2->getDoubleValue()
                 ? -1
                 : pa1->getDoubleValue() > pa2->getDoubleValue() ? 1 : 0;
    } else if (pa1->_datatype == ESP_DataType::ESP_DataType_STRING) {
      return pa1->getStringValue().compare(pa2->getStringValue());
    }
  }
  return -1;  // Not comparable
}

int ESP_Attribute::getDataTypeFromString(std::string type) {
  if (type.compare("int") == 0) {
    return ESP_DataType::ESP_DataType_INT;
  } else if (type.compare("float") == 0) {
    return ESP_DataType::ESP_DataType_FLOAT;
  } else if (type.compare("long") == 0) {
    return ESP_DataType::ESP_DataType_LONG;
  } else if (type.compare("double") == 0) {
    return ESP_DataType::ESP_DataType_DOUBLE;
  } else if (type.compare("string") == 0) {
    return ESP_DataType::ESP_DataType_STRING;
  }

  return ESP_DataType::ESP_DataType_UNKNOWN;
}

int ESP_Attribute::getDataCodeFromString(std::string type) {
  if (type.compare("msb") == 0) {
    return ESP_DataCode::ESP_DataCode_MSB;
  } else if (type.compare("lsb") == 0) {
    return ESP_DataCode::ESP_DataCode_LSB;
  } else if (type.compare("string") == 0) {
    return ESP_DataCode::ESP_DataCode_STRING;
  } else if (type.compare("stringdelim") == 0) {
    return ESP_DataCode::ESP_DataCode_STRINGDELIM;
  }

  return ESP_DataCode::ESP_DataCode_UNKNOWN;
}

int ESP_Attribute::getIntValue() { return (_value) ? *(int*)_value : 0; }

float ESP_Attribute::getFloatValue() {
  return (_value) ? *(float*)_value : 0.0f;
}

long long ESP_Attribute::getLongValue() {
  return (_value) ? *(long long*)_value : 0ll;
}

double ESP_Attribute::getDoubleValue() {
  return (_value) ? *(double*)_value : 0.0;
}

std::string ESP_Attribute::getStringValue() {
  std::string result = "";
  if (_value != NULL) {
    result = std::string((const char*)_value, _datasize);
  }
  return result;
}

std::string ESP_Attribute::getName() { return _name; }

std::string ESP_Attribute::getValueAsString() {
  std::stringstream ss;

  switch (this->_datatype) {
    case ESP_DataType::ESP_DataType_INT: {
      ss << getIntValue();
      break;
    }
    case ESP_DataType::ESP_DataType_FLOAT: {
      ss << getFloatValue();
      break;
    }
    case ESP_DataType::ESP_DataType_LONG: {
      ss << getLongValue();
      break;
    }
    case ESP_DataType::ESP_DataType_DOUBLE: {
      ss << getDoubleValue();
      break;
    }
    case ESP_DataType::ESP_DataType_STRING: {
      ss << getStringValue();
      break;
    }
  };

  return ss.str();
}

ESP_Attribute* ESP_Attribute::searchAttributeRefByName(
    CC_AttributesType* attributes, std::string name) {
  // Search for all, but store only the last one found
  ESP_Attribute* attribute = NULL;

  CC_AttributesType::iterator it;

  // Search
  it = attributes->find(name);
  if (it != attributes->end()) {
    attribute = &it->second;
  }

  return attribute;
}
