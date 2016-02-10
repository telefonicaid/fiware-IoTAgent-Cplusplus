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
#include "rest_handle.h"
#include "rest_functions.h"
#include "riot_conf.h"
#include "iot_duration.h"
#include "iot_stat_value.h"
#include "types.h"
#include "services/admin_service.h"
#include "util/dev_file.h"
#include "util/FuncUtil.h"
#include "util/iota_exception.h"
#include "util/iot_url.h"
#include <boost/bind.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include "util/device_collection.h"
#include "util/service_collection.h"
#include "util/service_mgmt_collection.h"
#include "util/alarm.h"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <ngsi/ContextElement.h>
#include <ngsi/UpdateContext.h>
#include "util/RiotISO8601.h"
#include "util/json_util.h"
#include <algorithm>

#define ERROR_MORE_THAN_ONE -100
#define ERROR_NO_SERVICE -404
#define ERROR_SERVICE_OK 0

boost::shared_ptr<iota::Device> get_func(boost::shared_ptr<iota::Device> item) {
  boost::shared_ptr<iota::Device> resu;

  try {
    iota::DeviceCollection dev_table;
    dev_table.findd(*item);

    if (dev_table.more()) {
      iota::Device aux = dev_table.nextd();
      resu.reset(new iota::Device(aux));
    }
  } catch (...) {
    // ERROR
  }
  return resu;
}

iota::RestHandle::RestHandle()
    : _enabled_stats(true),
      m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())),
      registeredDevices(iota::types::MAX_SIZE_CACHE, false),
      _manager_endpoint("") {
  IOTA_LOG_DEBUG(m_logger, "RestHandle constructor");
  std::string devices_store = "./devices.json";

  try {
    const iota::JsonValue& storage =
        iota::Configurator::instance()->get(iota::store::types::STORAGE);
    if (storage.HasMember(iota::store::types::TYPE.c_str())) {
      _storage_type.assign(
          storage[iota::store::types::TYPE.c_str()].GetString());
      IOTA_LOG_INFO(m_logger, "type_store:" << _storage_type);
      if (_storage_type.compare(iota::store::types::MONGODB) == 0) {
        unsigned short tm =
            iota::Configurator::instance()->get("timeout").GetInt64();
        registeredDevices.set_time_to_life(tm);

        IOTA_LOG_DEBUG(m_logger,
                       "Setting function get in cache to find in mongo");
        registeredDevices.set_function(boost::bind(get_func, _1));
        registeredDevices.set_entity_function(boost::bind(get_func, _1));

      } else {
        if (storage.HasMember(iota::store::types::FILE.c_str())) {
          devices_store.assign(
              storage[iota::store::types::FILE.c_str()].GetString());
          iota::DevicesFile::instance()->initialize(devices_store);
        } else {
          IOTA_LOG_INFO(m_logger, "in config devices store: no file defined");
        }

        IOTA_LOG_INFO(m_logger, "devices store: " << devices_store);
        IOTA_LOG_INFO(m_logger, "Parsing cache from devices file");
        iota::DevicesFile::instance()->parse_to_cache(&registeredDevices);
      }
    } else {
      IOTA_LOG_ERROR(m_logger, "Config file has not got storage");
    }

  } catch (...) {
    IOTA_LOG_INFO(m_logger, " Incomplete onfiguration file");
  }
}

iota::RestHandle::~RestHandle() {}

void iota::RestHandle::set_option(const std::string& name,
                                  const std::string& value) {
  if (name.compare("ProtocolDescription") == 0) {
    _protocol_data.description = value;
  }
}

std::string iota::RestHandle::get_my_url_base() { return _my_url_base; }

void iota::RestHandle::set_my_url_base(std::string st) { _my_url_base = st; }

void iota::RestHandle::register_plugin() {
  iota::AdminService* AdminService_ptr =
      iota::Process::get_process().get_admin_service();
  if (AdminService_ptr != NULL) {
    AdminService_ptr->add_service(get_resource(), this);
  } else {
    IOTA_LOG_DEBUG(m_logger, "Module without registering " << get_resource());
  }

  iota::Configurator* configurator = iota::Configurator::instance();

  try {
    const iota::JsonValue& res_obj =
        configurator->getResourceObject(get_resource());
    if (res_obj.HasMember(iota::types::CONF_FILE_STATISTICS.c_str()) &&
        res_obj.IsTrue()) {
      add_statistic_counter(iota::types::STAT_TRAFFIC, true);
    }
  } catch (std::exception& e) {
    IOTA_LOG_DEBUG(m_logger,
                   "No internal stats information in configuration file "
                       << get_resource());
  }

  // Statistics by service
  try {
    const iota::JsonValue& res_obj =
        configurator->getResourceObject(get_resource());
    if ((res_obj.HasMember(iota::types::CONF_FILE_SERVICES.c_str())) &&
        (res_obj[iota::types::CONF_FILE_SERVICES.c_str()].IsArray())) {
      const iota::JsonValue& services =
          res_obj[iota::types::CONF_FILE_SERVICES.c_str()];
      for (rapidjson::SizeType j = 0; j < services.Size(); j++) {
        if ((services[j].HasMember(iota::types::CONF_FILE_SERVICE.c_str())) &&
            (services[j][iota::types::CONF_FILE_SERVICE.c_str()].IsString())) {
          std::string ak =
              services[j][iota::types::CONF_FILE_SERVICE.c_str()].GetString();

          if ((services[j].HasMember(
                  iota::types::CONF_FILE_STATISTICS.c_str())) &&
              (services[j][iota::types::CONF_FILE_SERVICE.c_str()].IsTrue())) {
            add_statistic_counter(ak, true);
          }
        }
      }
    }
  } catch (std::exception& e) {
    IOTA_LOG_INFO(m_logger,
                  "No internal stats information in configuration file "
                      << get_resource());
  }

  // IoTA Manager register
  try {
    const iota::JsonValue& manager_endpoint =
        iota::Configurator::instance()->get(
            iota::types::CONF_FILE_IOTA_MANAGER);
    std::string m_end(manager_endpoint.GetString());
    set_iota_manager_endpoint(m_end);
    register_iota_manager();
  } catch (std::exception& e) {
    IOTA_LOG_INFO(m_logger, "No IoTA-Manager configured in " << get_resource());
  }
}

