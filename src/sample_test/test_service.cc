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
#include "test_service.h"
#include "services/admin_service.h"
#include "rest/riot_conf.h"
#include "rest/iot_cb_comm.h"
#include "util/iot_url.h"
#include "util/KVP.h"
#include "util/FuncUtil.h"
#include "ngsi/UpdateContext.h"
#include "ngsi/ContextElement.h"

iota::TestService::TestService()
    : m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  IOTA_LOG_DEBUG(m_logger, "iota::TestService::TestService");
}

iota::TestService::~TestService() {
  IOTA_LOG_DEBUG(m_logger, "Destructor iota::TestService::TestService");
}

iota::ProtocolData iota::TestService::get_protocol_data() {
  iota::ProtocolData protocol_data;
  protocol_data.description = "test Protocol";
  protocol_data.protocol = "PDI-IoTA-test";
  return protocol_data;
}

std::string iota::TestService::get_protocol_commands() {
  return "{\"commands\":[{\"name\": \"tcs_1\", \"type\": \"command\", \"value\": \"\"},{\"name\":\"PING\",\"type\":\"command\",\"value\" : \"44\"}]}";
}


void iota::TestService::start() {
  IOTA_LOG_DEBUG(m_logger, "START SAMPLE PLUGIN");

  std::map<std::string, std::string> filters;
  add_url("", filters, REST_HANDLE(&iota::TestService::service), this);

  // This function should be called only if plugin needs receive NGSI messages
  // Get ngsi service.
  // enable_ngsi_service(filters, REST_HANDLE(&iota::TestService::op_ngsi),
  // this);
}

void iota::TestService::op_ngsi(
    pion::http::request_ptr& http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response, std::string& response) {
  IOTA_LOG_DEBUG(m_logger, "op_ngsi TestService");

  http_response.set_status_code(pion::http::types::RESPONSE_CODE_OK);
  response = "It should be a correct NGSI response";
}

void iota::TestService::service(
    pion::http::request_ptr& http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response, std::string& response) {
  IOTA_LOG_DEBUG(m_logger, "TestService service");

  boost::system::error_code error_code;
  int code_resp = pion::http::types::RESPONSE_CODE_OK;

  bool url_update = false;

  // Getting HTTP request information using PION methods
  std::string method = http_request_ptr->get_method();
  std::string resource_o = http_request_ptr->get_original_resource();
  std::string ip = http_request_ptr->get_remote_ip().to_string();
  std::string resource = http_request_ptr->get_resource();
  IOTA_LOG_DEBUG(m_logger, method << " " << resource_o);

  std::string content;
  std::string s_query = http_request_ptr->get_query_string();
  std::vector<iota::KVP> query;
  bool hay_p = riot_getQueryParams(s_query, query);
  std::string device;
  std::string apikey;

  // Getting query parameters
  try {
    int i = 0;
    for (i = 0; i < query.size(); i++) {
      IOTA_LOG_DEBUG(m_logger, "QUERY " << query[i].getKey());
      if (query[i].getKey().compare("i") == 0) {
        device = query[i].getValue();
      } else if (query[i].getKey().compare("k") == 0) {
        apikey.append(query[i].getValue());
      }
    }

  } catch (...) {
    IOTA_LOG_ERROR(m_logger, "translate error ");
    code_resp = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
  }

  // Getting payload
  std::string content_type;
  int cl = 0;
  if ((http_request_ptr->get_content_length() != 0) &&
      (http_request_ptr->get_content() != NULL)) {
    cl = http_request_ptr->get_content_length();
    content = http_request_ptr->get_content();
    IOTA_LOG_DEBUG(m_logger, "Message from " << ip << " to " << resource
                                             << " CONTENT " << content);
    IOTA_LOG_DEBUG(m_logger, "CONTENT " << content);
  }

  // Generating a NGSI updateContext and sending it to contextBroker
  std::string cb_response;

  send_update_context(device, apikey, content, cb_response);

  http_response.set_status_code(code_resp);
  response = "It should be a response in device protocol";
}

int iota::TestService::send_update_context(const std::string& dev,
                                           const std::string& apikey,
                                           const std::string& content,
                                           std::string& cb_response) {
  boost::property_tree::ptree pt_cb;
  std::string cb_url;
  std::string entity_name;
  std::string entity_type("thing");

  // Getting service information associated to received apikey
  try {
    get_service_by_apiKey(pt_cb, apikey);
    cb_url.assign(pt_cb.get<std::string>("cbroker", ""));
    entity_type.assign(pt_cb.get<std::string>("entity_type", ""));

  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, "Configuration error " << e.what());
  }

  // Generating an updateContext
  std::string att_name = "sample_att_name";
  iota::Attribute att(att_name, "raw_text", content);

  entity_name = dev;
  iota::ContextElement ngsi_context_element(entity_name, entity_type, "false");
  ngsi_context_element.add_attribute(att);

  std::string updateAction("APPEND");
  iota::UpdateContext uc(updateAction);
  uc.add_context_element(ngsi_context_element);

  iota::ContextBrokerCommunicator cb_communicator;
  cb_url.append(cb_communicator.get_ngsi_operation("updateContext"));

  // Sending updateContext to contextBroker
  IOTA_LOG_DEBUG(m_logger, "Sending to contextBroker URL : " << cb_url);
  IOTA_LOG_DEBUG(m_logger, "NGSI updateContext json : " << uc.get_string());

  cb_response = cb_communicator.send(cb_url, uc.get_string(), pt_cb);

  IOTA_LOG_DEBUG(m_logger, "contextBroker response : " << cb_response);
}

// creates new TestService objects
//

extern "C" PION_PLUGIN iota::TestService* pion_create_TestService(void) {
  return new iota::TestService();
}

/// destroys TestService objects

extern "C" PION_PLUGIN void pion_destroy_TestService(
    iota::TestService* service_ptr) {
  delete service_ptr;
}
