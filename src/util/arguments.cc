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
#include "argument.h"
#include <boost/lexical_cast.hpp>


namespace iota {
extern std::string logger;
}

iota::Arguments::Arguments() {
}

void iota::Arguments::parser(int argc, char* argv[]) {

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
          return argument_error();
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
        return argument_error();
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
      return argument_error();
    }
  }

  if (service_config_file.empty() && (resource_name.empty()
                                      || service_name.empty())) {
    argument_error();
    return 1;
  }

  return "";
}

void iota::Arguments::argument_error(void) {
  std::string result("usage:   iotagent [OPTIONS] -f CONFIG_FILE RESOURCE WEBSERVICE" );
  result.append("\n");
  result.append("         iotagent [OPTIONS (except -o)] -c SERVICE_CONFIG_FILE");
  result.append("\n");
  result.append("options: [-m] [–ipv4] [–ipv6] [-i IP] [-p PORT] [-u URL_BASE] [-n IOTAGENT_NAME] [-d PLUGINS_DIR] [-o OPTION=VALUE] [-v LOG_LEVEL]");
  result.append("\n");
}