std::string iota::RestHandle::get_public_ip() {
  std::string public_ip;
  try {
    const JsonValue& conf_public_ip =
        iota::Configurator::instance()->get(iota::store::types::PUBLIC_IP);
    public_ip = conf_public_ip.GetString();
  } catch (std::exception& e) {
    IOTA_LOG_INFO(m_logger, "No public ip");
  }
  if (public_ip.empty()) {
    // Own endpoint to register

    std::string my_ip = iota::Configurator::instance()->get_listen_ip();
    unsigned short my_port = iota::Configurator::instance()->get_listen_port();

    // unsigned short my_port =  my_endpoint.port();
    public_ip.append(my_ip);
    public_ip.append(":");
    public_ip.append(boost::lexical_cast<std::string>(my_port));
  }
  if (!public_ip.empty() &&
      public_ip.find("http://", 0, 7) == std::string::npos) {
    public_ip.insert(0, "http://");
  }
  return public_ip;
}

std::string iota::RestHandle::get_iotagent_identifier() {
  // first identifier defined in command line
  // second identifier defined in config.json
  std::string iotagent_id =
      iota::Configurator::instance()->get_iotagent_identifier();
  // third compnent name defined in config.json
  if (iotagent_id.empty()) {
    iotagent_id = iota::Configurator::instance()->get_iotagent_name();
  }

  // last public ip
  if (iotagent_id.empty()) {
    // use ip as identifier
    iotagent_id = get_public_ip();
    if (boost::starts_with(iotagent_id, "http://")) {
      iotagent_id = iotagent_id.substr(7);
    }
  }

  std::size_t found = iotagent_id.find(":");
  if (found == std::string::npos) {
    // add port
    unsigned short my_port = iota::Configurator::instance()->get_listen_port();

    iotagent_id.append(":");
    iotagent_id.append(boost::lexical_cast<std::string>(my_port));
  }

  return iotagent_id;
}

void iota::RestHandle::register_iota_manager() {
  std::string iota_manager_endpoint = get_iota_manager_endpoint();
  std::string log_message;
  log_message.append(typeid(this).name());
  log_message.append("register_iota_manager resource=");
  log_message.append(get_resource());
  log_message.append(" manager=");
  log_message.append(iota_manager_endpoint);
  IOTA_LOG_DEBUG(m_logger, log_message);
  if (iota_manager_endpoint.empty()) {
    return;
  }
  std::string public_ip = get_public_ip();
  public_ip.append(iota::Process::get_url_base());

  try {
    bool using_database = true;
    const iota::JsonValue& storage =
        iota::Configurator::instance()->get(iota::store::types::STORAGE);
    if (storage.HasMember(iota::store::types::TYPE.c_str())) {
      std::string storage_type;
      storage_type.assign(
          storage[iota::store::types::TYPE.c_str()].GetString());
      if (storage_type.compare(iota::store::types::MONGODB) != 0) {
        using_database = false;
      }
    } else {
      IOTA_LOG_ERROR(m_logger, "Config file has not got storage");
    }

    if (using_database) {
      iota::ServiceCollection srv_table;
      mongo::BSONObj srv_find =
          BSON(iota::store::types::RESOURCE << get_resource());
      int code_res = srv_table.find(srv_find);
      iota::ProtocolData protocol_data = get_protocol_data();
      if (!protocol_data.protocol.empty() &&
          !protocol_data.description.empty()) {
        mongo::BSONObjBuilder json_builder;
        mongo::BSONArrayBuilder services_builder;
        json_builder.append(iota::store::types::PROTOCOL,
                            protocol_data.protocol);
        json_builder.append(iota::store::types::PROTOCOL_DESCRIPTION,
                            protocol_data.description);
        json_builder.append(iota::store::types::IOTAGENT, public_ip);
        // new parameter to avoid changing dynamic ip
        json_builder.append(iota::store::types::IOTAGENT_ID,
                            get_iotagent_identifier());
        json_builder.append(iota::store::types::RESOURCE, get_resource());
        while (srv_table.more()) {
          mongo::BSONObj srv_resu = srv_table.next();
          services_builder.append(srv_resu);
        }
        if (services_builder.arrSize() > 0) {
          json_builder.append(iota::store::types::SERVICES,
                              services_builder.arr());
        }
        std::string json_post(json_builder.obj().jsonString());
        log_message.append(json_post);
        iota::IoTUrl dest(iota_manager_endpoint);
        boost::shared_ptr<iota::HttpClient> http_client(
            new iota::HttpClient(iota::Process::get_process().get_io_service(),
                                 dest.getHost(), dest.getPort()));
        boost::property_tree::ptree additional_info;
        pion::http::request_ptr request(new pion::http::request());
        request->set_method(pion::http::types::REQUEST_METHOD_POST);
        request->set_resource(dest.getPath());
        request->set_content(json_post);
        request->set_content_type(iota::types::IOT_CONTENT_TYPE_JSON);
        request->add_header(iota::types::IOT_HTTP_HEADER_ACCEPT,
                            iota::types::IOT_CONTENT_TYPE_JSON);
        std::string server(dest.getHost());
        server.append(":");
        server.append(boost::lexical_cast<std::string>(dest.getPort()));
        request->add_header(pion::http::types::HEADER_HOST, server);
        http_client->async_send(
            request, get_default_timeout(), "",
            boost::bind(&iota::RestHandle::receive_event_from_manager, this, _1,
                        _2, _3));
        IOTA_LOG_INFO(m_logger, json_post);
      }
    } else {
      IOTA_LOG_ERROR(m_logger, "No protocol information for "
                                   << get_resource());
    }
  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, e.what());
  }
}

