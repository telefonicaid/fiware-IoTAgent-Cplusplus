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
#include "util/iota_logger.h"
#include <boost/asio.hpp>
#include <pion/process.hpp>
#include <pion/http/plugin_server.hpp>
#include "services/admin_service.h"
#include "services/ngsi_service.h"
#include "rest/riot_conf.h"
#include "rest/types.h"
#ifdef IOTA_USE_LOG4CPP
#include <log4cpp/Category.hh>
#include <log4cpp/RollingFileAppender.hh>
#include <log4cpp/PatternLayout.hh>
#else
#include <log4cplus/fileappender.h>
#endif
#include "util/common.h"
#include "mongo/client/init.h"
#include "rest/tcp_service.h"
#include "services/admin_mgmt_service.h"

namespace iota {
std::string logger = "main";
std::string URL_BASE = "/iot";
}
iota::AdminService* AdminService_ptr;

// displays an error message if the arguments are invalid
void argument_error(void) {
  std::cerr << "usage:   iotagent [OPTIONS] -f CONFIG_FILE RESOURCE WEBSERVICE" <<
            std::endl
            << "         iotagent [OPTIONS (except -o)] -c SERVICE_CONFIG_FILE" << std::endl
            << "options: [-m] [-i IP] [-p PORT] [-u URL_BASE] [-n IOTAGENT_NAME] [-d PLUGINS_DIR] [-o OPTION=VALUE] [-v LOG_LEVEL]"
            << std::endl;
}

void config_error(const std::string& err) {
  std::cerr << "ERROR" <<  err << std::endl;
}

