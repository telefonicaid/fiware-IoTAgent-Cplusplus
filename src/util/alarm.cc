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
#include "alarm.h"
#include <boost/lexical_cast.hpp>

iota::Alarm* iota::Alarm::pinstance = 0;  // Inicializar el puntero

iota::Alarm::Alarm()
    : m_log(PION_GET_LOGGER(iota::Process::get_logger_name())) {}

iota::Alarm* iota::Alarm::instance() {
  if (pinstance == 0) {
    pinstance = new Alarm();
  }
  return pinstance;
}

void iota::Alarm::error(int code, const std::string& endpoint,
                        const std::string& status, const std::string& text) {
  instance()->put(code, endpoint, status, text);
}

void iota::Alarm::error(int code, const std::string& endpoint,
                    const std::string &content,
                    const boost::property_tree::ptree &additional_info,
                    const std::string& status, const std::string& text) {
  instance()->put(code, endpoint, content, additional_info, status, text);
}

void iota::Alarm::info(int code, const std::string& endpoint,
                       const std::string& status, const std::string& text) {
  instance()->remove(code, endpoint, status, text);
}

std::string iota::Alarm::message(int code, const std::string& endpoint,
                                 const std::string& status,
                                 const std::string& text) {
  std::string message;
  message.append(" code=");
  message.append(boost::lexical_cast<std::string>(code));
  message.append(" to=");
  message.append(endpoint);
  message.append(" info=");
  message.append(text);

  return message;
}


std::string iota::Alarm::message(int code, const std::string& endpoint,
                                 const std::string &content,
                                 const boost::property_tree::ptree &servicept,
                                 const std::string& status,
                                 const std::string& text) {


  std::string message;
  message.append(" code=");
  message.append(boost::lexical_cast<std::string>(code));
  message.append(" to=");
  message.append(endpoint);
  message.append(" service=");
  message.append(servicept.get<std::string>("service", ""));
  message.append(" service_path=");
  message.append(servicept.get<std::string>("service_path", "/"));
  message.append(" content=");
  message.append(content);
  message.append(" info=");
  message.append(text);

  return message;
}


void iota::Alarm::put(int code, const std::string& endpoint,
                      const std::string& status, const std::string& text) {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
  std::string errorSTR = " event=ALARM";
  errorSTR.append(message(code, endpoint, status, text));

  IOTA_LOG_ERROR(m_log, errorSTR);
  _alarms.insert(std::pair<std::string, std::string>(
      get_key(code, endpoint, status), errorSTR));
}

void iota::Alarm::put(int code, const std::string& endpoint,
                      const std::string &content,
                      const boost::property_tree::ptree &additional_info,
                      const std::string& status, const std::string& text) {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
  std::string errorSTR = " event=ALARM";
  errorSTR.append(message(code, endpoint, content, additional_info, status, text));

  IOTA_LOG_ERROR(m_log, errorSTR);
  _alarms.insert(std::pair<std::string, std::string>(
      get_key(code, endpoint, status), errorSTR));
}

void iota::Alarm::remove(int code, const std::string& endpoint,
                         const std::string& status, const std::string& text) {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
  std::string errorSTR = " event=END-ALARM";
  errorSTR.append(message(code, endpoint, status, text));

  int num = _alarms.erase(get_key(code, endpoint, status));
  if (num > 0) {
    IOTA_LOG_ERROR(m_log, errorSTR);
  } else {
    IOTA_LOG_DEBUG(m_log, errorSTR);
  }
}

void iota::Alarm::reset() {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
  _alarms.clear();
}

const std::string& iota::Alarm::get_last() { return _alarms.begin()->second; }

std::string iota::Alarm::get_key(int code, const std::string& endpoint,
                                 const std::string& status) {
  std::string key;

  key.append(boost::lexical_cast<std::string>(code));
  key.append(endpoint);

  return key;
}

int iota::Alarm::size() { return _alarms.size(); }
