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
#include "iot_cb_comm.h"
#include "util/iot_url.h"
#include "util/oauth_comm.h"
#include <boost/make_shared.hpp>
#include "util/RiotISO8601.h"
#include "util/alarm.h"
#include "rest/riot_conf.h"
#include "rest/types.h"

const std::string iota::ContextBrokerCommunicator::NUMBER_OF_TRIES =
    "number_of_tries";
iota::ContextBrokerCommunicator::ContextBrokerCommunicator()
    : _io_service(iota::Process::get_process().get_io_service()),
      _method(pion::http::types::REQUEST_METHOD_POST),
      m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {}

iota::ContextBrokerCommunicator::ContextBrokerCommunicator(
    boost::asio::io_service& io_service)
    : _io_service(io_service),
      _method(pion::http::types::REQUEST_METHOD_POST),
      m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {}

iota::ContextBrokerCommunicator::~ContextBrokerCommunicator(){};

void iota::ContextBrokerCommunicator::method(std::string http_method) {
  _method = http_method;
}

void iota::ContextBrokerCommunicator::receive_event(
    std::string url, std::string content,
    boost::property_tree::ptree additional_info,
    boost::shared_ptr<iota::HttpClient> connection,
    pion::http::response_ptr response_ptr,
    const boost::system::error_code& error) {
  int number_of_tries = additional_info.get<int>(
      iota::ContextBrokerCommunicator::NUMBER_OF_TRIES, 0);
  IOTA_LOG_DEBUG(m_logger, "url=" << url << " error=" << error
                                  << " tries=" << number_of_tries);
  pion::http::response_ptr response = connection->get_response();
  if ((!error) && (response.get() != NULL)) {
    std::string cb_response = response->get_content();
    IOTA_LOG_DEBUG(m_logger, "res_code=" << response->get_status_code()
                                         << " text=" << cb_response);
    if (response->get_status_code() ==
            pion::http::types::RESPONSE_CODE_UNAUTHORIZED &&
        number_of_tries < 1) {
      additional_info.put(iota::ContextBrokerCommunicator::NUMBER_OF_TRIES,
                          ++number_of_tries);
      async_send(url, content, additional_info, _callback,
                 pion::http::types::RESPONSE_CODE_UNAUTHORIZED);
    } else if (response->get_status_code() ==
               iota::types::RESPONSE_CODE_BAD_GATEWAY) {
      iota::Alarm::error(iota::types::ALARM_CODE_NO_CB, url, content,
                         additional_info, iota::types::ERROR,
                         iota::types::RESPONSE_MESSAGE_BAD_GATEWAY);

    } else {
      iota::Alarm::info(iota::types::ALARM_CODE_NO_CB, url, iota::types::ERROR,
                        "send ok");
      if (_callback) {
        _callback(cb_response, response->get_status_code());
      }
    }
  } else {
    std::stringstream ss;
    ss << "Communication error";
    ss << " -> ";
    ss << connection->getRemoteEndpoint();
    ss << ": ";
    ss << error.message();
    IOTA_LOG_ERROR(m_logger, ss.str());
    if (_callback) {
      _callback("", (-1) * error.value());
    }

    iota::Alarm::error(iota::types::ALARM_CODE_NO_CB, url, content,
                       additional_info, iota::types::ERROR, error.message());
  }

  remove_connection(connection);
}

