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
#ifndef SRC_UTIL_ARGUMENTS_H_
#define SRC_UTIL_ARGUMENTS_H_

#include <string>
#include "util/iota_logger.h"


namespace iota {

/**
* A Arguments class. Parser for command line arguments for iota program.
*/
class Arguments {
  public:

    /**
     * @name    parser
     * @brief   read all parameter from argv
     *          puts default value if parameter does not exists
     *
     * @param [in] argc,   number of parameter
     * @param [in] argv,  command line value from main
     * @return  error text or empty if everything is ok
     *
     */
    std::string parser(int argc, char* argv[]);

    /**
     * @name    argument_error
     * @brief   displays an error message if the arguments are invalid
     *
     * @return  error text or empty if everything is ok
     *
     */
    std::string argument_error();

    std::string get_service_config_file(){
      return service_config_file;
    }

    std::string get_resource_name(){
      return resource_name;
    }

    std::string get_service_name(){
      return service_name;
    }

    std::string get_ssl_pem_file(){
      return ssl_pem_file;
    }

    std::string get_url_base(){
      return url_base;
    }

    std::string get_iotagent_name(){
      return iotagent_name;
    }

    std::string get_log_level(){
      return log_level;
    }

    std::string get_standalone_config_file(){
      return standalone_config_file;
    }

    std::string get_component_name(){
      return component_name;
    }

    bool get_manager(){
      return manager;
    }

    bool get_ssl_flag(){
      return ssl_flag;
    }

    bool get_verbose_flag(){
      return verbose_flag;
    }

    int  get_port(){
      return port;
    }

    std::string  get_ZERO_IP(){
      return ZERO_IP;
    }

    unsigned int get_DEFAULT_PORT (){
      return DEFAULT_PORT;
    }

  protected:


  private:

    std::string service_config_file;
    std::string resource_name;
    std::string service_name;
    /** file for ssl */
    std::string ssl_pem_file;
    /** url to start web server where receive orders */
    std::string url_base;
    /** name for iota component */
    std::string iotagent_name;
    /** level for logs, FATAL,ERROR, INFO, WARNING, DEBUG */
    std::string log_level;
    /** */
    std::string standalone_config_file;
    /** name for the component */
    std::string component_name("iota");
    /** true if the program is an iota manager, false if it is an iota agent */
    bool manager = false;
    /** if iota must have a port with ssl */
    bool ssl_flag = false;
    /** level for logs, FATAL,ERROR, INFO, WARNING, DEBUG */
    bool verbose_flag = false;
    /** port where is started the server */
    int  port = DEFAULT_PORT;

    std::string  ZERO_IP = "0.0.0.0";

    unsigned int DEFAULT_PORT =8080;

};
};

#endif