void iota::RestHandle::receive_event_from_manager(
    boost::shared_ptr<iota::HttpClient> connection,
    pion::http::response_ptr response_ptr,
    const boost::system::error_code& error) {
  int code = -1;
  std::string iota_manager_endpoint = get_iota_manager_endpoint();

  if (response_ptr.get() != NULL) {
    code = response_ptr->get_status_code();
  }

  if (error) {
    iota::AdminService* AdminService_p =
        iota::Process::get_process().get_admin_service();
    if (AdminService_p != NULL) {
      AdminService_p->set_register_retries(true);
    }
    iota::Alarm::error(iota::types::ALARM_CODE_NO_IOTA_MANAGER,
                       iota_manager_endpoint, iota::types::ERROR,
                       error.message());
  } else {
    iota::Alarm::info(iota::types::ALARM_CODE_NO_IOTA_MANAGER,
                      iota_manager_endpoint, iota::types::ERROR,
                      error.message());
    if (code != pion::http::types::RESPONSE_CODE_CREATED) {
      IOTA_LOG_ERROR(m_logger, " resource=" + get_resource() + " code=" +
                                   boost::lexical_cast<std::string>(code) +
                                   " error=" + error.message());
    }
  }
}

std::string iota::RestHandle::add_url(
    std::string url, std::map<std::string, std::string>& filters,
    iota::RestHandle::HandleFunction_t handle, iota::RestHandle* context) {
  register_plugin();
  IOTA_LOG_DEBUG(m_logger, "Add url " << url << " to url base "
                                      << iota::Process::get_url_base());
  struct ResourceHandler resource_handler;
  std::string url_base_plus_url(get_resource());
  std::string url_trimmed = remove_url_base(url);
  if ((url_trimmed.empty() == false) && (url_trimmed[0] != '/')) {
    url_base_plus_url.append("/");
  }
  url_base_plus_url.append(url_trimmed);
  format_pattern(url_base_plus_url, filters, resource_handler.url_regex,
                 resource_handler.url_placeholders);
  resource_handler.rest_function =
      boost::bind(handle, context, _1, _2, _3, _4, _5);
  _handlers.insert(
      std::make_pair<std::string, ResourceHandler>(url, resource_handler));
  IOTA_LOG_DEBUG(m_logger, "Added url " << url_base_plus_url);
  _my_url_base = url_base_plus_url;
  return _my_url_base;
}

void iota::RestHandle::operator()(pion::http::request_ptr& http_request_ptr,
                                  pion::tcp::connection_ptr& tcp_conn) {
  IOTA_LOG_DEBUG(m_logger, iota::http2string(*http_request_ptr));
  tcp_conn->set_lifecycle(pion::tcp::connection::LIFECYCLE_CLOSE);
  boost::shared_ptr<iota::IoTStatistic> stat =
      get_statistic_counter(iota::types::STAT_TRAFFIC);

  // Request statistic
  double tr_in = get_payload_length(http_request_ptr);
  tr_in += get_query_length(http_request_ptr);
  IoTValue v((*stat)[iota::types::STAT_TRAFFIC_IN], tr_in);
  /* Uncomment for SSL
  IOTA_LOG_DEBUG(m_logger,
  SSL_get_verify_result(tcp_conn->get_ssl_socket().native_handle()));
  X509* client_certificate =
  SSL_get_peer_certificate(tcp_conn->get_ssl_socket().native_handle());
  if (client_certificate != NULL) {
    char subject_name[256];
    X509_NAME_oneline(X509_get_subject_name(client_certificate),
  reinterpret_cast<char*>(subject_name), 256);
    IOTA_LOG_DEBUG(m_logger, subject_name);
  }
  */
  // Add header to trace
  http_request_ptr->add_header(iota::types::HEADER_TRACE_MESSAGES,
                               riot_uuid(get_resource()));
  IOTA_LOG_DEBUG(m_logger, "Processing request "
                               << http_request_ptr->get_header(
                                   iota::types::HEADER_TRACE_MESSAGES));
  if (_pre_filters.size() > 0) {
    execute_filters(http_request_ptr, tcp_conn, 0);
  } else {
    handle_request(http_request_ptr, tcp_conn);
  }
}

void iota::RestHandle::execute_filters(
    pion::http::request_ptr& http_request_ptr,
    pion::tcp::connection_ptr& tcp_conn, int num_filter, int status) {
  IOTA_LOG_DEBUG(m_logger, "Processing request "
                               << http_request_ptr->get_header(
                                   iota::types::HEADER_TRACE_MESSAGES));
  IOTA_LOG_DEBUG(m_logger, "execute_filters status:  " << status);

  if (status != iota::types::RESPONSE_CODE_OK) {
    handle_end_filters(http_request_ptr, tcp_conn, num_filter, status);
  } else {
    if (num_filter != (_pre_filters.size() - 1)) {
      int n_filter = num_filter;
      _pre_filters.at(num_filter)
          ->set_async_filter(
              http_request_ptr->get_header(iota::types::HEADER_TRACE_MESSAGES),
              boost::bind(&iota::RestHandle::execute_filters, this,
                          http_request_ptr, tcp_conn, ++n_filter, _1));
    } else {
      _pre_filters.at(num_filter)
          ->set_async_filter(
              http_request_ptr->get_header(iota::types::HEADER_TRACE_MESSAGES),
              boost::bind(&iota::RestHandle::handle_end_filters, this,
                          http_request_ptr, tcp_conn, num_filter, _1));
    }

    _pre_filters.at(num_filter)
        ->get_io_service()
        .post(boost::bind(&HTTPFilter::handle_request,
                          _pre_filters.at(num_filter), http_request_ptr,
                          tcp_conn));
  }
}

