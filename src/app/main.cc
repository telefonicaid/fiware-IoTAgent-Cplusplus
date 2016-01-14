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
#include "util/iot_url.h"
#ifdef IOTA_USE_LOG4CPP
#include <log4cpp/Category.hh>
#include <log4cpp/RollingFileAppender.hh>
#include <log4cpp/PatternLayout.hh>
#else
#include <log4cplus/fileappender.h>
#endif
#include "util/common.h"
#include "util/arguments.h"
#include "services/admin_mgmt_service.h"
#include "version.h"

void config_error(const std::string& err) {
  std::cerr << "ERROR" << err << std::endl;
}

int main(int argc, const char* argv[]) {
  iota::Arguments arguments;

  iota::Configurator* conf_iotagent = NULL;

  std::string error = arguments.parser(argc, argv);
  if (!error.empty()) {
    std::cout << error << std::endl;
    return 1;
  }

  /* Process initialization */
  iota::Process& process = iota::Process::initialize(
      arguments.get_url_base(), arguments.get_num_threads());

  try {
    pion::plugin::add_plugin_directory(arguments.get_plugin_directory());
  } catch(pion::error::directory_not_found& e) {
    std::cerr << "Plugin directory does not exist " << arguments.get_plugin_directory() << std::endl;
    return 1;
  }
  // Initialization Configurator
  if (!arguments.get_standalone_config_file().empty()) {
    conf_iotagent =
        iota::Configurator::initialize(arguments.get_standalone_config_file());
  } else if (!arguments.get_service_config_file().empty()) {
    conf_iotagent =
        iota::Configurator::initialize(arguments.get_service_config_file());
  }

  if (conf_iotagent == NULL) {
    config_error("Configuration error. Check configuration file");
    return 1;
  }

  iota::Configurator::instance()->set_listen_port(arguments.get_port());
  iota::Configurator::instance()->set_listen_ip(arguments.get_prov_ip());

  // add iotagent name
  if (!arguments.get_iotagent_name().empty()) {
    iota::Configurator::instance()->set_iotagent_name(
        arguments.get_iotagent_name());
  }

  // add iotagent identifier
  if (!arguments.get_iotagent_identifier().empty()) {
    iota::Configurator::instance()->set_iotagent_identifier(
        arguments.get_iotagent_identifier());
  }

  // Path logs
  std::string dir_log("/tmp/");
  try {
    const iota::JsonValue& log_obj = iota::Configurator::instance()->get(
        iota::types::CONF_FILE_DIR_LOG.c_str());
    if (log_obj.IsString()) {
      dir_log.assign(log_obj.GetString());
    }
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }

  pion::logger pion_log(PION_GET_LOGGER("pion"));
  pion::logger main_log(PION_GET_LOGGER(process.get_logger_name()));

  std::string log_file(dir_log);
  log_file.append(arguments.get_log_file());

  std::string log_level = arguments.get_log_level();
  if (arguments.get_verbose_flag()) {
    if (log_level.empty() == true) {
      PION_LOG_SETLEVEL_INFO(pion_log);
      PION_LOG_SETLEVEL_INFO(main_log);
    } else if (log_level.compare("FATAL") == 0) {
      PION_LOG_SETLEVEL_FATAL(pion_log);
      PION_LOG_SETLEVEL_FATAL(main_log);
    } else if (log_level.compare("ERROR") == 0) {
      PION_LOG_SETLEVEL_ERROR(pion_log);
      PION_LOG_SETLEVEL_ERROR(main_log);
    } else if (log_level.compare("WARNING") == 0) {
      PION_LOG_SETLEVEL_ERROR(pion_log);
      PION_LOG_SETLEVEL_WARN(main_log);
    } else if (log_level.compare("INFO") == 0) {
      PION_LOG_SETLEVEL_ERROR(pion_log);
      PION_LOG_SETLEVEL_INFO(main_log);
    } else if (log_level.compare("DEBUG") == 0) {
      PION_LOG_SETLEVEL_INFO(pion_log);
      PION_LOG_SETLEVEL_DEBUG(main_log);
    }
  } else {
    PION_LOG_SETLEVEL_ERROR(pion_log);
    PION_LOG_SETLEVEL_ERROR(main_log);
  }
#ifdef IOTA_USE_LOG4CPP
  log4cpp::Appender* ptrApp = new log4cpp::RollingFileAppender(
      "cppApp", log_file, 10 * 1024 * 1024, 5, true);
  std::string pattern = "time=%d{%Y-%m-%dT%H:%M:%S,%l%Z} | lvl=%5p | comp=" +
                        component_name + " %m %n";
  log4cpp::Layout* layout = new log4cpp::PatternLayout();
  ((log4cpp::PatternLayout*)layout)->setConversionPattern(pattern);
  ptrApp->setLayout(layout);
  log4cpp::Category::getRoot().setAppender(ptrApp);

#else
  log4cplus::SharedAppenderPtr ptrApp(
      new log4cplus::RollingFileAppender(log_file, 10 * 1024 * 1024, 5, true));

  log4cplus::tstring pattern =

      LOG4CPLUS_TEXT("time=%D{%Y-%m-%dT%H:%M:%S,%Q%Z} | lvl=%-5p | comp=" +
                     arguments.get_component_name() + " %m %n");
  // LOG4CPLUS_TEXT("%-5p %D{%d-%m-%y %H:%M:%S,%Q %Z} [%t][%b] - %m %n");

  ptrApp->setLayout(
      std::auto_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(pattern)));

  pion_log.addAppender(ptrApp);
  main_log.addAppender(ptrApp);

