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
#ifndef __ESP_STRINGUTILS_H
#define __ESP_STRINGUTILS_H

#include <string>
#include <vector>
#include "TDA.h"

class ESP_StringUtils {
 public:
  static std::string AddEndingString(const std::string& inOriginalString,
                                     const std::string& inSearchString = "/");

  /**
   * @brief Apply format to string and return result
   * @param source the string to format
   * @param format string format to apply
   * @return the string formatted
   */
  static std::string applyFormat(std::string source, std::string format);

  /**
   * @brief Remove surrounding whitespace from a std::string.
   * @param s The string to be modified.
   * @param t The set of characters to delete from each end of the string.
   * @return The same string passed in as a parameter reference.
   */
  static std::string trim(std::string s, std::string t = " \t\n\r\f\v");

  /**
   * @brief Remove empty items of a string vector
   * @param vector The vector of string to be modified.
   */

  static void trim(std::vector<std::string>* vector);

  /**
   * @brief Replace in character string dest, one char with other
   * @param dest Character string where the replace will be done
   * @param srch Character for searching. This char will be replaced.
   * @param repl Character for replacing. This char will be used for to replace
   * the srch character.
   * @return Result character string with the replacement just done.
   */
  static const char* replaceChar(const char* dest, char srch, char repl);

  /**
   * @brief Convert the argument string in a new string with all character in
   * low case
   * @param str Is the argument string that will be converted
   * @return String with all its characters in low case
   */
  static std::string strToLower(std::string str);

  /**
   * @brief Convert the argument string in a new string with all character in
   * upper case
   * @param str Is the argument string that will be converted
   * @return String with all its characters in upper case
   */
  static std::string strToUpper(std::string str);

  /**
  * @brief Convert the argument string in a new string with only first character
  * in upper case
  * @param str Is the argument string that will be converted
  * @return String with all its characters in upper case
  */
  static std::string strToFirstCapital(std::string str);

  /**
   * @brief Test if a string is a real number value
   * @param str Is the argument string that will be tested
   * @return True if str is a real number value, false otherwise
   */
  static bool isRealNumber(std::string str);

  /**
   * @brief Test if a string is a real number value
   * @param str Is the argument string that will be tested
   * @return True if str is a real number value, false otherwise
   */
  static bool isIntegerNumber(std::string str);

  /**
   * @brief Test if a string is a valid login/password field (only plain
   * letters, numbers)
   * @param str Is the argument string that will be tested
   * @return True if str is a valid field
   */
  static bool isValidLogin(std::string str);

  /**
   * @brief Convert the argument const char ansi codification to utf-8
   * codification
   * @param pIn Is the argument char* that will be converted
   * @return const char * in utf-8 codification
   */
  static std::string AnsiToUtf8(std::string pIn);
  static bool isValidUtf8(const char* pIn);

  static bool startsWith(std::string const& fullString,
                         std::string const& starting);
  static bool endsWith(std::string const& fullString,
                       std::string const& ending);

  static std::string replaceNonAlphanumeric(std::string origin, char newChar);
  static std::string replaceString(std::string origin, std::string oldStr,
                                   std::string newStr);

  static std::string condense(std::string str);
  static bool splitPathAndFilename(std::string fullPath, std::string* path,
                                   std::string* filename);

  // Returns the index of the n ocurrence of the search string
  static size_t getStringOcurrencesPosition(std::string str, std::string search,
                                            int n);

  // CONVERTER METHODS
  static std::string intToString(int n);
  static std::string longToString(uint64 n);
  static std::string floatToString(float n);
  static std::string doubleToString(double n);
  static std::string convertToPassword(std::string str, bool convert);

  // DEBUG METHODS
  static std::string getStringBytesDebug(std::string str);

  /*
      Format Enum to add to Strings
  */
  struct FormatType {
    enum FormatTypeEnum {
      FORMAT_UPPERCASE,
      FORMAT_LOWERCASE,
      FORMAT_FIRSTCAPITAL,
      FORMAT_ADDSEMICOLON
    };
  };

  static ccInt64 atoll(const char* nptr);

  static CC_ParamsType parseParamsString(std::string str, bool valueToLower);
};

class CC_StringTokenizer {
 private:
  std::vector<std::string> elements;
  int index;
  int count;
  std::string sep;

 public:
  CC_StringTokenizer(std::string, std::string = ",", unsigned int = 0);
  int countElements();
  std::string elementAt(int);

  std::string getNextElement();

  std::string getStringBetweenElements(int initialIndex, int finalIndex);
  std::vector<std::string> getVectorStringBetweenElements(int initialIndex,
                                                          int finalIndex);
};

#endif