bool iota::ContextBrokerCommunicator::async_send(
    std::string url, std::string content,
    boost::property_tree::ptree additional_info, app_callback_t callback,
    int status_code) {
  _callback = callback;
  bool result = true;
  try {
    iota::IoTUrl dest(url);
    std::string resource = dest.getPath();
    std::string query = dest.getQuery();
    std::string server = dest.getHost();
    std::string compound_server(server);

    compound_server.append(":");
    compound_server.append(boost::lexical_cast<std::string>(dest.getPort()));
    // IoTAgent trust token
    std::string token = additional_info.get<std::string>("token", "");
    std::string oauth;
    std::string iotagent_user;
    std::string iotagent_pass;

    iota::Configurator* configurator = iota::Configurator::instance();
    if (configurator != NULL) {
      try {
        std::map<std::string, std::string> to_map;

        configurator->get("oauth", to_map);
        oauth = to_map["on_behalf_trust_url"];
        iotagent_user = to_map["on_behalf_user"];
        iotagent_pass = to_map["on_behalf_password"];
      } catch (std::runtime_error& e) {
        IOTA_LOG_DEBUG(m_logger, "oauth not found :" << e.what());
      }
    }

    if (!token.empty() && !oauth.empty()) {
      iota::OAuth oauth_comm(_io_service);
      oauth_comm.set_sync_service();
      oauth_comm.set_oauth_trust(oauth);
      // Setting trust_token  before identity
      oauth_comm.set_trust_token(token);
      oauth_comm.set_identity(OAUTH_ON_BEHALF_TRUST, iotagent_user,
                              iotagent_pass);
      std::string auth_token = oauth_comm.get_token(status_code);
      additional_info.put<std::string>("token", auth_token);
    }

    pion::http::request_ptr request = create_request(
        compound_server, resource, content, query, additional_info);

    boost::shared_ptr<iota::HttpClient> http_client(
        new iota::HttpClient(_io_service, server, dest.getPort()));

    // add_connection(http_client);

    int timeout = boost::lexical_cast<int>(
        additional_info.get<std::string>("timeout", "5"));
    std::string proxy = additional_info.get<std::string>("proxy", "");
    http_client->async_send(
        request, timeout, proxy,
        boost::bind(&iota::ContextBrokerCommunicator::receive_event,
                    shared_from_this(), url, content, additional_info, _1, _2,
                    _3));

  } catch (std::exception& e) {

    iota::Alarm::error(iota::types::ALARM_CODE_NO_CB, url, content,
                       additional_info, iota::types::ERROR, e.what());
  }
}

std::string iota::ContextBrokerCommunicator::send(
    std::string url, std::string content,
    boost::property_tree::ptree additional_info, int status_code) {
  std::string cb_response;
  boost::shared_ptr<iota::HttpClient> http_client;
  pion::http::response_ptr response;
  // IoTAgent trust token
  std::string token = additional_info.get<std::string>("token", "");

  std::string oauth;
  std::string iotagent_user;
  std::string iotagent_pass;
  std::string service = additional_info.get<std::string>("service", "");
  std::string service_path =
      additional_info.get<std::string>("service_path", "");

  iota::Configurator* configurator = iota::Configurator::instance();
  if (configurator != NULL) {
    try {
      std::map<std::string, std::string> to_map;

      configurator->get("oauth", to_map);
      oauth = to_map["on_behalf_trust_url"];
      iotagent_user = to_map["on_behalf_user"];
      iotagent_pass = to_map["on_behalf_password"];
    } catch (std::runtime_error& e) {
      IOTA_LOG_DEBUG(m_logger, "oauth not found :" << e.what());
    }
  }
  int number_of_tries = additional_info.get<int>(
      iota::ContextBrokerCommunicator::NUMBER_OF_TRIES, 0);
  try {
    iota::IoTUrl dest(url);
    std::string resource = dest.getPath();
    std::string query = dest.getQuery();
    std::string server = dest.getHost();
    std::string compound_server(server);
    compound_server.append(":");
    compound_server.append(boost::lexical_cast<std::string>(dest.getPort()));
    IOTA_LOG_DEBUG(m_logger, "send Server:" << server << "service:" << service
                                            << " service_path:" << service_path
                                            << " tries=" << number_of_tries);

    if (!token.empty() && !oauth.empty()) {
      iota::OAuth oauth_comm(_io_service);
      oauth_comm.set_oauth_trust(oauth);
      oauth_comm.set_sync_service();
      // Setting trust_token  before of identity
      oauth_comm.set_trust_token(token);
      oauth_comm.set_identity(OAUTH_ON_BEHALF_TRUST, iotagent_user,
                              iotagent_pass);
      std::string auth_token = oauth_comm.get_token(status_code);
      additional_info.put<std::string>("token", auth_token);
    }
    pion::http::request_ptr request = create_request(
        compound_server, resource, content, query, additional_info);

    http_client.reset(
        // new iota::HttpClient(_io_service, server, dest.getPort()));
        new iota::HttpClient(server, dest.getPort()));
    // TODO check add (sync)
    // add_connection(http_client);
    int timeout = boost::lexical_cast<int>(
        additional_info.get<std::string>("timeout", "5"));
    std::string proxy = additional_info.get<std::string>("proxy", "");
    response = http_client->send(request, timeout, proxy);
    cb_response = process_response(url, http_client, response, content,
                                   additional_info, http_client->get_error());
  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, e.what());
    iota::Alarm::error(iota::types::ALARM_CODE_NO_CB, url, content,
                       additional_info, iota::types::ERROR, e.what());
  }
  // TODO check remove (sync)
  // remove_connection(http_client);
  if (response.get() != NULL && !token.empty() && !oauth.empty() &&
      response->get_status_code() ==
          pion::http::types::RESPONSE_CODE_UNAUTHORIZED &&
      number_of_tries < 1) {
    additional_info.put(iota::ContextBrokerCommunicator::NUMBER_OF_TRIES,
                        ++number_of_tries);
    cb_response = send(url, content, additional_info,
                       pion::http::types::RESPONSE_CODE_UNAUTHORIZED);
  }
  return cb_response;
}