void iota::RestHandle::handle_end_filters(
    pion::http::request_ptr& http_request_ptr,
    pion::tcp::connection_ptr& tcp_conn, int num_filter, int status) {
  IOTA_LOG_DEBUG(m_logger, "Processing request "
                               << http_request_ptr->get_header(
                                   iota::types::HEADER_TRACE_MESSAGES));
  IOTA_LOG_DEBUG(m_logger, "End filters status " << status);
  if (status != pion::http::types::RESPONSE_CODE_OK) {
    // Filters return NOK. End request
    std::string& response_buffer = create_buffer(tcp_conn);
    pion::http::response_writer_ptr writer(pion::http::response_writer::create(
        tcp_conn, *http_request_ptr,
        boost::bind(&iota::RestHandle::finish, this, tcp_conn)));
    std::string reason(iota::types::RESPONSE_MESSAGE_BAD_REQUEST);
    std::string details;
    if (status == iota::types::RESPONSE_CODE_NOT_ACCEPTABLE) {
      details = iota::types::RESPONSE_MESSAGE_NOT_ACCEPTABLE;
    } else if (status == iota::types::RESPONSE_CODE_UNSUPPORTED_MEDIA_TYPE) {
      details = iota::types::RESPONSE_MESSAGE_UNSUPPORTED_MEDIA_TYPE;
    } else if (status == iota::types::RESPONSE_CODE_FIWARE_SERVICE_ERROR) {
      reason = iota::types::REASON_MALFORMED_HEADER;
      details = iota::types::DETAILS_HEADER_FIWARE_SERVICE;
      status = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
    } else if (status == iota::types::RESPONSE_CODE_FIWARE_SERVICE_PATH_ERROR) {
      reason = iota::types::REASON_MALFORMED_HEADER;
      details = iota::types::DETAILS_HEADER_FIWARE_SERVICE_PATH;
      status = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
    } else if (status == iota::types::RESPONSE_CODE_FORBIDDEN_CHARACTERS) {
      reason = iota::types::REASON_MALFORMED_HEADER;
      details = iota::types::DETAILS_FORBIDDEN_CHARACTERS;
      status = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
    }

    error_response(writer->get_response(), reason, details, response_buffer,
                   status);
    send_http_response(writer, response_buffer);
  } else {
    handle_request(http_request_ptr, tcp_conn, status);
  }
}
void iota::RestHandle::handle_request(pion::http::request_ptr& http_request_ptr,
                                      pion::tcp::connection_ptr& tcp_conn,
                                      int status) {
  IOTA_LOG_DEBUG(m_logger, "Processing request "
                               << http_request_ptr->get_header(
                                   iota::types::HEADER_TRACE_MESSAGES));
  IOTA_LOG_DEBUG(m_logger, "Proccessing in handle " << get_resource());

  boost::shared_ptr<iota::IoTStatistic> stat =
      get_statistic_counter(iota::types::STAT_TRAFFIC);
  Duration d((*stat)[iota::types::STAT_TRAFFIC_DURATION]);

  // Response to request
  std::string& response_buffer = create_buffer(tcp_conn);
  pion::http::response_writer_ptr writer(pion::http::response_writer::create(
      tcp_conn, *http_request_ptr,
      boost::bind(&iota::RestHandle::finish, this, tcp_conn)));

  try {
    std::string requested_operation = http_request_ptr->get_method();
    requested_operation.append(" ");
    requested_operation.append(http_request_ptr->get_original_resource());
    std::string resource = http_request_ptr->get_resource();
    resource.append(" - ");
    resource.append(requested_operation);

    // Handler parameters
    std::map<std::string, std::string> url_args;
    std::multimap<std::string, std::string> query_parameters;
    pion::ihash_multimap q_param;
    pion::http::parser::parse_url_encoded(q_param,
                                          http_request_ptr->get_query_string());
    for (pion::ihash_multimap::const_iterator i = q_param.begin();
         i != q_param.end(); ++i) {
      query_parameters.insert(
          std::make_pair<std::string, std::string>(i->first, i->second));
    }

    std::map<std::string, ResourceHandler>::iterator it_handle =
        _handlers.begin();
    // Search callback
    bool handler_found = false;
    while ((handler_found == false) && (it_handle != _handlers.end())) {
      handler_found = restResourceParse(it_handle->second.url_regex,
                                        it_handle->second.url_placeholders,
                                        requested_operation, url_args);
      if (handler_found == false) {
        ++it_handle;
      }
    }
    if (it_handle == _handlers.end()) {
      error_response(writer->get_response(),
                     iota::types::RESPONSE_MESSAGE_BAD_REQUEST, "",
                     response_buffer);

    } else {
      std::string host_header =
          http_request_ptr->get_header(pion::http::types::HEADER_HOST);
      if (!host_header.empty()) {
        writer->get_response().add_header(pion::http::types::HEADER_HOST,
                                          host_header);
      }
      it_handle->second.rest_function(http_request_ptr, url_args,
                                      query_parameters, writer->get_response(),
                                      response_buffer);
    }
  } catch (std::exception& e) {
    // Response
    error_response(writer->get_response(),
                   iota::types::RESPONSE_MESSAGE_BAD_REQUEST, e.what(),
                   response_buffer);

  }

  send_http_response(writer, response_buffer);
}

void iota::RestHandle::finish(pion::tcp::connection_ptr& tcp_conn) {
  // Close connection and remove buffer
  clear_buffer(tcp_conn);
  tcp_conn->finish();
  IOTA_LOG_DEBUG(m_logger, "finish connection " << tcp_conn.use_count());
}

void iota::RestHandle::clear_buffer(pion::tcp::connection_ptr& conn) {
  boost::mutex::scoped_lock lock(m_mutex);
  _async_buffers.erase(conn);
}

std::string& iota::RestHandle::create_buffer(pion::tcp::connection_ptr& conn) {
  boost::mutex::scoped_lock lock(m_mutex);
  std::string buffer;
  _async_buffers[conn] = buffer;
  return _async_buffers[conn];
}

bool iota::RestHandle::add_pre_filter(
    boost::shared_ptr<iota::HTTPFilter> filter) {
  _pre_filters.push_back(filter);
}
bool iota::RestHandle::add_post_filter(
    boost::shared_ptr<iota::HTTPFilter> filter) {
  _post_filters.push_back(filter);
}

void iota::RestHandle::error_response(pion::http::response& http_response,
                                      std::string reason, std::string details,
                                      std::string& buffer,
                                      unsigned int status_code) {
  if (status_code != 0) {
    http_response.set_status_code(status_code);
    http_response.set_status_message(
        iota::Configurator::instance()->getHttpMessage(status_code));
  } else {
    http_response.set_status_code(pion::http::types::RESPONSE_CODE_NOT_FOUND);
    http_response.set_status_message(
        iota::Configurator::instance()->getHttpMessage(
            pion::http::types::RESPONSE_CODE_NOT_FOUND));
  }
  if (reason.empty() == false) {
    buffer.clear();
    buffer.assign("{");
    buffer.append("\"reason\": \"");
    std::string o_reason(reason);
    boost::erase_all(o_reason, "\"");
    buffer.append(o_reason);

    if (details.empty() == false) {
      buffer.append("\",");
      std::string o_details(details);
      boost::erase_all(o_details, "\"");
      buffer.append("\"details\": \"");
      buffer.append(o_details);
    }
    buffer.append("\"}");
  }
}

