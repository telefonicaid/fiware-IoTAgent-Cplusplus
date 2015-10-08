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
//
// Logger class definition file
//
#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <TDA.h>
#include <fstream>
#include <stdarg.h>
#include <vector>
#include <string>
#include <pthread.h>

#define LOGGER_MAX_FILES 5
#define LOGGER_MAX_FILE_SIZE 512000

class FileDescriptor {
 public:
  int size;
  std::string name;
  ccInt64 dateModified;

  static std::vector<FileDescriptor> enumerateDir(std::string path, int flags);
};

class Date {
 public:
  static void getLocalDate(int& year, int& month, int& day, int& hour,
                           int& minute, int& second, int& ms, int& dayOfWeek);
  static ccInt64 getDateMS(int year, int month, int day, int h, int m, int s,
                           int ms);
};

class CC_Logger {
 private:
  static CC_Logger* instance;

  std::string dirPath;
  // int maxLength;
  void log(int level, const char* fmt, va_list args);
  int level;

  static pthread_mutex_t mutex;

 public:
  /// ////////////////////
  /// Default Constructor
  /// ////////////////////
  CC_Logger();
  ~CC_Logger();
  FILE* fout;

  /// ////////////////////
  /// Array of Files
  /// ////////////////////
  std::vector<FileDescriptor> files;

  char actualFileName[256];

  /// ////////////////////
  /// get Singleton Instance
  /// ////////////////////
  static CC_Logger* getSingleton();
  static bool destroySingleton();

  /// ////////////////////
  /// get date from Log Name
  /// ////////////////////
  static ccInt64 parseDateFromLogFileName(char* fileName);

  /// ////////////////////
  /// Set Level of Log
  /// ////////////////////
  void setLevel(int level);
  void setLevel(std::string level);

  /// ////////////////////
  /// Close actual file
  /// ////////////////////
  void close();

  /// ////////////////////
  /// Re-Open actual file
  /// ////////////////////
  void reopen();

  /// ////////////////////
  /// Sets
  /// ////////////////////
  void setDirPath(std::string DirPath);
  void setMaxLength(int length);
  void setFileNumber(int number);

  /// ////////////////////
  /// Gets
  /// ////////////////////
  std::string getDirPath();

  void logInfo(const char* msg, ...);
  void logDebug(const char* msg, ...);
  void logError(const char* msg, ...);
  void readFiles();
  void removeMaxFiles();

  struct LogLevel {
    enum LogLevelEnum {
      LOG_LEVEL_DEFAULT,
      LOG_LEVEL_INFO,
      LOG_LEVEL_ERROR,
      LOG_LEVEL_DEBUG,
      LOG_LEVEL_ALL
    };
  };
};

#endif
