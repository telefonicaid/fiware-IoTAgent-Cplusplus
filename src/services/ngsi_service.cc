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

#include "services/ngsi_service.h"
#include "rest/oauth_filter.h"
#include "rest/media_filter.h"
#include <sstream>
#include <pion/http/response_writer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

iota::NgsiService::NgsiService()
    : m_log(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  IOTA_LOG_DEBUG(m_log, "iota::NgsiService::NgsiService");
}

iota::NgsiService::~NgsiService() {}

void iota::NgsiService::start() {
  IOTA_LOG_DEBUG(m_log, "Start NgsiService");
  std::map<std::string, std::string> filters;
  // add_url("/<operation>", filters,
  // REST_HANDLE(&iota::NgsiService::ngsi_operation), this);

  // OAuth Nothbound
  try {
    std::map<std::string, std::string> oauth_map;
    iota::Configurator::instance()->get(iota::types::CONF_FILE_OAUTH,
                                        oauth_map);

    if (oauth_map.size() > 0 &&
        !oauth_map[iota::types::CONF_FILE_OAUTH_VALIDATE_TOKEN_URL].empty() &&
        !oauth_map[iota::types::CONF_FILE_OAUTH_ROLES_URL].empty() &&
        !oauth_map[iota::types::CONF_FILE_OAUTH_PROJECTS_URL].empty() &&
        !oauth_map[iota::types::CONF_FILE_ACCESS_CONTROL].empty()) {
      boost::shared_ptr<iota::OAuthFilter> auth_ptr(
          new iota::OAuthFilter(iota::Process::get_process().get_io_service()));
      auth_ptr->set_filter_url_base(iota::Process::get_url_base());
      auth_ptr->set_configuration(oauth_map);
      auth_ptr->set_pep_rules(iota::Configurator::instance()->get_pep_rules());
      add_pre_filter(auth_ptr);
    }
  } catch (std::exception& e) {
    IOTA_LOG_INFO(m_log, "OAuth for northbound is not configured");
  }

  boost::shared_ptr<iota::MediaFilter> media_ptr(
      new iota::MediaFilter(iota::Process::get_process().get_io_service()));
  add_pre_filter(media_ptr);
}

void iota::NgsiService::ngsi_operation(
    pion::http::request_ptr& http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response, std::string& response) {
  IOTA_LOG_INFO(m_log, "iota::NgsiService::ngsi_operation "
                           << url_args["operation"]);
  http_response.set_status_code(pion::http::types::RESPONSE_CODE_NOT_FOUND);
  http_response.set_status_message(
      iota::Configurator::instance()->getHttpMessage(
          pion::http::types::RESPONSE_CODE_NOT_FOUND));
}
