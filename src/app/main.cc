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

#include <boost/asio.hpp>
#include <rest/process.h>
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
#include "services/admin_mgmt_service.h"

// displays an error message if the arguments are invalid
void argument_error(void) {
  std::cerr << "usage:   iotagent [OPTIONS] -f CONFIG_FILE RESOURCE WEBSERVICE" <<
            std::endl
            << "         iotagent [OPTIONS (except -o)] -c SERVICE_CONFIG_FILE" << std::endl
            << "options: [-m] [–ipv4] [–ipv6] [-i IP] [-p PORT] [-u URL_BASE] [-n IOTAGENT_NAME] [-d PLUGINS_DIR] [-o OPTION=VALUE] [-v LOG_LEVEL]"
            << std::endl;
}

void config_error(const std::string& err) {
  std::cerr << "ERROR" <<  err << std::endl;
}


int main(int argc, char* argv[]) {

  static const unsigned int DEFAULT_PORT = 8080;
  std::string  ZERO_IP = "0.0.0.0";

  std::string prov_ip = ZERO_IP;



  // used to keep track of web service name=value options
  typedef std::vector<std::pair<std::string, std::string> >   ServiceOptionsType;
  ServiceOptionsType service_options;
  iota::Configurator* conf_iotagent = NULL;

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
        port = strtoul(argv[argnum], 0, 10);
        if (port == 0) {
          port = DEFAULT_PORT;
        }
      }
      else if (argv[argnum][1] == 'i' && argv[argnum][2] == '\0' &&
               argnum+1 < argc) {
        // set ip address
        prov_ip = argv[++argnum];
      }
      else if (argv[argnum][1] == 'u' && argv[argnum][2] == '\0'
               && argnum+1 < argc) {
        url_base.assign(argv[++argnum]);
      }
      else if (argv[argnum][1] == 'f' && argv[argnum][2] == '\0'
               && argnum+1 < argc) {
        standalone_config_file.assign(argv[++argnum]);
      }
      else if (argv[argnum][1] == 'i' && argv[argnum][2] == 'p'
            && argv[argnum][3] == 'v' && argv[argnum][4] == '4'
            && argv[argnum][5] == '\0'
               && argnum+1 < argc) {
        // default ip
        ZERO_IP = "0.0.0.0";
      }
      else if (argv[argnum][1] == 'i' && argv[argnum][2] == 'p'
            && argv[argnum][3] == 'v' && argv[argnum][4] == '6'
            && argv[argnum][5] == '\0'
               && argnum+1 < argc) {
        ZERO_IP = "::";
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

  iota::Process& process = iota::Process::initialize(url_base, 8);
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

  pion::logger pion_log(PION_GET_LOGGER("pion"));
  pion::logger main_log(PION_GET_LOGGER(process.get_logger_name()));

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

    std::string endpoint_ws(ZERO_IP);
    endpoint_ws.append(":");
    endpoint_ws.append(boost::lexical_cast<std::string>(port));

    pion::http::plugin_server_ptr http_server = process.add_http_server("", endpoint_ws);

    if (!manager) {
      IOTA_LOG_INFO(main_log,
                    "======= IoTAgent StartingWebServer: " << http_server->get_address() <<
                    " ========");
    }
    else {
      IOTA_LOG_INFO(main_log,
                    "======= IoTAgent Manager StartingWebServer: " << http_server->get_address() <<
                    " ========");
    }

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
            process.add_tcp_server("", endpoint);
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


    // Static service
    iota::AdminService* AdminService_ptr;
    if (manager) {
      AdminService_ptr = new iota::AdminManagerService();
    }
    else {
      AdminService_ptr = new iota::AdminService();
    }

    process.set_admin_service(AdminService_ptr);


    if (ssl_flag) {
#ifdef PION_HAVE_SSL
      // configure server for SSL
      IOTA_LOG_INFO(pion_log, "SSL support enabled using key file: " << ssl_pem_file);
      http_server->set_ssl_key_file(ssl_pem_file);
      /*
			web_server->get_ssl_context_type().set_options(boost::asio::ssl::context::default_workarounds |
			                                               boost::asio::ssl::context::no_sslv2 |
			                                               boost::asio::ssl::context::single_dh_use);
			web_server->get_ssl_context_type().set_verify_mode(boost::asio::ssl::verify_peer |
			                                               boost::asio::ssl::verify_fail_if_no_peer_cert);
      web_server->get_ssl_context_type().use_certificate_file(ssl_pem_file, boost::asio::ssl::context::pem);
      web_server->get_ssl_context_type().use_private_key_file(ssl_pem_file, boost::asio::ssl::context::pem);
      web_server->get_ssl_context_type().load_verify_file("/home/develop/Projects/fiware-IoTAgent-Cplusplus/build/Debug/server.crt");
      */

#else
      IOTA_LOG_ERROR(main_log, "SSL support is not enabled");
#endif
    }

    std::string url_complete(process.get_url_base());
    if (service_config_file.empty()) {
      // load a single web service using the command line arguments
      // after url base
      url_complete.append("/");
      url_complete.append(resource_name);
      http_server->load_service(url_complete, service_name);

      // set web service options if any are defined
      for (ServiceOptionsType::iterator i = service_options.begin();
           i != service_options.end(); ++i) {
        http_server->set_service_option(url_complete, i->first, i->second);
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
                  http_server->load_service(res, s_n);
                  for (iota::JsonValue::ConstMemberIterator it_r = options.MemberBegin();
                       it_r != options.MemberEnd(); ++it_r) {
                    std::string name(it_r->name.GetString());
                    std::string value(it_r->value.GetString());
                    IOTA_LOG_DEBUG(main_log, "set_service_option: " << name << " " << value);
										try {
                    http_server->set_service_option(res, name, value);
										}
										catch(boost::exception& e) {
                      IOTA_LOG_INFO(main_log, "Setting option " << boost::diagnostic_information(e));
										}

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
    process.start();
    iota::Process::wait_for_shutdown();
  }
  catch (std::exception& e) {
    IOTA_LOG_FATAL(pion_log, pion::diagnostic_information(e));
  }
}





