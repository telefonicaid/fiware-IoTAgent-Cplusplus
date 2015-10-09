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
#include "parser_basic/ESP_Plugin_Parser_Basic.h"
#include "ESP_XmlUtils.h"
#include "CC_Logger.h"
#include "ESP_StringUtils.h"

ESP_Plugin_Parser_Basic* ESP_Plugin_Parser_Basic::instance = NULL;

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< PLUGIN PARSER BASIC >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
ESP_Tag_Base* ESP_Plugin_Parser_Basic::createParser(TiXmlElement* element) {
  std::string type = ESP_XmlUtils::queryStringValue(element, "type");
  std::string name = ESP_XmlUtils::queryStringValue(element, "name");
  ESP_Tag_Base* result = NULL;

  if (type == "read") {
    result = new ESP_Tag_Read();
  } else if (type == "write") {
    result = new ESP_Tag_Write();
  } else if (type == "value") {
    result = new ESP_Tag_Value();
  } else if (type == "constraint") {
    result = new ESP_Tag_Constraint();
  } else if (type == "close") {
    result = new ESP_Tag_Close();
  } else if (type == "condition") {
    result = new ESP_Tag_Condition();
  } else if (type == "loop") {
    result = new ESP_Tag_Loop();
  } else if (type == "clear") {
    result = new ESP_Tag_Clear();
  } else if (type == "result") {
    result = new ESP_Tag_Result();
  } else if (type == "switch") {
    result = new ESP_Tag_Switch();
  } else if (type == "option") {
    result = new ESP_Tag_Option();
  } else if (type == "break") {
    result = new ESP_Tag_Break();
  }

  // Assigns
  if (result != NULL) {
    result->_type = type;
    result->_name = name;
  }

  return result;
}

