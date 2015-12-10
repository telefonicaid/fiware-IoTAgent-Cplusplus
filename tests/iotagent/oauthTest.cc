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
#include "oauthTest.h"
#include "util/oauth_comm.h"
#include "../mocks/util_functions.h"
#include "util/access_control.h"
#include "rest/oauth_filter.h"
#include "services/admin_service.h"
#include "../mocks/http_mock.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include "util/alarm.h"

CPPUNIT_TEST_SUITE_REGISTRATION(OAuthTest);
namespace iota {
std::string logger("main");
std::string URL_BASE("/iot");
}

bool handler_invoked = false;
bool access_control_handler = false;

void handler_function(pion::http::request_ptr& request,
                      pion::tcp::connection_ptr& connection,
                      boost::shared_ptr<iota::OAuth> oauth_comm) {
  std::cout << "Content in handler " << request->get_content() << std::endl;
  handler_invoked = true;
  CPPUNIT_ASSERT_MESSAGE("Check async oauth user",
                         oauth_comm->get_user_id().compare(
                             "5e817c5e0d624ee68dfb7a72d0d31ce4") == 0);
};

void ac_handler_function(boost::shared_ptr<iota::AccessControl> ac_ptr,
                         bool authorized) {
  access_control_handler = true;
  CPPUNIT_ASSERT_MESSAGE("Check ac", authorized);
};
OAuthTest::OAuthTest() {}
void OAuthTest::setUp() {}
void OAuthTest::tearDown() {}
void OAuthTest::testIdentity() {
  std::cout << "START testIdentity" << std::endl;

  std::string endpoint("http://0.0.0.0:10000");
  std::string trust_token("trust-token");
  std::string username("iotagent");
  std::string password("iotagent");
  std::string user_pep("pep");
  std::string pass_pep("pep");

  // Default constructor
  iota::OAuth oauth(iota::Process::get_process().get_io_service());
  CPPUNIT_ASSERT(oauth.get_timeout() == 5);
  CPPUNIT_ASSERT(oauth.get_oauth_validate().empty());
  oauth.set_timeout(10);
  oauth.set_oauth_trust(endpoint);
  CPPUNIT_ASSERT(oauth.get_timeout() == 10);
  CPPUNIT_ASSERT(oauth.get_oauth_trust().compare(endpoint) == 0);

  // Constructor
  iota::OAuth oauth_1(iota::Process::get_process().get_io_service(), 10);
  oauth_1.set_oauth_validate(endpoint);
  CPPUNIT_ASSERT(oauth_1.get_timeout() == 10);
  CPPUNIT_ASSERT(oauth_1.get_oauth_validate().compare(endpoint) == 0);

  // Trust token
  oauth_1.set_trust_token(trust_token);
  oauth_1.set_oauth_trust(endpoint);
  CPPUNIT_ASSERT(oauth_1.get_trust_token().compare(trust_token) == 0);

  // Identity
  oauth_1.set_identity(OAUTH_ON_BEHALF_TRUST, username, password);
  boost::property_tree::ptree auth_data = oauth_1.get_auth_data();
  // std::ostringstream os;
  // boost::property_tree::write_json(os, auth_data);
  // std::cout << os.str() << std::endl;
  std::string path_to_trust_token("auth.scope.OS-TRUST:trust.id");
  CPPUNIT_ASSERT_MESSAGE("Checking trust token from auth data ",
                         auth_data.get<std::string>(path_to_trust_token, "")
                                 .compare(trust_token) == 0);
  CPPUNIT_ASSERT_MESSAGE(
      "Checking username from auth data ",
      auth_data.get<std::string>("auth.identity.password.user.name", "")
              .compare(username) == 0);
  CPPUNIT_ASSERT_MESSAGE(
      "Checking password from auth data ",
      auth_data.get<std::string>("auth.identity.password.user.password", "")
              .compare(password) == 0);
  CPPUNIT_ASSERT_MESSAGE(
      "Checking domain from auth data ",
      auth_data.get<std::string>("auth.identity.password.user.domain.name", "")
              .compare("Default") == 0);
  boost::property_tree::ptree methods =
      auth_data.get_child("auth.identity.methods");

  CPPUNIT_ASSERT_MESSAGE("Checking size of methods array ",
                         methods.size() == 1);
  BOOST_FOREACH (boost::property_tree::ptree::value_type& vt,
                 auth_data.get_child("auth.identity.methods")) {
    CPPUNIT_ASSERT_MESSAGE("Checking methods array ",
                           vt.second.data().compare("password") == 0);
  }

  oauth_1.set_identity(OAUTH_PEP, user_pep, pass_pep);
  auth_data = oauth_1.get_auth_data();
  // std::ostringstream os;
  // boost::property_tree::write_json(os, auth_data);
  // std::cout << os.str() << std::endl;
  std::string path_to_scope("auth.scope.domain.name");
  CPPUNIT_ASSERT_MESSAGE("Checking pep scope from auth data ",
                         auth_data.get<std::string>(path_to_scope, "")
                                 .compare("admin_domain") == 0);
  CPPUNIT_ASSERT_MESSAGE(
      "Checking username from auth data ",
      auth_data.get<std::string>("auth.identity.password.user.name", "")
              .compare(user_pep) == 0);
  CPPUNIT_ASSERT_MESSAGE(
      "Checking password from auth data ",
      auth_data.get<std::string>("auth.identity.password.user.password", "")
              .compare(pass_pep) == 0);
  CPPUNIT_ASSERT_MESSAGE(
      "Checking domain from auth data ",
      auth_data.get<std::string>("auth.identity.password.user.domain.name", "")
              .compare("admin_domain") == 0);
  methods = auth_data.get_child("auth.identity.methods");

  CPPUNIT_ASSERT_MESSAGE("Checking size of methods array ",
                         methods.size() == 1);
  BOOST_FOREACH (boost::property_tree::ptree::value_type& vt,
                 auth_data.get_child("auth.identity.methods")) {
    CPPUNIT_ASSERT_MESSAGE("Checking methods array ",
                           vt.second.data().compare("password") == 0);
  }

  // is_pep
  CPPUNIT_ASSERT_MESSAGE("Checking is_pep ", oauth_1.is_pep());
  std::cout << "END testIdentity" << std::endl;
}

