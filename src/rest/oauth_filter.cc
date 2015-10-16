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
#include "oauth_filter.h"
#include "rest/types.h"
#include "rest/rest_handle.h"
#include "rest/rest_functions.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>

namespace iota {
std::map<std::string, std::string> actions = boost::assign::map_list_of(
    "GET", "read")("POST", "create")("PUT", "update")("DELETE", "delete");
};

iota::OAuthFilter::OAuthFilter(boost::asio::io_service& io_service)
    : _timeout(3),
      HTTPFilter(io_service,
                 PION_GET_LOGGER(iota::Process::get_logger_name())) {
  std::string context("POST[[:space:]]+");
  context += "/";
  context += iota::NGSI_SERVICE;
  context += "/([^/]+)/";
  std::string u_context(context);
  std::string q_context(context);
  u_context += "updateContext";
  q_context += "queryContext";
  _uri_actions.insert(std::pair<std::string, std::string>("create", u_context));
  _uri_actions.insert(std::pair<std::string, std::string>("read", q_context));
};

iota::OAuthFilter::~OAuthFilter(){};

void iota::OAuthFilter::set_configuration(
    std::map<std::string, std::string>& oauth_conf) {
  _auth_endpoint_validate =
      oauth_conf[iota::types::CONF_FILE_OAUTH_VALIDATE_TOKEN_URL];
  _auth_endpoint_roles = oauth_conf[iota::types::CONF_FILE_OAUTH_ROLES_URL];
  _auth_endpoint_projects =
      oauth_conf[iota::types::CONF_FILE_OAUTH_PROJECTS_URL];
  _ac_endpoint = oauth_conf[iota::types::CONF_FILE_ACCESS_CONTROL];
  _user = oauth_conf[iota::types::CONF_FILE_PEP_USER];
  _password = oauth_conf[iota::types::CONF_FILE_PEP_PASSWORD];
  _domain = oauth_conf[iota::types::CONF_FILE_PEP_DOMAIN];
  _timeout = boost::lexical_cast<int>(
      oauth_conf[iota::types::CONF_FILE_OAUTH_TIMEOUT]);
}

void iota::OAuthFilter::set_pep_rules(
    std::multimap<std::string, iota::PepRule>& pep_rules) {
  std::multimap<std::string, iota::PepRule>::iterator i_pep_rules =
      pep_rules.begin();
  while (i_pep_rules != pep_rules.end()) {
    std::string expr_regex;
    std::map<std::string, std::string> verb;
    std::vector<std::string> url_args;
    verb["method"] = i_pep_rules->second.verb;
    iota::format_pattern(i_pep_rules->second.uri, verb, expr_regex, url_args);
    _pep_rules.insert(
        std::pair<std::string, std::string>(i_pep_rules->first, expr_regex));
    i_pep_rules++;
  }
}

bool iota::OAuthFilter::handle_request(
    pion::http::request_ptr& http_request_ptr,
    pion::tcp::connection_ptr& tcp_conn) {
  bool allowed = true;

  // URI for protocols is not authorized
  // Exception for about and protocols
  if (http_request_ptr->get_resource().compare(
          iota::Process::get_url_base() + iota::ADMIN_SERVICE_ABOUT) == 0 ||
      http_request_ptr->get_resource().compare(
          iota::Process::get_url_base() + iota::ADMIN_SERVICE_PROTOCOLS) == 0) {
    tcp_conn->get_io_service().post(boost::bind(
        &iota::OAuthFilter::call_to_callback, shared_from_this(),
        http_request_ptr->get_header(iota::types::HEADER_TRACE_MESSAGES),
        pion::http::types::RESPONSE_CODE_OK));
  } else {
    IOTA_LOG_DEBUG(m_logger, "OAuthFilter handle_request "
                                 << http_request_ptr->get_header(
                                     iota::types::HEADER_TRACE_MESSAGES));

    // Headers
    std::string fiware_service =
        http_request_ptr->get_header(HTTP_HEADER_FIWARE_SERVICE);
    std::string fiware_servicepath =
        http_request_ptr->get_header(HTTP_HEADER_FIWARE_SERVICEPATH);
    if (fiware_servicepath.empty()) {
      fiware_servicepath = "/";
    }
    std::string x_auth_token = http_request_ptr->get_header(HTTP_HEADER_AUTH);
    int status = pion::http::types::RESPONSE_CODE_UNAUTHORIZED;
    IOTA_LOG_DEBUG(
        m_logger, " Fiware-Service=" + fiware_service + " Fiware-ServicePath=" +
                      fiware_servicepath + " X-Auth-Token=" + x_auth_token +
                      " endpoint-validate=" + _auth_endpoint_validate +
                      " endpoint-roles=" + _auth_endpoint_roles +
                      " endpoint-projects=" + _auth_endpoint_projects +
                      " endpoint-access-control=" + _ac_endpoint + " user=" +
                      _user + " password=" + _password + " domain=" + _domain);
    if (fiware_service.empty() || fiware_servicepath.empty() ||
        x_auth_token.empty() || _auth_endpoint_validate.empty() ||
        _auth_endpoint_roles.empty() || _auth_endpoint_projects.empty() ||
        _user.empty() || _password.empty() || _domain.empty() ||
        _ac_endpoint.empty()) {
      // This function add event with post
      handle_no_allowed(http_request_ptr, tcp_conn, status);
    } else {
      boost::shared_ptr<iota::OAuth> oauth_comm(
          new iota::OAuth(_io_service, _timeout));
      oauth_comm->set_oauth_validate(_auth_endpoint_validate);
      oauth_comm->set_oauth_roles(_auth_endpoint_roles);
      oauth_comm->set_oauth_projects(_auth_endpoint_projects);
      oauth_comm->set_identity(_domain, _user, _password);
      oauth_comm->set_domain(fiware_service);
      oauth_comm->set_project(fiware_servicepath);
      // add_connection(oauth_comm);
      oauth_comm->validate_user_token(
          x_auth_token, boost::bind(&iota::OAuthFilter::authorize, this,
                                    http_request_ptr, tcp_conn, _1));
    }
  }
  return allowed;
};

