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
#include "oauth_comm.h"
#include "util/iot_url.h"
#include "util/FuncUtil.h"
#include "util/alarm.h"
#include "rest/types.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

iota::OAuth::OAuth(boost::asio::io_service& io_service)
    : _io_service(io_service),
      _timeout(5),
      _sync(false),
      m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  if (_id.empty() == true) {
    _id = riot_uuid();
  }
}
iota::OAuth::OAuth(boost::asio::io_service& io_service, int timeout)
    : _io_service(io_service),
      _sync(false),
      _timeout(timeout),
      m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  if (_id.empty() == true) {
    _id = riot_uuid();
  }
}

iota::OAuth::~OAuth() {}
void iota::OAuth::set_oauth_validate(std::string val_url) {
  _oauth_validate.assign(val_url);
}

void iota::OAuth::set_oauth_roles(std::string roles_url) {
  _oauth_roles.assign(roles_url);
}

void iota::OAuth::set_oauth_projects(std::string projects_url) {
  _oauth_projects.assign(projects_url);
}

void iota::OAuth::set_oauth_trust(std::string trust_url) {
  _oauth_trust = trust_url;
}

void iota::OAuth::set_timeout(int timeout) { _timeout = timeout; }

int iota::OAuth::get_timeout() { return _timeout; }

std::string iota::OAuth::get_oauth_validate() { return _oauth_validate; }

std::string iota::OAuth::get_oauth_roles() { return _oauth_roles; }

std::string iota::OAuth::get_oauth_projects() { return _oauth_projects; }

std::string iota::OAuth::get_oauth_trust() { return _oauth_trust; }

void iota::OAuth::set_sync_service() { _sync = true; }

void iota::OAuth::set_identity(std::string scope_type, std::string username,
                               std::string password) {
  // If _auth is set, clear.
  _auth.clear();
  boost::property_tree::ptree auth;
  boost::property_tree::ptree identity;
  boost::property_tree::ptree domain;
  boost::property_tree::ptree user;
  boost::property_tree::ptree p_password;
  boost::property_tree::ptree scope;
  boost::property_tree::ptree methods;
  if (scope_type.compare(OAUTH_ON_BEHALF_TRUST) == 0) {
    domain.add("name", "Default");
    boost::property_tree::ptree trust;
    trust.add("id", _trust_token);
    scope.add_child(scope_type, trust);
  } else {
    domain.add("name", scope_type);
    scope.add_child("domain", domain);
  }
  user.add_child("domain", domain);
  user.add("name", username);
  user.add("password", password);
  p_password.add_child("user", user);

  methods.push_back(std::make_pair("", "password"));

  identity.add_child("methods", methods);
  identity.add_child("password", p_password);

  auth.add_child("identity", identity);
  auth.add_child("scope", scope);
  _auth.add_child("auth", auth);
}

boost::property_tree::ptree iota::OAuth::get_auth_data() { return _auth; }

void iota::OAuth::set_trust_token(std::string trust_token) {
  _trust_token = trust_token;
}

std::string iota::OAuth::get_trust_token() { return _trust_token; }

// trust token: If status_code is 401, we must renew the token
// If no token renew
// If pep, validate user token and login as pep (always)
std::string iota::OAuth::get_token(int status_code) {
  if (!is_pep()) {
    if ((status_code == pion::http::types::RESPONSE_CODE_UNAUTHORIZED) ||
        (_data._subject_token.empty())) {
      renew_token(_oauth_trust);
    }
  } else {
    // Login as pep
    // IoT Agent as PEP only for some urls and operations.
    // Always new token
    renew_token(_oauth_validate);
  }
  return _data._subject_token;
}

