/**
* Copyright 2015 Telefonica Investigaci√≥n y Desarrollo, S.A.U
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
#include "ESP_StringUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>

// Remove surrounding whitespace from a std::string.
std::string ESP_StringUtils::trim(std::string s, std::string t) {
  s.erase(0, s.find_first_not_of(t));
  s.erase(s.find_last_not_of(t) + 1);
  return s;
}

// Remove empty items of a string vector
void ESP_StringUtils::trim(std::vector<std::string>* vector) {
  std::vector<std::string>::iterator it = vector->begin();
  while (it != vector->end()) {
    if (it->length() == 0) {
      vector->erase(it);
    } else {
      it++;
    }
  }
}

std::string ESP_StringUtils::applyFormat(std::string source,
                                         std::string format) {
  std::string operation = strToLower(format);
  if (operation.compare("uppercase") == 0) {
    source = strToUpper(source);
  } else if (operation.compare("lowercase") == 0) {
    source = strToLower(source);
  } else if (operation.compare("firstcapital") == 0) {
    source = strToFirstCapital(source);
  } else if (operation.compare("addsemicolon") == 0) {
    source = AddEndingString(source, ":");
  }
  return source;
}

// [AddEndingCharacter]
// -----------------------------------------------------------------------------
// /////////////////////////////////////////////////////////////////////////////

std::string ESP_StringUtils::AddEndingString(
    const std::string& inOriginalString, const std::string& inSearchString) {
  std::string sOutString = inOriginalString;
  size_t nPos = sOutString.rfind(inSearchString);

  if ((nPos == std::string::npos) ||
      ((nPos + inSearchString.size()) < sOutString.size())) {
    sOutString += inSearchString;
  }

  return sOutString;
}

const char* ESP_StringUtils::replaceChar(const char* dest, char srch,
                                         char repl) {
  int i;
  int n = strlen(dest);
  char* ptr = (char*)dest;

  for (i = 0; i < n; i++) {
    if (ptr[i] == srch) {
      ptr[i] = repl;
    }
  }

  return dest;
}

std::string ESP_StringUtils::strToLower(std::string str) { return str; }

std::string ESP_StringUtils::strToUpper(std::string str) { return str; }

std::string ESP_StringUtils::strToFirstCapital(std::string str) { return str; }

bool ESP_StringUtils::isRealNumber(std::string str) {
  double testNumber;
  std::istringstream iss(str);
  iss >> testNumber;

  // was any input successfully consumed/converted?
  if (!iss) {
    return false;
  }

  // was all the input successfully consumed/converted?
  return (iss.rdbuf()->in_avail() == 0);
}

bool ESP_StringUtils::isIntegerNumber(std::string str) {
  int testNumber;
  std::istringstream iss(str);
  iss >> testNumber;

  // was any input successfully consumed/converted?
  if (!iss) {
    return false;
  }

  // was all the input successfully consumed/converted?
  return (iss.rdbuf()->in_avail() == 0);

  return false;
}

bool ESP_StringUtils::isValidLogin(std::string str) {
  for (unsigned int i = 0; i < str.length(); i++) {
    if (!((str.at(i) >= 'A' && str.at(i) <= 'Z') ||
          (str.at(i) >= 'a' && str.at(i) <= 'z') ||
          (str.at(i) >= '0' && str.at(i) <= '9'))) {
      return false;
    }
  }
  return true;
}

bool ESP_StringUtils::isValidUtf8(const char* pIn) {
  /* The chars invalid in UTF8 are (http://en.wikipedia.org/wiki/UTF-8)
      c0 - c1: 192 - 193
      f5 - ff: 245 - 255
  */

  int len = strlen(pIn);
  for (int i = 0; i < len; i++) {
    unsigned char c = pIn[i];
    if (c == 192 || c == 193 || (c >= 245 && c <= 255)) {
      return false;
    }
  }
  return true;
}

std::string ESP_StringUtils::getStringBytesDebug(std::string str) {
  const char* strchar = str.c_str();
  int n = strlen(strchar);

  std::stringstream ss;
  ss.str("");
  ss << str << ": ";
  for (int i = 0; i <= n; i++) {
    unsigned char byte = (unsigned char)strchar[i];
    ss << std::hex << (unsigned int)byte << " ";
  }

  return ss.str();
}

