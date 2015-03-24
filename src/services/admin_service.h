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
#ifndef SRC_SERVICES_ADMIN_SERVICE_H_
#define SRC_SERVICES_ADMIN_SERVICE_H_

#include <pion/http/plugin_service.hpp>
#include <pion/http/plugin_server.hpp>
#include <rest/command_handle.h>
#include <rapidjson/document.h>
#include <boost/date_time/local_time/local_time.hpp>


namespace iota {        // begin namespace iota

class AdminService :
  public iota::RestHandle {
  public:
    AdminService(pion::http::plugin_server_ptr web_server);
    AdminService();
    virtual ~AdminService();
    virtual void start();
    virtual void stop();
    void set_timezone_database(std::string timezone_str);
    boost::posix_time::ptime get_local_time_from_timezone(std::string timezone_str);

    void agents(pion::http::request_ptr& http_request_ptr,
                std::map<std::string, std::string>& url_args,
                std::multimap<std::string, std::string>& query_parameters,
                pion::http::response& http_response,
                std::string& response);

    void agent(pion::http::request_ptr& http_request_ptr,
               std::map<std::string, std::string>& url_args,
               std::multimap<std::string, std::string>& query_parameters,
               pion::http::response& http_response,
               std::string& response);

    void devices(pion::http::request_ptr& http_request_ptr,
                 std::map<std::string, std::string>& url_args,
                 std::multimap<std::string, std::string>& query_parameters,
                 pion::http::response& http_response,
                 std::string& response);

    void device(pion::http::request_ptr& http_request_ptr,
                std::map<std::string, std::string>& url_args,
                std::multimap<std::string, std::string>& query_parameters,
                pion::http::response& http_response,
                std::string& response);

    void services(pion::http::request_ptr& http_request_ptr,
                  std::map<std::string, std::string>& url_args,
                  std::multimap<std::string, std::string>& query_parameters,
                  pion::http::response& http_response,
                  std::string& response);

    void service(pion::http::request_ptr& http_request_ptr,
                 std::map<std::string, std::string>& url_args,
                 std::multimap<std::string, std::string>& query_parameters,
                 pion::http::response& http_response,
                 std::string& response);

    void about(pion::http::request_ptr& http_request_ptr,
               std::map<std::string, std::string>& url_args,
               std::multimap<std::string, std::string>& query_parameters,
               pion::http::response& http_response,
               std::string& response);

    // static iota::RestHandle* get_service(const std::string& resource);
    iota::RestHandle* get_service(const std::string& resource);
    void add_service(const std::string& resource,
                     iota::RestHandle* service_ptr);

    virtual void error_response(pion::http::response& http_response,
                                std::string& buffer,
                                unsigned int status_code = 0);
    int create_response(
      const unsigned int status_code,
      const std::string& content,
      const std::string& error_details,
      pion::http::response& http_response,
      std::string& response);

    pion::http::plugin_server_ptr get_web_server();

    /**
         * @name    get_all_json
         * @brief   API Rest method POST to insert a device (using json)
         *
         *
         *
         * Example Request:
         * @code
         *    GET /iot/devices/device1?limit=10&skip=1&limit=100 HTTP/1.1
         * @endcode
         *
         * <h2>Return values</h2>
         * @param [out] sequence  id sequence from updateContext .
         *
         * Example response:
         * @code
         *    Content-Type: application/json
         *    Status: 200
         * @endcode
         */
    int get_all_devices_json(
      const std::string& service,
      const std::string& service_path,
      int limit,
      int offset,
      const std::string& detailed,
      const std::string& entity,
      pion::http::response& http_response,
      std::string& response);


    int get_a_device_json(
      const std::string& service,
      const std::string& service_path,
      const std::string& device_id,
      pion::http::response& http_response,
      std::string& response);

    /**
      * @name    put_json
      * @brief   API Rest method PUT modify a device (using json)
      *
      *
      * @param [in] deviceID  name of device.
      *
      *
      * Example Request:
      * @code
      *   PUT /iot/devices/device1 HTTP/1.1
      *    Content-Type: application/json
      *    Fiware-service:service1
      *    Fiware-servicePath:/
      *
      *  {"entity_type": "entity_type4"}
      * @endcode
      *
      * @param [out] sequence  id sequence from updateContext .
      *
      * Example response:
      * @code
      *    Content-Type: application/json
      *    Status: 200
      * @endcode
      */
    int put_device_json(
      const std::string& service,
      const std::string& service_path,
      const std::string& device_id,
      const std::string& body,
      pion::http::response& http_response,
      std::string& response);

    /**
       * @name    post_json
       * @brief   API Rest method POST to insert a device (using json)
       *
       *
       * Example Request:
       * @code
       *    POST /iot/devices HTTP/1.1
       *    Content-Type: application/json
       *    Fiware-service:service1
       *    Fiware-servicePath:/
       *
       * @endcode
       *
       * @param [out] sequence  id sequence from updateContext .
       *
       * Example response:
       * @code
       *    Content-Type: application/json
       *    Status: 200
       * @endcode
       */
    int post_device_json(
      const std::string& service,
      const std::string& service_path,
      const std::string& body,
      pion::http::response& http_response,
      std::string& response);

    /**
       * @name    post_json
       * @brief   API Rest method POST to insert a device (using json)
       *
       *
       *
       * Example Request:
       * @code
       *    updateCommand("ping", "22", dev, "2345fefe4343Ŕ", service);
       * @endcode
       *
       * <h2>Return values</h2>
       * @param [out] sequence  id sequence from updateContext .
       *
       * Example response:
       * @code
       *    updateCommand("ping", "22", dev, "2345fefe4343Ŕ", service);
       * @endcode
       */
    int delete_device_json(
      const std::string& service,
      const std::string& service_path,
      const std::string& id_device,
      pion::http::response& http_response,
      std::string& response);

    /**
         * @name    get_all_json
         * @brief   API Rest method POST to insert a device (using json)
         *
         *
         *
         * Example Request:
         * @code
         *    GET /iot/devices/device1?limit=10&skip=1&limit=100 HTTP/1.1
         * @endcode
         *
         * <h2>Return values</h2>
         * @param [out] sequence  id sequence from updateContext .
         *
         * Example response:
         * @code
         *    Content-Type: application/json
         *    Status: 200
         * @endcode
         */
    int get_all_services_json(
      const std::string& service,
      const std::string& service_path,
      int limit,
      int offset,
      const std::string& detailed,
      const std::string& resource,
      pion::http::response& http_response,
      std::string& response);


    int get_a_service_json(
      const std::string& service,
      const std::string& service_path,
      const std::string& device_id,
      pion::http::response& http_response,
      std::string& response);

    /**
      * @name    put_json
      * @brief   API Rest method PUT modify a device (using json)
      *
      *
      * @param [in] deviceID  name of device.
      *
      *
      * Example Request:
      * @code
      *   PUT /iot/devices/device1 HTTP/1.1
      *    Content-Type: application/json
      *    Fiware-service:service1
      *    Fiware-servicePath:/
      *
      *  {"entity_type": "entity_type4"}
      * @endcode
      *
      * @param [out] sequence  id sequence from updateContext .
      *
      * Example response:
      * @code
      *    Content-Type: application/json
      *    Status: 200
      * @endcode
      */
    int put_service_json(
      const std::string& service,
      const std::string& service_path,
      const std::string& service_id,
      const std::string& apikey,
      const std::string& resource,
      const std::string& body,
      pion::http::response& http_response,
      std::string& response);

    /**
       * @name    post_json
       * @brief   API Rest method POST to insert a device (using json)
       *
       *
       * Example Request:
       * @code
       *    POST /iot/devices HTTP/1.1
       *    Content-Type: application/json
       *    Fiware-service:service1
       *    Fiware-servicePath:/
       *
       * @endcode
       *
       * @param [out] sequence  id sequence from updateContext .
       *
       * Example response:
       * @code
       *    Content-Type: application/json
       *    Status: 200
       * @endcode
       */
    int post_service_json(
      const std::string& service,
      const std::string& service_path,
      const std::string& body,
      pion::http::response& http_response,
      std::string& response);

    /**
       * @name    post_json
       * @brief   API Rest method POST to insert a device (using json)
       *
       *
       *
       * Example Request:
       * @code
       *    updateCommand("ping", "22", dev, "2345fefe4343Ŕ", service);
       * @endcode
       *
       * <h2>Return values</h2>
       * @param [out] sequence  id sequence from updateContext .
       *
       * Example response:
       * @code
       *    updateCommand("ping", "22", dev, "2345fefe4343Ŕ", service);
       * @endcode
       */
    int delete_service_json(
      const std::string& service,
      const std::string& service_path,
      const std::string& id_service,
      const std::string& apikey,
      const std::string& resource,
      bool remove_devices,
      pion::http::response& http_response,
      std::string& response);

    /**
      * check inf mongois configurated in config.json
      * avoid to call API Rest with no mongodb typw in config
      */
    void  check_mongo_config();

    void set_log_file(std::string& log_file);

  private:

    void remove_from_cache(Device &device);
    void check_uri(const std::string& data);

    // API holders
    static const std::string _api_service_holder;
    static const std::string _api_device_holder;
    pion::logger m_log;

    boost::mutex m_sm;

    // Store we server in weak pointer
    //pion::http::plugin_server_ptr _web_server;
    boost::weak_ptr<pion::http::plugin_server> _web_server;

    std::map<std::string, iota::RestHandle*> _service_manager;
    // Timezones
    boost::local_time::tz_database _timezone_database;

    void start_plugin(std::string& resource, std::string& plugin_name);
    void get_info_agent(iota::RestHandle* agent,
                        std::multimap<std::string, std::string>& query_parameters,
                        JsonDocument& d);
    void get_object_json(const std::string& obj_str,
                         JsonValue& obj,
                         JsonDocument& d);
    std::string conf_global_adm(pion::http::request_ptr& http_request_ptr,
                                std::map<std::string, std::string>& url_args,
                                std::multimap<std::string, std::string>& query_parameters,
                                pion::http::response& http_response,
                                std::string& response);

    std::string check_json(const std::string& json_str, JsonDocument& doc);

    std::string get_service_json(
      const std::string& service,
      const std::string& service_path);


    bool is_mongo_active();
    bool validate_json_schema(
      const std::string& json_str,
      const std::string& table,
      const std::string& method,
      std::string& response);

    void deploy_device(Device& device);
    void print_services();

    // Timer for checking logs
    static const unsigned short TIME_FOR_LOG;
    static const unsigned short TIME_TO_LOG;
    void timeout_check_logs(const boost::system::error_code& ec);
    void check_logs();
    boost::shared_ptr<boost::asio::deadline_timer> _timer;
    std::string _log_file;
};

}   // end namespace iota

#endif

