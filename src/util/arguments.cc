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
#include "arguments.h"

#include <boost/lexical_cast.hpp>
#include "rest/types.h"

iota::Arguments::Arguments() {
  component_name = "iota";
  manager = false;
  ssl_flag = false;
  verbose_flag = false;
  DEFAULT_PORT = 8080;
  port = DEFAULT_PORT;
  ZERO_IP = "0.0.0.0";
  prov_ip = ZERO_IP;
  num_threads = 8;
}

bool iota::Arguments::check_parameter(const char short_param,
                                      const std::string& large_param,
                                      int argnum, int argc, const char* argv[],
                                      bool has_parameter) {
  bool result = false;
  if (argv[argnum][0] == '-') {
    if (argv[argnum][1] == short_param && argv[argnum][2] == '\0') {
      result = true;
    } else if (argv[argnum][1] == '-') {
      result = (large_param.compare(argv[argnum]) == 0);
    }
  }

  if (has_parameter && argnum + 1 >= argc) {
    result = false;
  }

  return result;
}

std::string iota::Arguments::parser(int argc, const char* argv[]) {
  for (int argnum = 1; argnum < argc; ++argnum) {
    if (check_parameter('p', "--port", argnum, argc, argv)) {
      // set port number
      ++argnum;
      port = strtoul(argv[argnum], 0, 10);
      if (port == 0) {
        port = DEFAULT_PORT;
      }
    } else if (check_parameter('i', "--ip", argnum, argc, argv)) {
      prov_ip = argv[++argnum];
    } else if (check_parameter('u', "--url", argnum, argc, argv)) {
      url_base.assign(argv[++argnum]);
    } else if (check_parameter('f', "--file", argnum, argc, argv)) {
      standalone_config_file.assign(argv[++argnum]);
    } else if (check_parameter('4', "--ipv4", argnum, argc, argv, false)) {
      // default ip
      ZERO_IP = "0.0.0.0";
    } else if (check_parameter('6', "--ipv6", argnum, argc, argv, false)) {
      ZERO_IP = "::";
    } else if (check_parameter('n', "--name", argnum, argc, argv)) {
      iotagent_name.assign(argv[++argnum]);
    } else if (check_parameter('c', "--config_file", argnum, argc, argv)) {
      service_config_file = argv[++argnum];
    } else if (check_parameter('d', "--plugins-dir", argnum, argc, argv)) {
      // add the service plug-ins directory to the search path
      plugin_directory = argv[++argnum];
    } else if (check_parameter('o', "--option", argnum, argc, argv)) {
      std::string option_name(argv[++argnum]);
      std::string::size_type pos = option_name.find('=');
      if (pos == std::string::npos) {
        return iota::types::HELP_MESSAGE_ERR_PARAMO + argument_error();
      }
      std::string option_value(option_name, pos + 1);
      option_name.resize(pos);
      service_options.push_back(std::make_pair(option_name, option_value));
    } else if (check_parameter('s', "--ssl", argnum, argc, argv)) {
      ssl_flag = true;
      ssl_pem_file = argv[++argnum];
    } else if (check_parameter('v', "--verbose", argnum, argc, argv)) {
      verbose_flag = true;
      if (argnum + 1 < argc) {
        log_level.assign(argv[++argnum]);
      }
    } else if (check_parameter('m', "--manager", argnum, argc, argv, false)) {
      // Start as IoTA Manager
      manager = true;
    } else if (check_parameter('h', "--help", argnum, argc, argv, false)) {
      // help
      return iota::types::HELP_MESSAGE_OPS + iota::types::HELP_MESSAGE;
    } else if (check_parameter('I', "--identifier", argnum, argc, argv)) {
      // identifier
      identifier.assign(argv[++argnum]);
    } else if (check_parameter('t', "--threads", argnum, argc, argv)) {
      // num threads
      ++argnum;
      num_threads = strtoul(argv[argnum], 0, 10);
      if (num_threads == 0) {
        num_threads = 8;
      }
    } else if (argv[argnum][0] == '-') {
      return iota::types::HELP_MESSAGE_ERR_BAD_PARAM + argument_error();
    } else if (argnum + 2 == argc) {
      // second to last argument = RESOURCE
      resource_name = argv[argnum];
    } else if (argnum + 1 == argc) {
      // last argument = WEBSERVICE
      service_name = argv[argnum];
    } else {
      return iota::types::HELP_MESSAGE_ERR_PARAM + argument_error();
    }
  }

  if (iotagent_name.empty() == false) {
    component_name.append(":" + iotagent_name);
  }

  // check minimun parametes
  if (service_config_file.empty() &&
      (resource_name.empty() || service_name.empty())) {
    return iota::types::HELP_MESSAGE_ERR_CONFIG + argument_error();
  }

  // debe existir el directorio para plugins de pion

  return "";
}

std::string iota::Arguments::argument_error(void) {
  return iota::types::HELP_MESSAGE_OPS;
}

void iota::Arguments::set_service_options(
    pion::http::plugin_server_ptr& web_server) {
  std::string url_complete(get_url_base());
  // load a single web service using the command line arguments
  // after url base
  url_complete.append("/");
  url_complete.append(resource_name);
  web_server->load_service(url_complete, service_name);

  // set web service options if any are defined

  for (std::vector<std::pair<std::string, std::string> >::iterator i =
           service_options.begin();
       i != service_options.end(); ++i) {
    web_server->set_service_option(url_complete, i->first, i->second);
  }
}

std::string iota::Arguments::get_log_file() {
  std::string log_file;
  log_file.append("IoTAgent");
  if (iotagent_name.empty() == false) {
    log_file.append("-");
    log_file.append(iotagent_name);
  }

  log_file.append(".log");

  return log_file;
}
