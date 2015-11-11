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
#include "test_command_service.h"
#include "services/admin_service.h"
#include "rest/riot_conf.h"
#include "rest/iot_cb_comm.h"
#include "util/iot_url.h"
#include "util/KVP.h"
#include "util/FuncUtil.h"
#include "ngsi/UpdateContext.h"
#include "ngsi/ContextElement.h"

iota::TestCommandService::TestCommandService()
    : m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  IOTA_LOG_DEBUG(m_logger, "iota::TestCommandService::TestCommandService");
}

iota::TestCommandService::~TestCommandService() {
  IOTA_LOG_DEBUG(m_logger,
                 "Destructor iota::TestCommandService::TestCommandService");
}

iota::ProtocolData iota::TestCommandService::get_protocol_data() {
  iota::ProtocolData protocol_data;
  protocol_data.description = "test command Protocol";
  protocol_data.protocol = "PDI-IoTA-test-command";
  return protocol_data;
}

std::string iota::TestCommandService::get_protocol_commands() {
  return "{ \"myfield\":\"tcommand\", \"commands\": [{\"name\": \"tcs_1\", \"type\": \"command\", \"value\": \"\"} ] }";
}

int iota::TestCommandService::get_timeout_commands() {
  return 13;
}


void iota::TestCommandService::start() {
  IOTA_LOG_DEBUG(m_logger, "START SAMPLE PLUGIN");

  std::map<std::string, std::string> filters;
  add_url("", filters, REST_HANDLE(&iota::TestCommandService::service), this);

  // This function should be called only if plugin needs receive NGSI messages
  // Get ngsi service.
  enable_ngsi_service(filters,
                      REST_HANDLE(&iota::CommandHandle::default_op_ngsi), this);
}

void iota::TestCommandService::service(
    pion::http::request_ptr& http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response, std::string& response) {
  IOTA_LOG_DEBUG(m_logger, "TestCommandService service");

  std::string method = http_request_ptr->get_method();

  if (method.compare("GET") == 0) {
    return get_commands_polling(http_request_ptr, url_args, query_parameters,
                                http_response, response);
  } else if (method.compare("PUT") == 0) {
    return receive_command_results(http_request_ptr, url_args, query_parameters,
                                   http_response, response);
  } else {
    return receive_observations(http_request_ptr, url_args, query_parameters,
                                http_response, response);
  }
}

int iota::TestCommandService::execute_command(
    const std::string& endpoint, const std::string& command_id,
    const boost::property_tree::ptree& command_to_send, int timeout,
    const boost::shared_ptr<iota::Device>& item_dev,
    const boost::property_tree::ptree& service, std::string& response,
    iota::HttpClient::application_callback_t callback) {
  int codigo_respuesta = pion::http::types::RESPONSE_CODE_OK;

  IOTA_LOG_DEBUG(m_logger, "execute_command  endpoint:" << endpoint);
  std::string body = command_to_send.get(iota::store::types::BODY, "");
  response.assign(body);
  response.append("|command_response");

  return codigo_respuesta;
}

void iota::TestCommandService::receive_observations(
    pion::http::request_ptr& http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response, std::string& response) {
  IOTA_LOG_DEBUG(m_logger, "TestCommandService receive_observations");

  boost::system::error_code error_code;
  int code_resp = pion::http::types::RESPONSE_CODE_OK;
  std::string content;
  std::string apikey;
  std::string device;

  std::string s_query = http_request_ptr->get_query_string();
  std::vector<iota::KVP> query;
  bool hay_p = riot_getQueryParams(s_query, query);
  std::string ip = http_request_ptr->get_remote_ip().to_string();
  std::string resource = http_request_ptr->get_resource();
  std::string method = http_request_ptr->get_method();

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

  // Getting query parameters
  IOTA_LOG_DEBUG(m_logger, "device sends observation");

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

  iota::Attribute att("attr_name", "attr_type", content);
  std::vector<iota::Attribute> atts;
  atts.push_back(att);
  send_update_context(apikey, device, atts);

  http_response.set_status_code(code_resp);
  response = "It should be a response in device protocol";
}