void OAuthTest::testGetTokenTrust() {
  std::cout << "START testGetTokenTrust" << std::endl;
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* http_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");

  std::string endpoint("http://127.0.0.1:" +
                       boost::lexical_cast<std::string>(port) +
                       "/mock/testGetTokenTrust");
  std::string trust_token("trust-token");
  std::string username("iotagent");
  std::string password("iotagent");

  iota::OAuth oauth(iota::Process::get_process().get_io_service());
  oauth.set_timeout(10);
  oauth.set_sync_service();
  oauth.set_oauth_trust(endpoint);
  oauth.set_trust_token(trust_token);
  oauth.set_identity(OAUTH_ON_BEHALF_TRUST, username, password);

  std::map<std::string, std::string> h;
  h["X-Subject-Token"] = "x-subject-token";
  http_mock->set_response("/mock/testGetTokenTrust", 200, "", h);
  http_mock->set_response("/mock/testGetTokenTrust", 200, "", h);
  CPPUNIT_ASSERT(oauth.get_token().compare("x-subject-token") == 0);

  CPPUNIT_ASSERT(oauth.get_token().compare("x-subject-token") == 0);
  CPPUNIT_ASSERT(oauth.get_token(401).compare("x-subject-token") == 0);

  std::cout << "END testGetTokenTrust" << std::endl;
}

