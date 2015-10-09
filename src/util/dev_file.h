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
#ifndef SRC_UTIL_DEV_FILE_H_
#define SRC_UTIL_DEV_FILE_H_

#include <string>
#include <map>
#include <rapidjson/document.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include "util/iota_logger.h"
#include "util/cache.h"
#include "util/common.h"

namespace iota {

class DevicesFile {
 public:
  static DevicesFile* instance();
  static DevicesFile* initialize(const std::string& filename);
  static void release();

  std::string read_file(std::stringstream& is);

  void parse_to_cache(Cache* pcache);

  const JsonValue& getDevicesObject();

 protected:
 private:
  DevicesFile();
  void set_filename(std::string filename);
  void init();

  boost::recursive_mutex m_mutex_document;
  pion::logger m_log;
  std::string _filename;
  std::string _error;
  static DevicesFile* pinstance;
  JsonDocument _document;
};
};
#endif