std::string ESP_StringUtils::AnsiToUtf8(std::string pIn) {
  std::string result;

  for (unsigned int i = 0; i < pIn.length(); i++) {
    char c = pIn.at(i);
    if (c & 0x80) {
      char c1 = 0xc0 | (c >> 6);
      char c2 = 0x80 | (c & 0x3f);
      result.push_back(c1);
      result.push_back(c2);
    } else {
      result.push_back(c);
    }
  }

  return result;
}

std::string ESP_StringUtils::replaceNonAlphanumeric(std::string origin,
                                                    char newChar) {
  for (std::string::iterator it = origin.begin(); it != origin.end(); it++) {
    if (!isalnum(*it)) {
      *it = newChar;
    }
  }

  return origin;
}

std::string ESP_StringUtils::replaceString(std::string origin,
                                           std::string oldStr,
                                           std::string newStr) {
  std::stringstream ss("");

  size_t pos = 0;
  int lastPos = 0;
  while (pos != std::string::npos) {
    pos = origin.find(oldStr, pos);

    if (pos != std::string::npos) {
      ss << origin.substr(lastPos, pos - lastPos) << newStr;

      pos += oldStr.length();
      lastPos = pos;
    }
  }

  ss << origin.substr(lastPos);

  std::string result(ss.str());

  return result;
}

bool ESP_StringUtils::startsWith(std::string const& fullString,
                                 std::string const& starting) {
  if (fullString.length() >= starting.length()) {
    return (0 == fullString.compare(0, starting.length(), starting));
  } else {
    return false;
  }
}

bool ESP_StringUtils::endsWith(std::string const& fullString,
                               std::string const& ending) {
  if (fullString.length() >= ending.length()) {
    return (0 ==
            fullString.compare(fullString.length() - ending.length(),
                               ending.length(), ending));
  } else {
    return false;
  }
}

ccInt64 ESP_StringUtils::atoll(const char* nptr) {
  ccInt64 longValue;
  sscanf(nptr, "%lld", &longValue);

  return longValue;
}

std::string ESP_StringUtils::condense(std::string str) {
  unsigned int iniSpaceString = std::string::npos;
  unsigned int endSpaceString = std::string::npos;
  unsigned int pos = std::string::npos;

  str = trim(str);

  // erase \n\t\r\f\v chars
  str = replaceString(str, "\n", " ");  // new line
  str = replaceString(str, "\t", " ");  // horizontal tab
  str = replaceString(str, "\r", " ");  // carriage return
  str = replaceString(str, "\f", " ");  // form feed
  str = replaceString(str, "\v", " ");  // vertical tab

  // convert spaces string in a unique space
  bool exists = true;
  while (exists) {
    exists = false;
    iniSpaceString = str.find("  ");  // find two spaces

    if (iniSpaceString != std::string::npos) {
      if (iniSpaceString + 1 < str.length()) {
        pos = str.find_first_not_of(" ", iniSpaceString + 1);
        if (pos != std::string::npos) {
          endSpaceString = pos - 1;
        } else {
          endSpaceString = str.length() - 1;
        }

        int spaceStringLen = endSpaceString - iniSpaceString + 1;

        if (spaceStringLen >= 2) {
          // Erasing all the spaces unless one
          str.erase(iniSpaceString + 1, spaceStringLen - 1);
          exists = true;
        }
      }
    }
  }

  return str;
}

bool ESP_StringUtils::splitPathAndFilename(std::string fullPath,
                                           std::string* path,
                                           std::string* filename) {
  size_t pos = 0;
  int n = fullPath.length();
  *path = "";
  *filename = "";
  bool result = true;

  pos = fullPath.rfind("\\");
  if (pos == std::string::npos) {
    pos = fullPath.rfind("/");
  }

  if (pos != std::string::npos) {
    if (pos - 1 >= 0 && pos - 1 < n) {
      *path = fullPath.substr(0, pos);
    } else {
      result = false;
    }

    if (pos + 1 >= 0 && pos + 1 < n) {
      *filename = fullPath.substr(pos + 1, std::string::npos);
    } else {
      result = false;
    }
  } else {
    *filename = fullPath;
    result = false;
  }

  return result;
}