void iota::OAuth::receive_event_renew_token(
    boost::shared_ptr<iota::HttpClient> connection,
    pion::http::response_ptr response, const boost::system::error_code& error) {
  IOTA_LOG_DEBUG(m_logger, "receive_event_renew_token is_pep="
                               << boost::lexical_cast<std::string>(is_pep())
                               << " oauth=" << connection->getRemoteEndpoint()
                               << " conn_error=" << connection->get_error());
  if (response.get() != NULL &&
      (response->get_status_code() == pion::http::types::RESPONSE_CODE_OK ||
       response->get_status_code() ==
           pion::http::types::RESPONSE_CODE_ACCEPTED ||
       response->get_status_code() ==
           pion::http::types::RESPONSE_CODE_CREATED) &&
      !response->get_header(HTTP_HEADER_TOKEN).empty()) {
    // Header
    _data._subject_token = response->get_header(HTTP_HEADER_TOKEN);
    _data._timestamp = boost::posix_time::second_clock::universal_time();
    IOTA_LOG_DEBUG(m_logger,
                   "renew_token is_pep="
                       << boost::lexical_cast<std::string>(is_pep())
                       << " oauth=" << connection->getRemoteEndpoint()
                       << " conn_error=" << connection->get_error()
                       << " http_code=" << response->get_status_code()
                       << " subject-token=" << _data._subject_token.empty());
  } else {
    _data._subject_token.clear();
    IOTA_LOG_ERROR(m_logger,
                   "renew_token is_pep="
                       << boost::lexical_cast<std::string>(is_pep())
                       << " oauth=" << connection->getRemoteEndpoint()
                       << " conn_error=" << connection->get_error()
                       << " subject-token=" << _data._subject_token.empty());
  }

  // If subject token is empty, that means bad configuration:
  // - As PEP, user/paswword is incorrect.
  if (is_pep() && _data._subject_token.empty()) {
    iota::Alarm::error(iota::types::ALARM_CODE_BAD_CONFIGURATION, connection->getRemoteEndpoint(), iota::types::RESPONSE_MESSAGE_BAD_CONFIG, "No token as PEP user");
  }
  if (is_pep() && !_sync) {
    if (_data._subject_token.empty()) {
      if (_application_callback != NULL) {
        _application_callback(shared_from_this());
      }
    } else {
      get_user(_user_token, _data._subject_token);
    }
  }
}

std::string iota::OAuth::receive_event_get_user(
    boost::shared_ptr<iota::HttpClient> connection,
    pion::http::response_ptr response, const boost::system::error_code& error) {
  // For asynch communications
  std::string str_response;
  if (response.get() != NULL) {
    str_response = response->get_content();
    IOTA_LOG_INFO(m_logger, "receive_event_get_user conn_error="
                                << connection->get_error()
                                << " http_resp=" << str_response << "["
                                << response->get_status_code() << "]");
  }

  boost::property_tree::ptree ptree_user = get_ptree(str_response);
  if (!ptree_user.empty()) {
    std::string user_id = ptree_user.get<std::string>("token.user.id", "");
    std::string domain =
        ptree_user.get<std::string>("token.user.domain.name", "");
    if (response->get_status_code() == pion::http::types::RESPONSE_CODE_OK &&
        !user_id.empty() && !domain.empty() && domain.compare(_domain) == 0) {
      _user_id = user_id;
      _service_id = ptree_user.get<std::string>("token.user.domain.id", "");
    }
  }
  IOTA_LOG_DEBUG(m_logger, "receive_event_get_user user="
                               << _user_id << " domain=" << _domain);
  if (is_pep() && !_sync) {
    if (_user_id.empty()) {
      if (_application_callback != NULL) {
        _application_callback(shared_from_this());
      }
    } else {
      get_subservice(_domain, _project);
    }
  }

  return str_response;
}

void iota::OAuth::renew_token(std::string scope) {
  boost::mutex::scoped_lock l(_m);
  _data._subject_token.clear();
  IoTUrl dest(scope);
  std::string resource = dest.getPath();
  std::string query = dest.getQuery();
  std::string server = dest.getHost();
  std::string compound_server(server);
  compound_server.append(":");
  compound_server.append(boost::lexical_cast<std::string>(dest.getPort()));

  // Content
  std::ostringstream c;
  boost::property_tree::write_json(c, _auth);
  boost::property_tree::ptree additional_info;
  pion::http::request_ptr request = create_request(
      compound_server, resource, c.str(), query, additional_info);
  request->set_method("POST");
  boost::shared_ptr<iota::HttpClient> http_client;
  std::string proxy;
  pion::http::response_ptr response;
  if (!_sync) {
    IOTA_LOG_DEBUG(m_logger, "renew_token is_pep=" << is_pep()
                                                   << " oauth=" << server
                                                   << " timeout=" << _timeout);
    http_client.reset(
        new iota::HttpClient(_io_service, server, dest.getPort()));
    // add_connection(http_client);
    http_client->async_send(request, _timeout, proxy,
                            boost::bind(&iota::OAuth::receive_event_renew_token,
                                        shared_from_this(), _1, _2, _3));
  } else {
    IOTA_LOG_DEBUG(m_logger,
                   "renew_token is_pep=" << is_pep() << " oauth=" << server
                                         << " timeout=" << _timeout << " sync");
    http_client.reset(new iota::HttpClient(server, dest.getPort()));
    // add_connection(http_client);
    response = http_client->send(request, _timeout, proxy);
    receive_event_renew_token(http_client, response, http_client->get_error());
  }
}

std::string iota::OAuth::get_user(std::string token, std::string token_pep) {
  std::string log_msg("get_user pep-token=" + token_pep + " user-token=" +
                      token);
  IOTA_LOG_DEBUG(m_logger, log_msg);

  std::string str_response;
  _user_id.clear();
  boost::property_tree::ptree headers;
  headers.put(HTTP_HEADER_AUTH, token_pep);
  headers.put(HTTP_HEADER_TOKEN, token);
  str_response =
      send_request(_oauth_validate, "GET", "", "", headers, "get_user",
                   boost::bind(&iota::OAuth::receive_event_get_user,
                               shared_from_this(), _1, _2, _3));

  return str_response;
}

