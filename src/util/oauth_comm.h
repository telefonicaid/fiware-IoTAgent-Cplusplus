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
#ifndef SRC_UTIL_OAUTH_COMM_H_
#define SRC_UTIL_OAUTH_COMM_H_

#include "util/iota_logger.h"
#include "rest/process.h"
#include "http_client.h"
#include <boost/thread/mutex.hpp>
#include <boost/property_tree/ptree.hpp>

const std::string CONTENT_TYPE_JSON = "application/json";
const std::string HTTP_HEADER_ACCEPT = "Accept";
const std::string HTTP_HEADER_FIWARE_SERVICE = "Fiware-Service";
const std::string HTTP_HEADER_FIWARE_SERVICEPATH = "Fiware-ServicePath";
const std::string HTTP_HEADER_AUTH = "X-Auth-Token";
const std::string HTTP_HEADER_TOKEN = "X-Subject-Token";

// Identities
const std::string OAUTH_ON_BEHALF_TRUST = "OS-TRUST:trust";
const std::string OAUTH_PEP = "admin_domain";

// URLS
const std::string OAUTH_TRUST_TOKEN_URL = "/v3/auth/tokens";
const std::string OAUTH_VALIDATE_TOKEN_URL = "/v3/auth/tokens";
const std::string OAUTH_ROLES_URL = "/v3/role_assignments";
const std::string OAUTH_PROJECTS_URL = "/v3/projects";

namespace iota {

class OAuth : public boost::enable_shared_from_this<OAuth> {
 public:
  typedef boost::function<std::string(
      boost::shared_ptr<iota::HttpClient>, pion::http::response_ptr,
      const boost::system::error_code&)> oauth_comm_t;
  /*
    typedef boost::function<void (
      pion::http::request_ptr&,
      pion::tcp::connection_ptr&)> app_callback_t;
  */
  typedef boost::function<void(boost::shared_ptr<OAuth>)> app_callback_t;
  OAuth(boost::asio::io_service& io_service);
  OAuth(boost::asio::io_service& io_service, int timeout);
  virtual ~OAuth();
  void set_domain(std::string service) { _domain = service; };
  void set_project(std::string subservice) { _project = subservice; };

  std::string get_user_id() { return _user_id; };

  std::string get_domain() { return _domain; };

  std::string get_project() { return _project; };

  boost::property_tree::ptree get_roles() { return _user_roles; };

  std::string get_subserviceId() { return _subservice_id; };

  std::string get_serviceId() { return _service_id; };

  void set_timeout(int timeout);
  int get_timeout();
  void set_oauth_validate(std::string val_url);
  void set_oauth_roles(std::string roles_url);
  void set_oauth_projects(std::string projects_url);
  void set_oauth_trust(std::string trust_url);
  std::string get_oauth_validate();
  std::string get_oauth_roles();
  std::string get_oauth_projects();
  std::string get_oauth_trust();
  std::string get_token(int status_code = 200);
  boost::property_tree::ptree validate_user_token(
      std::string token, app_callback_t = app_callback_t());
  void set_identity(std::string scope_type, std::string username,
                    std::string password);
  boost::property_tree::ptree get_auth_data();
  void set_trust_token(std::string trust_token);
  std::string get_trust_token();
  bool is_pep();
  std::string get_identifier();

  // This function is deprecated
  void set_sync_service();

  boost::property_tree::ptree get_ptree(std::string data);
  boost::property_tree::ptree get_user_roles(std::string user_id);
  std::string get_subservice(std::string domain, std::string project);

  void receive_event_renew_token(boost::shared_ptr<iota::HttpClient> connection,
                                 pion::http::response_ptr http_response_ptr,
                                 const boost::system::error_code& error);
  std::string receive_event_get_user(
      boost::shared_ptr<iota::HttpClient> connection,
      pion::http::response_ptr http_response_ptr,
      const boost::system::error_code& error);
  std::string receive_event_get_subservice(
      boost::shared_ptr<iota::HttpClient> connection,
      pion::http::response_ptr http_response_ptr,
      const boost::system::error_code& error);
  std::string receive_event_get_user_roles(
      boost::shared_ptr<iota::HttpClient> connection,
      pion::http::response_ptr http_response_ptr,
      const boost::system::error_code& error);

 protected:
 private:
  std::string _trust_token;
  struct TokenData {
    std::string _subject_token;
    boost::posix_time::ptime _timestamp;
  };
  std::string _user_token;
  std::string _domain;
  std::string _project;
  std::string _user_id;
  std::string _subservice_id;
  std::string _service_id;
  boost::property_tree::ptree _user_roles;
  TokenData _data;
  std::string _oauth_validate;
  std::string _oauth_roles;
  std::string _oauth_projects;
  std::string _oauth_trust;
  boost::property_tree::ptree _auth;
  boost::mutex _m;
  boost::mutex _m_tmp;
  boost::mutex _m_c;
  pion::logger m_logger;
  int _timeout;
  std::string _id;
  boost::asio::io_service& _io_service;

  // Deprecated and temporary variable
  bool _sync;
  std::map<std::string, boost::shared_ptr<iota::HttpClient> > _connections;
  void add_connection(boost::shared_ptr<iota::HttpClient> connection);
  void remove_connection(boost::shared_ptr<iota::HttpClient> connection);

  void renew_token(std::string scope);
  std::string get_user(std::string token, std::string token_pep);

  pion::http::request_ptr create_request(
      std::string server, std::string resource, std::string content,
      std::string query, boost::property_tree::ptree additional_info);

  std::string send_request(std::string path, std::string method,
                           std::string content, std::string query,
                           boost::property_tree::ptree headers, std::string f,
                           oauth_comm_t handler = oauth_comm_t());
  app_callback_t _application_callback;
};
};

#endif