ESP_Plugin_Parser_Base* ESP_Plugin_Parser_Basic::getSingleton() {
  if (ESP_Plugin_Parser_Basic::instance == NULL) {
    ESP_Plugin_Parser_Basic::instance = new ESP_Plugin_Parser_Basic();
  }
  return ESP_Plugin_Parser_Basic::instance;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< READ >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
ESP_Tag_Read::ESP_Tag_Read() {}

void ESP_Tag_Read::parseCustomElement(TiXmlElement* element) {}

int ESP_Tag_Read::execute(ESP_Context* context) {
  const char* result = NULL;

  if (context->readPInputData(this->_datasize, result)) {
    return ESP_Tag_Base::EXECUTE_RESULT_OK;
  } else {
    return ESP_Tag_Base::EXECUTE_RESULT_IDLE;
  }

  return ESP_Tag_Base::EXECUTE_RESULT_OK;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< WRITE >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
ESP_Tag_Write::ESP_Tag_Write() {}

void ESP_Tag_Write::parseCustomElement(TiXmlElement* element) {}

int ESP_Tag_Write::execute(ESP_Context* context) {
  // Update Ref
  if (!_valueref.empty()) {
    ESP_Attribute* attrtemp =
        ESP_Attribute::searchAttributeRefByName(&context->temp, _valueref);
    if (attrtemp != NULL) {
      this->_value = *attrtemp;
    }
  }

  // Generate Output
  context->addOutputData((const char*)this->_value._value,
                         this->_value._datasize);
  return ESP_Tag_Base::EXECUTE_RESULT_OK;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< CLOSE >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
ESP_Tag_Close::ESP_Tag_Close() {}

void ESP_Tag_Close::parseCustomElement(TiXmlElement* element) {}

int ESP_Tag_Close::execute(ESP_Context* context) {
  /*
  ESP_Attribute closeAttribute;
  int value = 1;
  closeAttribute.setValue("close",(const char
  *)&value,sizeof(value),ESP_DataType::ESP_DataType_INT,0);
  */
  context->close = true;
  return ESP_Tag_Base::EXECUTE_RESULT_OK;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< CONSTRAINT >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
ESP_Tag_Constraint::ESP_Tag_Constraint() {}

void ESP_Tag_Constraint::parseCustomElement(TiXmlElement* element) {
  // printf("Parsing Tag Constraint\n");
}

int ESP_Tag_Constraint::execute(ESP_Context* context) {
  const char* result = NULL;
  if (context->readPInputData(this->_datasize, result)) {
    // printf("Executing Tag Constraint\n");

    ESP_Attribute atri_result;
    atri_result.setValue(_name, result, this->_datasize, this->_datatype,
                         this->_datacode);

    if (ESP_Attribute::compareValue(&this->_value, &atri_result) == 0) {
      return ESP_Tag_Base::EXECUTE_RESULT_OK;
    } else {
      return ESP_Tag_Base::EXECUTE_RESULT_ERROR;
    }
  } else {
    return ESP_Tag_Base::EXECUTE_RESULT_IDLE;
  }

  return ESP_Tag_Base::EXECUTE_RESULT_OK;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< VALUE >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
ESP_Tag_Value::ESP_Tag_Value() { _resetend = false; }

void ESP_Tag_Value::parseCustomElement(TiXmlElement* element) {
  // printf("Parsing Tag Value\n");
  this->_nameref = ESP_XmlUtils::queryStringValue(element, "nameref");
  this->_valueref = ESP_XmlUtils::queryStringValue(element, "valueref");
  this->_resetend = ESP_XmlUtils::queryBoolValue(element, "resetend");
}

int ESP_Tag_Value::execute(ESP_Context* context) {
  const char* result = NULL;
  int finalsize = 0;
  int dataread = 0;

  // Save Index
  context->saveIndex();

  // Ref
  if (!_valueref.empty()) {
    // NameRef
    if (!_nameref.empty()) {
      ESP_Attribute* nameref =
          ESP_Attribute::searchAttributeRefByName(&context->temp, _nameref);
      _name = nameref->getValueAsString();
    }

    // ValueRef
    ESP_Attribute* valueref =
        ESP_Attribute::searchAttributeRefByName(&context->temp, _valueref);
    if (!_name.empty() && valueref != NULL) {
      result = (char*)valueref->_value;
      finalsize = dataread = valueref->_datasize;
      if (this->_datatype == 0) {
        this->_datatype = valueref->_datatype;
      }
      if (this->_datacode == 0) {
        this->_datacode = valueref->_datacode;
      }
    }
  }
  // stringdelim (switch to valuestring)
  else if (this->_datacode == ESP_DataCode::ESP_DataCode_STRINGDELIM) {
    // Search delimiter if any
    int index = context->searchAttributeValue(&this->_value);
    if (index >= 0) {
      finalsize = context->getSizeFromIndex(index);

      // If value datasize zero we return all remaining buffer
      if (this->_value._datasize == 0) {
        finalsize = context->getAvailableInputData();
      }

      // Update dataread
      dataread = context->readPInputData(finalsize, result);

      // Read Delim
      const char* delim = NULL;
      context->readPInputData(this->_value._datasize, delim);
    }
    // Get All Remaining Buffer (TODO: CHECK THIS, CAN BE A WRONG DECISION IF
    // STREAMING DATA)
    else {
      finalsize = context->getAvailableInputData();
      dataread = context->readPInputData(finalsize, result);
    }
  }
  // Normal Value
  else {
    finalsize = this->_datasize;
    dataread = context->readPInputData(finalsize, result);
  }

  // Restore
  if (this->_resetend) {
    context->restoreIndex();
  }

  // Read Value
  if (dataread >= 0) {
    // printf("Executing Tag Value\n");
    std::string finalname = _name;

    // Loop Name if $
    if (finalname.find("$") != std::string::npos) {
      ESP_Tag_Base* parentloop = ESP_Tag_Base::searchParent(this, "loop");
      if (parentloop != NULL) {
        finalname = ESP_StringUtils::replaceString(
            finalname, "$",
            ESP_StringUtils::intToString(((ESP_Tag_Loop*)parentloop)->niter));
      }
    }

    ESP_Attribute atri_result;
    atri_result.setValue(finalname, result, finalsize, this->_datatype,
                         this->_datacode);

    context->temp[finalname] = atri_result;  // Overwrite
    return ESP_Tag_Base::EXECUTE_RESULT_OK;
  } else {
    return ESP_Tag_Base::EXECUTE_RESULT_IDLE;
  }

  return ESP_Tag_Base::EXECUTE_RESULT_OK;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< CONDITION >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
ESP_Tag_Condition::ESP_Tag_Condition() {}

void ESP_Tag_Condition::parseCustomElement(TiXmlElement* element) {
  this->_operation = ESP_XmlUtils::queryStringValue(element, "operation");
}

int ESP_Tag_Condition::execute(ESP_Context* context) {
  // Get Reference from Context
  ESP_Attribute* valueref =
      ESP_Attribute::searchAttributeRefByName(&context->temp, _valueref);

  // Reference
  if (valueref != NULL) {
    int compare = ESP_Attribute::compareValue(valueref, &_value);
    if ((_operation == "" || _operation == "equal") && compare == 0) {
      // OK
    } else if (_operation == "notequal" && compare != 0) {
      // OK
    } else if (_operation == "notcontains" &&
               valueref->getValueAsString().find(_value.getValueAsString()) ==
                   std::string::npos) {
      // OK
    } else if (_operation == "contains" &&
               valueref->getValueAsString().find(_value.getValueAsString()) !=
                   std::string::npos) {
      // OK
    } else if (_operation == "default") {
      // it will just return OK no matter what value is (in fact, there should
      // be no value).
    } else {
      // Cancel Children
      setChildrenParsed(true, true);
    }
  }
  // From Buffer
  else {
    int index = context->searchAttributeValueInRange(&this->_value, _byteoffset,
                                                     this->_datasize);
    if (index < 0) {
      // Cancel Children
      setChildrenParsed(true, true);
    }
  }

  return ESP_Tag_Base::EXECUTE_RESULT_OK;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< SWITCH >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
ESP_Tag_Switch::ESP_Tag_Switch() {}

void ESP_Tag_Switch::parseCustomElement(TiXmlElement* element) {
  _evaluated = false;
}

int ESP_Tag_Switch::execute(ESP_Context* context) {
  // Update Ref
  if (!_valueref.empty()) {
    ESP_Attribute* attrtemp =
        ESP_Attribute::searchAttributeRefByName(&context->temp, _valueref);
    if (attrtemp != NULL) {
      this->_value = *attrtemp;
    }
  }

  return ESP_Tag_Base::EXECUTE_RESULT_OK;
}

void ESP_Tag_Switch::reset() {
  ESP_Tag_Base::reset();
  _evaluated = false;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< OPTION >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
ESP_Tag_Option::ESP_Tag_Option() {}

void ESP_Tag_Option::parseCustomElement(TiXmlElement* element) {}

int ESP_Tag_Option::execute(ESP_Context* context) {
  if (parent != NULL && parent->_type == "switch") {
    ESP_Tag_Switch* switchtag = (ESP_Tag_Switch*)parent;
    ESP_Attribute* ref = ESP_Attribute::searchAttributeRefByName(
        &context->temp, switchtag->_valueref);
    int compare = ESP_Attribute::compareValue(ref, &_value);
    if ((compare == 0 || _value._datasize == 0) && !switchtag->_evaluated) {
      switchtag->_evaluated = true;
    } else {
      setChildrenParsed(true, true);
    }

    return ESP_Tag_Base::EXECUTE_RESULT_OK;
  } else {
    return ESP_Tag_Base::EXECUTE_RESULT_ERROR;
  }
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< LOOP >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
ESP_Tag_Loop::ESP_Tag_Loop() {
  niter = 0;
  iter = 0;
  _iterations = 0;
  _breakonempty = false;
}

void ESP_Tag_Loop::parseCustomElement(TiXmlElement* element) {
  this->_breakonempty = ESP_XmlUtils::queryBoolValue(element, "breakonempty");
  this->_iterations = ESP_XmlUtils::queryIntValue(element, "iterations");
  this->iter = this->_iterations;
}

int ESP_Tag_Loop::execute(ESP_Context* context) {
  // Because last execution is valid we set > 1
  if (iter != 0 && (!_breakonempty || context->getAvailableInputData() > 0)) {
    // Normal execution wont reset niter
    int oldniter = niter;
    reset();
    niter = oldniter;

    // Next Iter
    _repeat = true;
    if (iter > 0) {
      iter--;
    }
    niter++;

    return ESP_Tag_Base::EXECUTE_RESULT_OK;
  } else {
    _repeat = false;
    iter = 0;  // Redundant

    // Cancel Children
    setChildrenParsed(true, true);

    return ESP_Tag_Base::EXECUTE_RESULT_OK;
  }
}

void ESP_Tag_Loop::reset() {
  ESP_Tag_Base::reset();
  iter = _iterations;
  niter = 0;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< BREAK >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
ESP_Tag_Break::ESP_Tag_Break() {}

void ESP_Tag_Break::parseCustomElement(TiXmlElement* element) {}

int ESP_Tag_Break::execute(ESP_Context* context) {
  // Search first breakable parent
  bool found = false;
  ESP_Tag_Base* parentloop = ESP_Tag_Base::searchParent(this, "loop");
  if (parentloop != NULL) {
    ((ESP_Tag_Loop*)parentloop)->iter = 0;  // No more iterations
  }

  return ESP_Tag_Base::EXECUTE_RESULT_OK;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< COMPOUNDVALUE >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
ESP_Tag_CompoundValue::ESP_Tag_CompoundValue() {}

void ESP_Tag_CompoundValue::parseCustomElement(TiXmlElement* element) {}

int ESP_Tag_CompoundValue::execute(ESP_Context* context) {
  return ESP_Tag_Base::EXECUTE_RESULT_OK;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< CLEAR >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
ESP_Tag_Clear::ESP_Tag_Clear() {}

void ESP_Tag_Clear::parseCustomElement(TiXmlElement* element) {}

int ESP_Tag_Clear::execute(ESP_Context* context) {
  context->temp.clear();
  return ESP_Tag_Base::EXECUTE_RESULT_OK;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< RETURN >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
ESP_Tag_Result::ESP_Tag_Result() { _nodata = false; }

void ESP_Tag_Result::parseCustomElement(TiXmlElement* element) {
  this->_nodata = ESP_XmlUtils::queryBoolValue(element, "nodata");
}

int ESP_Tag_Result::execute(ESP_Context* context) {
  if (_nodata == false ||
      (_nodata == true && context->getAvailableInputData() == 0)) {
    context->addResultData();
  }
  return ESP_Tag_Base::EXECUTE_RESULT_OK;
}