void OAuthTest::testValidateToken() {
  std::cout << "START testValidateToken" << std::endl;
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* http_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  std::map<std::string, std::string> h;
  h["X-Subject-Token"] = "x-auth-token";
  http_mock->set_response("/mock/testValidateToken/v3/auth/tokens", 200, "", h);
  std::string user_token("X-USER-TOKEN");

  std::string content_validate_token(
      "{\"token\": {\"issued_at\": \"2014-10-06T08:20:13.484880Z\",\"extras\": "
      "{},\"methods\": [\"password\"],\"expires_at\": "
      "\"2014-10-06T09:20:13.484827Z\",");
  content_validate_token.append(
      "\"user\": {\"domain\": {\"id\": "
      "\"f7a5b8e303ec43e8a912fe26fa79dc02\",\"name\": "
      "\"SmartValencia\"},\"id\": "
      "\"5e817c5e0d624ee68dfb7a72d0d31ce4\",\"name\": \"alice\"}}}");

  std::string endpoint("http://127.0.0.1:" +
                       boost::lexical_cast<std::string>(port) +
                       "/mock/testValidateToken");
  std::string username("pep");
  std::string password("pep");

  // No trust token
  boost::shared_ptr<iota::OAuth> oauth(
      new iota::OAuth(iota::Process::get_process().get_io_service()));
  oauth->set_timeout(10);
  oauth->set_sync_service();
  oauth->set_oauth_validate(endpoint + OAUTH_VALIDATE_TOKEN_URL);
  oauth->set_oauth_roles(endpoint + OAUTH_ROLES_URL);
  oauth->set_oauth_projects(endpoint + OAUTH_PROJECTS_URL);
  oauth->set_identity(OAUTH_PEP, username, password);
  oauth->set_domain("SmartValencia");
  oauth->set_project("Electricidad");


  std::string my_token = oauth->get_token();

  CPPUNIT_ASSERT(my_token.compare("x-auth-token") == 0);

  http_mock->set_response("/mock/testValidateToken/v3/auth/tokens", 200, "", h);
  CPPUNIT_ASSERT(oauth->get_token().compare("x-auth-token") == 0);
  http_mock->set_response("/mock/testValidateToken/v3/auth/tokens", 200, "", h);
  CPPUNIT_ASSERT(oauth->get_token(401).compare("x-auth-token") == 0);

  h.clear();
  h["X-Subject-Token"] = "x-subject-token";
  http_mock->set_response("/mock/testValidateToken/v3/auth/tokens", 200, "", h);
  http_mock->set_response("/mock/testValidateToken/v3/auth/tokens", 200,
                          content_validate_token);

  std::cout << "Start validate token" << std::endl;
  boost::property_tree::ptree pt_response =
      oauth->validate_user_token(user_token);

  CPPUNIT_ASSERT_MESSAGE("Checking user ", pt_response.size() > 0);


  std::cout << "END testValidateToken" << std::endl;
}

void OAuthTest::testGetUserRoles() {
  std::cout << "Start testGetUserRoles" << std::endl;
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* http_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  std::string oauth_resource("/mock/testGetUserRoles");
  std::string endpoint("http://0.0.0.0:" +
                       boost::lexical_cast<std::string>(port) + oauth_resource);
  std::string username("pep");
  std::string password("pep");
  std::string user_token("X-USER-TOKEN");
  std::string subservicio("c6851f8ef57c4b91b567ab62ca3d0aef");
  std::string role("a6407b6c597e4e1dad37a3420b6137dd");

  std::map<std::string, std::string> h;
  h["X-Subject-Token"] = "x-auth-token";
  http_mock->set_response(oauth_resource + OAUTH_VALIDATE_TOKEN_URL, 200, "",
                          h);

  boost::shared_ptr<iota::OAuth> oauth(
      new iota::OAuth(iota::Process::get_process().get_io_service()));
  oauth->set_timeout(10);
  oauth->set_sync_service();
  oauth->set_oauth_validate(endpoint + OAUTH_VALIDATE_TOKEN_URL);
  oauth->set_oauth_roles(endpoint + OAUTH_ROLES_URL);
  oauth->set_oauth_projects(endpoint + OAUTH_PROJECTS_URL);
  oauth->set_identity(OAUTH_PEP, username, password);
  oauth->get_token();

  // No roles
  boost::property_tree::ptree roles =
      oauth->get_user_roles("5e817c5e0d624ee68dfb7a72d0d31ce4");
  CPPUNIT_ASSERT_MESSAGE("No user roles", roles.empty());

  h.clear();
  std::string user_roles(
      "{\"role_assignments\": [{\"scope\": {\"project\": {\"id\": "
      "\"c6851f8ef57c4b91b567ab62ca3d0aef\"}},\"role\": {\"id\": "
      "\"a6407b6c597e4e1dad37a3420b6137dd\"},\"user\": {\"id\": "
      "\"5e817c5e0d624ee68dfb7a72d0d31ce4\"},\"links\": {\"assignment\": "
      "\"puthere\"}}],\"links\": {\"self\": "
      "\"http://${KEYSTONE_HOST}/v3/role_assignments\",\"previous\": "
      "null,\"next\": null}}");
  http_mock->set_response(oauth_resource + OAUTH_ROLES_URL, 200, user_roles);
  roles = oauth->get_user_roles("5e817c5e0d624ee68dfb7a72d0d31ce4");
  CPPUNIT_ASSERT_MESSAGE("Checking user roles: response", !roles.empty());
  CPPUNIT_ASSERT_MESSAGE("Checking number user roles ",
                         roles.get_child("role_assignments").size() == 1);
  BOOST_FOREACH (boost::property_tree::ptree::value_type& v,
                 roles.get_child("role_assignments")) {
    CPPUNIT_ASSERT_MESSAGE("Checking subservice ",
                           v.second.get<std::string>("scope.project.id", "")
                                   .compare(subservicio) == 0);
    CPPUNIT_ASSERT_MESSAGE(
        "Checking role ",
        v.second.get<std::string>("role.id", "").compare(role) == 0);
  }

  http_mock->stop();
  std::cout << "End testGetUserRoles" << std::endl;
}

