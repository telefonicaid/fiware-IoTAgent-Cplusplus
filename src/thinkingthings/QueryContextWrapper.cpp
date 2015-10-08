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
#include "QueryContextWrapper.h"
#include <rest/riot_conf.h>

iota::esp::tt::QueryContextWrapper::QueryContextWrapper(
    boost::property_tree::ptree* propTree)
    : m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  this->pt_cb = propTree;

  std::string cbroker = propTree->get<std::string>("cbroker", "");

  if (!cbroker.empty()) {
    cb_url.assign(cbroker);
    std::string queryContext = propTree->get<std::string>("queryContext", "");
    if (!queryContext.empty()) {
      cb_url.append(queryContext);
    } else {
      const iota::JsonValue& ngsi_urls =
          iota::Configurator::instance()->get("ngsi_url");
      if (ngsi_urls.HasMember("queryContext")) {
        cb_url.append(ngsi_urls["queryContext"].GetString());
      } else {
        std::ostringstream what;
        what << "Not Found ";
        what << "[updateContext]";
        throw std::runtime_error(what.str());
      }
    }
  }
  IOTA_LOG_DEBUG(m_logger, "QueryContextWrapper: CB URL formed: [" << cb_url
                                                                   << "]");
}

iota::esp::tt::QueryContextWrapper::~QueryContextWrapper() {
  // dtor
}

::iota::ContextResponses iota::esp::tt::QueryContextWrapper::queryContext(
    ::iota::QueryContext& qc) {
  return doQueryContext(qc);
}

::iota::ContextResponses iota::esp::tt::QueryContextWrapper::doQueryContext(
    ::iota::QueryContext& qc) {
  IOTA_LOG_DEBUG(m_logger, "doQueryContext: SENDING to Context Broker... ["
                               << qc.get_string() << "]");
  cb_response.assign(cb_communicator.send(cb_url, qc.get_string(), *pt_cb));

  std::istringstream iss(cb_response);

  IOTA_LOG_DEBUG(m_logger,
                 "doQueryContext: SENDING to Context Broker...  DONE");
  IOTA_LOG_DEBUG(m_logger, "doQueryContext: Formatting response: [ "
                               << cb_response << " ]");

  ::iota::ContextResponses responses(iss);
  IOTA_LOG_DEBUG(m_logger, "doQueryContext: Formatting response: ...  DONE");
  return responses;
}
