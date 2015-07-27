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
#ifndef SRC_REST_PROCESS_H_
#define SRC_REST_PROCESS_H_

#include "rest/tcp_service.h"
#include "util/iota_logger.h"
#include <boost/noncopyable.hpp>
#include <pion/scheduler.hpp>
#include <pion/http/plugin_server.hpp>

namespace iota {
class AdminService;

class Process: private boost::noncopyable {

  public:

    ~Process() {};
    static iota::Process& initialize(std::string url_base,
                                     unsigned int num_main_threads = 1);
    void shutdown();
    static void wait_for_shutdown();
		pion::scheduler& get_scheduler();
    boost::asio::io_service& get_io_service();
    static std::string& get_logger_name();
    static std::string& get_url_base();
    pion::tcp::server_ptr add_tcp_server(std::string server_name, std::string endpoint);
    pion::http::plugin_server_ptr add_http_server(std::string server_name, std::string endpoint);
    void set_admin_service(iota::AdminService* admin_service);
    iota::AdminService* get_admin_service();
    void start();
    static iota::Process& get_process();
    unsigned int get_http_port();
    pion::http::plugin_service* get_service(std::string http_resource);
    void add_service(std::string http_resource, pion::http::plugin_service* http_service);
  protected:
  private:
    Process(unsigned int n_threads);
		Process(Process const&) {};
		void operator=(Process const&) {};
    pion::one_to_one_scheduler _scheduler;
    static std::string _logger_name;
    static std::string _url_base;
    std::map<std::string, pion::tcp::server_ptr> _tcp_servers;
    std::map<std::string, pion::http::plugin_server_ptr> _http_servers;
    static iota::Process* _process;
    iota::AdminService* _admin_service;
    boost::asio::ip::tcp::endpoint get_endpoint(std::string endpoint);
    boost::mutex _m_mutex;
};
};
#endif
