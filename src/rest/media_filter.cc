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
#include "media_filter.h"
#include "rest/types.h"
#include "rest/rest_handle.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>
#include "rest/iot_cb_comm.h"
#include "util/FuncUtil.h"
#include "util/iota_exception.h"

iota::MediaFilter::MediaFilter(boost::asio::io_service& io_service)
    : HTTPFilter(io_service,
                 PION_GET_LOGGER(iota::Process::get_logger_name())){};

iota::MediaFilter::~MediaFilter(){};

bool iota::MediaFilter::handle_request(
    pion::http::request_ptr& http_request_ptr,
    pion::tcp::connection_ptr& tcp_conn) {
  bool allowed = true;
  std::string resource = http_request_ptr->get_resource();
  IOTA_LOG_DEBUG(m_logger, "MediaFilter handle_request "
                               << http_request_ptr->get_header(
                                      iota::types::HEADER_TRACE_MESSAGES)
                               << resource);

  unsigned int resu = iota::types::RESPONSE_CODE_OK;

  try {
    // Checking headers Fiware-Service and Fiware-ServicePath
    std::string service(
        http_request_ptr->get_header(iota::types::FIWARE_SERVICE));
    std::string service_path(
        http_request_ptr->get_header(iota::types::FIWARE_SERVICEPATH));

    // Exception for about and protocols
    if (http_request_ptr->get_resource().compare(
            iota::Process::get_url_base() + iota::ADMIN_SERVICE_ABOUT) != 0 &&
        http_request_ptr->get_resource().compare(
            iota::Process::get_url_base() + iota::ADMIN_SERVICE_PROTOCOLS) !=
            0) {
      iota::check_fiware_service_name(service);
      iota::check_fiware_service_path_name(service_path);
    }

    // Checking for forbidden characters
    std::string content(http_request_ptr->get_content());
    bool forbidden =
        iota::check_forbidden_characters(iota::types::IOTA_FORBIDDEN_CHARACTERS,
                                         service) ||
        iota::check_forbidden_characters(iota::types::IOTA_FORBIDDEN_CHARACTERS,
                                         service_path) ||
        iota::check_forbidden_characters(iota::types::IOTA_FORBIDDEN_CHARACTERS,
                                         resource);

    if (forbidden == true) {
      resu = iota::types::RESPONSE_CODE_FORBIDDEN_CHARACTERS;
    }
    // Accept header
    if (!forbidden &&
        http_request_ptr->has_header(iota::types::IOT_HTTP_HEADER_ACCEPT)) {
      std::string rec_accept =
          http_request_ptr->get_header(iota::types::IOT_HTTP_HEADER_ACCEPT);

      IOTA_LOG_DEBUG(m_logger, "Accept: " << rec_accept);
      if (rec_accept.find(iota::types::IOT_CONTENT_SUBTYPE_JSON) ==
              std::string::npos &&
          rec_accept.find("/*") == std::string::npos) {
        IOTA_LOG_ERROR(m_logger,
                       "Accept different from application/json, text/json");
        resu = iota::types::RESPONSE_CODE_NOT_ACCEPTABLE;
      }
    }

    // Content-Type

    if (resu == iota::types::RESPONSE_CODE_OK && !content.empty() &&
        http_request_ptr->has_header(pion::http::types::HEADER_CONTENT_TYPE)) {
      std::string rec_content_type =
          http_request_ptr->get_header(pion::http::types::HEADER_CONTENT_TYPE);

      IOTA_LOG_DEBUG(m_logger, "Content-Type: " << rec_content_type);
      if (rec_content_type.find(iota::types::IOT_CONTENT_SUBTYPE_JSON) ==
          std::string::npos) {
        IOTA_LOG_ERROR(
            m_logger,
            "Content-Type different of application/json or text/json");
        resu = iota::types::RESPONSE_CODE_UNSUPPORTED_MEDIA_TYPE;
      }
    }

  } catch (iota::IotaException& e) {
    resu = e.status();
  }
  if (resu != iota::types::RESPONSE_CODE_OK) {
    // This function add event with post
    handle_no_allowed(http_request_ptr, tcp_conn, resu);
  } else {
    tcp_conn->get_io_service().post(boost::bind(
        &iota::MediaFilter::call_to_callback, shared_from_this(),
        http_request_ptr->get_header(iota::types::HEADER_TRACE_MESSAGES),
        resu));
  }
  return allowed;
};
