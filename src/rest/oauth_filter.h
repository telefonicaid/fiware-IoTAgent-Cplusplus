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
#ifndef SRC_REST_OAUTH_FILTER_H_
#define SRC_REST_OAUTH_FILTER_H_

#include "http_filter.h"
#include "util/oauth_comm.h"
#include "util/access_control.h"
#include <map>

namespace iota {

class OAuthFilter : public virtual iota::HTTPFilter {
 public:
  OAuthFilter(boost::asio::io_service& io_service);
  virtual ~OAuthFilter();
  void set_configuration(std::map<std::string, std::string>& oauth_conf);
  void set_pep_rules(std::multimap<std::string, iota::PepRule>& pep_rules);
  virtual bool handle_request(pion::http::request_ptr& http_request_ptr,
                              pion::tcp::connection_ptr& tcp_conn);
  void set_authorization_validate(std::string validate_url);
  void set_authorization_roles(std::string roles_url);
  void set_authorization_projects(std::string projects_url);
  void set_access_control_endpoint(std::string endpoint);
  void set_filter_url_base(std::string filter_url_base);
  void set_pep_user_information(std::string domain, std::string user,
                                std::string password);
  void authorize(pion::http::request_ptr& http_request_ptr,
                 pion::tcp::connection_ptr& tcp_conn,
                 boost::shared_ptr<iota::OAuth> oauth_comm);
  void access_control(pion::http::request_ptr& http_request_ptr,
                      pion::tcp::connection_ptr& tcp_conn,
                      boost::shared_ptr<iota::AccessControl> ac,
                      bool authorized);
  std::string get_action(std::string verb, std::string uri);

 protected:
 private:
  std::string _auth_endpoint_validate;
  std::string _auth_endpoint_roles;
  std::string _auth_endpoint_projects;
  std::string _ac_endpoint;
  std::string _user;
  std::string _password;
  std::string _domain;
  int _timeout;
  std::multimap<std::string, std::string> _pep_rules;
  std::multimap<std::string, std::string> _uri_actions;
  std::string _filter_url;
  std::map<std::string, boost::shared_ptr<iota::OAuth> > _connections;
  std::map<std::string, boost::shared_ptr<iota::AccessControl> >
      _connections_ac;
  boost::mutex _m;
  void add_connection(boost::shared_ptr<iota::OAuth> connection);
  void remove_connection(boost::shared_ptr<iota::OAuth> connection);
  void add_connection_ac(std::string id,
                         boost::shared_ptr<iota::AccessControl> connection);
  void remove_connection_ac(std::string id);
  std::string get_resource(std::string service, std::string subservice,
                           std::string path);
  std::string get_relative_resource(std::string resource);
};
}
#endif