std::string iota::RestHandle::remove_url_base(std::string url) {
  if (url.substr(0, iota::Process::get_url_base().size()) ==
      iota::Process::get_url_base()) {
    return url.substr(iota::Process::get_url_base().size());
  }
  return url;
}

std::string iota::RestHandle::get_statistics() {
  boost::mutex::scoped_lock lock(m_mutex_stat);
  IOTA_LOG_DEBUG(m_logger, "Get statistics " << get_resource() << " Counters "
                                             << _statistics.size());

  rapidjson::Document stats;
  stats.SetArray();

  std::map<std::string, boost::shared_ptr<iota::IoTStatistic> >::iterator
      it_stats = _statistics.begin();

  while (it_stats != _statistics.end()) {
    std::string stat_name(it_stats->first);

    try {
      if (it_stats->second.get() == NULL) {
        PION_LOG_INFO(m_logger, "t_stats->second is NULL ");
        continue;
      }
      std::map<long,
               std::map<std::string, iota::IoTStatistic::iot_accumulator_ptr> >
          accs = it_stats->second->get_counters();

      IOTA_LOG_DEBUG(m_logger, "Get statistics accs Counters " << accs.size());
      std::map<long,
               std::map<std::string,
                        iota::IoTStatistic::iot_accumulator_ptr> >::iterator
          it_tm = accs.begin();
      while (it_tm != accs.end()) {
        rapidjson::Value stat_element;
        stat_element.SetObject();
        rapidjson::Value counter;
        counter.SetObject();

        // Timestamp
        stat_element.AddMember("timestamp",
                               rapidjson::Value().SetInt64(it_tm->first),
                               stats.GetAllocator());

        std::map<std::string, iota::IoTStatistic::iot_accumulator_ptr>::iterator
            it = it_tm->second.begin();

        while (it != it_tm->second.end()) {
          std::string acc_name(it->first);
          rapidjson::Value acc_o;
          acc_o.SetObject();

          if (it->second.get() == NULL) {
            PION_LOG_INFO(m_logger, "it->second is NULL ");
            continue;
          }

          acc_o.AddMember("count",
                          rapidjson::Value().SetDouble(
                              boost::accumulators::count(*(it->second))),
                          stats.GetAllocator());
          acc_o.AddMember("sum", rapidjson::Value().SetDouble(
                                     boost::accumulators::sum(*(it->second))),
                          stats.GetAllocator());
          acc_o.AddMember("max", rapidjson::Value().SetDouble(
                                     boost::accumulators::max(*(it->second))),
                          stats.GetAllocator());
          acc_o.AddMember("min", rapidjson::Value().SetDouble(
                                     boost::accumulators::min(*(it->second))),
                          stats.GetAllocator());
          if (boost::accumulators::count(*(it->second)) == 0) {
            acc_o.AddMember("mean", rapidjson::Value().SetDouble(0),
                            stats.GetAllocator());
          } else {
            acc_o.AddMember("mean",
                            rapidjson::Value().SetDouble(
                                boost::accumulators::mean(*(it->second))),
                            stats.GetAllocator());
          }
          counter.AddMember(acc_name.c_str(), acc_o, stats.GetAllocator());
          ++it;
        }
        stat_element.AddMember(stat_name.c_str(), counter,
                               stats.GetAllocator());
        // this if could be removed
        if (stat_element.HasMember(stat_name.c_str())) {
          stats.PushBack(stat_element, stats.GetAllocator());
        }
        ++it_tm;
      }
    } catch (std::exception& e) {
      IOTA_LOG_ERROR(m_logger, "Stats " << e.what());
    }
    ++it_stats;
  }
  // stats.AddMember("statistics", resource, stats.GetAllocator());
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  stats.Accept(writer);
  return buffer.GetString();
}

boost::shared_ptr<iota::IoTStatistic> iota::RestHandle::add_statistic_counter(
    const std::string& name_stat, bool enabled) {
  boost::mutex::scoped_lock loc(m_mutex_stat);
  boost::shared_ptr<iota::IoTStatistic> handler_statistic(
      new iota::IoTStatistic(name_stat));
  handler_statistic->set_enable(enabled);
  _statistics.insert(
      std::pair<std::string, boost::shared_ptr<iota::IoTStatistic> >(
          name_stat, handler_statistic));
  return _statistics[name_stat];
}

boost::shared_ptr<iota::IoTStatistic> iota::RestHandle::get_statistic_counter(
    const std::string& name_stat) {
  boost::mutex::scoped_lock loc(m_mutex_stat);
  std::map<std::string, boost::shared_ptr<iota::IoTStatistic> >::iterator it =
      _statistics.begin();
  it = _statistics.find(name_stat);
  if (it == _statistics.end()) {
    boost::shared_ptr<iota::IoTStatistic> handler_statistic(
        new iota::IoTStatistic(name_stat));
    _statistics.insert(
        std::pair<std::string, boost::shared_ptr<iota::IoTStatistic> >(
            name_stat, handler_statistic));
  }
  return _statistics[name_stat];
}

void iota::RestHandle::remove_devices_from_cache(Device& device) {
  registeredDevices.remove(device);
}

