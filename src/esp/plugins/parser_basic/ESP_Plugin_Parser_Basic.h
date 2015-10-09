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
#ifndef __ESP_PLUGIN_PARSER_BASIC_H__
#define __ESP_PLUGIN_PARSER_BASIC_H__
#include <TDA.h>
#include <string>
#include <stdlib.h>

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< PLUGIN PARSER BASIC >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
class ESP_Plugin_Parser_Basic : public ESP_Plugin_Parser_Base {
 private:
  static ESP_Plugin_Parser_Basic* instance;

 public:
  static ESP_Plugin_Parser_Base* getSingleton();

  // -------------------------------------------------------------------------
  // [public-methods]
  // -------------------------------------------------------------------------
  ESP_Tag_Base* createParser(TiXmlElement* element);
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
};

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/**< BASIC TAGS >*/
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

// -------------------------------------------------------------------------
// [ESP_Tag_Switch]
// -------------------------------------------------------------------------
class ESP_Tag_Switch : public ESP_Tag_Base {
 public:
  // -------------------------------------------------------------------------
  // [public-attributes]
  // -------------------------------------------------------------------------
  bool _evaluated;

  // -------------------------------------------------------------------------
  // [public-methods]
  // -------------------------------------------------------------------------
  ESP_Tag_Switch();
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
  void reset();
};

// -------------------------------------------------------------------------
// [ESP_Tag_Option]
// -------------------------------------------------------------------------
class ESP_Tag_Option : public ESP_Tag_Base {
 public:
  // -------------------------------------------------------------------------
  // [public-methods]
  // -------------------------------------------------------------------------
  ESP_Tag_Option();
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
};

// -------------------------------------------------------------------------
// [ESP_Tag_Reference]
// -------------------------------------------------------------------------
class ESP_Tag_Reference : public ESP_Tag_Base {
 public:
  // -------------------------------------------------------------------------
  // [public-methods]
  // -------------------------------------------------------------------------
  ESP_Tag_Reference();
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
};

// -------------------------------------------------------------------------
// [ESP_Tag_Constraint]
// -------------------------------------------------------------------------
class ESP_Tag_Constraint : public ESP_Tag_Base {
 public:
  // -------------------------------------------------------------------------
  // [public-methods]
  // -------------------------------------------------------------------------
  ESP_Tag_Constraint();
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
};

// -------------------------------------------------------------------------
// [ESP_Tag_Value]
// -------------------------------------------------------------------------
class ESP_Tag_Value : public ESP_Tag_Base {
 public:
  // -------------------------------------------------------------------------
  // [public-attributes]
  // -------------------------------------------------------------------------
  std::string _nameref;
  std::string _valueref;
  bool _resetend;  // Back to original pointer to buffer end after reading the
                   // value

  // -------------------------------------------------------------------------
  // [public-methods]
  // -------------------------------------------------------------------------
  ESP_Tag_Value();
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
};

// -------------------------------------------------------------------------
// [ESP_Tag_Read]
// -------------------------------------------------------------------------
class ESP_Tag_Read : public ESP_Tag_Base {
 public:
  // -------------------------------------------------------------------------
  // [public-methods]
  // -------------------------------------------------------------------------
  ESP_Tag_Read();
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
};

// -------------------------------------------------------------------------
// [ESP_Tag_Write]
// -------------------------------------------------------------------------
class ESP_Tag_Write : public ESP_Tag_Base {
 public:
  // -------------------------------------------------------------------------
  // [public-methods]
  // -------------------------------------------------------------------------
  ESP_Tag_Write();
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
};

// -------------------------------------------------------------------------
// [ESP_Tag_Close]
// -------------------------------------------------------------------------
class ESP_Tag_Close : public ESP_Tag_Base {
 public:
  // -------------------------------------------------------------------------
  // [public-methods]
  // -------------------------------------------------------------------------
  ESP_Tag_Close();
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
};

// -------------------------------------------------------------------------
// [ESP_Tag_Condition]
// -------------------------------------------------------------------------
class ESP_Tag_Condition : public ESP_Tag_Base {
 public:
  // -------------------------------------------------------------------------
  // [public-attributes]
  // -------------------------------------------------------------------------
  std::string _operation;

  // -------------------------------------------------------------------------
  // [public-methods]
  // -------------------------------------------------------------------------
  ESP_Tag_Condition();
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
};

// -------------------------------------------------------------------------
// [ESP_Tag_Loop]
// -------------------------------------------------------------------------
class ESP_Tag_Loop : public ESP_Tag_Base {
 private:
  bool _breakonempty;
  int _iterations;

 public:
  // -------------------------------------------------------------------------
  // [public-attributes]
  // -------------------------------------------------------------------------
  int iter;
  int niter;

  // -------------------------------------------------------------------------
  // [public-methods]
  // -------------------------------------------------------------------------
  ESP_Tag_Loop();
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
  void reset();
};

// -------------------------------------------------------------------------
// [ESP_Tag_Break]
// -------------------------------------------------------------------------
class ESP_Tag_Break : public ESP_Tag_Base {
 private:
 public:
  // -------------------------------------------------------------------------
  // [public-attributes]
  // -------------------------------------------------------------------------
  int iter;

  // -------------------------------------------------------------------------
  // [public-methods]
  // -------------------------------------------------------------------------
  ESP_Tag_Break();
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
};

// -------------------------------------------------------------------------
// [ESP_Tag_CompoundValue]
// -------------------------------------------------------------------------
class ESP_Tag_CompoundValue : public ESP_Tag_Base {
 public:
  // -------------------------------------------------------------------------
  // [public-attributes]
  // -------------------------------------------------------------------------

  // -------------------------------------------------------------------------
  // [public-methods]
  // -------------------------------------------------------------------------
  ESP_Tag_CompoundValue();
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
};

// -------------------------------------------------------------------------
// [ESP_Tag_Clear]
// Clear context
// -------------------------------------------------------------------------
class ESP_Tag_Clear : public ESP_Tag_Base {
 public:
  // -------------------------------------------------------------------------
  // [public-methods]
  // -------------------------------------------------------------------------
  ESP_Tag_Clear();
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
};

// -------------------------------------------------------------------------
// [ESP_Tag_Result]
// Returns a valid data
// -------------------------------------------------------------------------
class ESP_Tag_Result : public ESP_Tag_Base {
 public:
  // -------------------------------------------------------------------------
  // [public-attributes]
  // -------------------------------------------------------------------------
  bool _nodata;

  // -------------------------------------------------------------------------
  // [public-methods]
  // -------------------------------------------------------------------------
  ESP_Tag_Result();
  void parseCustomElement(TiXmlElement* element);
  int execute(ESP_Context* context);
};

#endif
