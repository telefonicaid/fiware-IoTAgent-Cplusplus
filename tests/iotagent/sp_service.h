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
#ifndef SRC_TESTS_IOTAGENT_SPSERVICE_H_
#define SRC_TESTS_IOTAGENT_SPSERVICE_H_

#include <pion/http/plugin_service.hpp>
#include <rest/rest_handle.h>
#include <rest/command_handle.h>
#include <ngsi/ContextElement.h>
#include <boost/property_tree/ptree.hpp>
#include <ngsi/RegisterContext.h>
#include <util/CommandResponse.h>
#include <util/http_client.h>
#include <util/timer.h>


namespace iota {        // begin namespace iota

class SPService :
  public iota::CommandHandle,
  public virtual Timer {
  public:
    SPService();
    virtual ~SPService();
    void start();

    void op_ngsi(pion::http::request_ptr& http_request_ptr,
                 std::map<std::string, std::string>& url_args,
                 std::multimap<std::string, std::string>& query_parameters,
                 pion::http::response& http_response, std::string& response);

    void service(pion::http::request_ptr& http_request_ptr,
                 std::map<std::string, std::string>& url_args,
                 std::multimap<std::string, std::string>& query_parameters,
                 pion::http::response& http_response,
                 std::string& response);

    /**
     * @name    isCommandResp
     * @brief   information from one command, recieved in updateConext
     *
     * This API provides certain actions as an example.
     *
     * @param [in] str_command_resp  body http from device communication.
     * @param [in] cmd_status   httpcode send by device in this communication.
     * @param [out] command_response  tranlate response from body .
     * @param [out] id_command  id for the command, to look for in cache commands .
     *
     * @return  -1,  if it is nos a command
     *          200,  if it is a command
     * Example Usage:
     * @code
     *    std::string command_response;
     *    std::string id_command;
     *    isCommandResp("dev1@ping|pingOK", 200, command_response, id_command);
     *    cout << command_response;  // pingOK
     *    cout << id_command;  // dev1@ping
     * @endcode
     */
    int isCommandResp(const std::string& str_command_resp,
                      const int& cmd_status,
                      std::string& command_response,
                      std::string& id_command);

    int execute_command(const std::string& endpoint,
                                const std::string& command_id,
                                const boost::property_tree::ptree& command_to_send,
                                int timeout,
                                const boost::shared_ptr<Device> &item_dev,
                                const boost::property_tree::ptree& service,
                                std::string& response,
                                iota::HttpClient::application_callback_t callback = NULL);


    void make_registrations(void);

    std::string getMyIP();

    int sendHTTP(const std::string& endpoint, int port,
                 const std::string& operation,
                 const std::string& path,
                 const std::string& proxy_server, int timeout,
                 bool ssl, const std::string& query,
                 const std::string& message, const std::string& outgoing_route,
                 std::string& response);

    void send_all_registrations(void);

    void send_optional_registration(std::string device, std::string service);

    int get_duration_seconds(std::string data);

  private:

    std::string get_id_command(std::string str_command);

    std::string get_ngsi_operation(const std::string& operation);


    pion::logger m_logger;
    int  _reg_timeout;

};


}   // end namespace iota

#endif