int iota::ContextBrokerCommunicator::send(
    iota::ContextElement ngsi_context_element, const std::string& opSTR,
    const boost::property_tree::ptree& service, std::string& cb_response) {
  boost::property_tree::ptree pt_cb;
  std::string cb_url;

  std::string cbrokerURL = service.get<std::string>("cbroker", "");
  if (!cbrokerURL.empty()) {
    cb_url.assign(cbrokerURL);
    cb_url.append(get_ngsi_operation("updateContext"));
  }

  // Envio
  //
  // UpdateContext
  std::string updateAction(opSTR);
  iota::UpdateContext op(updateAction);
  op.add_context_element(ngsi_context_element);

  IOTA_LOG_DEBUG(m_logger,
                 "-----sendtoCB:" << ngsi_context_element.get_string());
  IOTA_LOG_DEBUG(m_logger, "send to CB:" << cb_url
                                         << ":op:" << op.get_string());
  iota::ContextBrokerCommunicator cb_communicator;
  cb_response.append(send(cb_url, op.get_string(), service));

  IOTA_LOG_DEBUG(m_logger, "CB  response " << cb_response);
  return pion::http::types::RESPONSE_CODE_OK;
}

std::string iota::ContextBrokerCommunicator::get_ngsi_operation(
    const std::string& operation) {
  std::string op("/NGSI10/");
  op.append(operation);

  try {
    const iota::JsonValue& ngsi_url =
        iota::Configurator::instance()->get("ngsi_url");
    std::string op_url = ngsi_url[operation.c_str()].GetString();
    op.assign(op_url);
  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, "Configuration error " << e.what());
  }
  return op;
}

int iota::ContextBrokerCommunicator::send_updateContext(
    const std::string& command_name, const std::string& command_att,
    const std::string& type, const std::string& value,
    const boost::shared_ptr<iota::Device>& item_dev,
    const boost::property_tree::ptree& service, const std::string& opSTR) {
  IOTA_LOG_DEBUG(m_logger, "send_updateContext "
                               << command_name << " " << command_att << " "
                               << value << " "
                               << item_dev->get_real_name(service) << " "
                               << item_dev->_entity_type);
  std::string cb_response;
  iota::RiotISO8601 mi_hora;
  std::string date_to_cb = mi_hora.toUTC().toString();

  std::string att_status = command_name + "_" + command_att;
  iota::Attribute att(att_status, type, value);
  iota::Attribute metadata_ts("TimeInstant", "ISO8601", date_to_cb);
  att.add_metadata(metadata_ts);

  iota::ContextElement ngsi_context_element;
  ngsi_context_element.set_env_info(service, item_dev);
  ngsi_context_element.add_attribute(att);

  // attribute with time
  iota::Attribute timeAT("TimeInstant", "ISO8601", date_to_cb);
  ngsi_context_element.add_attribute(timeAT);
  IOTA_LOG_DEBUG(m_logger, "<<<" << ngsi_context_element.get_string());
  int code_resp = send(ngsi_context_element, opSTR, service, cb_response);

  return code_resp;
}

