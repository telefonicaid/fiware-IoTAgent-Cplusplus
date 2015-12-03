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
#include <pion/http/request.hpp>
#include <rest/command_handle.h>
#include <rapidjson/document.h>
#include <boost/date_time/local_time/local_time.hpp>
#include "util/service_collection.h"
#include "util/device_collection.h"

namespace iota {  // begin namespace iota

class AdminService : public iota::RestHandle {
 public:
  AdminService();
  virtual ~AdminService();
  virtual void start();
  virtual void stop();

  virtual void create_collection(
      boost::shared_ptr<iota::ServiceCollection>& col);

  void add_common_urls(std::map<std::string, std::string>& filters);

  void add_oauth_media_filters();
  void check_for_logs();

  void checkIndexes();
  void set_timezone_database(std::string timezone_str);

  boost::posix_time::ptime get_local_time_from_timezone(
      std::string timezone_str);
  boost::local_time::time_zone_ptr get_timezone(std::string region);
  /**
       * @name    getServiceCollectionName
       * @brief   return the name of collection
       *
       * @return if adminservice is manager returns SERVICE_MGMT if not returns
   * SERVICE
       */
  const std::string& getServiceCollectionName();

  void agents(pion::http::request_ptr& http_request_ptr,
              std::map<std::string, std::string>& url_args,
              std::multimap<std::string, std::string>& query_parameters,
              pion::http::response& http_response, std::string& response);

  void agent(pion::http::request_ptr& http_request_ptr,
             std::map<std::string, std::string>& url_args,
             std::multimap<std::string, std::string>& query_parameters,
             pion::http::response& http_response, std::string& response);

  void devices(pion::http::request_ptr& http_request_ptr,
               std::map<std::string, std::string>& url_args,
               std::multimap<std::string, std::string>& query_parameters,
               pion::http::response& http_response, std::string& response);

  void device(pion::http::request_ptr& http_request_ptr,
              std::map<std::string, std::string>& url_args,
              std::multimap<std::string, std::string>& query_parameters,
              pion::http::response& http_response, std::string& response);

  void services(pion::http::request_ptr& http_request_ptr,
                std::map<std::string, std::string>& url_args,
                std::multimap<std::string, std::string>& query_parameters,
                pion::http::response& http_response, std::string& response);

  void service(pion::http::request_ptr& http_request_ptr,
               std::map<std::string, std::string>& url_args,
               std::multimap<std::string, std::string>& query_parameters,
               pion::http::response& http_response, std::string& response);

  void about(pion::http::request_ptr& http_request_ptr,
             std::map<std::string, std::string>& url_args,
             std::multimap<std::string, std::string>& query_parameters,
             pion::http::response& http_response, std::string& response);

  // static iota::RestHandle* get_service(const std::string& resource);
  iota::RestHandle* get_service(const std::string& resource);
  void add_service(const std::string& resource, iota::RestHandle* service_ptr);

  virtual void error_response(pion::http::response& http_response,
                              std::string& buffer,
                              unsigned int status_code = 0);
  int create_response(const unsigned int status_code,
                      const std::string& content,
                      const std::string& error_details,
                      pion::http::response& http_response,
                      std::string& response);

  // pion::http::plugin_server_ptr get_web_server();

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
  virtual int get_all_devices_json(
      const std::string& service, const std::string& service_path, int limit,
      int offset, const std::string& detailed, const std::string& entity,
      pion::http::response& http_response, std::string& response,
      std::string request_id, std::string x_auth_token,
      std::string protocol_filter);

  virtual int get_a_device_json(const std::string& service,
                                const std::string& service_path,
                                const std::string& device_id,
                                pion::http::response& http_response,
                                std::string& response, std::string request_id,
                                std::string x_auth_token,
                                std::string protocol_filter);

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
  virtual int put_device_json(
      const std::string& service, const std::string& service_path,
      const std::string& device_id, const std::string& body,
      pion::http::response& http_response, std::string& response,
      const std::string& x_auth_token = "", const std::string& protocol = "");

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
  virtual int post_device_json(const std::string& service,
                               const std::string& service_path,
                               const std::string& body,
                               pion::http::response& http_response,
                               std::string& response, std::string token = "");

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
  virtual int delete_device_json(const std::string& service,
                                 const std::string& service_path,
                                 const std::string& id_device,
                                 pion::http::response& http_response,
                                 std::string& response, std::string token,
                                 const std::string& protocol = "");

  /**
     * @name    get_device_protocol
     * @brief   return handle if service and service_path has a resource (plugin)
     *          with this protocol
     *
     *
     * Example Request:
     * @code
     *    check_device_protocol("UL20", "s1", "/s1");
     * @endcode
     *
     * <h2>Return values</h2>
     * @return handle of pluin if there is a plugin, NULL  if there ins't
     *
     */
  iota::RestHandle* get_device_protocol(
      const std::string& protocol_name, const std::string& service_name,
      const std::string& service_path,
      const boost::shared_ptr<iota::ServiceCollection>& table);

