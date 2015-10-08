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
#include "TTCBPublisher.h"
#include "util/iota_logger.h"

iota::tt::TTCBPublisher::TTCBPublisher()
    : m_logger((PION_GET_LOGGER(iota::Process::get_logger_name()))) {
  // ctor
}

iota::tt::TTCBPublisher::~TTCBPublisher() {
  // dtor
}

std::string iota::tt::TTCBPublisher::publishContextBroker(
    ::iota::ContextElement& cElement, std::vector<std::string>& vJsons,
    boost::property_tree::ptree& pt, iota::RiotISO8601& time) {
  std::string url = buildcbURL(pt);
  return doPublishContextBroker(cElement, vJsons, url, pt, time);
}

std::string iota::tt::TTCBPublisher::doPublishContextBroker(
    ::iota::ContextElement& cElement, std::vector<std::string>& vJsons,
    std::string& cb_url, boost::property_tree::ptree& pt_cb,
    iota::RiotISO8601& time) {
  std::string strTimestamp = time.toUTC().toString();
  std::string cb_response;

  ::iota::Attribute metadataTS("TimeInstant", "ISO8601", strTimestamp);
  ::iota::Attribute attributeTS("TimeInstant", "ISO8601", strTimestamp);

  IOTA_LOG_INFO(
      m_logger,
      "doPublishContext Broker: Adding attributes to ContextElement: ");
  int i;
  for (i = 0; i < vJsons.size(); i++) {
    std::istringstream ss_json(vJsons[i]);
    ::iota::Attribute jsonAttribute(ss_json);
    jsonAttribute.add_metadata(metadataTS);  // timeInstant added as metadata.
    cElement.add_attribute(jsonAttribute);
  }
  IOTA_LOG_INFO(
      m_logger,
      "doPublishContext Broker: Adding attributes to ContextElement: ["
          << i << "] added");

  if (!checkTimeInstantPresent(vJsons)) {
    cElement.add_attribute(attributeTS);  // Adding Timestamp as attribute
    IOTA_LOG_INFO(
        m_logger,
        "doPublishContext Broker: TimeInstant added to ContextElement.");
  }

  std::string updateAction(OP_STRING);

  ::iota::UpdateContext op(updateAction);

  op.add_context_element(cElement);

  ContextBrokerCommunicator cb_communicator;
  cb_response.append(cb_communicator.send(cb_url, op.get_string(), pt_cb));

  IOTA_LOG_DEBUG(m_logger, updateAction << ":cb:" << cb_url
                                        << ":op:" << op.get_string());
  IOTA_LOG_DEBUG(m_logger, "RESPONSE: " << cb_response);
  return cb_response;
}

bool iota::tt::TTCBPublisher::checkTimeInstantPresent(
    std::vector<std::string>&) {
  return false;
}

std::string iota::tt::TTCBPublisher::buildcbURL(
    const boost::property_tree::ptree& pt) {
  std::string cb_url;

  cb_url.assign(pt.get<std::string>("cbroker", ""));

  if (!cb_url.empty()) {
    std::string updateContex = pt.get<std::string>("updateContex", "");
    if (!updateContex.empty()) {
      cb_url.append(updateContex);
    } else {
      // Not sure if this will work the way I thought, like getting
      // updateContext from the service in first place.
      const JsonValue& ngsi_urls =
          iota::Configurator::instance()->get("ngsi_url");
      if (ngsi_urls.HasMember("updateContext")) {
        cb_url.append(ngsi_urls["updateContext"].GetString());
      } else {
        std::ostringstream what;
        what << "Not Found ";
        what << "[updateContext]";
        throw std::runtime_error(what.str());
      }
    }
  }

  return cb_url;
}
