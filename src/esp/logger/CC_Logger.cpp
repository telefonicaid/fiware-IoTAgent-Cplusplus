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
// Logger class implementation file
//

#include <CC_Logger.h>
#include <errno.h>
#include <algorithm>
#include <sys/stat.h>
// ---------------------
#include <dirent.h>
#if defined(_WIN32_WINNT)
#include <direct.h>
#define va_copy(dest, src) (dest = src)
#endif
// ---------------------
#include "ESP_StringUtils.h"

CC_Logger* CC_Logger::instance = NULL;
pthread_mutex_t CC_Logger::mutex;

bool compareFiles(FileDescriptor a, FileDescriptor b) {
  // Compare by date (order descend)
  return a.dateModified > b.dateModified;
}

/// Default Constructor
CC_Logger::CC_Logger() {
  fout = NULL;
  dirPath = "";
  // maxLength = 1024 * 1024; // 1MB
  setDirPath(".");
  level = LogLevel::LOG_LEVEL_ALL;
  pthread_mutex_init(&mutex, NULL);
}

/// Default Destructor
CC_Logger::~CC_Logger() {
  close();
  pthread_mutex_destroy(&mutex);
}

void CC_Logger::setLevel(int level) { this->level = level; }

void CC_Logger::setLevel(std::string level) {
  if (ESP_StringUtils::strToLower(level).compare("default") == 0) {
    setLevel(LogLevel::LOG_LEVEL_DEFAULT);
  } else if (ESP_StringUtils::strToLower(level).compare("info") == 0) {
    setLevel(LogLevel::LOG_LEVEL_INFO);
  } else if (ESP_StringUtils::strToLower(level).compare("debug") == 0) {
    setLevel(LogLevel::LOG_LEVEL_DEBUG);
  } else if (ESP_StringUtils::strToLower(level).compare("error") == 0) {
    setLevel(LogLevel::LOG_LEVEL_ERROR);
  } else if (ESP_StringUtils::strToLower(level).compare("all") == 0) {
    setLevel(LogLevel::LOG_LEVEL_ALL);
  }
}

void CC_Logger::close() {
  if (fout) {
    fflush(fout);
    fclose(fout);
  }
  fout = NULL;
}

void CC_Logger::reopen() {
  /*
  if ( fout == NULL )
  {
      std::string fullPath = dirPath + actualFileName ;
      fout = fopen(fullPath.c_str(),"a") ;
  }
  */
}

// Get Singleton
CC_Logger* CC_Logger::getSingleton() {
  if (instance == NULL) {
    instance = new CC_Logger();
  }
  return instance;
}

bool CC_Logger::destroySingleton() {
  if (CC_Logger::instance != NULL) {
    delete CC_Logger::instance;
    CC_Logger::instance = NULL;
    return true;
  } else {
    return false;
  }
}

// Set Path for Files
void CC_Logger::setDirPath(std::string path) {
  this->dirPath = path;

  // Close actual File
  close();

  // Set new Path
  if (dirPath.length() == 0) {
    dirPath = ".";
  }
  if (dirPath.at(dirPath.length() - 1) != '/') {
    dirPath = dirPath + "/";
  }
  // dirPath = dirPath + "logs/";

  // MKDIR
  int result = 0;
#if defined(_WIN32_WINNT)
  result = mkdir(dirPath.c_str());
#else
  result =
      mkdir(dirPath.c_str(), 0777);  // notice that 777 is different than 0777
#endif
  if (result == -1 && errno != EEXIST) {
    // Ok
  }

  // Read Files
  readFiles();
}

// Add Log Entry
void CC_Logger::log(int level, const char* fmt, va_list args) {
  va_list copy;
  pthread_mutex_lock(&mutex);

  // Get New Log File
  int year, month, day, h, m, s, ms, dayOfWeek;
  Date::getLocalDate(year, month, day, h, m, s, ms, dayOfWeek);
  memset(actualFileName, 0x0, sizeof(char) * 256);
  sprintf(actualFileName, "log_%d_%d_%d", day, month, year);

  FileDescriptor fs;
  fs.name = std::string(actualFileName);
  bool found = false;
  if (files.size() > 0) {
    // File Found
    if (files[0].name.find(fs.name) != std::string::npos) {
      fs = files[0];
      found = true;
    }
  }
  // Max File Size
  if (fs.size > LOGGER_MAX_FILE_SIZE) {
    found = false;
  }

  // Not Found, create new
  if (!found) {
    close();
    sprintf(actualFileName, "log_%d_%d_%d_%d_%d_%d.txt", day, month, year, h, m,
            s);
    fs.size = 0;
    fs.name = std::string(actualFileName);
    files.insert(files.begin(), fs);
    removeMaxFiles();
  }

  // Create File
  if (fout == NULL) {
    std::string fullPath = dirPath + fs.name;
    fout = fopen(fullPath.c_str(), "a");
  }

  // Date and Time
  char bufferDateTime[64];
  Date::getLocalDate(year, month, day, h, m, s, ms, dayOfWeek);
  sprintf(bufferDateTime, "%02d-%02d-%4d %02d:%02d:%02d", day, month, year, h,
          m, s);

  // Level Name
  std::string level_name = "Default";
  if (level == CC_Logger::LogLevel::LOG_LEVEL_DEBUG) {
    level_name = "DEBUG";
  } else if (level == CC_Logger::LogLevel::LOG_LEVEL_ERROR) {
    level_name = "ERROR";
  } else if (level == CC_Logger::LogLevel::LOG_LEVEL_INFO) {
    level_name = "INFO";
  }

  // Compose Message
  std::string fullMsg = bufferDateTime + std::string(" [") + level_name +
                        std::string("] : ") + std::string(fmt) +
                        std::string("\n");

  // Printf
  va_copy(copy, args);
  int n = vprintf(fullMsg.c_str(), copy);
  if (n > 0) {
    fs.size += n;  // add size
  }
  va_end(copy);

  // Write
  if (fout) {
    va_copy(copy, args);
    vfprintf(fout, fullMsg.c_str(), copy);
    va_end(copy);
    //*fout << fullMsg;
    // fout->flush();
    fflush(fout);
  }

  pthread_mutex_unlock(&mutex);
}