int main(int argc, char* argv[]) {

  static const unsigned int DEFAULT_PORT = 8080;
  static const std::string  ZERO_IP = "0.0.0.0";

  std::string prov_ip = ZERO_IP;

  mongo::client::initialize();

  // used to keep track of web service name=value options
  typedef std::vector<std::pair<std::string, std::string> >   ServiceOptionsType;
  ServiceOptionsType service_options;
  iota::Configurator* conf_iotagent = NULL;
  // parse command line: determine port number, RESOURCE and WEBSERVICE
  boost::asio::ip::tcp::endpoint cfg_endpoint(boost::asio::ip::tcp::v4(),
      DEFAULT_PORT);
  std::string service_config_file;
  std::string resource_name;
  std::string service_name;
  std::string ssl_pem_file;
  std::string url_base;
  std::string iotagent_name;
  std::string log_level;
  std::string standalone_config_file;
  std::string component_name("iota");
  bool manager = false;
  bool ssl_flag = false;
  bool verbose_flag = false;
  int  port = DEFAULT_PORT;

  for (int argnum=1; argnum < argc; ++argnum) {
    if (argv[argnum][0] == '-') {
      if (argv[argnum][1] == 'p' && argv[argnum][2] == '\0' && argnum+1 < argc) {
        // set port number
        ++argnum;
        cfg_endpoint.port(strtoul(argv[argnum], 0, 10));
        if (cfg_endpoint.port() == 0) {
          cfg_endpoint.port(DEFAULT_PORT);
        }
        port = strtoul(argv[argnum], 0, 10);
        if (port == 0) {
          port = DEFAULT_PORT;
        }
      }
      else if (argv[argnum][1] == 'i' && argv[argnum][2] == '\0' &&
               argnum+1 < argc) {
        // set ip address
        cfg_endpoint.address(boost::asio::ip::address::from_string(ZERO_IP));
        prov_ip = argv[++argnum];
      }
      else if (argv[argnum][1] == 'u' && argv[argnum][2] == '\0'
               && argnum+1 < argc) {
        url_base.assign(argv[++argnum]);
        if (url_base.empty() == false) {
          iota::URL_BASE.assign(url_base);
        }
      }
      else if (argv[argnum][1] == 'f' && argv[argnum][2] == '\0'
               && argnum+1 < argc) {
        standalone_config_file.assign(argv[++argnum]);
      }
      else if (argv[argnum][1] == 'n' && argv[argnum][2] == '\0'
               && argnum+1 < argc) {
        iotagent_name.assign(argv[++argnum]);
      }
      else if (argv[argnum][1] == 'c' && argv[argnum][2] == '\0'
               && argnum+1 < argc) {
        service_config_file = argv[++argnum];
      }
      else if (argv[argnum][1] == 'd' && argv[argnum][2] == '\0'
               && argnum+1 < argc) {
        // add the service plug-ins directory to the search path
        try {
          pion::plugin::add_plugin_directory(argv[++argnum]);
        }
        catch (pion::error::directory_not_found&) {
          std::cerr << "piond: Web service plug-ins directory does not exist: "
                    << argv[argnum] << std::endl;
          return 1;
        }
      }
      else if (argv[argnum][1] == 'o' && argv[argnum][2] == '\0'
               && argnum+1 < argc) {
        std::string option_name(argv[++argnum]);
        std::string::size_type pos = option_name.find('=');
        if (pos == std::string::npos) {
          argument_error();
          return 1;
        }
        std::string option_value(option_name, pos + 1);
        option_name.resize(pos);
        service_options.push_back(std::make_pair(option_name, option_value));
      }
      else if (argv[argnum][1] == 's' && argv[argnum][2] == 's' &&
               argv[argnum][3] == 'l' && argv[argnum][4] == '\0' && argnum+1 < argc) {
        ssl_flag = true;
        ssl_pem_file = argv[++argnum];
      }
      else if (argv[argnum][1] == 'v' && argv[argnum][2] == '\0') {
        verbose_flag = true;
        if (argnum+1 < argc) {
          log_level.assign(argv[++argnum]);
        }
      }
      else if (argv[argnum][1] == 'm' && argv[argnum][2] == '\0') {
        // Start as IoTA Manager
        manager = true;
      }
      else {
        argument_error();
        return 1;
      }
    }
    else if (argnum+2 == argc) {
      // second to last argument = RESOURCE
      resource_name = argv[argnum];
    }
    else if (argnum+1 == argc) {
      // last argument = WEBSERVICE
      service_name = argv[argnum];
    }
    else {
      argument_error();
      return 1;
    }
  }

  if (service_config_file.empty() && (resource_name.empty()
                                      || service_name.empty())) {
    argument_error();
    return 1;
  }

  // Initialization Configurator
  if (!standalone_config_file.empty()) {
    conf_iotagent = iota::Configurator::initialize(standalone_config_file);
  }
  else if (!service_config_file.empty()) {
    conf_iotagent = iota::Configurator::initialize(service_config_file);
  }

  if (conf_iotagent == NULL) {
    config_error("Configuration error. Check configuration file");
    return 1;
  }
  /*
  if (iota::Configurator::instance()->hasError()) {
    config_error(iota::Configurator::instance()->getError());
    //return 1;
  }
  */

  iota::Configurator::instance()->set_listen_port(port);
  iota::Configurator::instance()->set_listen_ip(prov_ip);

  // Path logs
  std::string dir_log("/tmp/");
  try {
    const iota::JsonValue& log_obj = iota::Configurator::instance()->get(
                                       iota::types::CONF_FILE_DIR_LOG.c_str());
    if (log_obj.IsString()) {
      dir_log.assign(log_obj.GetString());
    }
  }
  catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }

  pion::process::initialize();
  pion::logger pion_log(PION_GET_LOGGER("pion"));
  pion::logger main_log(PION_GET_LOGGER("main"));

  std::string log_file(dir_log);
  log_file.append("IoTAgent");
  if (iotagent_name.empty() == false) {
    log_file.append("-");
    log_file.append(iotagent_name);
    component_name.append(":" + iotagent_name);
  }

  log_file.append(".log");

  if (verbose_flag) {
    if (log_level.empty() == true) {
      PION_LOG_SETLEVEL_INFO(pion_log);
      PION_LOG_SETLEVEL_INFO(main_log);
    }
    else if (log_level.compare("FATAL") == 0) {
      PION_LOG_SETLEVEL_FATAL(pion_log);
      PION_LOG_SETLEVEL_FATAL(main_log);
    }
    else if (log_level.compare("ERROR") == 0) {
      PION_LOG_SETLEVEL_ERROR(pion_log);
      PION_LOG_SETLEVEL_ERROR(main_log);
    }
    else if (log_level.compare("WARNING") == 0) {
      PION_LOG_SETLEVEL_ERROR(pion_log);
      PION_LOG_SETLEVEL_WARN(main_log);
    }
    else if (log_level.compare("INFO") == 0) {
      PION_LOG_SETLEVEL_ERROR(pion_log);
      PION_LOG_SETLEVEL_INFO(main_log);
    }
    else if (log_level.compare("DEBUG") == 0) {
      PION_LOG_SETLEVEL_INFO(pion_log);
      PION_LOG_SETLEVEL_DEBUG(main_log);
    }
  }
  else {
    PION_LOG_SETLEVEL_ERROR(pion_log);
    PION_LOG_SETLEVEL_ERROR(main_log);
  }