void iota::OAuthFilter::authorize(pion::http::request_ptr& http_request_ptr,
                                  pion::tcp::connection_ptr& tcp_conn,
                                  boost::shared_ptr<iota::OAuth> oauth_comm) {
  // remove_connection(oauth_comm);
  std::string user_id = oauth_comm->get_user_id();
  std::string domain = oauth_comm->get_domain();
  std::string project = oauth_comm->get_project();
  std::string subservice_id = oauth_comm->get_subserviceId();
  std::string service_id = oauth_comm->get_serviceId();
  boost::property_tree::ptree roles = oauth_comm->get_roles();
  pion::user_ptr user(new pion::user(user_id + " " + domain + " " + project));
  http_request_ptr->set_user(user);
  std::vector<std::string> user_subservice_roles;
  std::string resource_id(
      get_resource(domain, project,
                   get_relative_resource(http_request_ptr->get_resource())));
  std::string action(
      get_action(http_request_ptr->get_method(),
                 get_relative_resource(http_request_ptr->get_resource())));
  try {
    BOOST_FOREACH (boost::property_tree::ptree::value_type& v,
                   roles.get_child("role_assignments")) {
      if (oauth_comm->get_project().compare("/") == 0) {
        if (v.second.get<std::string>("scope.domain.id", "")
                .compare(service_id) == 0) {
          user_subservice_roles.push_back(
              v.second.get<std::string>("role.id", ""));
        }
      } else {
        if (v.second.get<std::string>("scope.project.id", "")
                .compare(subservice_id) == 0) {
          user_subservice_roles.push_back(
              v.second.get<std::string>("role.id", ""));
        }
      }
    }

  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, "authorize=" << e.what() << " roles="
                                          << user_subservice_roles.size()
                                          << " resource" << resource_id
                                          << " action=" << action);
  }
  // http_request_ptr->set_user(user);
  int status = pion::http::types::RESPONSE_CODE_UNAUTHORIZED;

  IOTA_LOG_INFO(m_logger, "authorize user_id="
                              << user_id << " domain=" << domain << " project="
                              << project << " service_id=" << service_id
                              << " subservice_id=" << subservice_id
                              << " roles=" << user_subservice_roles.size()
                              << " resource=" << resource_id);
  if (user_id.empty() || roles.size() == 0 || resource_id.empty() ||
      action.empty() || _ac_endpoint.empty()) {
    handle_no_allowed(http_request_ptr, tcp_conn, status);
  } else {
    boost::shared_ptr<iota::AccessControl> access_control(
        new iota::AccessControl(_ac_endpoint, 5, _io_service));
    access_control->set_identifier(oauth_comm->get_identifier());
    // add_connection_ac(oauth_comm->get_identifier(), access_control);
    // Headers to AccessControl
    boost::property_tree::ptree headers;
    headers.put(
        iota::types::HEADER_TRACE_MESSAGES,
        http_request_ptr->get_header(iota::types::HEADER_TRACE_MESSAGES));
    headers.put(HTTP_HEADER_FIWARE_SERVICE,
                http_request_ptr->get_header(HTTP_HEADER_FIWARE_SERVICE));
    headers.put(HTTP_HEADER_FIWARE_SERVICEPATH,
                http_request_ptr->get_header(HTTP_HEADER_FIWARE_SERVICEPATH));
    headers.put(HTTP_HEADER_AUTH,
                http_request_ptr->get_header(HTTP_HEADER_AUTH));
    access_control->authorize(
        user_subservice_roles, resource_id, action, headers,
        boost::bind(&iota::OAuthFilter::access_control, this, http_request_ptr,
                    tcp_conn, _1, _2));
  }
}