void iota::TestCommandService::receive_command_results(
    pion::http::request_ptr& http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response, std::string& response) {
  IOTA_LOG_DEBUG(m_logger, "receive_command_results service");

  boost::system::error_code error_code;
  int code_resp = pion::http::types::RESPONSE_CODE_OK;
  std::string content;
  std::string apikey;
  std::string device;

  std::string s_query = http_request_ptr->get_query_string();
  std::vector<iota::KVP> query;
  bool hay_p = riot_getQueryParams(s_query, query);
  std::string ip = http_request_ptr->get_remote_ip().to_string();
  std::string resource = http_request_ptr->get_resource();
  std::string method = http_request_ptr->get_method();

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

  std::string command;
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

  IOTA_LOG_DEBUG(m_logger, "device sends command result device:"
                               << device << " " << content);
  boost::property_tree::ptree service_ptree;
  boost::shared_ptr<iota::Device> dev;

  get_service_by_apiKey(service_ptree, apikey);
  std::string id_command;
  std::size_t found = content.find("|");
  if (found != std::string::npos) {
    id_command = content.substr(0, found);
  } else {
    id_command = content;
  }
  IOTA_LOG_DEBUG(m_logger, "id_command:" << id_command);
  std::string srv =
      service_ptree.get<std::string>(iota::store::types::SERVICE, "");
  std::string srv_path =
      service_ptree.get<std::string>(iota::store::types::SERVICE_PATH, "");

  dev = get_device(device, srv, srv_path);

  iota::CommandPtr commandPtr = get_command(id_command, srv, srv_path);
  if (commandPtr.get() == NULL) {
    IOTA_LOG_ERROR(m_logger,
                   "already responsed, command not in cache id_command:"
                       << id_command << " service:" << srv << " " << srv_path);
  } else {
    IOTA_LOG_DEBUG(m_logger, "command in cache id_command:"
                                 << id_command << " service:" << srv << " "
                                 << srv_path);
    command = commandPtr->get_name();
    commandPtr->cancel();
    // It is very important to remove command
    remove_command(id_command, srv, srv_path);

    response_command(command, content, dev, service_ptree);
  }
}

void iota::TestCommandService::get_commands_polling(
    pion::http::request_ptr& http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response, std::string& response) {
  IOTA_LOG_DEBUG(m_logger, "TestCommandService service");

  boost::system::error_code error_code;
  int code_resp = pion::http::types::RESPONSE_CODE_OK;
  std::string content;
  std::string apikey;
  std::string device;

  std::string s_query = http_request_ptr->get_query_string();
  std::vector<iota::KVP> query;
  bool hay_p = riot_getQueryParams(s_query, query);
  std::string ip = http_request_ptr->get_remote_ip().to_string();
  std::string resource = http_request_ptr->get_resource();
  std::string method = http_request_ptr->get_method();
  std::string body;

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

  iota::CommandVect cmdPtes;
  IOTA_LOG_DEBUG(m_logger,
                 "device asks for commands, returns all commands from a "
                 "device, separated with #");
  cmdPtes = get_all_command(device, apikey);
  IOTA_LOG_DEBUG(m_logger, "N. commands " << cmdPtes.size());
  std::string commandsSTR;
  iota::CommandVect::const_iterator i;
  iota::CommandPtr ptr;
  for (iota::CommandVect::iterator it_v = cmdPtes.begin();
       it_v != cmdPtes.end(); ++it_v) {
    ptr = *it_v;
    body = ptr->get_command().get(iota::store::types::BODY, "");
    commandsSTR.append(ptr->get_id());
    commandsSTR.append("|@|");
    commandsSTR.append(body);
    if (it_v != cmdPtes.end() - 1) {
      commandsSTR.append("#");
    }
  }

  IOTA_LOG_DEBUG(m_logger, "commands " << commandsSTR);
  http_response.set_status_code(code_resp);
  http_response.set_status_message(commandsSTR);
  response.assign(commandsSTR);
}

// creates new TestService objects
//

extern "C" PION_PLUGIN iota::TestCommandService* pion_create_TestCommandService(
    void) {
  return new iota::TestCommandService();
}

/// destroys TestService objects

extern "C" PION_PLUGIN void pion_destroy_TestCommandService(
    iota::TestCommandService* service_ptr) {
  delete service_ptr;
}