const boost::shared_ptr<iota::Device> iota::RestHandle::get_device_by_entity(
    const std::string& name, const std::string& entity_type,
    const std::string& service, const std::string& service_path) {
  boost::shared_ptr<iota::Device> itemQ(new iota::Device("", service));
  itemQ->_entity_name = name;
  itemQ->_entity_type = entity_type;

  if (service_path.empty()) {
    itemQ->_service_path = iota::types::FIWARE_SERVICEPATH_DEFAULT;
  } else {
    itemQ->_service_path = service_path;
  }

  // use protocol
  iota::ProtocolData protocol_data = get_protocol_data();
  if (!protocol_data.protocol.empty()) {
    itemQ->_protocol = protocol_data.protocol;
  }

  boost::shared_ptr<iota::Device> result =
      registeredDevices.get_by_entity(itemQ);
  if (result.get() == NULL) {
    // doble busqueda, campo contrario por defecto
    // el nombre debe ser entity_type:name_device, nos quedamos con el nombre de
    // device

    size_t pos_i = name.find_first_of(":");
    if (pos_i != std::string::npos) {
      std::string name_device = name.substr(pos_i + 1);
      IOTA_LOG_INFO(
          m_logger,
          "doubleSearch, device registered with no entity, name device: "
              << name_device << " " << service);
      boost::shared_ptr<iota::Device> itemQD(
          new iota::Device(name_device, service));
      if (service_path.empty()) {
        itemQD->_service_path = service_path;
      } else {
        itemQ->_service_path = iota::types::FIWARE_SERVICEPATH_DEFAULT;
      }
      result = registeredDevices.get(itemQD);
    } else {
      IOTA_LOG_INFO(m_logger, "doubleSearch, with no entity_type "
                                  << name << " " << service);
      itemQ->_entity_type = "";
      result = registeredDevices.get_by_entity(itemQ);
    }
  }

  if (result.get() != NULL) {
    if (!result->_active) {
      std::string text_error;
      text_error.append("iota::Device ");
      text_error.append(name);
      text_error.append(" is not active. Service ");
      text_error.append(service);
      text_error.append(" ");
      text_error.append(service_path);
      text_error.append(" entity type: ");
      text_error.append(entity_type);

      IOTA_LOG_ERROR(m_logger, text_error);
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DEVICE_NO_ACTIVE,
                                text_error,
                                iota::types::RESPONSE_CODE_DEVICE_NO_ACTIVE);
    }
  } else {
    std::string text_error;
    text_error.append("RestHandle::get_device_by_entity no exists device ");
    text_error.append(name);
    text_error.append(service);
    text_error.append(" ");
    text_error.append(service_path);
    text_error.append(" entity type: ");
    text_error.append(entity_type);

    IOTA_LOG_ERROR(m_logger, text_error);
    throw iota::IotaException(
        iota::types::RESPONSE_MESSAGE_NO_DEVICE, text_error,
        iota::types::RESPONSE_CODE_CONTEXT_ELEMENT_NOT_FOUND);
  }

  return result;
}

bool iota::RestHandle::get_service_by_name(boost::property_tree::ptree& pt,
                                           const std::string& item_name,
                                           const std::string& service_path) {
  int code = ERROR_NO_SERVICE;

  if (_storage_type.compare(iota::store::types::MONGODB) == 0) {
    code = get_service_by_name_bbdd(pt, item_name, service_path);
  }
  if (code == ERROR_NO_SERVICE) {
    get_service_by_name_file(pt, item_name, service_path);
  }

  // Add information from plugin
  complete_info(pt);

  return true;
}

bool iota::RestHandle::get_service_by_apiKey(boost::property_tree::ptree& pt,
                                             const std::string& apiKey) {
  int code = ERROR_NO_SERVICE;

  if (_storage_type.compare(iota::store::types::MONGODB) == 0) {
    code = get_service_by_apiKey_bbdd(pt, apiKey);
  }
  if (code == ERROR_NO_SERVICE) {
    get_service_by_apiKey_file(pt, apiKey);
  }

  // Add information from plugin
  complete_info(pt);

  return true;
}

void iota::RestHandle::fill_service_with_bson(const mongo::BSONObj& bson,
                                              boost::property_tree::ptree& pt) {
  int default_timeout = get_default_timeout();
  std::string default_context_broker = get_default_context_broker();
  std::string http_proxy = get_http_proxy();

  bson_to_ptree(bson, pt);
  if (pt.get<std::string>(iota::store::types::CBROKER, "").empty()) {
    pt.put(iota::store::types::CBROKER, default_context_broker);
  }
  if (pt.get<int>(iota::store::types::TIMEOUT, -1) == -1) {
    pt.put(iota::store::types::TIMEOUT, default_timeout);
  }
  pt.put(iota::types::CONF_FILE_PROXY, http_proxy);

  // add protocol default data
  complete_info(pt);
}

int iota::RestHandle::get_service_by_name_bbdd(
    boost::property_tree::ptree& pt, const std::string& name,
    const std::string& service_path) {
  std::string resource = get_resource();
  IOTA_LOG_DEBUG(m_logger, "get_service_by_name_bbdd-"
                               << "resource:" << resource);

  iota::Collection q1(iota::store::types::SERVICE_TABLE);
  mongo::BSONObjBuilder p2;
  p2.append(iota::store::types::SERVICE, name);
  if (!service_path.empty()) {
    p2.append(iota::store::types::SERVICE_PATH, service_path);
  }
  if (!resource.empty()) {
    p2.append(iota::store::types::RESOURCE, resource);
  }

  int code_res = q1.find(p2.obj());
  if (q1.more()) {
    mongo::BSONObj r1 = q1.next();
    fill_service_with_bson(r1, pt);
  } else {
    IOTA_LOG_ERROR(m_logger, "get_service_by_name_bbdd no service for "
                                 << name << " " << service_path
                                 << "resource:" << resource);
    return ERROR_NO_SERVICE;
  }

  if (q1.more()) {
    IOTA_LOG_ERROR(m_logger, "get_service_by_name_bbdd there are more than one"
                                 << name << " " << service_path
                                 << "resource:" << resource);
    return ERROR_MORE_THAN_ONE;
  }

  return ERROR_SERVICE_OK;
}