void OAuthTest::testGetSubservice() {
  std::cout << "Start testGetSubservice" << std::endl;
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* http_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  std::string mock_port = boost::lexical_cast<std::string>(port);
  std::string oauth_resource("/mock/testGetUserRoles");
  std::string endpoint("http://0.0.0.0:" + mock_port + oauth_resource);
  std::string username("pep");
  std::string password("pep");
  std::map<std::string, std::string> h;
  h["X-Subject-Token"] = "x-auth-token";
  http_mock->set_response(oauth_resource + OAUTH_VALIDATE_TOKEN_URL, 200, "",
                          h);
  boost::shared_ptr<iota::OAuth> oauth(
      new iota::OAuth(iota::Process::get_process().get_io_service()));
  oauth->set_timeout(10);
  oauth->set_sync_service();
  oauth->set_oauth_validate(endpoint + OAUTH_VALIDATE_TOKEN_URL);
  oauth->set_oauth_roles(endpoint + OAUTH_ROLES_URL);
  oauth->set_oauth_projects(endpoint + OAUTH_PROJECTS_URL);
  oauth->set_identity(OAUTH_PEP, username, password);
  oauth->set_domain("SmartValencia");
  oauth->set_project("Electricidad");
  oauth->get_token();
  std::string domain("f7a5b8e303ec43e8a912fe26fa79dc02");
  std::string subservicio("c6851f8ef57c4b91b567ab62ca3d0aef");
  std::string projects(
      "{\"projects\": [{\"description\": \"SmartValencia Subservicio "
      "Electricidad\",\"links\": {\"self\": "
      "\"http://${KEYSTONE_HOST}/v3/projects/"
      "c6851f8ef57c4b91b567ab62ca3d0aef\"},\"enabled\": true,\"id\": "
      "\"c6851f8ef57c4b91b567ab62ca3d0aef\",\"domain_id\": "
      "\"f7a5b8e303ec43e8a912fe26fa79dc02\",\"name\": \"Electricidad\"}]}");
  http_mock->set_response(oauth_resource + OAUTH_PROJECTS_URL, 200, projects,
                          h);
  CPPUNIT_ASSERT_MESSAGE(
      "Checking subservice ",
      oauth->get_subservice(domain, "Electricidad").compare(subservicio) == 0);
  std::cout << "End testGetSubservice" << std::endl;
}

void OAuthTest::testAccessControl() {
  std::cout << "Start testAccessControl (pending)" << std::endl;
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* http_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  std::string mock_port = boost::lexical_cast<std::string>(port);
  http_mock->set_response("/mock/oauth", 200, "Permit");
  std::string endpoint("http://0.0.0.0:" + mock_port + "/mock/oauth");
  boost::asio::io_service& io_service =
      iota::Process::get_process().get_io_service();
  boost::shared_ptr<iota::AccessControl> ac(
      new iota::AccessControl(endpoint, 5, io_service));

  std::vector<std::string> role;
  role.push_back("admin");
  std::string resource_id("frn:iotagent:43");
  std::string action("read");
  ac->set_endpoint_ac(endpoint);
  ac->set_timeout(10);
  CPPUNIT_ASSERT_MESSAGE("Checking endpoint ",
                         ac->get_endpoint_ac().compare(endpoint) == 0);
  CPPUNIT_ASSERT_MESSAGE("Checking timeout ", ac->get_timeout() == 10);

  http_mock->set_response("/mock/oauth", 200, "");
  boost::property_tree::ptree headers;
  bool authorization = ac->authorize(role, resource_id, action, headers,
                                     boost::bind(&ac_handler_function, _1, _2));
  while (!access_control_handler) {
    sleep(1);
  }
  std::cout << "End testAccessControl" << std::endl;
}

