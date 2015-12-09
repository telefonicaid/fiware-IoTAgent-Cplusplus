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
#ifndef SRC_UTIL_ALARM_H_
#define SRC_UTIL_ALARM_H_

#include <string>
#include <map>
#include "util/iota_logger.h"
#include "rest/process.h"
#include <boost/thread/recursive_mutex.hpp>
#include <boost/property_tree/ptree.hpp>

namespace iota {

class Alarm {
 public:
  static Alarm* instance();

  static void error(int code, const std::string& endpoint,
                    const std::string& status, const std::string& text);

  static void error(int code, const std::string& endpoint,
                    const std::string &content,
                    const boost::property_tree::ptree &additional_info,
                    const std::string& status, const std::string& text);

  static void info(int code, const std::string& endpoint,
                   const std::string& status, const std::string& text);

  void put(int code, const std::string& endpoint, const std::string& status,
           const std::string& text);

  void put(int code, const std::string& endpoint,
           const std::string &content,
           const boost::property_tree::ptree &additional_info,
           const std::string& status, const std::string& text);

  void remove(int code, const std::string& endpoint, const std::string& status,
              const std::string& text);

  std::string message(int code, const std::string& endpoint,
                      const std::string& status, const std::string& text);

  std::string message(int code, const std::string& endpoint,
                      const std::string &content,
                      const boost::property_tree::ptree &servicept,
                      const std::string& status,
                      const std::string& text);

  int size();

  void reset();

  /** return the message of the first alarm */
  const std::string& get_last();

 protected:
  std::string get_key(int code, const std::string& endpoint,
                      const std::string& status);

 private:
  static Alarm* pinstance;
  pion::logger m_log;

  boost::recursive_mutex m_mutex;

  Alarm();

  std::map<std::string, std::string> _alarms;
};
};

#endif