size_t ESP_StringUtils::getStringOcurrencesPosition(std::string str,
                                                    std::string search, int n) {
  size_t result = std::string::npos;
  int occurrences = 0;
  std::string::size_type start = 0;

  while ((start = str.find(search, start)) != std::string::npos &&
         occurrences < n) {
    result = start;
    occurrences++;
    start += search.length();  // see the note
  }

  return result;
}

/*! Convierte un int a un string */
std::string ESP_StringUtils::intToString(int n) {
  std::stringstream result;
  result << n;
  return result.str();
}

/*! Convierte un long a un string */
std::string ESP_StringUtils::longToString(uint64 n) {
  std::stringstream result;
  result << n;
  return result.str();
}

/*! Convierte un float a un string */
std::string ESP_StringUtils::floatToString(float n) {
  std::stringstream result;
  result << n;
  return result.str();
}

/*! Convierte un double a un string */
std::string ESP_StringUtils::doubleToString(double n) {
  std::stringstream result;
  result << n;
  return result.str();
}

std::string ESP_StringUtils::convertToPassword(std::string str, bool convert) {
  std::string result;
  for (unsigned int i = 0; i < str.length(); i++) {
    if (convert) {
      result = result + "*";
    } else {
      result = result + str.at(i);
    }
  }

  return result;
}

CC_ParamsType ESP_StringUtils::parseParamsString(std::string str,
                                                 bool valueToLower) {
  CC_ParamsType pt;

  CC_StringTokenizer ST(str, ",");
  for (int i = 0; i < ST.countElements(); i++) {
    CC_StringTokenizer ST2(ST.elementAt(i), "=");
    if (ST2.countElements() == 2) {
      pt.insert(CC_ParamsPair(
          ESP_StringUtils::strToLower(ST2.elementAt(0)),
          valueToLower ? ESP_StringUtils::strToLower(ST2.elementAt(1))
                       : ST2.elementAt(1)));
    }
  }

  return pt;
}

// Constructor that takes 2 arguments
// first argument is of string type that to be tokenized.
// second argument is of string type that is used as token seperator
// and default seperator is space
CC_StringTokenizer::CC_StringTokenizer(std::string str, std::string sep,
                                       unsigned int maxtokens) {
  index = 0;
  count = 0;
  this->sep = sep;
  std::string str1 = "";
  for (unsigned int i = 0; i < str.length() && sep.length() == 1; i++) {
    // Hay un Separador y no se ha alcanzado el n∫ maximo de tokens
    if (str.at(i) == sep.at(0)) {
      if (maxtokens <= 0 || elements.size() < (maxtokens)) {
        elements.insert(elements.end(), str1);
      }
      str1 = "";
    } else {
      str1 += str.at(i);
    }
  }
  // Add Last
  if (!str1.empty() && (maxtokens <= 0 || elements.size() < (maxtokens))) {
    elements.insert(elements.end(), str1);
  }
  count = (int)elements.size();
}

// Method is used to fetch the tokens.
std::string CC_StringTokenizer::getNextElement() {
  std::string result = "";

  if (index < count) {
    result = elements[index];
    index++;
  }

  return result;
}
// method used to fetch the count of tokens from the string
int CC_StringTokenizer::countElements() { return count; }

// fetch the elements at given position
std::string CC_StringTokenizer::elementAt(int index) {
  if (index >= count || index < 0) {
    return "";
    // throw CString("Index out of Bounds");
  } else {
    return elements[index];
  }
}

std::string CC_StringTokenizer::getStringBetweenElements(int initialIndex,
                                                         int finalIndex) {
  std::string result = "";

  for (int i = initialIndex; i <= finalIndex; i++) {
    result += elements[i];
    if (i < finalIndex) {
      result += sep;
    }
  }

  return result;
}

std::vector<std::string> CC_StringTokenizer::getVectorStringBetweenElements(
    int initialIndex, int finalIndex) {
  std::vector<std::string> vector;

  for (int i = initialIndex; i <= finalIndex; i++) {
    vector.push_back(elements[i]);
  }

  return vector;
}