#endif
  if (arguments.get_iotagent_name().empty() == true) {
    PION_LOG_CONFIG_BASIC;
  }

  try {
    std::string endpoint_http_container(arguments.get_ZERO_IP());
    endpoint_http_container.append(":");
    endpoint_http_container.append(
        boost::lexical_cast<std::string>(arguments.get_port()));

    pion::http::plugin_server_ptr http_server =
        process.add_http_server("", endpoint_http_container);

    if (!arguments.get_manager()) {
      IOTA_LOG_INFO(main_log, "======== IoTAgent StartingWebServer: "
                                  << http_server->get_address() << " ========");
    } else {
      IOTA_LOG_INFO(main_log, "======== IoTAgent Manager StartingWebServer: "
                                  << http_server->get_address() << " ========");
    }
    std::ostringstream ss;
    ss << "Welcome to IoTAgents ";
    ss << " identifier:";
    ss << arguments.get_iotagent_identifier();
    ss << "  ";
    ss << gV_GIT;
    ss << " commit ";
    ss << gV_COMMIT;
    ss << " in ";
    ss << __DATE__;
    ss << " threads: ";
    ss << arguments.get_num_threads();
    IOTA_LOG_INFO(main_log, ss.str());

    // static service
    iota::AdminService* AdminService_ptr;
    if (arguments.get_manager()) {
      AdminService_ptr = new iota::AdminManagerService();
    } else {
      AdminService_ptr = new iota::AdminService();
    }
    process.set_admin_service(AdminService_ptr);

    if (arguments.get_ssl_flag()) {
#ifdef PION_HAVE_SSL
      // configure server for SSL
      IOTA_LOG_INFO(pion_log, "SSL support enabled using key file: "
                                  << arguments.get_ssl_pem_file());
      http_server->set_ssl_key_file(arguments.get_ssl_pem_file());
/*
web_server->get_ssl_context_type().set_options(boost::asio::ssl::context::default_workarounds
|
                                               boost::asio::ssl::context::no_sslv2
|
                                               boost::asio::ssl::context::single_dh_use);
web_server->get_ssl_context_type().set_verify_mode(boost::asio::ssl::verify_peer
|
                                               boost::asio::ssl::verify_fail_if_no_peer_cert);
web_server->get_ssl_context_type().use_certificate_file(ssl_pem_file,
boost::asio::ssl::context::pem);
web_server->get_ssl_context_type().use_private_key_file(ssl_pem_file,
boost::asio::ssl::context::pem);
web_server->get_ssl_context_type().load_verify_file("/home/develop/Projects/fiware-IoTAgent-Cplusplus/build/Debug/server.crt");
*/

#else
      IOTA_LOG_ERROR(main_log, "SSL support is not enabled");
#endif
    }

    if (arguments.get_service_config_file().empty()) {
      arguments.set_service_options(http_server);
    } else if (!arguments.get_manager()) {
      // load services using the configuration file
      try {
        IOTA_LOG_DEBUG(main_log, "Config file "
                                     << arguments.get_service_config_file());
        const iota::JsonValue& resources = iota::Configurator::instance()->get(
            iota::types::CONF_FILE_RESOURCES.c_str());
        if (!resources.IsArray()) {
          IOTA_LOG_FATAL(main_log, "ERROR in Config File "
                                       << arguments.get_service_config_file()
                                       << " Configuration error [resources]");
          return 1;
        }

        for (rapidjson::SizeType i = 0; i < resources.Size(); i++) {
          try {
            if (resources[i].HasMember(
                    iota::types::CONF_FILE_RESOURCE.c_str())) {
              std::string res =
                  resources[i][iota::types::CONF_FILE_RESOURCE.c_str()]
                      .GetString();
              if (resources[i].HasMember(
                      iota::types::CONF_FILE_OPTIONS.c_str())) {
                const iota::JsonValue& options =
                    resources[i][iota::types::CONF_FILE_OPTIONS.c_str()];
                if ((options.HasMember(
                        iota::types::CONF_FILE_FILE_NAME.c_str())) &&
                    (options[iota::types::CONF_FILE_FILE_NAME.c_str()]
                         .IsString())) {
                  std::string s_n(
                      options[iota::types::CONF_FILE_FILE_NAME.c_str()]
                          .GetString());
                  IOTA_LOG_DEBUG(main_log, "Starting___ " << res);
                  // If resource has tcp url, it is a TCP service
                  try {
                    iota::IoTUrl resource_url(res);
                    IOTA_LOG_DEBUG(main_log, res);
                    if (resource_url.getProtocol() == URL_PROTOCOL_TCP) {
                      // Add service tcp
                      std::string str_endpoint =
                          resource_url.getHost() + ":" +
                          boost::lexical_cast<std::string>(
                              resource_url.getPort());
                      IOTA_LOG_DEBUG(main_log, "tcp server: " + str_endpoint);
                      process.add_tcp_server("", str_endpoint);

                      // Creating plugin
                      IOTA_LOG_DEBUG(main_log, res + " " + s_n);
                      pion::http::plugin_service* plugin_ptr =
                          process.add_tcp_service(res, s_n);

                      for (iota::JsonValue::ConstMemberIterator it_r =
                               options.MemberBegin();
                           it_r != options.MemberEnd(); ++it_r) {
                        std::string name(it_r->name.GetString());
                        std::string value(it_r->value.GetString());
                        IOTA_LOG_DEBUG(main_log, "set_service_option: "
                                                     << name << " " << value);
                        try {
                          plugin_ptr->set_option(name, value);

                        } catch (boost::exception& e) {
                          IOTA_LOG_INFO(
                              main_log,
                              "Setting option "
                                  << boost::diagnostic_information(e));
                        }
                      }
                      // Additional option: plugin must know endpoint
                      // registering tcp service
                      plugin_ptr->set_option(
                          "ServerEndpoint",
                          resource_url.getHost() + ":" +
                              boost::lexical_cast<std::string>(
                                  resource_url.getPort()));
                      plugin_ptr->set_resource(resource_url.getPath());
                      plugin_ptr->start();
                    }
                  } catch (std::exception& e) {
                    IOTA_LOG_INFO(
                        main_log,
                        "No tcp address " + pion::diagnostic_information(e));

                    http_server->load_service(res, s_n);
                    for (iota::JsonValue::ConstMemberIterator it_r =
                             options.MemberBegin();
                         it_r != options.MemberEnd(); ++it_r) {
                      std::string name(it_r->name.GetString());
                      std::string value(it_r->value.GetString());
                      IOTA_LOG_DEBUG(main_log, "set_service_option: "
                                                   << name << " " << value);
                      try {
                        http_server->set_service_option(res, name, value);
                      } catch (boost::exception& e) {
                        IOTA_LOG_INFO(main_log,
                                      "Setting option "
                                          << boost::diagnostic_information(e));
                      }
                    }
                  }
                }
              }
            }
          } catch (std::exception& e) {
            IOTA_LOG_FATAL(main_log, e.what());
          }
        }
      } catch (std::exception& e) {
        IOTA_LOG_FATAL(main_log, "ERROR in Config File "
                                     << arguments.get_service_config_file());
        IOTA_LOG_FATAL(main_log, e.what());
        return 1;
      }
    }

    // Start
    process.start();
    iota::Process::wait_for_shutdown();
    process.shutdown();
  } catch (std::exception& e) {
    IOTA_LOG_FATAL(pion_log, pion::diagnostic_information(e));
  }
}