std::string iota::OAuth::receive_event_get_subservice(
    boost::shared_ptr<iota::HttpClient> connection,
    pion::http::response_ptr response, const boost::system::error_code& error) {
  // For asynch communications
  std::string str_response;
  std::string subservice;
  int status_code = pion::http::types::RESPONSE_CODE_BAD_REQUEST;

  // If subservice is empty, that is nor an error if response is OK
  if (response.get() != NULL) {
    str_response = response->get_content();
    status_code = response->get_status_code();
    IOTA_LOG_INFO(m_logger, "receive_event_get_subservice conn_error="
                                << connection->get_error() << " http_resp="
                                << str_response << "[" << status_code << "]");
  }

  // remove_connection(connection);
  if (!str_response.empty() &&
      status_code == pion::http::types::RESPONSE_CODE_OK) {
    boost::property_tree::ptree json_projects = get_ptree(str_response);
    try {
      bool found = false;
      BOOST_FOREACH (boost::property_tree::ptree::value_type& v,
                     json_projects.get_child("projects")) {
        // Requests contains service path no "/"
        if (_project.compare("/") != 0) {
          if (v.second.get<std::string>("name", "").compare(_project) == 0) {
            subservice = v.second.get<std::string>("id", "");
            _subservice_id = subservice;
            found = true;
            status_code = pion::http::types::RESPONSE_CODE_OK;
          } else {
            // TODO Review. Bad style
            if (found == false) {
              status_code = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
            }
          }
        }
      }
    } catch (std::exception& e) {
      IOTA_LOG_ERROR(m_logger, "get_subservice is_pep="
                                   << boost::lexical_cast<std::string>(is_pep())
                                   << " projects=" << e.what() << " token-pep="
                                   << _data._subject_token.empty());
    }
  } else {
    IOTA_LOG_ERROR(m_logger,
                   "get_subservice is_pep="
                       << boost::lexical_cast<std::string>(is_pep())
                       << " projects=" << str_response
                       << " token-pep=" << _data._subject_token.empty());
  }
  if (is_pep() && !_sync) {
    // When result is not from a OK response, we finalize.
    if (status_code == pion::http::types::RESPONSE_CODE_BAD_REQUEST) {
      if (_application_callback != NULL) {
        _application_callback(shared_from_this());
      }
    } else {
      get_user_roles(_user_id);
    }
  }
  return subservice;
}

std::string iota::OAuth::receive_event_get_user_roles(
    boost::shared_ptr<iota::HttpClient> connection,
    pion::http::response_ptr response, const boost::system::error_code& error) {
  // For asynch communications
  std::string str_response;
  if (response.get() != NULL) {
    str_response = response->get_content();
    IOTA_LOG_INFO(m_logger, "receive_event_get_user_roles conn_error="
                                << connection->get_error()
                                << " http_resp=" << str_response << "["
                                << response->get_status_code() << "]");
  }

  if (is_pep() && !_sync) {
    if (!str_response.empty()) {
      _user_roles = get_ptree(str_response);
    }
    if (_application_callback != NULL) {
      _application_callback(shared_from_this());
    }
  }

  return str_response;
}

std::string iota::OAuth::get_subservice(std::string domain,
                                        std::string project) {
  std::string str_response;
  // std::string query("domain_id=" + _service_id + "&name=" + project);
  std::string query("domain_id=" + _service_id);
  boost::property_tree::ptree headers;
  std::string token_pep(_data._subject_token);
  headers.put(HTTP_HEADER_AUTH, token_pep);
  str_response =
      send_request(_oauth_projects, pion::http::types::REQUEST_METHOD_GET, "",
                   query, headers, "get_subservice",
                   boost::bind(&iota::OAuth::receive_event_get_subservice,
                               shared_from_this(), _1, _2, _3));
  return str_response;
}

boost::property_tree::ptree iota::OAuth::validate_user_token(
    std::string token, iota::OAuth::app_callback_t callback) {
  boost::property_tree::ptree user_from_auth_token;
  _application_callback = callback;
  if (!token.empty()) {
    IOTA_LOG_DEBUG(m_logger, "validate_user_token is_pep="
                                 << boost::lexical_cast<std::string>(is_pep()));
    if (!is_pep()) {
      return user_from_auth_token;
    }
    if (_sync) {
      std::string pep_token = get_token();
      user_from_auth_token = get_ptree(get_user(token, pep_token));
    } else {
      _user_token = token;
      get_token();
    }
  }
  IOTA_LOG_DEBUG(m_logger, "validate_user_token is_pep="
                               << boost::lexical_cast<std::string>(is_pep())
                               << " token=" << token.empty()
                               << " user_from_auth_token="
                               << user_from_auth_token.empty());
  return user_from_auth_token;
}