#ifdef IOTA_USE_LOG4CPP
  log4cpp::Appender* ptrApp = new log4cpp::RollingFileAppender("cppApp",
                                        log_file,
                                        10*1024*1024,
                                        5,
                                        true);
  std::string pattern = "time=%d{%Y-%m-%dT%H:%M:%S,%l%Z} | lvl=%5p | comp=" + component_name + " %m %n";
  log4cpp::Layout *layout = new log4cpp::PatternLayout();
   ((log4cpp::PatternLayout *)layout)->setConversionPattern(pattern);
  ptrApp->setLayout(layout);
  log4cpp::Category::getRoot().setAppender(ptrApp);

#else
  log4cplus::SharedAppenderPtr ptrApp(new log4cplus::RollingFileAppender(
                                        log_file,
                                        10*1024*1024,
                                        5,
                                        true));

  log4cplus::tstring pattern =
    LOG4CPLUS_TEXT("time=%D{%Y-%m-%dT%H:%M:%S,%Q%Z} | lvl=%5p | comp=" + component_name +
                   " %m %n");
  //LOG4CPLUS_TEXT("%-5p %D{%d-%m-%y %H:%M:%S,%Q %Z} [%t][%b] - %m %n");

  ptrApp->setLayout(std::auto_ptr<log4cplus::Layout>(
                      new log4cplus::PatternLayout(pattern)));

  pion_log.addAppender(ptrApp);
  main_log.addAppender(ptrApp);

