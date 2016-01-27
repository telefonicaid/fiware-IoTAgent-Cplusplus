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
#ifndef SRC_REST_REST_HANDLE_H_
#define SRC_REST_REST_HANDLE_H_

#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include "rest/process.h"
#include <pion/http/plugin_service.hpp>
#include <pion/http/response_writer.hpp>
#include "util/iota_logger.h"
#include "http_filter.h"
#include "riot_conf.h"
#include "iot_statistic.h"
#include <util/CommandResponse.h>
#include <util/cache.h>
#include <util/device.h>
#include "util/http_client.h"
#include <ngsi/Attribute.h>

#include <util/async_comm.h>
#include <mongo/client/dbclient.h>

#include <util/common.h>

#define REST_HANDLE(x) static_cast<iota::RestHandle::HandleFunction_t>(x)

namespace iota {

const std::string NGSI_SERVICE = "ngsi";
const std::string ADMIN_SERVICE_AGENTS = "/agents";
const std::string ADMIN_SERVICE_DEVICES = "/devices";
const std::string ADMIN_SERVICE_SERVICES = "/services";
const std::string ADMIN_SERVICE_ABOUT = "/about";
const std::string ADMIN_SERVICE_PROTOCOLS = "/protocols";

typedef boost::function<void(pion::http::request_ptr&,
                             std::map<std::string, std::string>&,
                             std::multimap<std::string, std::string>&,
                             pion::http::response&, std::string&)> handle_t;

class RestHandle : public pion::http::plugin_service,
                   public boost::enable_shared_from_this<RestHandle> {
 public:
  typedef void (iota::RestHandle::*HandleFunction_t)(
      pion::http::request_ptr&, std::map<std::string, std::string>&,
      std::multimap<std::string, std::string>&, pion::http::response&,
      std::string&);

  RestHandle();
  virtual ~RestHandle();
  virtual void set_option(const std::string& name, const std::string& value);
  virtual void operator()(pion::http::request_ptr& http_request_ptr,
                          pion::tcp::connection_ptr& tcp_conn);
  void handle_request(pion::http::request_ptr& http_request_ptr,
                      pion::tcp::connection_ptr& tcp_conn, int status = 200);
  void handle_end_filters(pion::http::request_ptr& http_request_ptr,
                          pion::tcp::connection_ptr& tcp_conn, int num_filter,
                          int status = 200);

  std::string add_url(std::string url,
                      std::map<std::string, std::string>& filters,
                      iota::RestHandle::HandleFunction_t handle,
                      iota::RestHandle* context);
  bool add_pre_filter(boost::shared_ptr<iota::HTTPFilter> filter);
  bool add_post_filter(boost::shared_ptr<iota::HTTPFilter> filter);
  virtual void error_response(pion::http::response& http_response,
                              std::string reason, std::string details,
                              std::string& buffer,
                              unsigned int status_code = 0);
  std::string get_statistics();
  boost::shared_ptr<IoTStatistic> add_statistic_counter(
      const std::string& name_stat, bool enable);
  boost::shared_ptr<IoTStatistic> get_statistic_counter(
      const std::string& name_stat);
  void reset_counters();

  const boost::shared_ptr<Device> get_device_empty_service_path(
      boost::shared_ptr<Device> itemQ);

  /**
     * @name    getDevice
     * @brief   get Device from cache, check active  and autocomplete, first
   * look for entity
     *          if ot founded  look for device
     *
     *
     * @param [in] name  device name.
     * @param [in] service  service name.
     *
     * @return device,  all data device
     *
     * Example Usage:
     * @code
     *     boost::shared_ptr<Device> dev = getDevice("room", "service1");
     * @endcode
     */
  const boost::shared_ptr<Device> get_device(
      const std::string& name, const std::string& service = std::string(),
      const std::string& service_path = std::string());

  const boost::shared_ptr<Device> get_device_by_entity(
      const std::string& name, const std::string& entity_type,
      const std::string& service = std::string(),
      const std::string& service_path = std::string());

  bool get_service_by_apiKey(boost::property_tree::ptree& pt_cb,
                             const std::string& apiKey);

  bool get_service_by_name(boost::property_tree::ptree& pt_cb,
                           const std::string& name = std::string(),
                           const std::string& service_path = std::string());

  std::string get_my_url_base();
  void set_my_url_base(std::string st);

  /**
     * @name    send_update_context
     * @brief   send an updateContext to context broker  with attributes
     *
     *
     * @param [in] apikey,  apikey that identifies a service, if no exists throw
   * exception.
     * @param [in] device_id, device name.
     * @param [in] attributes, vector witjh all attributes to send.
     *
     *
     * Example Usage:
     * @code
     *     send_update_context("apikey", "device", attributes);
     * @endcode
     */
  void send_update_context(const std::string& apikey,
                           const std::string& device_id,
                           const std::vector<iota::Attribute>& attributes);

  void remove_devices_from_cache(Device& device);

  /**
  * @name get_protocol_data
  * @brief get information about a resource implementing a protocol. This
  *function must be implemented
  *        for every modules if it want manager knows module.
  *
  * @return a struct with protocol and description
  **/
  virtual iota::ProtocolData get_protocol_data();

 /**
  * @name get_protocol_commands
  * @brief some protocols have special commands for all devices
  *        post_device_json call this function when create a device
  *
  * @return "[{"name": "attr_1_dev_1", "type": "command", "value": ""},... ]"
  **/
  virtual std::string get_protocol_commands();

  virtual void complete_info(boost::property_tree::ptree& pt);

  virtual int check_provisioned_data(const std::string& data, std::string& error_str);

 /**
  * @name get_timeout_commands
  * @brief some protocols have special timeout for commands
  *        by default it it config.json  timeout
  *
  * @return timeout
  **/
  virtual int get_timeout_commands();

  /**
  * @name get_iota_manager_endpoint
  * @brief get manager endpoint to register protocols.
  * @return endpoint of iota manager
  */
  std::string get_iota_manager_endpoint();

  /**
  * @name set_iota_manager_endpoint
  * @brief set manager endpoint to register protocols
  *
  * @param [in] manager endpoint as URL
  */
  void set_iota_manager_endpoint(std::string manager_endpoint);

  /**
  * @name register_iota_manager
  * @brief send services and protocols to iota manager if defined
  */
  void register_iota_manager();

  /**
   * @name get_public_ip
   * @brief return public ip (command line parameter  -i or --ip )
   *        this information is sent to the manager to use that IP to
   * communicate with us
   *
   * @result identifier + ":" + port
  */
  std::string get_public_ip();

  /**
   * @name get_iotagent_identifier
   * @brief read iotagent identifier for iota manager, look it in
   *        first,  command line parameter -I or --identifier,
   *        second, config.json value identifier
   *        third,  command line parameter -n or --name
   *        last,   ip
   *
   * @result identifier + ":" + port
   */
  std::string get_iotagent_identifier();

  void update_endpoint_device(const boost::shared_ptr<Device>& dev,
                              const std::string& new_endpoint);

  void fill_service_with_bson(const mongo::BSONObj& bson,
                              boost::property_tree::ptree& pt);

 protected:
  std::string remove_url_base(std::string url);
  void register_plugin();
  double get_payload_length(pion::http::request_ptr& http_request_ptr);
  double get_query_length(pion::http::request_ptr& http_request_ptr);
  double get_payload_length(pion::http::response& http_response);
  void set_protocol_data(iota::ProtocolData p_data);
  std::map<std::string, std::string> get_multipart_content(
      pion::http::request_ptr& request_ptr);

  std::string _manager_endpoint;

 private:
  /**
    * search the service in database using apiKey
    * if there is more than one throw excepciton
    **/
  int get_service_by_apiKey_bbdd(boost::property_tree::ptree& pt_cb,
                                 const std::string& apiKey);

  /**
     * search the service in database using name and service_path
     * if service_path is empty only use name
     * if there is more than one throw excepciton
     **/
  int get_service_by_name_bbdd(boost::property_tree::ptree& pt_cb,
                               const std::string& name,
                               const std::string& service_path = std::string());

  /**
     * search the service in config.file using apiKey
     * if service_path is empty only use name
     * if there is more than one throw excepciton
     **/
  const JsonValue& get_service_by_apiKey_file(
      boost::property_tree::ptree& pt_cb, const std::string& apiKey);

  /**
     * search the service in config.file using name and service_path
     * if service_path is empty only use name
     * if there is more than one throw excepciton
     **/
  const JsonValue& get_service_by_name_file(
      boost::property_tree::ptree& pt_cb, const std::string& name,
      const std::string& service_path = std::string());

  /**
  * Get default context broker
  */
  std::string get_default_context_broker();

  /**
  *
  */
  std::string get_http_proxy();

  /**
  * Get default timeout
  */
  int get_default_timeout();

  void receive_event_from_manager(
      boost::shared_ptr<iota::HttpClient> connection,
      pion::http::response_ptr response_ptr,
      const boost::system::error_code& error);

  void finish(pion::tcp::connection_ptr& tcp_conn);
  void clear_buffer(pion::tcp::connection_ptr& tcp_conn);
  std::string& create_buffer(pion::tcp::connection_ptr& tcp_conn);

  void execute_filters(pion::http::request_ptr& http_request_ptr,
                       pion::tcp::connection_ptr& tcp_conn, int num_filter,
                       int status = 200);

  void send_http_response(pion::http::response_writer_ptr& writer,
                          std::string& response_buffer);
  struct ResourceHandler {
    std::string url_regex;
    std::vector<std::string> url_placeholders;
    iota::handle_t rest_function;
  };
  std::map<std::string, ResourceHandler> _handlers;
  std::vector<boost::shared_ptr<iota::HTTPFilter> > _pre_filters;
  std::vector<boost::shared_ptr<iota::HTTPFilter> > _post_filters;

  // Buffers to answers (async)
  std::map<pion::tcp::connection_ptr, std::string> _async_buffers;

  // Lock buffer map
  boost::mutex m_mutex;

  // lockf statistic vector
  boost::mutex m_mutex_stat;

  pion::logger m_logger;

  std::map<std::string, boost::shared_ptr<IoTStatistic> > _statistics;

  // Only internal stats
  bool _enabled_stats;

  std::string _my_url_base;

 protected:
  // type for storage
  std::string _storage_type;

  // cache with registered devices
  iota::Cache registeredDevices;

  // Protocol data
  iota::ProtocolData _protocol_data;
};

}  // end namespace iota

#endif