boost::property_tree::ptree iota::OAuth::get_ptree(std::string data) {
  boost::property_tree::ptree pt_;
  std::istringstream is(data);
  // boost::mutex::scoped_lock l(_m_tmp);
  try {
    boost::property_tree::read_json(is, pt_);
  } catch (...) {
    // No data
    // IOTA_LOG_ERROR(m_logger, "No data to ptree");
  }
  return pt_;
}

boost::property_tree::ptree iota::OAuth::get_user_roles(std::string user_id) {
  boost::property_tree::ptree pt_user_roles;
  if (!is_pep()) {
    return pt_user_roles;
  }
  // This request follows get_user_data only if is PEP
  std::string str_response;
  std::string query("user.id=" + user_id + "&effective");
  boost::property_tree::ptree headers;
  headers.put(HTTP_HEADER_AUTH, _data._subject_token);
  str_response =
      send_request(_oauth_roles, pion::http::types::REQUEST_METHOD_GET, "",
                   query, headers, "get_user_roles",
                   // boost::bind(&iota::OAuth::receive_event_get_user_roles,
                   // this, _1, _2, _3));
                   boost::bind(&iota::OAuth::receive_event_get_user_roles,
                               shared_from_this(), _1, _2, _3));

  if (!str_response.empty() && _sync) {
    pt_user_roles = get_ptree(str_response);
  }

  return pt_user_roles;
}

bool iota::OAuth::is_pep() {
  std::string path_to_trust_token("auth.scope.OS-TRUST:trust.id");
  std::string trust_id = _auth.get<std::string>(path_to_trust_token, "");
  return (trust_id.empty());
}

pion::http::request_ptr iota::OAuth::create_request(
    std::string server, std::string resource, std::string content,
    std::string query, boost::property_tree::ptree additional_info) {
  pion::http::request_ptr request(new pion::http::request());
  request->set_resource(resource);
  if (!content.empty()) {
    request->set_content(content);
    request->set_content_type(CONTENT_TYPE_JSON);
  }

  if (query.empty() == false) {
    request->set_query_string(query);
  }
  request->add_header(HTTP_HEADER_ACCEPT, CONTENT_TYPE_JSON);
  request->add_header(pion::http::types::HEADER_HOST, server);
  return request;
}

std::string iota::OAuth::send_request(std::string endpoint, std::string method,
                                      std::string content, std::string query,
                                      boost::property_tree::ptree headers,
                                      std::string f, oauth_comm_t handler) {
  std::string p_request(" operation=" + f + " endpoint=" + endpoint +
                        " method=" + method + " content=" + content +
                        " query=" + query);
  std::string str_response;
  IoTUrl dest(endpoint);
  std::string resource = dest.getPath();
  std::string query_url = dest.getQuery();
  std::string server = dest.getHost();
  std::string compound_server(server);
  compound_server.append(":");
  compound_server.append(boost::lexical_cast<std::string>(dest.getPort()));

  boost::property_tree::ptree additional_info;
  pion::http::request_ptr request = create_request(
      compound_server, resource, content, query, additional_info);
  request->set_method(method);
  BOOST_FOREACH (boost::property_tree::ptree::value_type& v, headers) {
    request->add_header(v.first, v.second.data());
  }

  boost::shared_ptr<iota::HttpClient> http_client;
  std::string proxy;
  pion::http::response_ptr response;
  if (!_sync) {
    http_client.reset(
        new iota::HttpClient(_io_service, server, dest.getPort()));
    // add_connection(http_client);
    http_client->async_send(request, _timeout, proxy, handler);
  } else {
    http_client.reset(new iota::HttpClient(server, dest.getPort()));
    // add_connection(http_client);
    response = http_client->send(request, _timeout, proxy);
    str_response = handler(http_client, response, http_client->get_error());
  }
  return str_response;
}

std::string iota::OAuth::get_identifier() { return _id; }
void iota::OAuth::add_connection(
    boost::shared_ptr<iota::HttpClient> connection) {
  boost::mutex::scoped_lock lock(_m_c);
  _connections.insert(
      std::pair<std::string, boost::shared_ptr<iota::HttpClient> >(
          connection->get_identifier(), connection));
}

void iota::OAuth::remove_connection(
    boost::shared_ptr<iota::HttpClient> connection) {
  boost::mutex::scoped_lock lock(_m_c);
  if (connection.get() != NULL) {
    _connections.erase(connection->get_identifier());
    // connection->stop();
  }
}
