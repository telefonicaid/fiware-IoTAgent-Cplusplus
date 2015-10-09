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
#include "access_control.h"
#include "util/iota_logger.h"
#include "util/iot_url.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

iota::AccessControl::AccessControl()
    : _timeout(5),
      _io_service(iota::Process::get_process().get_io_service()),
      m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {}
iota::AccessControl::AccessControl(std::string endpoint_ac, int timeout,
                                   boost::asio::io_service& io_service)
    : _endpoint_ac(endpoint_ac),
      _timeout(timeout),
      _io_service(io_service),
      m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {}

iota::AccessControl::~AccessControl() {
  std::cout << "Destructor ACCESS CONTROL" << std::endl;
}
void iota::AccessControl::set_endpoint_ac(std::string endpoint_ac) {
  _endpoint_ac.assign(endpoint_ac);
}

void iota::AccessControl::set_timeout(int timeout) { _timeout = timeout; }

int iota::AccessControl::get_timeout() { return _timeout; }

std::string iota::AccessControl::get_endpoint_ac() { return _endpoint_ac; }

bool iota::AccessControl::authorize(std::vector<std::string> roles,
                                    std::string resource_id, std::string action,
                                    boost::property_tree::ptree additional_info,
                                    app_callback_t callback) {
  // Addictional info contains headers
  IOTA_LOG_DEBUG(m_logger, "access_control_authorize roles="
                               << roles.size() << " resource_id=" << resource_id
                               << " action=" << action);

  _application_callback = callback;
  std::string str_response;
  bool authorization = false;
  std::string endpoint(_endpoint_ac);
  IoTUrl dest(endpoint);
  std::string resource = dest.getPath();
  std::string query = dest.getQuery();
  std::string server = dest.getHost();
  std::string compound_server(server);
  compound_server.append(":");
  compound_server.append(boost::lexical_cast<std::string>(dest.getPort()));

  // Content
  std::string content = create_xml_access_control(roles, resource_id, action);
  pion::http::request_ptr request = create_request(
      compound_server, resource, content, query, additional_info);

  boost::shared_ptr<iota::HttpClient> http_client(
      new iota::HttpClient(_io_service, server, dest.getPort()));
  std::string proxy;
  http_client->async_send(request, _timeout, proxy,
                          boost::bind(&iota::AccessControl::receive_event,
                                      shared_from_this(), _1, _2, _3));
  return authorization;
}

void iota::AccessControl::receive_event(
    boost::shared_ptr<iota::HttpClient> connection,
    pion::http::response_ptr response, const boost::system::error_code& error) {
  IOTA_LOG_DEBUG(m_logger, "receive_event=access_control conn_error="
                               << connection->get_error());
  bool authorized = false;
  std::string str_response;
  if (response.get() != NULL) {
    IOTA_LOG_DEBUG(m_logger, "receive_event=access_control content="
                                 << response->get_content() << " http-status="
                                 << response->get_status_code());
    if (response->get_status_code() == pion::http::types::RESPONSE_CODE_OK) {
      str_response = response->get_content();
      if (str_response.find("Permit") != std::string::npos) {
        authorized = true;
      }
    }
  }
  if (_application_callback) {
    _application_callback(shared_from_this(), authorized);
  }
}

pion::http::request_ptr iota::AccessControl::create_request(
    std::string server, std::string resource, std::string content,
    std::string query, boost::property_tree::ptree additional_info) {
  pion::http::request_ptr request(new pion::http::request());
  request->set_method(pion::http::types::REQUEST_METHOD_POST);
  request->set_resource(resource);
  request->set_content(content);
  request->add_header(pion::http::types::HEADER_CONTENT_TYPE,
                      pion::http::types::CONTENT_TYPE_XML);

  IOTA_LOG_DEBUG(m_logger, content);
  if (query.empty() == false) {
    request->set_query_string(query);
  }
  request->add_header(pion::http::types::HEADER_HOST, server);
  BOOST_FOREACH (boost::property_tree::ptree::value_type& v, additional_info) {
    request->add_header(v.first, v.second.data());
  }
  return request;
}