void iota::OAuthFilter::access_control(
    pion::http::request_ptr& http_request_ptr,
    pion::tcp::connection_ptr& tcp_conn,
    boost::shared_ptr<iota::AccessControl> ac, bool authorized) {
  IOTA_LOG_DEBUG(m_logger, "access_control_result " << authorized);
  // remove_connection_ac(ac->get_identifier());
  int status = pion::http::types::RESPONSE_CODE_FORBIDDEN;
  if (authorized == true) {
    status = pion::http::types::RESPONSE_CODE_OK;
  }

  tcp_conn->get_io_service().post(boost::bind(
      &iota::OAuthFilter::call_to_callback, shared_from_this(),
      http_request_ptr->get_header(iota::types::HEADER_TRACE_MESSAGES),
      status));
}

void iota::OAuthFilter::set_authorization_validate(std::string validate_url) {
  _auth_endpoint_validate = validate_url;
}

void iota::OAuthFilter::set_authorization_roles(std::string roles_url) {
  _auth_endpoint_roles = roles_url;
}

void iota::OAuthFilter::set_authorization_projects(std::string projects_url) {
  _auth_endpoint_projects = projects_url;
}

void iota::OAuthFilter::set_access_control_endpoint(std::string endpoint) {
  _ac_endpoint = endpoint;
}

void iota::OAuthFilter::set_pep_user_information(std::string domain,
                                                 std::string user,
                                                 std::string password) {
  _user = user;
  _password = password;
  _domain = domain;
}

void iota::OAuthFilter::add_connection(
    boost::shared_ptr<iota::OAuth> connection) {
  boost::mutex::scoped_lock lock(_m);
  _connections.insert(std::pair<std::string, boost::shared_ptr<iota::OAuth> >(
      connection->get_identifier(), connection));
}

void iota::OAuthFilter::remove_connection(
    boost::shared_ptr<iota::OAuth> connection) {
  boost::mutex::scoped_lock lock(_m);
  if (connection.get() != NULL) {
    _connections.erase(connection->get_identifier());
  }
}

void iota::OAuthFilter::add_connection_ac(
    std::string id, boost::shared_ptr<iota::AccessControl> connection) {
  boost::mutex::scoped_lock lock(_m);
  _connections_ac.insert(
      std::pair<std::string, boost::shared_ptr<iota::AccessControl> >(
          id, connection));
}

void iota::OAuthFilter::remove_connection_ac(std::string id) {
  boost::mutex::scoped_lock lock(_m);
  _connections_ac.erase(id);
}

std::string iota::OAuthFilter::get_resource(std::string service,
                                            std::string subservice,
                                            std::string path) {
  std::string resource("fiware:iotagent:" + service + ":" + subservice + ":" +
                       path);
  return resource;
}

std::string iota::OAuthFilter::get_relative_resource(std::string resource) {
  if (resource.substr(0, _filter_url.size()) == _filter_url) {
    return resource.substr(_filter_url.size());
  }

  return resource;
}

void iota::OAuthFilter::set_filter_url_base(std::string filter_url_base) {
  _filter_url = filter_url_base;
}

std::string iota::OAuthFilter::get_action(std::string verb, std::string uri) {
  // First, rules by configuration
  // Second, uri_actions
  // Third, only verbs

  // Uri to regex

  std::string uri_regex(verb);
  uri_regex += " ";
  uri_regex += uri;
  std::vector<std::string> url_args;
  std::map<std::string, std::string> url_placeholders;

  std::string action;

  std::multimap<std::string, std::string>::iterator i_pep_rules =
      _pep_rules.begin();
  while (i_pep_rules != _pep_rules.end() && action.empty()) {
    bool res = iota::restResourceParse(i_pep_rules->second, url_args, uri_regex,
                                       url_placeholders);
    if (res) {
      action.assign(i_pep_rules->first);
    }
    i_pep_rules++;
  }

  if (action.empty()) {
    i_pep_rules = _uri_actions.begin();
    while (i_pep_rules != _uri_actions.end() && action.empty()) {
      bool res = iota::restResourceParse(i_pep_rules->second, url_args,
                                         uri_regex, url_placeholders);
      if (res) {
        action.assign(i_pep_rules->first);
      }
      ++i_pep_rules;
    }
  }
  if (action.empty()) {
    action.assign(iota::actions[verb]);
  }
  return action;
}