void OAuthTest::testValidateAsync() {
  std::cout << "START testValidateAsync" << std::endl;
  unsigned int port = iota::Process::get_process().get_http_port();
  MockService* http_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  std::string mock_port = boost::lexical_cast<std::string>(port);
  boost::asio::io_service& io_service =
      iota::Process::get_process().get_io_service();

  std::map<std::string, std::string> h;

  std::string content_validate_token(
      "{\"token\": {\"issued_at\": \"2014-10-06T08:20:13.484880Z\",\"extras\": "
      "{},\"methods\": [\"password\"],\"expires_at\": "
      "\"2014-10-06T09:20:13.484827Z\",");
  content_validate_token.append(
      "\"user\": {\"domain\": {\"id\": "
      "\"f7a5b8e303ec43e8a912fe26fa79dc02\",\"name\": "
      "\"SmartValencia\"},\"id\": "
      "\"5e817c5e0d624ee68dfb7a72d0d31ce4\",\"name\": \"alice\"}}}");

  std::string endpoint("http://0.0.0.0:" + mock_port +
                       "/mock/testValidateAsync");
  std::string username("pep");
  std::string password("pep");
  std::string user_token("X-USER-TOKEN");
  // No trust token
  boost::shared_ptr<iota::OAuth> oauth(new iota::OAuth(io_service));
  oauth->set_timeout(10);
  oauth->set_oauth_validate(endpoint + OAUTH_VALIDATE_TOKEN_URL);
  oauth->set_oauth_roles(endpoint + OAUTH_ROLES_URL);
  oauth->set_oauth_projects(endpoint + OAUTH_PROJECTS_URL);
  oauth->set_identity(OAUTH_PEP, username, password);
  oauth->set_domain("SmartValencia");
  oauth->set_project("Electricidad");
  h.clear();
  h["X-Subject-Token"] = "x-subject-token";
  std::string auth_resource("/mock/testValidateAsync");
  http_mock->set_response(auth_resource + OAUTH_VALIDATE_TOKEN_URL, 200, "", h);
  http_mock->set_response(auth_resource + OAUTH_VALIDATE_TOKEN_URL, 200,
                          content_validate_token);
  std::string projects(
      "{\"projects\": [{\"description\": \"SmartValencia Subservicio "
      "Electricidad\",\"links\": {\"self\": "
      "\"http://${KEYSTONE_HOST}/v3/projects/"
      "c6851f8ef57c4b91b567ab62ca3d0aef\"},\"enabled\": true,\"id\": "
      "\"c6851f8ef57c4b91b567ab62ca3d0aef\",\"domain_id\": "
      "\"f7a5b8e303ec43e8a912fe26fa79dc02\",\"name\": \"Electricidad\"}]}");
  http_mock->set_response(auth_resource + OAUTH_PROJECTS_URL, 200, projects, h);
  std::string user_roles(
      "{\"role_assignments\": [{\"scope\": {\"project\": {\"id\": "
      "\"c6851f8ef57c4b91b567ab62ca3d0aef\"}},\"role\": {\"id\": "
      "\"a6407b6c597e4e1dad37a3420b6137dd\"},\"user\": {\"id\": "
      "\"5e817c5e0d624ee68dfb7a72d0d31ce4\"},\"links\": {\"assignment\": "
      "\"puthere\"}}],\"links\": {\"self\": "
      "\"http://${KEYSTONE_HOST}/v3/role_assignments\",\"previous\": "
      "null,\"next\": null}}");
  http_mock->set_response(auth_resource + OAUTH_ROLES_URL, 200, user_roles);
  pion::http::request_ptr req(new pion::http::request());
  pion::tcp::connection_ptr connection(new pion::tcp::connection(io_service));
  req->set_content("Hola");
  std::cout << "Start validate token" << std::endl;
  oauth->validate_user_token(user_token,
                             boost::bind(handler_function, boost::ref(req),
                                         boost::ref(connection), oauth));
  while (!handler_invoked) {
    sleep(1);
  }
  CPPUNIT_ASSERT_MESSAGE(
      "Checking user ",
      oauth->get_user_id().compare("5e817c5e0d624ee68dfb7a72d0d31ce4") == 0);
  CPPUNIT_ASSERT_MESSAGE("Cheching roles ", oauth->get_roles().size() > 0);
  std::cout << "ENF testValidateAsync" << std::endl;
}

