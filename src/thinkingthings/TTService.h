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
#ifndef SRC_THINKINGTHINGS_TTSERVICE_H_
#define SRC_THINKINGTHINGS_TTSERVICE_H_

#include <pion/http/plugin_service.hpp>
#include <rest/rest_handle.h>
#include <boost/property_tree/ptree.hpp>
#include <ESPLib.h>

#include "util/TTCBPublisher.h"
#include "util/RiotISO8601.h"

#include "../esp/plugins/postprocessor_tt/ESP_Plugin_Postprocessor_TT.h"

namespace iota {

namespace esp {

class TTService : public iota::RestHandle {
 public:
  int idsensor;
  static TTService* instance;
  static ESPLib* getESPLib();

  TTService();
  TTService(boost::shared_ptr<iota::tt::TTCBPublisher> ttPublisher_ptr);

  virtual ~TTService();
  void start();
  void service(pion::http::request_ptr& http_request_ptr,
               std::map<std::string, std::string>& url_args,
               std::multimap<std::string, std::string>& query_parameters,
               pion::http::response& http_response, std::string& response);
  virtual void set_option(const std::string& name, const std::string& value);

  int initESPLib(std::string& pathToLog, std::string& sensorFile);
  void add_info(boost::property_tree::ptree& pt, const std::string& iotService,
                const std::string& apiKey);  // Can be taken to another class

 protected:
  pion::logger m_logger;

 private:
  boost::shared_ptr<iota::tt::TTCBPublisher> contextBrokerPub;

  static ESPLib* esplib_instance;
  int idSensor;
  std::string resource;
  boost::property_tree::ptree _service_configuration;

  ESP_Postprocessor_TT* tt_post_ptr_;
};
}
}
#endif  // TTSERVICE_H