std::string iota::AccessControl::create_xml_access_control(
    std::vector<std::string>& roles, std::string& resource_id,
    std::string& action) {
  boost::property_tree::ptree ptree_xml;

  boost::property_tree::ptree root_node;
  boost::property_tree::ptree attributes_resource_node;
  boost::property_tree::ptree attributes_action_node;
  boost::property_tree::ptree attributes_roles_node;
  root_node.put("<xmlattr>.xmlns",
                std::string("urn:oasis:names:tc:xacml:3.0:core:schema:wd-17"));
  root_node.put("<xmlattr>.xmlns:xsi",
                std::string("http://www.w3.org/2001/XMLSchema-instance"));
  root_node.put("<xmlattr>.xsi:schemaLocation",
                std::string("urn:oasis:names:tc:xacml:3.0:core:schema:wd-17 "
                            "http://docs.oasis-open.org/xacml/3.0/"
                            "xacml-core-v3-schema-wd-17.xsd"));
  root_node.put("<xmlattr>.ReturnPolicyIdList", std::string("false"));
  root_node.put("<xmlattr>.CombinedDecision", std::string("false"));
  attributes_resource_node.put(
      "<xmlattr>.Category",
      std::string("urn:oasis:names:tc:xacml:3.0:attribute-category:resource"));
  boost::property_tree::ptree attribute_resource_value;
  boost::property_tree::ptree attribute_resource;
  attribute_resource_value.put(
      "<xmlattr>.DataType",
      std::string("http://www.w3.org/2001/XMLSchema#string"));
  attribute_resource_value.put("", resource_id);
  attribute_resource.put("<xmlattr>.IncludeInResult", std::string("false"));
  attribute_resource.put(
      "<xmlattr>.AttributeId",
      std::string("urn:oasis:names:tc:xacml:1.0:resource:resource-id"));
  attribute_resource.add_child("AttributeValue", attribute_resource_value);
  attributes_resource_node.add_child("Attribute", attribute_resource);
  root_node.add_child("Attributes", attributes_resource_node);

  attributes_action_node.put(
      "<xmlattr>.Category",
      std::string("urn:oasis:names:tc:xacml:3.0:attribute-category:action"));
  boost::property_tree::ptree attribute_action_value;
  boost::property_tree::ptree attribute_action;
  attribute_action_value.put(
      "<xmlattr>.DataType",
      std::string("http://www.w3.org/2001/XMLSchema#string"));
  attribute_action_value.put("", action);
  attribute_action.put("<xmlattr>.IncludeInResult", std::string("false"));
  attribute_action.put(
      "<xmlattr>.AttributeId",
      std::string("urn:oasis:names:tc:xacml:1.0:action:action-id"));
  attribute_action.add_child("AttributeValue", attribute_action_value);
  attributes_action_node.add_child("Attribute", attribute_action);
  root_node.add_child("Attributes", attributes_action_node);

  attributes_roles_node.put(
      "<xmlattr>.Category",
      std::string(
          "urn:oasis:names:tc:xacml:1.0:subject-category:access-subject"));
  for (int i = 0; i < roles.size(); i++) {
    boost::property_tree::ptree attribute_role_value;
    boost::property_tree::ptree attribute_role;
    attribute_role_value.put(
        "<xmlattr>.DataType",
        std::string("http://www.w3.org/2001/XMLSchema#string"));
    attribute_role_value.put("", roles[i]);
    attribute_role.put("<xmlattr>.IncludeInResult", std::string("false"));
    attribute_role.put(
        "<xmlattr>.AttributeId",
        std::string("urn:oasis:names:tc:xacml:1.0:subject:subject-id"));
    attribute_role.add_child("AttributeValue", attribute_role_value);
    attributes_roles_node.add_child("Attribute", attribute_role);
  }
  root_node.add_child("Attributes", attributes_roles_node);
  ptree_xml.add_child("Request", root_node);
  std::ostringstream os;
  boost::property_tree::xml_parser::write_xml(os, ptree_xml);
  return os.str();
}