void OAuthTest::testActions() {
  iota::OAuthFilter oauth_filter(iota::Process::get_process().get_io_service());
  std::string verb("POST");
  std::multimap<std::string, iota::PepRule> rules;
  iota::PepRule rule1;
  rule1.verb = "POST";
  rule1.uri = "/ngsi/<protocol>/updateContext";
  iota::PepRule rule2;
  rule2.verb = "POST";
  rule2.uri = "/ngsi/<protocol>/queryContext";
  rules.insert(std::pair<std::string, iota::PepRule>("foo", rule1));
  rules.insert(std::pair<std::string, iota::PepRule>("smile", rule2));

  // NO rules, only verb
  CPPUNIT_ASSERT_MESSAGE(
      "Action only by verb ",
      oauth_filter.get_action("DELETE", "/device").compare("delete") == 0);
  CPPUNIT_ASSERT_MESSAGE("Action only by verb ",
                         oauth_filter.get_action("POST", "/devices/<device_id>")
                                 .compare("create") == 0);

  // Uri actions rules
  CPPUNIT_ASSERT_MESSAGE(
      "Action for updateContext ",
      oauth_filter.get_action("POST", "/ngsi/d/updateContext")
              .compare("create") == 0);
  CPPUNIT_ASSERT_MESSAGE("Action for queryContext ",
                         oauth_filter.get_action("POST", "/ngsi/d/queryContext")
                                 .compare("read") == 0);

  // Action
  oauth_filter.set_pep_rules(rules);
  CPPUNIT_ASSERT_MESSAGE(
      "Action for updateContext ",
      oauth_filter.get_action("POST", "/ngsi/d/updateContext").compare("foo") ==
          0);
  CPPUNIT_ASSERT_MESSAGE("Action for queryContext ",
                         oauth_filter.get_action("POST", "/ngsi/d/queryContext")
                                 .compare("smile") == 0);
}

void OAuthTest::test502() {
  std::cout << "Start test502" << std::endl;
  unsigned int port = iota::Process::get_process().get_http_port();

  MockService* http_mock =
      (MockService*)iota::Process::get_process().get_service("/mock");
  std::string mock_port = boost::lexical_cast<std::string>(port);
  std::string oauth_resource("/mock/testGetUserRoles");
  std::string endpoint("http://0.0.0.0:" + mock_port + oauth_resource);
  std::string username("pep");
  std::string password("pep");
  std::map<std::string, std::string> h;

  iota::Alarm* palarm = iota::Alarm::instance();
  palarm->reset();

  h["X-Subject-Token"] = "x-auth-token";
  http_mock->set_response(oauth_resource + OAUTH_VALIDATE_TOKEN_URL, 502,
                          "Bad Gateway", h);
  boost::shared_ptr<iota::OAuth> oauth(
      new iota::OAuth(iota::Process::get_process().get_io_service()));
  oauth->set_timeout(10);
  oauth->set_sync_service();
  oauth->set_oauth_validate(endpoint + OAUTH_VALIDATE_TOKEN_URL);
  oauth->set_oauth_roles(endpoint + OAUTH_ROLES_URL);
  oauth->set_oauth_projects(endpoint + OAUTH_PROJECTS_URL);
  oauth->set_identity(OAUTH_PEP, username, password);
  oauth->set_domain("SmartValencia");
  oauth->set_project("Electricidad");
  oauth->get_token();
  std::string domain("f7a5b8e303ec43e8a912fe26fa79dc02");
  std::string subservicio("c6851f8ef57c4b91b567ab62ca3d0aef");
  std::string badG("Bad Gateway");
  http_mock->set_response(oauth_resource + OAUTH_PROJECTS_URL, 502, badG, h);

  std::cout << "@UT@alarms <-->" << palarm->size() << std::endl;
  CPPUNIT_ASSERT_MESSAGE("alarms one2 ", palarm->size() == 1);

  std::cout << "End test502" << std::endl;
}
