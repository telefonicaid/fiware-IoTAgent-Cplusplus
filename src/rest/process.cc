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

#include "rest/process.h"
#include "services/admin_service.h"
#include "services/ngsi_service.h"
#include <mongo/client/init.h>
#include <pion/process.hpp>

std::string iota::Process::_url_base = "/iot";
std::string iota::Process::_logger_name = "main";
iota::Process* iota::Process::_process = NULL;

iota::Process::Process(unsigned int num_main_threads) {
  _scheduler.set_num_threads(num_main_threads);
  _server_scheduler.set_num_threads(num_main_threads);
}

iota::Process& iota::Process::initialize(std::string url_base,
                                         unsigned int num_main_threads) {
  if (iota::Process::_process == NULL) {
    iota::Process::_process = new Process(num_main_threads);
    if (!url_base.empty()) {
      iota::Process::_url_base = url_base;
    }
    pion::process::initialize();
    mongo::client::initialize();
  }

  return *iota::Process::_process;
}
void iota::Process::shutdown() {
  stop();
  _http_servers.clear();
  _tcp_servers.clear();
  pion::process::shutdown();
}

void iota::Process::wait_for_shutdown() {
  pion::process::wait_for_shutdown();
  iota::Process::get_process().shutdown();
}

boost::asio::io_service& iota::Process::get_io_service() {
  return _scheduler.get_io_service();
}

pion::scheduler& iota::Process::get_scheduler() { return _scheduler; }

std::string& iota::Process::get_logger_name() { return _logger_name; }

std::string& iota::Process::get_url_base() { return _url_base; }

pion::tcp::server_ptr iota::Process::add_tcp_server(std::string server_name,
                                                    std::string endpoint) {
  boost::asio::ip::tcp::endpoint e = get_endpoint(endpoint);
  boost::shared_ptr<iota::TcpService> tcp_server(
      new iota::TcpService(get_scheduler(), e));
  if (server_name.empty()) {
    server_name = endpoint;
  }
  _tcp_servers.insert(
      std::pair<std::string, boost::shared_ptr<iota::TcpService> >(server_name,
                                                                   tcp_server));
  return tcp_server;
}

boost::shared_ptr<iota::TcpService> iota::Process::get_tcp_service(
    std::string tcp_server) {
  std::map<std::string, boost::shared_ptr<iota::TcpService> >::iterator i_tcp =
      _tcp_servers.begin();
  boost::shared_ptr<iota::TcpService> tcp_s;
  i_tcp = _tcp_servers.find(tcp_server);
  if (i_tcp != _tcp_servers.end()) {
    tcp_s = i_tcp->second;
  }
  return tcp_s;
}

pion::http::plugin_server_ptr iota::Process::add_http_server(
    std::string server_name, std::string endpoint) {
  // Now only ONE http server
  pion::http::plugin_server_ptr http_server;
  if (_http_servers.size() == 0) {
    if (endpoint.empty()) {
      http_server.reset(new pion::http::plugin_server(_scheduler));
      endpoint = http_server->get_address().to_string();
      endpoint.append(":");
      endpoint.append(
          boost::lexical_cast<std::string>(http_server->get_port()));
    } else {
      boost::asio::ip::tcp::endpoint e = get_endpoint(endpoint);
      http_server.reset(new pion::http::plugin_server(_server_scheduler, e));
    }
    if (server_name.empty()) {
      server_name = endpoint;
    }
    _http_servers.insert(std::pair<std::string, pion::http::plugin_server_ptr>(
        server_name, http_server));
  }
  return http_server;
}

void iota::Process::set_admin_service(iota::AdminService* admin_service) {
  // Add ngsi service, but if manager maybe it is not neccesary.
  // In this version only ONE http server. This loop has only ONE iteration
  std::string adm_service_url(iota::Process::get_url_base());
  add_service(adm_service_url, admin_service);

  iota::NgsiService* ngsi_ptr = new iota::NgsiService();
  std::string url_ngsi_common(iota::Process::get_url_base());
  url_ngsi_common.append("/");
  url_ngsi_common.append(iota::NGSI_SERVICE);
  admin_service->add_service(url_ngsi_common, ngsi_ptr);
  add_service(url_ngsi_common, ngsi_ptr);
  _admin_service = admin_service;
}

iota::AdminService* iota::Process::get_admin_service() {
  // In this version only a http server
  return _admin_service;
}

iota::Process& iota::Process::get_process() {
  if (iota::Process::_process == NULL) {
    iota::Process::initialize("");
  }
  return *iota::Process::_process;
}

boost::asio::ip::tcp::endpoint iota::Process::get_endpoint(
    std::string endpoint) {
  std::size_t endpoint_a = endpoint.find_last_of(':');
  boost::asio::ip::address address =
      boost::asio::ip::address::from_string(endpoint.substr(0, endpoint_a));
  boost::asio::ip::tcp::endpoint e(
      address,
      boost::lexical_cast<unsigned short>(endpoint.substr(endpoint_a + 1)));
  return e;
}

void iota::Process::start() {
  std::map<std::string, boost::shared_ptr<iota::TcpService> >::iterator i_tcp =
      _tcp_servers.begin();
  std::map<std::string, pion::http::plugin_server_ptr>::iterator i_http =
      _http_servers.begin();
  while (i_tcp != _tcp_servers.end()) {
    i_tcp->second->start();
    ++i_tcp;
  }
  while (i_http != _http_servers.end()) {
    i_http->second->start();
    ++i_http;
  }
  _scheduler.startup();
}

void iota::Process::stop() {
  std::map<std::string, boost::shared_ptr<iota::TcpService> >::iterator i_tcp =
      _tcp_servers.begin();
  std::map<std::string, pion::http::plugin_server_ptr>::iterator i_http =
      _http_servers.begin();
  while (i_tcp != _tcp_servers.end()) {
    i_tcp->second->stop();
    ++i_tcp;
  }
  while (i_http != _http_servers.end()) {
    i_http->second->stop();
    ++i_http;
  }
  _scheduler.shutdown();
  _server_scheduler.shutdown();
}

unsigned int iota::Process::get_http_port() {
  return _http_servers.begin()->second->get_port();
}

pion::http::plugin_service* iota::Process::get_service(
    std::string http_resource) {
  return _admin_service->get_service(http_resource);
}

void iota::Process::add_service(std::string http_resource,
                                iota::RestHandle* http_service) {
  std::map<std::string, pion::http::plugin_server_ptr>::iterator i_http =
      _http_servers.begin();
  while (i_http != _http_servers.end()) {
    i_http->second->add_service(http_resource, http_service);
    ++i_http;
  }
}

pion::http::plugin_service* iota::Process::add_tcp_service(
    std::string http_resource, std::string file_name) {
  return _tcp_plugin_manager.load(http_resource, file_name);
}