  /**
     * @name    check_device_commands_duplicate
     * @brief   check if exists another device with same
     *             service, subservice, protocol, entity_name
     *             and a command with same name
     *
     * Example Request:
     * @code
     *    check_device_commands_duplicate("PDI-SMS-REPSOL", "s1", "/s1",
   * insertObj, devTable);
     * @endcode
     *
     * <h2>Return values</h2>
     * @return true if there is not a device.
     *
     */
  bool check_device_commands_duplicate(
      const std::string& protocol_name, const std::string& service_name,
      const std::string& service_path, const mongo::BSONObj& insertObj,
      const boost::shared_ptr<iota::DeviceCollection>& table);

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
      const boost::shared_ptr<iota::ServiceCollection>& table,
      const std::string& service, const std::string& service_path, int limit,
      int offset, const std::string& detailed, const std::string& resource,
      pion::http::response& http_response, std::string& response);

  int get_a_service_json(
      const boost::shared_ptr<iota::ServiceCollection>& table,
      const std::string& service, const std::string& service_path,
      const std::string& device_id, pion::http::response& http_response,
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
  virtual int put_service_json(
      const boost::shared_ptr<iota::ServiceCollection>& table,
      const std::string& service, const std::string& service_path,
      const std::string& service_id, const std::string& apikey,
      const std::string& resource, const std::string& body,
      pion::http::response& http_response, std::string& response,
      std::string token, std::string request_dientifier);

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
  virtual int post_service_json(
      const boost::shared_ptr<iota::ServiceCollection>& table,
      const std::string& service, const std::string& service_path,
      const std::string& body, pion::http::response& http_response,
      std::string& response, std::string token, std::string request_identifier);

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
  virtual int delete_service_json(
      const boost::shared_ptr<iota::ServiceCollection>& table,
      const std::string& service, const std::string& service_path,
      const std::string& id_service, const std::string& apikey,
      const std::string& resource, bool remove_devices,
      pion::http::response& http_response, std::string& response,
      std::string x_auth_token, std::string request_identifier);

  /**
    * check inf mongois configurated in config.json
    * avoid to call API Rest with no mongodb typw in config
    */
  void check_mongo_config();

  void set_log_file(std::string& log_file);

  void set_register_retries(bool enable);

  iota::ProtocolData get_protocol_data();

  void check_existing_resource(const std::string& resource);

 protected:
  virtual std::string get_class_name();
  virtual std::string get_role() { return ""; };

  void read_schema(std::string file_name, std::string& schema);

  bool validate_json_schema(const std::string& json_str, std::string& schema,
                            std::string& response);

  static std::string _PUT_DEVICE_SCHEMA;
  static std::string _POST_DEVICE_SCHEMA;
  static std::string _PUT_SERVICE_SCHEMA;
  static std::string _POST_SERVICE_SCHEMA;

  void timeout_register_iota_manager(const boost::system::error_code& ec);

 private:
  virtual void check_required_put_parameters(
      std::multimap<std::string, std::string>& query_parameters){};

  void remove_from_cache(Device& device);
  void check_uri(const std::string& data);

  // API holders
  static const std::string _api_service_holder;
  static const std::string _api_device_holder;
  pion::logger m_log;

  boost::mutex m_sm;

  // Store we server in weak pointer
  // pion::http::plugin_server_ptr _web_server;
  // boost::weak_ptr<pion::http::plugin_server> _web_server;

  std::map<std::string, iota::RestHandle*> _service_manager;
  // Timezones
  boost::local_time::tz_database _timezone_database;

  void start_plugin(std::string& resource, std::string& plugin_name);
  void get_info_agent(iota::RestHandle* agent,
                      std::multimap<std::string, std::string>& query_parameters,
                      JsonDocument& d);
  void get_object_json(const std::string& obj_str, JsonValue& obj,
                       JsonDocument& d);
  std::string conf_global_adm(
      pion::http::request_ptr& http_request_ptr,
      std::map<std::string, std::string>& url_args,
      std::multimap<std::string, std::string>& query_parameters,
      pion::http::response& http_response, std::string& response);

  std::string check_json(const std::string& json_str, JsonDocument& doc);

  virtual std::string get_param_resource(
      const std::multimap<std::string, std::string>& query_parameters,
      bool mandatory);

  std::string get_service_json(
      const boost::shared_ptr<iota::ServiceCollection>& table,
      const std::string& service, const std::string& service_path);

  bool is_mongo_active();

  std::string joinCommands(const std::string &obj1,
                           const std::string &obj2);

  void deploy_device(Device& device);
  void undeploy_device(const boost::shared_ptr<Device> device);

  void register_iota_manager();
  void print_services();

  // Timer for checking logs
  static const unsigned short TIME_FOR_LOG;
  static const unsigned short TIME_TO_LOG;
  void timeout_check_logs(const boost::system::error_code& ec);
  void check_logs();
  boost::shared_ptr<boost::asio::deadline_timer> _timer;
  std::string _log_file;

  std::string _class_name;

  unsigned short _timeout_retries;
  bool retries_set;
  boost::shared_ptr<boost::asio::deadline_timer> _timer_register;
};

}  // end namespace iota

#endif
