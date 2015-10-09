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
#ifndef SRC_UTIL_TTCBPUBLISHER_H_
#define SRC_UTIL_TTCBPUBLISHER_H_
#include <string>
#include <vector>
#include "ngsi/UpdateContext.h"
#include "rest/iot_cb_comm.h"
#include <pion/http/response_writer.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "util/RiotISO8601.h"
#define OP_STRING "APPEND"

namespace iota {

namespace tt {
class TTCBPublisher {
 public:
  TTCBPublisher();
  std::string publishContextBroker(iota::ContextElement& cElement,
                                   std::vector<std::string>& vJsons,
                                   boost::property_tree::ptree&,
                                   iota::RiotISO8601& time);

  virtual ~TTCBPublisher();

 protected:
 private:
  pion::logger m_logger;
  virtual std::string doPublishContextBroker(iota::ContextElement& cElement,
                                             std::vector<std::string>& vJsons,
                                             std::string& url,
                                             boost::property_tree::ptree&,
                                             iota::RiotISO8601& time);
  virtual bool checkTimeInstantPresent(
      std::vector<std::string>&);  // For future use, in case a device already
                                   // sends its timestamp.
  virtual std::string buildcbURL(const boost::property_tree::ptree& pt);
};
}
}
#endif  // TTCBPUBLISHER_H