void iota::ContextBrokerCommunicator::add_updateContext(
    const std::string& command_name, const std::string& command_att,
    const std::string& type, const std::string& value,
    const boost::shared_ptr<iota::Device>& item_dev,
    const boost::property_tree::ptree& service,
    iota::ContextElement& ngsi_context_element) {
  iota::RiotISO8601 mi_hora;
  std::string date_to_cb = mi_hora.toUTC().toString();

  std::string att_status = command_name + "_" + command_att;
  iota::Attribute att(att_status, type, value);
  iota::Attribute metadata_ts("TimeInstant", "ISO8601", date_to_cb);
  att.add_metadata(metadata_ts);

  ngsi_context_element.add_attribute(att);
}

pion::http::request_ptr iota::ContextBrokerCommunicator::create_request(
    std::string& server, std::string& resource, std::string& content,
    std::string& query, boost::property_tree::ptree& additional_info) {
  pion::http::request_ptr request(new pion::http::request());
  request->set_method(_method);
  request->set_resource(resource);
  request->set_content(content);
  request->set_content_type(iota::types::IOT_CONTENT_TYPE_JSON);
  // request->setContentLength(content.size());

  if (query.empty() == false) {
    request->set_query_string(query);
  }

  std::string forced_accept =
      additional_info.get<std::string>(iota::types::IOT_HTTP_HEADER_ACCEPT, "");
  if (forced_accept.empty()) {
    request->add_header(iota::types::IOT_HTTP_HEADER_ACCEPT,
                        iota::types::IOT_CONTENT_TYPE_JSON);
  } else {
    request->add_header(iota::types::IOT_HTTP_HEADER_ACCEPT, forced_accept);
  }

  request->add_header(pion::http::types::HEADER_HOST, server);
  std::string service = additional_info.get<std::string>("service", "");
  std::string service_path =
      additional_info.get<std::string>("service_path", "/");
  request->add_header(iota::types::FIWARE_SERVICE, service);
  request->add_header(iota::types::FIWARE_SERVICEPATH, service_path);
  std::string token = additional_info.get<std::string>("token", "");
  IOTA_LOG_DEBUG(m_logger, "IotAgent token " << token);
  if (token.empty() == false) {
    request->change_header(iota::types::IOT_HTTP_HEADER_AUTH, token);
  }
  std::string trace_header =
      additional_info.get<std::string>(iota::types::HEADER_TRACE_MESSAGES, "");
  if (!trace_header.empty()) {
    request->change_header(iota::types::HEADER_TRACE_MESSAGES, trace_header);
  }
  return request;
}

void iota::ContextBrokerCommunicator::add_connection(
    boost::shared_ptr<iota::HttpClient> connection) {
  boost::mutex::scoped_lock lock(_m);
  _connections.insert(
      std::pair<std::string, boost::shared_ptr<iota::HttpClient> >(
          connection->get_identifier(), connection));
}

boost::shared_ptr<iota::HttpClient>
iota::ContextBrokerCommunicator::get_connection(
    const std::string& id_connection) {
  boost::mutex::scoped_lock lock(_m);
  boost::shared_ptr<iota::HttpClient> connection;

  std::map<std::string, boost::shared_ptr<iota::HttpClient> >::iterator it =
      _connections.begin();

  it = _connections.find(id_connection);

  if (it != _connections.end()) {
    connection = it->second;
  }

  return connection;
}
void iota::ContextBrokerCommunicator::remove_connection(
    boost::shared_ptr<iota::HttpClient> connection) {
  boost::mutex::scoped_lock lock(_m);
  if (connection.get() != NULL) {
    _connections.erase(connection->get_identifier());
    // connection->stop();
  }
}

std::string iota::ContextBrokerCommunicator::process_response(
    const std::string& url, boost::shared_ptr<iota::HttpClient> connection,
    pion::http::response_ptr resp, const std::string& content,
    const boost::property_tree::ptree& additional_info,
    const boost::system::error_code& error) {
  std::string response;
  if ((!error) && (resp.get() != NULL)) {
    response = resp->get_content();
    iota::Alarm::info(iota::types::ALARM_CODE_NO_CB, url, iota::types::ERROR,
                      "send ok " + response);
  } else {
    std::stringstream ss;
    ss << "Response error ";
    ss << " -> ";
    ss << connection->getRemoteEndpoint();
    ss << ": ";
    ss << error.message();
    iota::Alarm::error(iota::types::ALARM_CODE_NO_CB, url, content,
                       additional_info, iota::types::ERROR, ss.str());
  }
  return response;
}