#endif
  if (iotagent_name.empty() == true) {
    PION_LOG_CONFIG_BASIC;
  }


  try {

    // Directorio de plugins
    // Por si queremos meter un directorios adicionales
    /*
    try {
     std::string plugin_directory(PION_PLUGINS_DIRECTORY);
     IOTA_LOG_DEBUG(pion_log, "Plugins Directory " << plugin_directory);
     pion::plugin::add_plugin_directory(plugin_directory);
    }
    catch (std::exception& e) {
     IOTA_LOG_ERROR(pion_log, pion::diagnostic_information(e));
    }
    */
    pion::one_to_one_scheduler pion_scheduler;
    pion_scheduler.set_num_threads(8);
    if (!manager) {
      IOTA_LOG_INFO(main_log,
                    "======= IoTAgent StartingWebServer: " << cfg_endpoint.address() <<
                    " ========");
    }
    else {
      IOTA_LOG_INFO(main_log,
                    "======= IoTAgent Manager StartingWebServer: " << cfg_endpoint.address() <<
                    " ========");
    }

    pion::http::plugin_server_ptr web_server(new pion::http::plugin_server(
          pion_scheduler,
          cfg_endpoint));

    // Map to store tcp servers
    std::map<boost::asio::ip::tcp::endpoint, pion::tcp::server_ptr> tcp_servers;
    try {
      const iota::JsonValue& tcp_s = iota::Configurator::instance()->get(
                                       iota::types::CONF_FILE_TCP_SERVERS.c_str());
      if (!tcp_s.IsArray()) {
        IOTA_LOG_ERROR(main_log, "ERROR in Config File " << service_config_file <<
                       " Configuration error [tcp_servers]");

      }
      else {

        try {
          for (rapidjson::SizeType it_r = 0; it_r < tcp_s.Size(); it_r++) {

            std::string endpoint(tcp_s[it_r].GetString());
            std::size_t p_points = endpoint.find(':');
            boost::asio::ip::address address = boost::asio::ip::address::from_string(
                                                 endpoint.substr(0, p_points));
            boost::asio::ip::tcp::endpoint e(address,
                                             boost::lexical_cast<unsigned short>(endpoint.substr(p_points+1)));
            IOTA_LOG_DEBUG(main_log, "tcp server: "  << e.address() << ":" << e.port());
            pion::tcp::server_ptr tcp_server(new iota::TcpService(e));
            tcp_servers[e] = tcp_server;
          }
        }
        catch (std::exception& e) {
          IOTA_LOG_FATAL(main_log, e.what());
        }
      }
    }
    catch (std::exception& e) {
      IOTA_LOG_ERROR(main_log, e.what());
    }
    // static service

    if (manager) {
      AdminService_ptr = new iota::AdminManagerService(web_server);
    }
    else {
      AdminService_ptr = new iota::AdminService(web_server);
    }

    AdminService_ptr->set_log_file(log_file);
    // Argument with url-base
    std::string adm_service_url(iota::URL_BASE);
    web_server->add_service(adm_service_url, AdminService_ptr);


    iota::NgsiService* ngsi_ptr = new iota::NgsiService();
    std::string url_ngsi_common(iota::URL_BASE);
    url_ngsi_common.append("/");
    url_ngsi_common.append(iota::NGSI_SERVICE);
    web_server->add_service(url_ngsi_common, ngsi_ptr);
    AdminService_ptr->add_service(url_ngsi_common, ngsi_ptr);


    if (ssl_flag) {
#ifdef PION_HAVE_SSL
      // configure server for SSL
      web_server->set_ssl_key_file(ssl_pem_file);
      PION_LOG_INFO(pion_log, "SSL support enabled using key file: " << ssl_pem_file);
#else
      IOTA_LOG_ERROR(main_log, "SSL support is not enabled");
#endif
    }

    std::string url_complete(iota::URL_BASE);
    if (service_config_file.empty()) {
      // load a single web service using the command line arguments
      // after url base
      url_complete.append("/");
      url_complete.append(resource_name);
      web_server->load_service(url_complete, service_name);

      // set web service options if any are defined
      for (ServiceOptionsType::iterator i = service_options.begin();
           i != service_options.end(); ++i) {
        web_server->set_service_option(url_complete, i->first, i->second);
      }
    }
    else if (!manager) {
      // load services using the configuration file
      try {
        IOTA_LOG_DEBUG(main_log, "Config file " << service_config_file);
        const iota::JsonValue& resources = iota::Configurator::instance()->get(
                                             iota::types::CONF_FILE_RESOURCES.c_str());
        if (!resources.IsArray()) {
          IOTA_LOG_FATAL(main_log, "ERROR in Config File " << service_config_file <<
                         " Configuration error [resources]");
          return 1;
        }


        for (rapidjson::SizeType i = 0; i < resources.Size(); i++) {
          try {
            if (resources[i].HasMember(iota::types::CONF_FILE_RESOURCE.c_str())) {
              std::string res =
                resources[i][iota::types::CONF_FILE_RESOURCE.c_str()].GetString();
              if (resources[i].HasMember(iota::types::CONF_FILE_OPTIONS.c_str())) {
                const iota::JsonValue& options =
                  resources[i][iota::types::CONF_FILE_OPTIONS.c_str()];
                if ((options.HasMember(iota::types::CONF_FILE_FILE_NAME.c_str()))
                    && (options[iota::types::CONF_FILE_FILE_NAME.c_str()].IsString())) {
                  std::string s_n(options[iota::types::CONF_FILE_FILE_NAME.c_str()].GetString());
                  IOTA_LOG_DEBUG(main_log, "Starting___ " << res);
                  web_server->load_service(res, s_n);
                  for (iota::JsonValue::ConstMemberIterator it_r = options.MemberBegin();
                       it_r != options.MemberEnd(); ++it_r) {
                    std::string name(it_r->name.GetString());
                    std::string value(it_r->value.GetString());
                    IOTA_LOG_DEBUG(main_log, "set_service_option: " << name << " " << value);
                    web_server->set_service_option(res, name, value);

                  }

                }

              }
            }
          }
          catch (std::exception& e) {
            IOTA_LOG_FATAL(main_log, e.what());
          }
        }
      }
      catch (std::exception& e) {
        IOTA_LOG_FATAL(main_log, "ERROR in Config File " << service_config_file);
        IOTA_LOG_FATAL(main_log, e.what());
        return 1;
      }
    }

    // Start
    web_server->start();
    std::map<boost::asio::ip::tcp::endpoint, pion::tcp::server_ptr>::iterator it_tcp
      = tcp_servers.begin();
    while (it_tcp != tcp_servers.end()) {
      it_tcp->second->start();
      it_tcp++;
    }

    pion::process::wait_for_shutdown();
    std::cout << "Server shutdown finish " << std::endl;

    //AdminService_ptr->stop();
  }
  catch (std::exception& e) {
    IOTA_LOG_FATAL(pion_log, pion::diagnostic_information(e));
  }
}