void CC_Logger::logInfo(const char* fmt, ...) {
  if (this->level < LogLevel::LOG_LEVEL_INFO) {
    return;
  }

  va_list args;
  va_start(args, fmt);
  log(CC_Logger::LogLevel::LOG_LEVEL_INFO, fmt, args);
  va_end(args);
}

void CC_Logger::logDebug(const char* fmt, ...) {
  if (this->level < LogLevel::LOG_LEVEL_DEBUG) {
    return;
  }

  va_list args;
  va_start(args, fmt);
  log(CC_Logger::LogLevel::LOG_LEVEL_DEBUG, fmt, args);
  va_end(args);
}

void CC_Logger::logError(const char* fmt, ...) {
  if (this->level < LogLevel::LOG_LEVEL_ERROR) {
    return;
  }

  va_list args;
  va_start(args, fmt);
  log(CC_Logger::LogLevel::LOG_LEVEL_ERROR, fmt, args);
  va_end(args);
}

void CC_Logger::readFiles() {
  files = FileDescriptor::enumerateDir(dirPath, 0);

  // Order by date (descend)
  std::sort(files.begin(), files.end(), compareFiles);

  removeMaxFiles();
  return;
}

void CC_Logger::removeMaxFiles() {
  // Delete old ones if more than MAX_FILES
  while (files.size() > LOGGER_MAX_FILES) {
    FileDescriptor fs = files.at(files.size() - 1);
    files.pop_back();
    remove((dirPath + fs.name).c_str());
  }
}

// Get Path for Files
std::string CC_Logger::getDirPath() { return dirPath; }

// Parse Date from Log Name
ccInt64 CC_Logger::parseDateFromLogFileName(char* fileName) {
  ccInt64 date = 0;

  // Remove LOG_
  CC_StringTokenizer s1(std::string(fileName), ".");
  if (s1.countElements() == 2) {
    CC_StringTokenizer s2(s1.elementAt(0), "_");
    if (s2.countElements() == 7) {
      int day = atoi(s2.elementAt(1).c_str());
      int month = atoi(s2.elementAt(2).c_str());
      int year = atoi(s2.elementAt(3).c_str());
      int hour = atoi(s2.elementAt(4).c_str());
      int minute = atoi(s2.elementAt(5).c_str());
      int second = atoi(s2.elementAt(6).c_str());
      date = Date::getDateMS(year, month, day, hour, minute, second, 0);
    }
  }

  return date;
}

std::vector<FileDescriptor> FileDescriptor::enumerateDir(std::string path,
                                                         int flags) {
  std::vector<FileDescriptor> result;
  DIR* dir = NULL;
  struct dirent* ent = NULL;
  if ((dir = opendir(path.c_str())) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (ent->d_type != DT_DIR) {
        std::string fileName(ent->d_name);
        if (fileName.find(".txt") != std::string::npos) {
          FileDescriptor fd;
          fd.name = ent->d_name;
          result.push_back(fd);
        }
      }
    }
  }
  closedir(dir);
  return result;
}

void Date::getLocalDate(int& year, int& month, int& day, int& hour, int& minute,
                        int& second, int& ms, int& dayOfWeek) {
  // Get struct tm Local
  time_t localsec = 0;
  time(&localsec);
  struct tm* timeInfoLocal = localtime(&localsec);
  // time_t localSec = mktime(timeInfoLocal);

  year = timeInfoLocal->tm_year + 1900;
  month = timeInfoLocal->tm_mon + 1;  // month 0-11.
  day = timeInfoLocal->tm_mday;
  hour = timeInfoLocal->tm_hour;
  minute = timeInfoLocal->tm_min;
  second = timeInfoLocal->tm_sec;
  ms = 0;
  dayOfWeek = 0;
}

ccInt64 Date::getDateMS(int year, int month, int day, int hour, int minute,
                        int second, int ms) {
  struct tm timeInfoLocal;
  timeInfoLocal.tm_year = year;
  timeInfoLocal.tm_mon = month;
  timeInfoLocal.tm_mday = day;
  timeInfoLocal.tm_hour = hour;
  timeInfoLocal.tm_min = minute;
  timeInfoLocal.tm_sec = second;
  time_t localSec = mktime(&timeInfoLocal);

  return localSec * 1000;
}