int iota::RestHandle::get_service_by_apiKey_bbdd(
    boost::property_tree::ptree& pt, const std::string& apiKey) {
  std::string resource = get_resource();
  std::string default_context_broker = get_default_context_broker();
  int default_timeout = get_default_timeout();
  std::string http_proxy = get_http_proxy();

  IOTA_LOG_DEBUG(m_logger, "get_service_by_apiKey_bbdd " << apiKey);

  iota::Collection q1(iota::store::types::SERVICE_TABLE);
  mongo::BSONObjBuilder p2;
  p2.append(iota::store::types::APIKEY, apiKey);
  if (!resource.empty()) {
    p2.append(iota::store::types::RESOURCE, resource);
  }
  int code_res = q1.find(p2.obj());
  if (q1.more()) {
    mongo::BSONObj r1 = q1.next();
    fill_service_with_bson(r1, pt);
  } else {
    IOTA_LOG_ERROR(m_logger, "get_service_by_apiKey_bbdd no service for apike"
                                 << apiKey);
    return ERROR_NO_SERVICE;
  }

  if (q1.more()) {
    IOTA_LOG_ERROR(m_logger,
                   "get_service_by_apiKey_bbdd there are more than one"
                       << apiKey);
    return ERROR_MORE_THAN_ONE;
  }

  return ERROR_SERVICE_OK;
}

const iota::JsonValue& iota::RestHandle::get_service_by_name_file(
    boost::property_tree::ptree& pt, const std::string& item_name,
    const std::string& service_path) {
  std::string default_context_broker = get_default_context_broker();
  int default_timeout = get_default_timeout();
  std::string http_proxy = get_http_proxy();

  const iota::JsonValue& service_object =
      iota::Configurator::instance()->getService(get_resource(), item_name,
                                                 service_path);

  std::stringstream ss;
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  service_object.Accept(writer);
  ss << buffer.GetString();
  boost::property_tree::read_json(ss, pt);
  if (pt.get<std::string>(iota::store::types::CBROKER, "").empty()) {
    pt.put(iota::store::types::CBROKER, default_context_broker);
  }
  if (pt.get<int>(iota::store::types::TIMEOUT, -1) == -1) {
    pt.put(iota::store::types::TIMEOUT, default_timeout);
  }
  pt.put(iota::types::CONF_FILE_PROXY, http_proxy);
  return service_object;
}

void iota::RestHandle::send_update_context(
    const std::string& apikey, const std::string& device_id,
    const std::vector<iota::Attribute>& attributes) {
  boost::property_tree::ptree pt_cb;
  boost::shared_ptr<iota::Device> dev;

  get_service_by_apiKey(pt_cb, apikey);

  dev = get_device(
      device_id, pt_cb.get<std::string>(iota::store::types::SERVICE, ""),
      pt_cb.get<std::string>(iota::store::types::SERVICE_PATH, ""));

  iota::RiotISO8601 mi_hora;
  std::string date_to_cb = mi_hora.toUTC().toString();
  iota::Attribute metadata_ts("TimeInstant", "ISO8601", date_to_cb);

  iota::ContextElement ngsi_context_element(device_id, "", "false");

  for (std::vector<iota::Attribute>::const_iterator it = attributes.begin();
       it != attributes.end(); ++it) {
    iota::Attribute att(*it);
    att.add_metadata(metadata_ts);
    ngsi_context_element.set_env_info(pt_cb, dev);
    ngsi_context_element.add_attribute(att);
  }

  ContextBrokerCommunicator cb_communicator;

  std::string cb_response;
  cb_communicator.send(ngsi_context_element, "APPEND", pt_cb, cb_response);
  IOTA_LOG_DEBUG(m_logger, "cb resposne : " << cb_response);
}

const iota::JsonValue& iota::RestHandle::get_service_by_apiKey_file(
    boost::property_tree::ptree& pt, const std::string& apiKey) {
  std::string default_context_broker = get_default_context_broker();
  int default_timeout = get_default_timeout();
  std::string http_proxy = get_http_proxy();

  const iota::JsonValue& service_object =
      iota::Configurator::instance()->getServicebyApiKey(get_resource(),
                                                         apiKey);

  std::stringstream ss;
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  service_object.Accept(writer);
  ss << buffer.GetString();
  boost::property_tree::read_json(ss, pt);
  if (pt.get<std::string>(iota::store::types::CBROKER, "").empty()) {
    pt.put(iota::store::types::CBROKER, default_context_broker);
  }
  if (pt.get<int>(iota::store::types::TIMEOUT, -1) == -1) {
    pt.put(iota::store::types::TIMEOUT, default_timeout);
  }
  pt.put(iota::types::CONF_FILE_PROXY, http_proxy);
  return service_object;
}

const boost::shared_ptr<iota::Device>
iota::RestHandle::get_device_empty_service_path(
    boost::shared_ptr<iota::Device> itemQ) {
  boost::shared_ptr<iota::Device> result;

  IOTA_LOG_DEBUG(m_logger, "Get all service path");

  try {
    std::vector<std::string> vec;
    iota::Configurator::instance()->getAllServicePath(get_resource(),
                                                      itemQ->_service, vec);

    for (int i = 0; i < vec.size(); i++) {
      std::string srvpath = vec[i];
      itemQ->_service_path = srvpath;
      result = registeredDevices.get(itemQ);
      if (result != NULL) {
        IOTA_LOG_DEBUG(m_logger, "found device: " << result->_name);
        return result;
      }
    }
  } catch (iota::IotaException& e) {
    IOTA_LOG_ERROR(m_logger, "Configuration error " << e.what());
    throw e;
  }

  return result;
}

const boost::shared_ptr<iota::Device> iota::RestHandle::get_device(
    const std::string& name, const std::string& service,
    const std::string& service_path) {
  boost::shared_ptr<iota::Device> itemQ(new iota::Device(name, service));
  if (!service_path.empty()) {
    itemQ->_service_path = service_path;
  }
  // if service_path empty  all devices
  boost::shared_ptr<iota::Device> result;

  if ((_storage_type.compare(iota::store::types::FILE) == 0) &&
      (service_path.empty()) && (!service.empty())) {
    result = get_device_empty_service_path(itemQ);
  } else {
    if (service.empty()) {
      result = registeredDevices.get_by_name(itemQ);
    } else {
      result = registeredDevices.get(itemQ);
    }
  }

  if (result.get() != NULL) {
    if (!result->_active) {
      std::string text_error;
      text_error.append(" iota::Device ");
      text_error.append(name);
      text_error.append(" is not active. Service ");
      text_error.append(service);
      text_error.append(" ");
      text_error.append(service_path);
      IOTA_LOG_ERROR(m_logger, text_error);
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DEVICE_NO_ACTIVE,
                                text_error,
                                iota::types::RESPONSE_CODE_DEVICE_NO_ACTIVE);
    }
  } else {
    std::string text_error;
    text_error.append("RestHandle:: in get_device, no device, return null");
    text_error.append(" ");
    text_error.append(name);
    text_error.append(":");
    text_error.append(service);
    text_error.append(":");
    text_error.append(service_path);
    IOTA_LOG_ERROR(m_logger, text_error);
    boost::shared_ptr<iota::Device> result2;
    return result2;
  }
  return result;
}

double iota::RestHandle::get_payload_length(
    pion::http::request_ptr& http_request_ptr) {
  return http_request_ptr->get_content_length();
}

double iota::RestHandle::get_query_length(
    pion::http::request_ptr& http_request_ptr) {
  return http_request_ptr->get_query_string().size();
}

double iota::RestHandle::get_payload_length(
    pion::http::response& http_response) {
  return http_response.get_content_length();
}

std::map<std::string, std::string> iota::RestHandle::get_multipart_content(
    pion::http::request_ptr& request_ptr) {
  IOTA_LOG_DEBUG(m_logger, "Content-Type" << request_ptr->get_header(
                               pion::http::types::HEADER_CONTENT_TYPE));
  std::map<std::string, std::string> parts;
  pion::ihash_multimap params;
  pion::http::parser::parse_multipart_form_data(
      params, request_ptr->get_header(pion::http::types::HEADER_CONTENT_TYPE),
      request_ptr->get_content());
  for (pion::ihash_multimap::const_iterator i = params.begin();
       i != params.end(); ++i) {
    parts.insert(std::make_pair<std::string, std::string>(i->first, i->second));
  }
  return parts;
}

void iota::RestHandle::send_http_response(
    pion::http::response_writer_ptr& writer, std::string& response_buffer) {
  // Response statistic
  boost::shared_ptr<iota::IoTStatistic> stat =
      get_statistic_counter(iota::types::STAT_TRAFFIC);
  // double tr_out = get_payload_length(writer->get_response());
  IoTValue v_out((*stat)[iota::types::STAT_TRAFFIC_OUT],
                 response_buffer.size());
  int status_code = writer->get_response().get_status_code();
  writer->get_response().set_status_message(
      iota::Configurator::instance()->getHttpMessage(status_code));
  writer->write_no_copy(response_buffer);
  // writer->write_no_copy(pion::http::types::STRING_CRLF);
  // writer->write_no_copy(pion::http::types::STRING_CRLF);
  writer->send();
}

std::string iota::RestHandle::get_default_context_broker() {
  std::string default_context_broker;
  try {
    const iota::JsonValue& ngsi_url =
        iota::Configurator::instance()->get("ngsi_url");
    if (ngsi_url.HasMember(iota::types::CONF_FILE_CBROKER.c_str()) &&
        ngsi_url[iota::types::CONF_FILE_CBROKER.c_str()].IsString()) {
      default_context_broker =
          ngsi_url[iota::types::CONF_FILE_CBROKER.c_str()].GetString();
    }
  } catch (std::exception& e) {
    IOTA_LOG_DEBUG(m_logger, "Default context broker endpoint not defined");
  }
  return default_context_broker;
}

std::string iota::RestHandle::get_iota_manager_endpoint() {
  return _manager_endpoint;
}

void iota::RestHandle::set_iota_manager_endpoint(std::string manager_endpoint) {
  try {
    iota::IoTUrl url_endpoint(manager_endpoint);
    _manager_endpoint = manager_endpoint;
  } catch (iota::IotaException& e) {
    IOTA_LOG_ERROR(m_logger, e.what());
  }
}

iota::ProtocolData iota::RestHandle::get_protocol_data() {
  return _protocol_data;
}

std::string iota::RestHandle::get_protocol_commands() {
  return "";
}

void iota::RestHandle::complete_info(boost::property_tree::ptree& pt) {
  return;
}

int iota::RestHandle::check_provisioned_data(const std::string& data, std::string& error_str) {
  return iota::types::RESPONSE_CODE_NONE;
}

int iota::RestHandle::get_timeout_commands() {
  int timeout = 0;
  const JsonValue& timeoutJSON =
      iota::Configurator::instance()->get(iota::types::CONF_FILE_TIMEOUT);
  if (timeoutJSON.IsNumber()) {
    timeout = timeoutJSON.GetInt();
  }
  IOTA_LOG_DEBUG(m_logger, "timeout from config.json: " << timeout);
  return timeout;
}


void iota::RestHandle::set_protocol_data(iota::ProtocolData p_data) {
  if (!p_data.description.empty()) {
    _protocol_data.description = p_data.description;
  }
}

std::string iota::RestHandle::get_http_proxy() {
  std::string http_proxy;
  try {
    http_proxy = iota::Configurator::instance()
                     ->get(iota::types::CONF_FILE_PROXY.c_str())
                     .GetString();

  } catch (std::exception& e) {
    IOTA_LOG_DEBUG(m_logger, "proxy not defined");
  }
  return http_proxy;
}
int iota::RestHandle::get_default_timeout() {
  int default_timeout = iota::DEFAULT_TIMEOUT;
  try {
    const iota::JsonValue& timeout =
        iota::Configurator::instance()->get(iota::types::CONF_FILE_TIMEOUT);
    default_timeout = timeout.GetInt64();
  } catch (std::exception& e) {
    IOTA_LOG_DEBUG(m_logger, "Default context broker endpoint not defined");
  }
  return default_timeout;
}

void iota::RestHandle::reset_counters() {
  _statistics.erase(iota::types::STAT_TRAFFIC);
}

void iota::RestHandle::update_endpoint_device(
    const boost::shared_ptr<Device>& dev, const std::string& new_endpoint) {
  if (_storage_type.compare(iota::store::types::MONGODB) == 0) {
    iota::Collection table(iota::store::types::DEVICE_TABLE);

    mongo::BSONObj query =
        BSON(iota::store::types::DEVICE_ID
             << dev->_name << iota::store::types::SERVICE << dev->_service
             << iota::store::types::SERVICE_PATH << dev->_service_path);
    mongo::BSONObj ap = BSON(iota::store::types::ENDPOINT << new_endpoint);
    table.update(query, ap);
  }
}
