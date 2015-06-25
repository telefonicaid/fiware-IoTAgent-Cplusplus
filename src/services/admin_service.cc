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
#include "admin_service.h"
#include "rest/riot_conf.h"
#include "util/csv_reader.h"
#include "rest/types.h"
#include "rest/oauth_filter.h"
#include "rest/media_filter.h"
#include "util/iota_exception.h"
#include <sstream>
#include <pion/http/response_writer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <pion/process.hpp>
#include "version.h"
#include "util/device_collection.h"
#include "util/service_collection.h"
#include "util/service_mgmt_collection.h"
#include "util/protocol_collection.h"
#include "util/command_collection.h"
#include "util/iot_url.h"

#if defined LIBVARIANT

#include <Variant/Schema.h>
#include <Variant/SchemaLoader.h>

#endif // defined


#include <dlfcn.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

namespace iota {
extern std::string URL_BASE;
extern std::string logger;
}
const std::string iota::AdminService::_api_service_holder ="service";
const std::string iota::AdminService::_api_device_holder = "device";
const unsigned short iota::AdminService::TIME_FOR_LOG = 2;
const unsigned short iota::AdminService::TIME_TO_LOG = 60;

iota::AdminService::AdminService(pion::http::plugin_server_ptr web_server):
  iota::RestHandle(),
  //_web_server(web_server),
  _class_name("iota::AdminService"),
  m_log(PION_GET_LOGGER(iota::logger)) {
  IOTA_LOG_DEBUG(m_log, "iota::AdminService::AdminService");
  checkIndexes();

}

iota::AdminService::AdminService(): m_log(PION_GET_LOGGER(iota::logger)),
  _class_name("iota::AdminService") {
  IOTA_LOG_DEBUG(m_log, "iota::AdminService::AdminService2");
  // iota::AdminService::_web_server = NULL;
  checkIndexes();
}

iota::AdminService::~AdminService() {
  //std::cout << "DESTRUCTOR AdminService " << _web_server.use_count() << std::endl;
  if (_timer.get() != NULL) {
    _timer->cancel();
  }
  boost::mutex::scoped_lock lock(iota::AdminService::m_sm);
  _service_manager.clear();
  lock.unlock();

}


void iota::AdminService::checkIndexes() {
  try {
    const iota::JsonValue& storage = iota::Configurator::instance()->get(
                                       iota::store::types::STORAGE);
    if (storage.HasMember(iota::store::types::TYPE.c_str())) {
      _storage_type.assign(storage[iota::store::types::TYPE.c_str()].GetString());
      IOTA_LOG_INFO(m_log, "type_store:" <<  _storage_type);
      if (_storage_type.compare(iota::store::types::MONGODB)==0) {
        IOTA_LOG_DEBUG(m_log, "Check tables in mongo");
        iota::DeviceCollection devcol;
        devcol.createTableAndIndex();
        iota::ServiceCollection servcol;
        servcol.createTableAndIndex();
        iota::ServiceMgmtCollection servmancol;
        servmancol.createTableAndIndex();
        iota::CommandCollection commandcol;
        commandcol.createTableAndIndex();
        iota::ProtocolCollection protocolcol;
        protocolcol.createTableAndIndex();
      }
    }
  }
  catch (std::exception& e) {
    IOTA_LOG_ERROR(m_log, "Check configuration, error in checkIndexes " <<
                   e.what());
  }
}

/*
pion::http::plugin_server_ptr iota::AdminService::get_web_server() {
  boost::shared_ptr<pion::http::plugin_server> w_s = _web_server.lock();
  return w_s;
}
*/

void iota::AdminService::set_timezone_database(std::string timezones_file) {

  _timezone_database.load_from_file(timezones_file);
  IOTA_LOG_DEBUG(m_log, "Load timezone database " << timezones_file);
}

void  iota::AdminService::check_mongo_config() {

  const JsonValue& storage=
    iota::Configurator::instance()->get(iota::store::types::STORAGE);

  //check if type is mongodb
  if (storage.HasMember(iota::store::types::TYPE.c_str())) {
    std::string type = storage[iota::store::types::TYPE.c_str()].GetString();
    if (type.compare(iota::types::CONF_FILE_MONGO) != 0) {
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_NOT_IMPLEMENTED,
                                " API Rest not implemented for this storage type [ " + type + "]",
                                iota::types::RESPONSE_CODE_BAD_CONFIG);
    }
  }
  else {
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_CONFIG,
                              " [ response:type ]",
                              iota::types::RESPONSE_CODE_BAD_CONFIG);
  }
}

void iota::AdminService::add_common_urls(std::map<std::string,std::string>&
    filters) {

  // Root
  /*
    add_url(ADMIN_SERVICE_AGENTS, filters, REST_HANDLE(&iota::AdminService::agents),
            this);
    // Individual agent
    add_url(ADMIN_SERVICE_AGENTS+"/<agent>", filters,
            REST_HANDLE(&iota::AdminService::agent), this);
  */
  add_url(ADMIN_SERVICE_DEVICES, filters,
          REST_HANDLE(&iota::AdminService::devices), this);
  add_url(ADMIN_SERVICE_DEVICES + "/<device>", filters,
          REST_HANDLE(&iota::AdminService::device), this);
  add_url(ADMIN_SERVICE_SERVICES, filters,
          REST_HANDLE(&iota::AdminService::services), this);
  add_url(ADMIN_SERVICE_SERVICES + "/<service>", filters,
          REST_HANDLE(&iota::AdminService::service), this);
  add_url(ADMIN_SERVICE_ABOUT, filters, REST_HANDLE(&iota::AdminService::about),
          this);
  /*
  add_url(ADMIN_SERVICE_AGENTS+"/<agent>/services", filters,
  REST_HANDLE(&iota::AdminService::services), this);
  add_url(ADMIN_SERVICE_AGENTS+"/<agent>/services" + "/<service>", filters,
  REST_HANDLE(&iota::AdminService::service), this);
  */

}

void iota::AdminService::add_oauth_media_filters() {
  try {
    std::map<std::string, std::string> oauth_map;
    iota::Configurator::instance()->get(iota::types::CONF_FILE_OAUTH, oauth_map);

    if (oauth_map.size() > 0 &&
        !oauth_map[iota::types::CONF_FILE_OAUTH_VALIDATE_TOKEN_URL].empty() &&
        !oauth_map[iota::types::CONF_FILE_OAUTH_ROLES_URL].empty() &&
        !oauth_map[iota::types::CONF_FILE_OAUTH_PROJECTS_URL].empty() &&
        !oauth_map[iota::types::CONF_FILE_ACCESS_CONTROL].empty()) {
      boost::shared_ptr<iota::OAuthFilter> auth_ptr(new iota::OAuthFilter());
      auth_ptr->set_filter_url_base(get_resource());
      auth_ptr->set_configuration(oauth_map);
      add_pre_filter(auth_ptr);
    }

  }
  catch (std::exception& e) {
    IOTA_LOG_ERROR(m_log, "OAuth for northbound is not configured");
  }


  boost::shared_ptr<iota::MediaFilter> media_ptr(new iota::MediaFilter());
  add_pre_filter(media_ptr);

}


void iota::AdminService::check_for_logs() {

  // Check for logs ok
  _timer.reset(new boost::asio::deadline_timer(*
               (_connectionManager->get_io_service())));
  _timer->expires_from_now(boost::posix_time::seconds(1));
  _timer->async_wait(boost::bind(&iota::AdminService::timeout_check_logs,
                                 this,
                                 boost::asio::placeholders::error));
}

void iota::AdminService::start() {


  std::map<std::string, std::string> filters;


  add_common_urls(filters);


  try {
    const JsonValue& tz_file = iota::Configurator::instance()->get(
                                 iota::types::CONF_FILE_TIMEZONES);
    if (tz_file.IsString()) {
      set_timezone_database(tz_file.GetString());
    }
  }
  catch (std::exception& e) {
    IOTA_LOG_DEBUG(m_log, "Timezone database is not configured");
  }


  add_oauth_media_filters();
  check_for_logs();


}

void iota::AdminService::stop() {
  /*
    if ( _web_server.get() != NULL ) {
      std::cout << "RESET" << std::endl;
      _web_server.reset();
    }
    */
}


void iota::AdminService::about(pion::http::request_ptr& http_request_ptr,
                               std::map<std::string, std::string>& url_args,
                               std::multimap<std::string, std::string>& query_parameters,
                               pion::http::response& http_response,
                               std::string& response) {
  std::ostringstream ss;
  ss << "Welcome to IoTAgents ";
  ss << get_role();
  ss << gV_GIT;
  ss << " commit ";
  ss << gV_COMMIT;
  ss << " in ";
  ss << __DATE__;

  response.append(ss.str());
  IOTA_LOG_DEBUG(m_log, response);

}

void iota::AdminService::agents(pion::http::request_ptr& http_request_ptr,
                                std::map<std::string, std::string>& url_args,
                                std::multimap<std::string, std::string>& query_parameters,
                                pion::http::response& http_response,
                                std::string& response) {

  IOTA_LOG_INFO(m_log, get_class_name()+"::agents");
  std::string method = http_request_ptr->get_method();
  boost::mutex::scoped_lock lock(iota::AdminService::m_sm);
  if (method.compare(pion::http::types::REQUEST_METHOD_POST) == 0) {

    std::string global_conf = conf_global_adm(http_request_ptr, url_args,
                              query_parameters, http_response, response);
    std::string local_conf;
    if (global_conf.empty() == false) {
      response.assign(global_conf);
      error_response(http_response, response,
                     pion::http::types::RESPONSE_CODE_BAD_REQUEST);

      /*
      IOTA_LOG_INFO(m_log, "No Global Configuration");
      // Se evalua la incorporacion dinamica de un nuevo recurso

      JsonDocument document;

      if ( ! document.Parse<0>(http_request_ptr->get_content()).HasParseError() ) {
        if ( document.IsObject() &&
             document.HasMember("resource") &&
             document.HasMember("options") &&
             document["options"].HasMember("FileName") ) {
          if ( document["resource"].IsString() &&
               document["options"].IsObject() &&
               document["options"]["FileName"].IsString() ) {
            local_conf = true;
            std::string local_resource = document["resource"].GetString();
            std::string option_file_name = document["options"]["FileName"].GetString();
            if ( local_resource.substr(0, URL_BASE.size()) != URL_BASE) {
              std::string pfx(URL_BASE);
              pfx.append("/");
              local_resource.insert(0, pfx);
            }

            //start_plugin(local_resource, option_file_name);

          }
        }
      }
      */
    }
  }
  else if (method.compare(pion::http::types::REQUEST_METHOD_GET) == 0) {
    // All information if no params

    iota::Configurator* configurator = iota::Configurator::instance();

    // Configured agents (resources)
    JsonDocument get_agents;
    get_agents.SetObject();

    std::multimap<std::string, std::string>::const_iterator it_p =
      query_parameters.begin();
    bool with_conf = false;
    bool with_stat = false;

    it_p = query_parameters.find("conf");
    if (it_p != query_parameters.end()) {
      with_conf = true;
    }
    it_p = query_parameters.begin();
    it_p = query_parameters.find("stats");
    if (it_p != query_parameters.end()) {
      with_stat = true;
    }
    if (with_stat == false && with_conf == false) {
      with_stat = true;
      with_conf = true;
    }

    // Configuration
    if (with_conf == true) {
      get_agents.Parse<0>(configurator->getAll().c_str());
    }

    // Statistics
    if (with_stat == true) {
      std::map<std::string, iota::RestHandle*>::const_iterator it =
        _service_manager.begin();
      while (it != _service_manager.end()) {
        iota::RestHandle* agent = it->second;
        if (agent != NULL) {
          try {
            // para no crearlo de nuevo
            JsonValue statistics;
            statistics.SetArray();
            if (!get_agents.HasMember("statistics")) {
              get_agents.AddMember("statistics", statistics, get_agents.GetAllocator());
            }
            get_info_agent(agent, query_parameters, get_agents);
          }
          catch (std::exception& e) {
            response.assign(e.what());
            error_response(http_response, response);
          }
        }
        ++it;
      }
    }
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    get_agents.Accept(writer);
    response.assign(buffer.GetString());
    IOTA_LOG_DEBUG(m_log, response);
  }
  else if (method.compare(pion::http::types::REQUEST_METHOD_DELETE) == 0) {
    error_response(http_response, response,
                   pion::http::types::RESPONSE_CODE_NOT_IMPLEMENTED);
  }
  else {
    error_response(http_response, response,
                   pion::http::types::RESPONSE_CODE_NOT_FOUND);
  }
}

void iota::AdminService::agent(pion::http::request_ptr& http_request_ptr,
                               std::map<std::string, std::string>& url_args,
                               std::multimap<std::string, std::string>& query_parameters,
                               pion::http::response& http_response,
                               std::string& response) {

  std::string res_conf;
  std::string method = http_request_ptr->get_method();
  IOTA_LOG_INFO(m_log, method << " " << url_args["agent"]);

  // Esto esta implementado suponiendo un despliegue a partir de una url-base
  std::string my_resource(iota::URL_BASE);
  my_resource.append("/");
  my_resource.append(url_args["agent"]);
  iota::RestHandle* agent = get_service(my_resource);
  iota::Configurator* configurator = iota::Configurator::instance();
  std::multimap<std::string, std::string>::const_iterator it_p =
    query_parameters.begin();
  bool with_conf = false;
  bool with_stat = false;

  it_p = query_parameters.find("conf");
  if (it_p != query_parameters.end()) {
    with_conf = true;
  }
  it_p = query_parameters.begin();
  it_p = query_parameters.find("stats");
  if (it_p != query_parameters.end()) {
    with_stat = true;
  }
  if (with_stat == false && with_conf == false) {
    with_stat = true;
    with_conf = true;
  }

  // GET Retrieve information
  if (method.compare(pion::http::types::REQUEST_METHOD_GET) == 0) {
    try {
      JsonDocument get_agent;
      get_agent.SetObject();
      if (with_conf == true) {
        std::string resource_str(configurator->getResource(my_resource));
        get_agent.Parse<0>(resource_str.c_str());

      }
      if (agent != NULL && with_stat == true) {
        JsonValue statistics;
        statistics.SetArray();
        if (!get_agent.HasMember("statistics")) {
          get_agent.AddMember("statistics", statistics, get_agent.GetAllocator());
        }
        get_info_agent(agent, query_parameters, get_agent);
      }
      rapidjson::StringBuffer buffer;
      rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
      get_agent.Accept(writer);
      response.assign(buffer.GetString());
      IOTA_LOG_DEBUG(m_log, response);
    }
    catch (std::exception& e) {
      response.assign(e.what());
      error_response(http_response, response);
    }
  }
  else if (method.compare(pion::http::types::REQUEST_METHOD_PUT) == 0) {

    std::string content(http_request_ptr->get_content());
    configurator->add_resource(my_resource, content, res_conf);

    if (res_conf.empty() == false) {
      response.assign(res_conf);
      error_response(http_response, response,
                     pion::http::types::RESPONSE_CODE_BAD_REQUEST);
    }

  }
  else {
    error_response(http_response, response,
                   pion::http::types::RESPONSE_CODE_BAD_REQUEST);
  }
}

std::string iota::AdminService::conf_global_adm(pion::http::request_ptr&
    http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response,
    std::string& response) {


  std::string method = http_request_ptr->get_method();
  IOTA_LOG_DEBUG(m_log, get_class_name()+"::conf_adm " << method);
  iota::Configurator* configurator = iota::Configurator::instance();
  std::string global_configuration;

  if (method.compare(pion::http::types::REQUEST_METHOD_POST) == 0) {
    // New configuration
    std::stringstream ss;
    ss << http_request_ptr->get_content();
    try {
      global_configuration = configurator->update_conf(ss);
    }
    catch (std::exception& e) {
      global_configuration = e.what();
    }
  }
  return global_configuration;
}

bool iota::AdminService::is_mongo_active() {
  const JsonValue& storage = iota::Configurator::instance()->get(
                               iota::store::types::STORAGE);
  if (storage.HasMember(iota::store::types::TYPE.c_str())) {
    std::string type_store = storage[iota::store::types::TYPE.c_str()].GetString();
    return (type_store.compare("mongodb") ==0);
  }

  return false;
}

void iota::AdminService::devices(pion::http::request_ptr& http_request_ptr,
                                 std::map<std::string, std::string>& url_args,
                                 std::multimap<std::string, std::string>& query_parameters,
                                 pion::http::response& http_response,
                                 std::string& response) {

  std::string trace_message = http_request_ptr->get_header(
                                iota::types::HEADER_TRACE_MESSAGES);

  std::string token = http_request_ptr->get_header(
                        iota::types::IOT_HTTP_HEADER_AUTH);

  std::string method = http_request_ptr->get_method();
  IOTA_LOG_INFO(m_log, get_class_name()+"::devices method:" +method +
                " trace_message:" + trace_message);

  std::string protocol_filter;

  std::string reason;
  std::string error_details;
  std::string op, temp;
  int code = pion::http::types::RESPONSE_CODE_OK;
  try {
    check_mongo_config();
    std::string content_type(http_request_ptr->get_header(
                               pion::http::types::HEADER_CONTENT_TYPE));
    std::string service(http_request_ptr->get_header(
                          iota::types::FIWARE_SERVICE));
    std::string service_path(http_request_ptr->get_header(
                               iota::types::FIWARE_SERVICEPATH));

    if (service.empty() || (!service_path.empty() && service_path[0] != '/')) {
      reason.append(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER);
      error_details.append(iota::types::FIWARE_SERVICE + "/" +
                           iota::types::FIWARE_SERVICEPATH + " not present or invalid");
      IOTA_LOG_ERROR(m_log, response);
      create_response(pion::http::types::RESPONSE_CODE_BAD_REQUEST,
                      reason, error_details,
                      http_response, response);
      IOTA_LOG_INFO(m_log, get_class_name()+"::devices method:" +method +
                    "  trace_message:" + trace_message+
                    " code: " + boost::lexical_cast<std::string>
                    (pion::http::types::RESPONSE_CODE_BAD_REQUEST)+
                    " response:" + response);
      return;
    }
    if (service_path.empty()) {
      service_path = iota::types::FIWARE_SERVICEPATH_DEFAULT;
    }

    if (method.compare(pion::http::types::REQUEST_METHOD_POST) == 0) {
      std::string content = http_request_ptr->get_content();
      boost::trim(content);
      code = post_device_json(service,  service_path,
                              content, http_response, response,token);
    }
    else if (method.compare(pion::http::types::REQUEST_METHOD_GET) == 0) {
      std::string  detailed, entity;
      int limit = types::LIMIT_DEFAULT, offset =0;
      std::multimap<std::string,std::string>::iterator it;
      it = query_parameters.find(iota::store::types::LIMIT);
      if (it != query_parameters.end()) {
        temp = it->second;
        op = "limit";
        if (!temp.empty()) {
          limit = boost::lexical_cast<int>(temp);
        }
        else {
          limit = types::LIMIT_DEFAULT;
        }
        if (limit < -1) {
          IOTA_LOG_ERROR(m_log, " bad limit using default");
          limit = types::LIMIT_DEFAULT;
        }
        else if (limit > types::LIMIT_MAX) {
          IOTA_LOG_ERROR(m_log, " bad limit using maximun");
          limit = types::LIMIT_MAX;
        }
      }
      it = query_parameters.find(iota::store::types::OFFSET);
      if (it != query_parameters.end()) {
        temp = it->second;
        op  ="offset";
        if (!temp.empty()) {
          offset = boost::lexical_cast<int>(temp);
        }
        else {
          offset = 0;
        }
      }
      it = query_parameters.find(iota::store::types::DETAILED);
      if (it != query_parameters.end()) {
        detailed = it->second;
      }
      it = query_parameters.find(iota::store::types::ENTITY);
      if (it != query_parameters.end()) {
        entity = it->second;
      }

      it = query_parameters.find(iota::store::types::PROTOCOL);
      if (it != query_parameters.end()) {
        protocol_filter = it->second;
      }

      code = get_all_devices_json(service, service_path, limit, offset, detailed,
                                  entity,
                                  http_response, response,trace_message,token,protocol_filter);

    }
    else {
      code = iota::types::RESPONSE_CODE_METHOD_NOT_ALLOWED;
      reason.assign(iota::types::RESPONSE_MESSAGE_METHOD_NOT_ALLOWED);
      create_response(code, reason, error_details, http_response, response);
    }
  }
  catch (const boost::bad_lexical_cast& e) {
    IOTA_LOG_ERROR(m_log,"Capturada boost::bad_lexical_cast en devices");
    IOTA_LOG_ERROR(m_log,e.what());
    reason.assign(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER);
    error_details.assign(op + " must be a number but it is " + temp);
    code = 400;
    create_response(code, reason, error_details, http_response, response);

  }
  catch (iota::IotaException& e) {
    IOTA_LOG_ERROR(m_log,"Capturada: Exception en devices_json");
    IOTA_LOG_ERROR(m_log,e.what());
    reason.assign(e.reason());
    error_details.assign(e.what());
    code = e.status();
    create_response(code, reason, error_details, http_response, response);
  }
  catch (std::exception& e) {
    IOTA_LOG_ERROR(m_log,"Excepcion en devices_json");
    reason.assign(iota::types::RESPONSE_MESSAGE_INTERNAL_ERROR);
    error_details.assign(e.what());
    code = pion::http::types::RESPONSE_CODE_SERVER_ERROR;
    create_response(code, reason, error_details, http_response, response);
  }

  IOTA_LOG_INFO(m_log, get_class_name()+"::devices method:" +method +
                " trace_message:" + trace_message+
                " code: " + boost::lexical_cast<std::string>(code)+
                " response:" + response);

}

void iota::AdminService::device(pion::http::request_ptr& http_request_ptr,
                                std::map<std::string, std::string>& url_args,
                                std::multimap<std::string, std::string>& query_parameters,
                                pion::http::response& http_response,
                                std::string& response) {

  std::string trace_message = http_request_ptr->get_header(
                                iota::types::HEADER_TRACE_MESSAGES);

  std::string token = http_request_ptr->get_header(
                        iota::types::IOT_HTTP_HEADER_AUTH);


  std::string method = http_request_ptr->get_method();
  IOTA_LOG_INFO(m_log, get_class_name()+"::device method:" +method +
                " trace_message:" + trace_message);
  std::string reason;
  std::string error_details;
  int code = pion::http::types::RESPONSE_CODE_OK;
  std::string device_in_url = url_args[_api_device_holder];
  try {
    std::string content_type(http_request_ptr->get_header(
                               pion::http::types::HEADER_CONTENT_TYPE));
    std::string service(http_request_ptr->get_header(
                          iota::types::FIWARE_SERVICE));
    std::string service_path(http_request_ptr->get_header(
                               iota::types::FIWARE_SERVICEPATH));

    std::string protocol_filter;
    //Following access to protocol_filter can be taken to a method.
    std::multimap<std::string, std::string>::iterator it = query_parameters.begin();
    it = query_parameters.find(iota::store::types::PROTOCOL);
    if (it != query_parameters.end()) {
      protocol_filter = it->second;
    }


    if (service.empty() || (!service_path.empty() && service_path[0] != '/')) {
      reason.append(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER);
      error_details.append(iota::types::FIWARE_SERVICE + "/" +
                           iota::types::FIWARE_SERVICEPATH + " not present or invalid");
      IOTA_LOG_ERROR(m_log, response);
      create_response(pion::http::types::RESPONSE_CODE_BAD_REQUEST,
                      reason, error_details,
                      http_response, response);
      IOTA_LOG_INFO(m_log, get_class_name()+"::device method:" +method +
                    " trace_message:" + trace_message+
                    " code: " + boost::lexical_cast<std::string>
                    (pion::http::types::RESPONSE_CODE_BAD_REQUEST)+
                    " response:" + response);
      return;
    }
    if (service_path.empty()) {
      service_path = iota::types::FIWARE_SERVICEPATH_DEFAULT;
    }

    if (method.compare(pion::http::types::REQUEST_METHOD_PUT) == 0) {
      std::string content = http_request_ptr->get_content();
      boost::trim(content);

      code = put_device_json(service,  service_path, device_in_url,
                             content, http_response, response,token,protocol_filter);
    }
    else if (method.compare(pion::http::types::REQUEST_METHOD_GET) == 0) {


      code = get_a_device_json(service, service_path, device_in_url, http_response,
                               response,trace_message,token,protocol_filter);
    }
    else if (method.compare(pion::http::types::REQUEST_METHOD_DELETE) == 0) {
      code = delete_device_json(service, service_path,
                                device_in_url, http_response, response, token,
                                protocol_filter);
    }
    else {
      code = iota::types::RESPONSE_CODE_METHOD_NOT_ALLOWED;
      create_response(code, reason, error_details, http_response, response);
    }

  }
  catch (iota::IotaException& e) {
    IOTA_LOG_ERROR(m_log,"Capturada: Exception en devices_json");
    IOTA_LOG_ERROR(m_log,e.what());
    reason.assign(e.reason());
    error_details.assign(e.what());
    code = e.status();
    create_response(code, reason, error_details, http_response, response);
  }
  catch (std::exception& e) {
    IOTA_LOG_ERROR(m_log,"Excepcion en devices_json");
    IOTA_LOG_ERROR(m_log,"Excepcion en devices_json");
    reason.assign(iota::types::RESPONSE_MESSAGE_INTERNAL_ERROR);
    error_details.assign(e.what());
    code = pion::http::types::RESPONSE_CODE_SERVER_ERROR;
    create_response(code, reason, error_details, http_response, response);
  }

  IOTA_LOG_INFO(m_log, get_class_name()+"::device method:" +method +
                " trace_message:" + trace_message+
                " code: " + boost::lexical_cast<std::string>(code)+
                " response:" + response);

}

void iota::AdminService::create_collection(
  boost::shared_ptr<iota::ServiceCollection>& col) {
  col.reset(new ServiceCollection());
}


std::string iota::AdminService::get_param_resource(
  const std::multimap<std::string, std::string>& query_parameters,
  bool mandatory) {
  std::string result;
  std::multimap<std::string,std::string>::const_iterator it;

  it = query_parameters.find(iota::store::types::RESOURCE);
  if (it != query_parameters.end()) {
    result = it->second;
  }

  if (mandatory && result.empty()) {
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_MISSING_PARAMETER,
                              "resource parameter is mandatory",
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  return result;
}

void iota::AdminService::services(pion::http::request_ptr& http_request_ptr,
                                  std::map<std::string, std::string>& url_args,
                                  std::multimap<std::string, std::string>& query_parameters,
                                  pion::http::response& http_response,
                                  std::string& response) {

  std::string trace_message = http_request_ptr->get_header(
                                iota::types::HEADER_TRACE_MESSAGES);
  std::string token = http_request_ptr->get_header(
                        iota::types::IOT_HTTP_HEADER_AUTH);

  std::string method = http_request_ptr->get_method();
  IOTA_LOG_INFO(m_log, get_class_name()+"::services method:" << method <<
                " trace_message:" << trace_message << "" << get_role());

  std::string reason;
  std::string error_details;
  int code = pion::http::types::RESPONSE_CODE_OK;
  std::string service_in_url;
  std::string temp, op;
  boost::shared_ptr<iota::ServiceCollection> col;

  try {
    check_mongo_config();

    std::string content_type(http_request_ptr->get_header(
                               pion::http::types::HEADER_CONTENT_TYPE));
    std::string service(http_request_ptr->get_header(
                          iota::types::FIWARE_SERVICE));
    std::string service_path(http_request_ptr->get_header(
                               iota::types::FIWARE_SERVICEPATH));
    if (service.empty() || (!service_path.empty() && service_path[0] != '/')) {
      reason.append(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER);
      error_details.append(iota::types::FIWARE_SERVICE + "/" +
                           iota::types::FIWARE_SERVICEPATH + " not present or invalid");
      IOTA_LOG_ERROR(m_log, reason << " service:" << service << " service_path:" <<
                     service_path);
      create_response(pion::http::types::RESPONSE_CODE_BAD_REQUEST,
                      reason, error_details,
                      http_response, response);
      IOTA_LOG_INFO(m_log, get_class_name()+"::services method:" +method +
                    " trace_message:" + trace_message+
                    " code: " + boost::lexical_cast<std::string>
                    (pion::http::types::RESPONSE_CODE_BAD_REQUEST)+
                    " response:" + response);
      return;
    }

    create_collection(col);

    if (service_path.empty()) {
      service_path = iota::types::FIWARE_SERVICEPATH_DEFAULT;
    }
    service_in_url = service;
    if (method.compare(pion::http::types::REQUEST_METHOD_POST) == 0) {
      std::string content = http_request_ptr->get_content();
      boost::trim(content);
      code = post_service_json(col, service,  service_path,
                               content, http_response, response, token, trace_message);
    }
    else if (method.compare(pion::http::types::REQUEST_METHOD_GET) == 0) {

      std::string detailed, resource;
      int limit = types::LIMIT_DEFAULT, offset =0;
      std::multimap<std::string,std::string>::iterator it;
      it = query_parameters.find(iota::store::types::LIMIT);
      if (it != query_parameters.end()) {
        temp = it->second;
        op = "limit";
        if (!temp.empty()) {
          limit = boost::lexical_cast<int>(temp);
        }
        else {
          limit = types::LIMIT_DEFAULT;
        }
        if (limit < -1) {
          IOTA_LOG_ERROR(m_log, " bad limit using default");
          limit = types::LIMIT_DEFAULT;
        }
        else if (limit > types::LIMIT_MAX) {
          IOTA_LOG_ERROR(m_log, " bad limit using maximun");
          limit = types::LIMIT_MAX;
        }
      }
      it = query_parameters.find(iota::store::types::OFFSET);
      if (it != query_parameters.end()) {
        temp = it->second;
        op = "offset";
        if (!temp.empty()) {
          offset = boost::lexical_cast<int>(temp);
        }
        else {
          offset = 0;
        }
      }
      it = query_parameters.find(iota::store::types::DETAILED);
      if (it != query_parameters.end()) {
        detailed = it->second;
      }

      resource = get_param_resource(query_parameters,false);

      code = get_all_services_json(col, service, service_path, limit,
                                   offset, "on", resource, http_response, response);

    }
    else if (method.compare(pion::http::types::REQUEST_METHOD_PUT) == 0) {
      // Not allowed
      if (service_path.size() == 2 && (service_path.find("/*") == 0 ||
                                       service_path.find("/#") == 0)) {
        code = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
        reason.assign(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER);
        error_details.assign("Fiware-ServicePath is invalid in PUT operation ["+
                             service_path
                             + "]");
        create_response(code, reason, error_details, http_response, response);
      }
      else {
        std::string content = http_request_ptr->get_content();
        boost::trim(content);
        std::multimap<std::string,std::string>::iterator it;
        std::string apikey, resource;
        it = query_parameters.find(iota::store::types::APIKEY);
        if (it != query_parameters.end()) {
          apikey = it->second;
        }
        it = query_parameters.find(iota::store::types::RESOURCE);
        if (it != query_parameters.end()) {
          resource = it->second;
        }

        code = put_service_json(col, service,  service_path, service_in_url, apikey,
                                resource,
                                content, http_response, response, token, trace_message);

      }
    }
    else if (method.compare(pion::http::types::REQUEST_METHOD_DELETE) == 0) {

      // Not allowed

      bool remove_devices = false;
      std::string apikey, resource;

      resource = get_param_resource(query_parameters,true);
      try {
        std::string r_d("false");
        std::multimap<std::string,std::string>::iterator it;
        it = query_parameters.find(iota::store::types::DEVICE_PAR);
        if (it != query_parameters.end()) {
          r_d = it->second;
        }
        //remove_devices = boost::lexical_cast<bool>(r_d);
        std::istringstream(r_d) >> std::boolalpha >> remove_devices;
        it = query_parameters.find(iota::store::types::APIKEY);
        if (it != query_parameters.end()) {
          apikey = it->second;
        }
      }
      catch (std::exception& e) {
        code = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
        reason.assign(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER);
        error_details.assign("device parameter is invalid [true/false]");
        create_response(code, reason, error_details, http_response, response);
        IOTA_LOG_INFO(m_log, get_class_name()+"::devices method:" +method +
                      " trace_message:" + trace_message+
                      " code: " + boost::lexical_cast<std::string>
                      (pion::http::types::RESPONSE_CODE_BAD_REQUEST)+
                      " response:" + response);
        return;
      }
      if (service_path.size() == 2 && (service_path.find("/*") == 0 ||
                                       service_path.find("/#") == 0) && remove_devices == true) {
        code = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
        reason.assign(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER);
        error_details.assign("Fiware-ServicePath is invalid in DELETE operation ["+
                             service_path
                             + "] removing devices");
        create_response(code, reason, error_details, http_response, response);
      }
      else {
        code = delete_service_json(col, service, service_path,
                                   service_in_url, apikey, resource, remove_devices,
                                   http_response, response, token, trace_message);
        create_response(code, reason, error_details, http_response, response);
      }
    }
    else {
      code = pion::http::types::RESPONSE_CODE_NOT_FOUND;
      create_response(code, reason, error_details, http_response, response);
    }

  }
  catch (const boost::bad_lexical_cast& e) {
    IOTA_LOG_ERROR(m_log,"Capturada boost::bad_lexical_cast en services");
    IOTA_LOG_ERROR(m_log,e.what());
    reason.assign(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER);
    error_details.assign(op + " must be a number but it is " + temp);
    code = 400;
    create_response(code, reason, error_details, http_response, response);
  }
  catch (iota::IotaException& e) {
    IOTA_LOG_ERROR(m_log,"Capturada: Exception en services");
    IOTA_LOG_ERROR(m_log,e.what());
    reason.assign(e.reason());
    error_details.assign(e.what());
    code = e.status();
    create_response(code, reason, error_details, http_response, response);
  }
  catch (std::exception& e) {
    IOTA_LOG_ERROR(m_log,"Excepcion en services");
    reason.assign(iota::types::RESPONSE_MESSAGE_INTERNAL_ERROR);
    error_details.assign(e.what());
    code = pion::http::types::RESPONSE_CODE_SERVER_ERROR;
    create_response(code, reason, error_details, http_response, response);
  }

  IOTA_LOG_INFO(m_log, get_class_name()+"::services method:" +method +
                " trace_message:" + trace_message+
                " code: " + boost::lexical_cast<std::string>(code)+
                " response:" + response);
}

void iota::AdminService::service(pion::http::request_ptr& http_request_ptr,
                                 std::map<std::string, std::string>& url_args,
                                 std::multimap<std::string, std::string>& query_parameters,
                                 pion::http::response& http_response,
                                 std::string& response) {

  std::string trace_message = http_request_ptr->get_header(
                                iota::types::HEADER_TRACE_MESSAGES);
  std::string token = http_request_ptr->get_header(
                        iota::types::IOT_HTTP_HEADER_AUTH);

  std::string method = http_request_ptr->get_method();
  IOTA_LOG_INFO(m_log, get_class_name()+"::service method:" +method +
                " trace_message:" + trace_message);
  std::string reason;
  std::string error_details;
  int code = pion::http::types::RESPONSE_CODE_OK;
  std::string service_in_url = url_args[_api_service_holder];
  boost::shared_ptr<iota::ServiceCollection> col;

  try {
    check_mongo_config();

    create_collection(col);


    std::string content_type(http_request_ptr->get_header(
                               pion::http::types::HEADER_CONTENT_TYPE));
    std::string service(http_request_ptr->get_header(
                          iota::types::FIWARE_SERVICE));
    std::string service_path(http_request_ptr->get_header(
                               iota::types::FIWARE_SERVICEPATH));

    if (service.empty() || (!service_path.empty() && service_path[0] != '/')) {
      reason.append(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER);
      error_details.append(iota::types::FIWARE_SERVICE + "/" +
                           iota::types::FIWARE_SERVICEPATH + " not present or invalid");
    }
    else if (service_in_url.empty()) {
      reason.append(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER);
      error_details.assign("<service_id> not provided in url");
      error_details.append(iota::types::FIWARE_SERVICE);
    }

    if (!reason.empty()) {
      create_response(pion::http::types::RESPONSE_CODE_BAD_REQUEST,
                      reason, error_details,
                      http_response, response);
      IOTA_LOG_INFO(m_log, get_class_name()+"::service method:" +method +
                    " trace_message:" + trace_message+
                    " code: " + boost::lexical_cast<std::string>
                    (pion::http::types::RESPONSE_CODE_BAD_REQUEST)+
                    " response:" + response);
      return;
    }

    if (service_path.empty()) {
      service_path = iota::types::FIWARE_SERVICEPATH_DEFAULT;
    }
    if (service.compare(service_in_url) != 0) {
      reason.assign(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER);
      error_details.assign("service in url conflicts with " +
                           iota::types::FIWARE_SERVICE);
      create_response(pion::http::types::RESPONSE_CODE_BAD_REQUEST,
                      reason, error_details,
                      http_response, response);
      IOTA_LOG_INFO(m_log, get_class_name()+"::service method:" +method +
                    " trace_message:" + trace_message+
                    " code: " + boost::lexical_cast<std::string>
                    (pion::http::types::RESPONSE_CODE_BAD_REQUEST)+
                    " response:" + response);
      return;
    }
    if (method.compare(pion::http::types::REQUEST_METHOD_PUT) == 0) {

      std::string content = http_request_ptr->get_content();
      boost::trim(content);
      std::multimap<std::string,std::string>::iterator it;
      std::string apikey, resource;
      it = query_parameters.find(iota::store::types::APIKEY);
      if (it != query_parameters.end()) {
        apikey = it->second;
      }
      it = query_parameters.find(iota::store::types::RESOURCE);
      if (it != query_parameters.end()) {
        resource = it->second;
      }
      if (resource.empty()) {
        code = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
        reason.assign(iota::types::RESPONSE_MESSAGE_MISSING_PARAMETER);
        error_details.assign("resource parameter is mandatory in PUT operation");
        create_response(code, reason, error_details, http_response, response);
      }
      else {
        code = put_service_json(col, service,  service_path, service_in_url, apikey,
                                resource,
                                content, http_response, response, token, trace_message);
      }
    }
    else if (method.compare(pion::http::types::REQUEST_METHOD_GET) == 0) {
      //code = get_a_service_json(service, service_path, service_in_url, http_response, response);
      std::multimap<std::string,std::string>::iterator it;
      std::string  resource;
      it = query_parameters.find(iota::store::types::RESOURCE);
      if (it != query_parameters.end()) {
        resource = it->second;
      }
      code = get_all_services_json(col, service, service_path, 0, 0,
                                   "on", resource, http_response, response);
    }
    else if (method.compare(pion::http::types::REQUEST_METHOD_DELETE) == 0) {
      bool remove_devices = false;
      std::string apikey, resource;

      resource = get_param_resource(query_parameters,true);
      try {
        std::string r_d("false");
        std::multimap<std::string,std::string>::iterator it;
        it = query_parameters.find(iota::store::types::DEVICE_PAR);
        if (it != query_parameters.end()) {
          r_d = it->second;
        }
        //remove_devices = boost::lexical_cast<bool>(r_d);
        std::istringstream(r_d) >> std::boolalpha >> remove_devices;
        it = query_parameters.find(iota::store::types::APIKEY);
        if (it != query_parameters.end()) {
          apikey = it->second;
        }

      }
      catch (std::exception& e) {
        code = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
        reason.assign(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER);
        error_details.assign("device parameter is invalid [true/false]");
        create_response(code, reason, error_details, http_response, response);
        IOTA_LOG_INFO(m_log, "iota::AdminService::service method:" +method +
                      " trace_message:" + trace_message+
                      " code: " + boost::lexical_cast<std::string>(code)+
                      " response:" + response);
        return;
      }
      if (service_path.size() == 2 && (service_path.find("/*") == 0 ||
                                       service_path.find("/#") == 0) && remove_devices == true) {
        code = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
        reason.assign(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER);
        error_details.assign("Fiware-ServicePath is invalid in DELETE operation ["+
                             service_path
                             + "] removing devices");
        create_response(code, reason, error_details, http_response, response);
      }
      else {
        code = delete_service_json(col, service, service_path,
                                   service_in_url, apikey, resource, remove_devices,
                                   http_response, response, token, trace_message);
      }
    }
    else {
      code = pion::http::types::RESPONSE_CODE_NOT_FOUND;
      create_response(code, reason, error_details, http_response, response);
    }

  }
  catch (iota::IotaException& e) {
    IOTA_LOG_ERROR(m_log,"Capturada: Exception en services");
    IOTA_LOG_ERROR(m_log,e.what());
    reason.assign(e.reason());
    error_details.assign(e.what());
    code = e.status();
    create_response(code, reason, error_details, http_response, response);
  }
  catch (std::exception& e) {
    IOTA_LOG_ERROR(m_log,"Excepcion en services");
    reason.assign(iota::types::RESPONSE_MESSAGE_INTERNAL_ERROR);
    error_details.assign(e.what());
    code = pion::http::types::RESPONSE_CODE_SERVER_ERROR;
    create_response(code, reason, error_details, http_response, response);
  }

  IOTA_LOG_INFO(m_log, "iota::AdminService::service method:" +method +
                " trace_message:" + trace_message+
                " code: " + boost::lexical_cast<std::string>(code)+
                " response:" + response);
}


void iota::AdminService::start_plugin(std::string& resource,
                                      std::string& plugin_name) {
	/*
  boost::shared_ptr<pion::http::plugin_server> w_s = _web_server.lock();
  w_s->load_service(resource, plugin_name);
	*/
}

int iota::AdminService::create_response(
  const unsigned int status_code,
  const std::string& content,
  const std::string& error_details,
  pion::http::response& http_response,
  std::string& response) {
  std::ostringstream stream;
  if (status_code >= 299) {
    if (!content.empty() && !error_details.empty()) {
      std::string o_content(content);
      std::string o_details(error_details);
      boost::erase_all(o_content, "\"");
      boost::erase_all(o_details, "\"");
      stream << "{"
             <<   "\"reason\":\"" << o_content << "\","
             <<   "\"details\":\"" << o_details << "\"}";
      response.assign(stream.str());
    }
  }
  else {
    response.assign(content);
  }
  if (!content.empty()) {
    http_response.add_header(pion::http::types::HEADER_CONTENT_TYPE,
                             iota::types::IOT_CONTENT_TYPE_JSON);
  }
  http_response.set_status_code(status_code);
  http_response.set_status_message(iota::Configurator::instance()->getHttpMessage(
                                     status_code));
  IOTA_LOG_DEBUG(m_log, "create_response: " << status_code <<
                 " " << response);

  return status_code;
}

void iota::AdminService::error_response(pion::http::response& http_response,
                                        std::string& buffer,
                                        unsigned int status_code) {
  IOTA_LOG_DEBUG(m_log, get_class_name()+"::error_response " << status_code);
  if (status_code != 0) {
    http_response.set_status_code(status_code);
    http_response.set_status_message(iota::Configurator::instance()->getHttpMessage(
                                       status_code));
  }
  else {
    http_response.set_status_code(pion::http::types::RESPONSE_CODE_NOT_FOUND);
    http_response.set_status_message(iota::Configurator::instance()->getHttpMessage(
                                       pion::http::types::RESPONSE_CODE_NOT_FOUND));
  }
  if (buffer.empty() == false) {
    std::string error_content(buffer);
    buffer.clear();
    buffer.assign("{");
    buffer.append("\"reason\": \"");
    buffer.append(error_content);
    buffer.append("\"}");
  }
}

void iota::AdminService::remove_from_cache(Device& device) {
  IOTA_LOG_DEBUG(m_log, "registeredDevices.remove ");

  for (std::map<std::string, iota::RestHandle*>::iterator it=
         _service_manager.begin();
       it!=_service_manager.end(); ++it) {
    IOTA_LOG_DEBUG(m_log, "registeredDevices.remove " << it->first);
    it->second->remove_devices_from_cache(device);
  }

}

void iota::AdminService::add_service(const std::string& resource,
                                     iota::RestHandle* service_ptr) {

  IOTA_LOG_DEBUG(m_log, "Registering " << resource);
  boost::mutex::scoped_lock lock(iota::AdminService::m_sm);
  std::pair<std::map<std::string, iota::RestHandle*>::iterator, bool> p;
  p = _service_manager.insert(std::pair<std::string, iota::RestHandle*>(resource,
                              service_ptr));
}

iota::RestHandle* iota::AdminService::get_service(const std::string& resource) {
  iota::RestHandle* service_ptr = NULL;
  boost::mutex::scoped_lock lock(iota::AdminService::m_sm);
  std::map<std::string, iota::RestHandle*>::iterator it =
    _service_manager.begin();
  it = _service_manager.find(resource);
  if (it != _service_manager.end()) {
    //return dynamic_cast<iota::RestHandle*>(service_ptr);
    return it->second;
  }
  return NULL;
}

void iota::AdminService::get_object_json(
  const std::string& obj_str,
  JsonValue& obj,
  JsonDocument& d) {

  JsonDocument document;
  if (check_json(obj_str, document).empty() == false) {
    std::ostringstream what;
    what << "Parsing document: ";
    what << obj_str;
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER,
                              what.str(),
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  if (document.IsObject()) {
    obj.SetObject();
    for (JsonValue::MemberIterator itr = document.MemberBegin();
         itr != document.MemberEnd(); itr++) {
      JsonValue v(rapidjson::kObjectType);
      v.SetString(itr->name.GetString(), itr->name.GetStringLength(),
                  d.GetAllocator());
      obj.AddMember(v, itr->value, d.GetAllocator());

    }
  }
  else if (document.IsArray()) {
    obj.SetArray();
    for (int i = 0; i < document.Size(); i++) {
      obj.PushBack(document[i], d.GetAllocator());
    }
  }

}

void iota::AdminService::get_info_agent(iota::RestHandle* agent,
                                        std::multimap<std::string, std::string>& query_parameters,
                                        JsonDocument& d) {


  // Admin no
	//
  if (agent->get_resource().compare(get_resource()) == 0) {
		IOTA_LOG_DEBUG(m_log, "Stats " << get_resource());
  }
  JsonValue obj_resource;
  obj_resource.SetObject();
  obj_resource.AddMember("resource", agent->get_resource().c_str(),
                         d.GetAllocator());
  std::string stats(agent->get_statistics());
  JsonValue obj;
  obj.SetObject();
  get_object_json(stats, obj, d);
  obj_resource.AddMember("data", obj, d.GetAllocator());
  d["statistics"].PushBack(obj_resource, d.GetAllocator());
}

boost::posix_time::ptime iota::AdminService::get_local_time_from_timezone(
  std::string timezone_str) {

  IOTA_LOG_DEBUG(m_log, "UTC from " << timezone_str);
  boost::posix_time::ptime utcNow =
    boost::posix_time::second_clock::universal_time();
  boost::local_time::time_zone_ptr tz = _timezone_database.time_zone_from_region(
                                          timezone_str);
  boost::posix_time::ptime my_time;

  if (tz.get() != NULL) {
    boost::local_time::local_date_time tz_date_time(utcNow, tz);
    my_time = tz_date_time.local_time();
  }
  else {
    my_time = utcNow;
  }
  IOTA_LOG_DEBUG(m_log, "End get_local_time_from_timezone " << my_time);
  return my_time;
}

bool iota::AdminService::validate_json_schema(
  const std::string& json_str,
  const boost::shared_ptr<iota::Collection>& table,
  const std::string& method,
  std::string& response) {

  std::ostringstream errorSTR;
  int count=0;
  std::string json_schema;
  bool res = true;

#if defined LIBVARIANT

  json_schema = table->getSchema(method);
  if (json_schema.empty()) {
    std::string err = "validate_json_schema for ";
    err.append(table->getBBDD());
    err.append(" is not implemented");
    IOTA_LOG_DEBUG(m_log, err);
    response.assign(err);
    return false;
  }

  IOTA_LOG_DEBUG(m_log, "json:" << json_str);
  IOTA_LOG_DEBUG(m_log, "jsonschema:" << table->getBBDD());

  try {
    libvariant::Variant data = libvariant::DeserializeGuess(json_str);
    libvariant::Variant schema_data = libvariant::DeserializeGuess(json_schema);
    libvariant::AdvSchemaLoader loader5;
    libvariant::SchemaResult result = libvariant::SchemaValidate(
                                        schema_data,
                                        data, &loader5);
    if (result.Error()) {
      std::string validation_error("Malformed data");
      std::string data_path;
      try {
        validation_error = result.GetErrors().at(0).GetMessage();
        data_path = result.GetErrors().at(0).GetDataPathStr();
      }
      catch (std::exception& e) {}
      errorSTR << validation_error;
      errorSTR << " [" << data_path << "]";
      response.assign(errorSTR.str());
      IOTA_LOG_DEBUG(m_log, "Validation error [" << response <<
                     "/" << data_path <<
                     "]");
      res = false;
    }
    else {
      IOTA_LOG_DEBUG(m_log, "validation ok");
    }

  }
  catch (const std::exception& exc) {
    IOTA_LOG_ERROR(m_log, "error validating the json: "  <<exc.what());
    response.assign(exc.what());
    res = false;
  }

#endif // defined
  return res;
}

std::string iota::AdminService::check_json(const std::string& json_str,
    JsonDocument& doc) {
  std::string error;
  if (doc.Parse<0>(json_str.c_str()).HasParseError()) {
    error.assign(doc.GetParseError());
  }
  return error;
}

void iota::AdminService::print_services() {
  std::map<std::string, iota::RestHandle*>::const_iterator it =
    _service_manager.begin();
  while (it != _service_manager.end()) {
    iota::RestHandle* agent = it->second;
    std::cout << it->first << std::endl;
    ++it;
  }

}

int iota::AdminService::post_device_json(
  const std::string& service,
  const std::string& service_path,
  const std::string& body,
  pion::http::response& http_response,
  std::string& response,
  std::string token) {

  std::string param_request("post_device_json service=" + service +
                            " service_path=" +
                            service_path + " content=" + body);
  IOTA_LOG_DEBUG(m_log, param_request);
  int code = pion::http::types::RESPONSE_CODE_CREATED;
  std::string reason;
  std::string error_details;
  std::string device_to_post;
  boost::shared_ptr<iota::ServiceCollection> table(new ServiceCollection());
  boost::shared_ptr<iota::DeviceCollection> devTable(new DeviceCollection());;
  std::string service_exists = get_service_json(table, service, service_path);

  if (body.empty()) {
    error_details.assign("empty body");
    reason.assign(types::RESPONSE_MESSAGE_BAD_REQUEST);
    code = types::RESPONSE_CODE_BAD_REQUEST;
  }
  else if (service_exists.empty()) {
    error_details.assign(" service:");
    error_details.append(service);
    error_details.append(" service_path:");
    error_details.append(service_path);
    reason.assign(types::RESPONSE_MESSAGE_NO_SERVICE);
    code = types::RESPONSE_CODE_NO_SERVICE;
  }
  else if (validate_json_schema(body, devTable,
                                "POST", error_details)) {

    mongo::BSONObj obj =  mongo::fromjson(body);
    std::vector<mongo::BSONElement> be = obj.getField(
                                           iota::store::types::DEVICES).Array();
    for (unsigned int i = 0; i<be.size(); i++) {
      mongo::BSONObj bo = be[i].embeddedObject();
      mongo::BSONObjBuilder builder;
      builder.appendElements(bo);
      builder.append(iota::store::types::SERVICE, service);
      builder.append(iota::store::types::SERVICE_PATH, service_path);
      device_to_post.assign(bo.getField(iota::store::types::DEVICE_ID).String());
      IOTA_LOG_DEBUG(m_log, "device:  " << device_to_post);

      mongo::BSONObj insertObj = builder.obj();
      std::string entity_name =
        insertObj.getStringField(store::types::ENTITY_NAME);
      if (!entity_name.empty()) {
        // "entity_name" : 1, "service" : 1, "service_path" : 1
        devTable->find(BSON(store::types::ENTITY_NAME <<  entity_name <<
                            store::types::SERVICE << service <<
                            store::types::SERVICE_PATH <<service_path));
        if (devTable->more()) {
          throw iota::IotaException(iota::types::RESPONSE_MESSAGE_ENTITY_ALREADY_EXISTS,
                                    " [ entity_name: " + entity_name + "]",
                                    iota::types::RESPONSE_CODE_ENTITY_ALREADY_EXISTS);
        }
      }

      std::string protocol_name =
        insertObj.getStringField(store::types::PROTOCOL_NAME);
      if (!check_device_protocol(protocol_name, service, service_path, table)) {
        throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_PROTOCOL,
                                  " [ protocol: " + protocol_name + "]",
                                  iota::types::RESPONSE_CODE_BAD_REQUEST);
      }

      devTable->insert(insertObj);

      // If commands or internal attributes, register
      Device dev(device_to_post, service);
      dev._service_path = service_path;
      dev._protocol = protocol_name;
      deploy_device(dev);
    }
    if (be.size() == 1) {
      http_response.add_header(pion::http::types::HEADER_LOCATION,
                               iota::URL_BASE + iota::ADMIN_SERVICE_DEVICES + "/" + device_to_post);
    }
  }
  else {
    reason.assign(types::RESPONSE_MESSAGE_BAD_REQUEST);
    code = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
  }

  return create_response(code, reason, error_details, http_response, response);
}


int iota::AdminService::put_device_json(
  const std::string& service,
  const std::string& service_path,
  const std::string& device_id,
  const std::string& body,
  pion::http::response& http_response,
  std::string& response,
  const std::string& x_auth_token,
  const std::string& protocol) {

  std::string param_request("put_device_json service=" + service +
                            " service_path=" +
                            service_path + " device=" + device_id + " content=" + body);

  IOTA_LOG_DEBUG(m_log, param_request);
  int code = pion::http::types::RESPONSE_CODE_NO_CONTENT;
  std::string reason;
  std::string error_details;
  boost::shared_ptr<iota::DeviceCollection> devTable(new
      iota::DeviceCollection());

  if (body.empty()) {
    error_details.assign("empty body");
    reason.assign(types::RESPONSE_MESSAGE_BAD_REQUEST);
    code = types::RESPONSE_CODE_BAD_REQUEST;
  }
  else if (validate_json_schema(body, devTable,
                                "PUT", error_details)) {
    mongo::BSONObj setbo =  mongo::fromjson(body);
    if (setbo.nFields() ==0) {
      error_details.assign("empty body");
      reason.assign(types::RESPONSE_MESSAGE_BAD_REQUEST);
      code = types::RESPONSE_CODE_BAD_REQUEST;
    }
    else {
      mongo::BSONObj query =  BSON(iota::store::types::SERVICE << service <<
                                   iota::store::types::SERVICE_PATH << service_path <<
                                   iota::store::types::DEVICE_ID << device_id);

      std::string entity_name = setbo.getStringField(store::types::ENTITY_NAME);

      // Protocol cannot be modified. Remove this field.
      setbo.removeField(iota::store::types::PROTOCOL);
      if (!entity_name.empty()) {
        // "entity_name" : 1, "service" : 1, "service_path" : 1
        devTable->find(BSON(store::types::ENTITY_NAME <<  entity_name <<
                            store::types::SERVICE << service <<
                            store::types::SERVICE_PATH <<service_path));
        if (devTable->more()) {
          throw iota::IotaException(iota::types::RESPONSE_MESSAGE_ENTITY_ALREADY_EXISTS,
                                    " [ entity_name: " + entity_name + "]",
                                    iota::types::RESPONSE_CODE_ENTITY_ALREADY_EXISTS);
        }
      }
      mongo::BSONObjBuilder put_builder;
      put_builder.appendElements(setbo);
      //put_builder.append(iota::store::types::REGISTRATION_ID, "");
      int count = devTable->update(query, put_builder.obj(), false);
      if (count == 0) {
        IOTA_LOG_INFO(m_log, "put_device_json no device " <<
                      " service=" << service << " service_path=" <<
                      service_path << " device=" << device_id << " content=" << body);
        throw iota::IotaException(iota::types::RESPONSE_MESSAGE_NO_DEVICE,
                                  " [ device: " + param_request + "]",
                                  iota::types::RESPONSE_CODE_DATA_NOT_FOUND);
      }
      else {
        std::string new_device(device_id);
        std::string new_device_json = setbo.getStringField(iota::store::types::DEVICE_ID);
        if (!new_device_json.empty()) {
          new_device.assign(new_device_json);
        }

        mongo::BSONObjBuilder returnFields;
        returnFields.append(iota::store::types::PROTOCOL, 1);
        devTable->find(query, returnFields);
        std::string protocol_name;
        Device dev(device_id, service);
        dev._service_path = service_path;
        if (devTable->more()) {
          mongo::BSONObj objDev = devTable->next();
          protocol_name = objDev.getStringField(iota::store::types::PROTOCOL);
          dev._protocol = protocol_name;
          deploy_device(dev);
        }else{
          PION_LOG_ERROR(m_log, "no device  in put " <<
                    "service=" << service << " service_path=" <<
                    service_path << " device=" << device_id );
        }

        //remove device from cache, to force reload new data
        remove_from_cache(dev);
      }
    }
  }
  else {
    reason.assign(types::RESPONSE_MESSAGE_BAD_REQUEST);
    code = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
  }

  IOTA_LOG_DEBUG(m_log, param_request << " status=" << code);
  return create_response(code, reason, error_details, http_response,
                         response);
}

int iota::AdminService::get_all_devices_json(
  const std::string& service,
  const std::string& service_path,
  int limit,
  int offset,
  const std::string& detailed,
  const std::string& entity,
  pion::http::response& http_response,
  std::string& response,
  std::string request_id,
  std::string token,
  std::string protocol_filter) {

  int code = pion::http::types::RESPONSE_CODE_OK;
  std::ostringstream res;
  mongo::BSONObj* fieldsToReturn = NULL;
  mongo::BSONObjBuilder bson_fields;
  std::string param_request("get_all_devices_json service=" + service +
                            " service_path=" +
                            service_path + " detailed=" +
                            detailed);
  IOTA_LOG_DEBUG(m_log, param_request);
  DeviceCollection devTable;
  mongo::BSONObj elto;
  mongo::BSONObjBuilder bson_query;
  bson_query.append(iota::store::types::SERVICE, service);

  mongo::BSONObjBuilder bson_sort;
  // se ordena de manera ascendente por nombre device
  bson_sort.append(store::types::DEVICE_SORT, 1);
  bson_sort.append(store::types::SERVICE_PATH, 1);

  ServiceCollection::addServicePath(service_path, bson_query);
  mongo::BSONObj obj_query;
  if (entity.empty() == false) {
    bson_query.append(iota::store::types::ENTITY_NAME, entity);
  }
  if (!protocol_filter.empty()) {
    bson_query.append(iota::store::types::PROTOCOL, protocol_filter);
  }

  obj_query = bson_query.obj();
  int count = devTable.count(obj_query);
  res << "{ \"count\": " << count << ",";
  res << "\"devices\": [";

  if (detailed.empty() ||
      (detailed.compare(iota::store::types::OFF) == 0)) {
    bson_fields.append(iota::store::types::DEVICE_ID, 1);
  }
  else if (detailed.compare(iota::store::types::ON) != 0) {
    IOTA_LOG_DEBUG(m_log, param_request << " status=" <<
                   pion::http::types::RESPONSE_CODE_BAD_REQUEST);
    return create_response(pion::http::types::RESPONSE_CODE_BAD_REQUEST,
                           types::RESPONSE_MESSAGE_BAD_REQUEST,
                           "parameter detailed must be on or off",
                           http_response, response);
  }
  if (limit >= 0){
    devTable.find(INT_MIN, obj_query, limit, offset,
                  bson_sort.obj(), bson_fields);
    while (devTable.more()) {
      elto = devTable.next();
      res << elto.jsonString();
      if (devTable.more()) {
        res << ",";
      }
    }
  }else{
    IOTA_LOG_DEBUG(m_log, "no find, limit :" << limit);
  }
  res << "]}";

  return create_response(code, res.str(), "", http_response, response);

}

int iota::AdminService::get_a_device_json(
  const std::string& service,
  const std::string& service_path,
  const std::string& device_id,
  pion::http::response& http_response,
  std::string& response,
  std::string request_id,
  std::string token,
  std::string protocol_filter) {

  int code = pion::http::types::RESPONSE_CODE_OK;
  std::string param_request("get_a_device_json service=" + service +
                            " service_path=" +
                            service_path + " device=" + device_id);
  IOTA_LOG_DEBUG(m_log, param_request);
  std::ostringstream res;
  Collection devTable(iota::store::types::DEVICE_TABLE);
  mongo::BSONObj elto;
  mongo::BSONObj query =  BSON(iota::store::types::DEVICE_ID << device_id
                               << iota::store::types::SERVICE << service
                               << iota::store::types::SERVICE_PATH << service_path);
  devTable.find(query);
  std::string details;
  if (devTable.more()) {
    elto = devTable.next();
    res << elto.jsonString();
  }
  else {
    code = iota::types::RESPONSE_CODE_CONTEXT_ELEMENT_NOT_FOUND;
    res << iota::types::RESPONSE_MESSAGE_NO_DEVICE;
    details =  device_id;
  }

  return create_response(code, res.str(), details, http_response, response);

}

int iota::AdminService::delete_device_json(
  const std::string& service,
  const std::string& service_path,
  const std::string& id_device,
  pion::http::response& http_response,
  std::string& response,
  std::string token,
  const std::string& protocol) {
  int code = pion::http::types::RESPONSE_CODE_NO_CONTENT;
  std::string param_request("delete_device_json service=" + service +
                            " service_path=" +
                            service_path + " device=" + id_device);
  IOTA_LOG_DEBUG(m_log, param_request);
  std::string reason;
  std::string error_details;

  mongo::BSONObjBuilder b;
  if (!id_device.empty()) {
    b.append(iota::store::types::DEVICE_ID, id_device);
  }
  if (!service.empty()) {
    b.append(iota::store::types::SERVICE, service);
  }
  if (!service_path.empty()) {
    b.append(iota::store::types::SERVICE_PATH, service_path);
  }
  Collection devTable(iota::store::types::DEVICE_TABLE);
  devTable.remove(b.obj());

  return create_response(code, reason, error_details, http_response,
                         response);
}

void iota::AdminService::check_uri(const std::string& data) {
  IoTUrl instance(data);
}


int iota::AdminService::post_service_json(
  const boost::shared_ptr<iota::ServiceCollection>& table,
  const std::string& service,
  const std::string& service_path,
  const std::string& body,
  pion::http::response& http_response,
  std::string& response,
  std::string token,
  std::string request_identifier) {

  std::string param_request("post_service_json service=" + service +
                            " service_path=" +
                            service_path);
  IOTA_LOG_DEBUG(m_log, param_request);
  int code = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
  std::string reason;
  std::string error_details;

  if (body.empty()) {
    error_details.assign("empty body");
    reason.assign(types::RESPONSE_MESSAGE_BAD_REQUEST);
    code = types::RESPONSE_CODE_BAD_REQUEST;
  }
  else if (validate_json_schema(body,  table,
                                "POST", error_details)) {
    mongo::BSONObj obj =  mongo::fromjson(body);
    mongo::BSONObj insObj;
    std::vector<mongo::BSONObj> be;
    table->getElementsFromBSON(obj, be);

    for (unsigned int i = 0; i<be.size(); i++) {
      mongo::BSONObjBuilder bo;
      IOTA_LOG_DEBUG(m_log, "add service:" << be[i]);
      bo.appendElements(be[i]);
      bo.append(iota::store::types::SERVICE_ID, service);
      bo.append(iota::store::types::SERVICE_PATH, service_path);
      insObj = bo.obj();

      // if cbroker is empty is ok
      std::string cbroker = insObj.getStringField(iota::store::types::CBROKER);
      if (!cbroker.empty()) {
        check_uri(cbroker);
      }

      table->insert(insObj);
    }
    code = pion::http::types::RESPONSE_CODE_CREATED;
    register_iota_manager();
    if (be.size() == 1) {
      http_response.add_header(pion::http::types::HEADER_LOCATION,
                               iota::URL_BASE + iota::ADMIN_SERVICE_SERVICES + "/" + service);
    }
  }
  else {
    reason.assign(types::RESPONSE_MESSAGE_BAD_REQUEST);
    code = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
  }

  IOTA_LOG_DEBUG(m_log, param_request << " status=" + code);
  return create_response(code, reason, error_details, http_response,
                         response);

}


int iota::AdminService::put_service_json(
  const boost::shared_ptr<iota::ServiceCollection>& table,
  const std::string& service,
  const std::string& service_path,
  const std::string& id,
  const std::string& apikey,
  const std::string& resource,
  const std::string& body,
  pion::http::response& http_response,
  std::string& response,
  std::string token,
  std::string request_identifier) {
  std::string param_request("put_service_json service=" + service +
                            " service_path=" +
                            service_path + " service_id=" + id +
                            " content=" + body + " resource=" + resource);
  IOTA_LOG_DEBUG(m_log, param_request);
  int code = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
  std::string reason;
  std::string error_details;

  if (resource.empty()) {
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_MISSING_PARAMETER,
                              "resource parameter is mandatory in PUT operation",
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  if (body.empty()) {
    error_details.assign("empty body");
    reason.assign(types::RESPONSE_MESSAGE_BAD_REQUEST);
    code = types::RESPONSE_CODE_BAD_REQUEST;
  }
  else if (validate_json_schema(body, table,
                                "PUT", error_details)) {
    mongo::BSONObj setbo =  mongo::fromjson(body);
    if (setbo.nFields() ==0) {
      error_details.assign("empty body");
      reason.assign(types::RESPONSE_MESSAGE_BAD_REQUEST);
      code = types::RESPONSE_CODE_BAD_REQUEST;
    }
    else {
      mongo::BSONObj query = BSON(iota::store::types::SERVICE_ID << id
                                  << iota::store::types::SERVICE_PATH << service_path
                                  << iota::store::types::APIKEY << apikey
                                  << iota::store::types::RESOURCE << resource);

      std::string cbroker = setbo.getStringField(iota::store::types::CBROKER);
      // if cbroker is empty is ok, check_uri  throw an exception
      if (!cbroker.empty()) {
        check_uri(cbroker);
      }


      int count = table->update(query, setbo, false);
      if (count == 0) {
        IOTA_LOG_INFO(m_log, "put_service_json no device " <<
                      " service=" << service << " service_path=" << service_path <<
                      " content=" << body);
        throw iota::IotaException(iota::types::RESPONSE_MESSAGE_NO_SERVICE,
                                  " [  " + param_request + "]",
                                  iota::types::RESPONSE_CODE_DATA_NOT_FOUND);
      }
      else {
        code = pion::http::types::RESPONSE_CODE_NO_CONTENT;
        register_iota_manager();
      }
    }
  }
  else {
    reason.assign(types::RESPONSE_MESSAGE_BAD_REQUEST);
    code = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
  }

  return create_response(code, reason, error_details, http_response,
                         response);
}

int iota::AdminService::get_all_services_json(
  const boost::shared_ptr<iota::ServiceCollection>& table,
  const std::string& service,
  const std::string& service_path,
  int limit,
  int offset,
  const std::string& detailed,
  const std::string& resource,
  pion::http::response& http_response,
  std::string& response) {
  int code = pion::http::types::RESPONSE_CODE_OK;
  std::string param_request("get_all_services_json service=" + service +
                            " service_path=" +
                            service_path +
                            " detailed=" + detailed + " resource=" + resource);
  IOTA_LOG_DEBUG(m_log, param_request);
  std::ostringstream res;
  mongo::BSONObj elto;
  mongo::BSONObjBuilder query;
  mongo::BSONObj p;
  mongo::BSONObjBuilder bson_fields;
  mongo::BSONObjBuilder bson_sort;
  // se ordena de manera ascendente poer servicio y subservicio, a fuego
  bson_sort.append(store::types::SERVICE, 1);
  bson_sort.append(store::types::SERVICE_PATH, 1);

  // Fiware-Service
  query.append(iota::store::types::SERVICE_ID, service);
  ServiceCollection::addServicePath(service_path, query);

  if (!resource.empty()) {

    query.append(table->get_resource_name(), resource);
  }
  p = query.obj();

  int count = table->count(p);
  res << "{ \"count\": " << count << ",";
  res << "\"services\": [";

  if (detailed.empty() ||
      (detailed.compare(iota::store::types::OFF) == 0)) {
    bson_fields.append(iota::store::types::SERVICE_ID, 1);

  }
  else if (detailed.compare(iota::store::types::ON) != 0) {
    return create_response(pion::http::types::RESPONSE_CODE_BAD_REQUEST,
                           types::RESPONSE_MESSAGE_BAD_REQUEST,
                           "parameter detailed must be on or off", http_response,
                           response);
  }

  if (limit >= 0){
    table->find(INT_MIN, p, limit, offset,
                bson_sort.obj(), bson_fields);
    while (table->more()) {
      elto = table->next();
      res << elto.jsonString();
      if (table->more()) {
        res << ",";
      }
    }
  }else{
    IOTA_LOG_DEBUG(m_log, "no find limit  : " << limit);
  }
  res << "]}";
  return create_response(code, res.str(), "", http_response, response);

}

int iota::AdminService::get_a_service_json(
  const boost::shared_ptr<iota::ServiceCollection>& table,
  const std::string& service,
  const std::string& service_path,
  const std::string& id,
  pion::http::response& http_response,
  std::string& response) {

  std::string param_request("get_a_service_json service=" + service +
                            " service_path=" +
                            service_path + " id=" + id);
  IOTA_LOG_DEBUG(m_log, param_request);

  int code= pion::http::types::RESPONSE_CODE_OK;
  std::string res = get_service_json(table, service, service_path);
  if (res.empty()) {
    code = pion::http::types::RESPONSE_CODE_NOT_FOUND;
  }
  return create_response(code, res, "", http_response, response);
}

std::string iota::AdminService::get_service_json(
  const boost::shared_ptr<iota::ServiceCollection>& table,
  const std::string& service,
  const std::string& service_path) {

  std::string param_request("get_a_service_json service=" + service +
                            " service_path=" +
                            service_path);
  IOTA_LOG_DEBUG(m_log, param_request);
  std::ostringstream res;
  mongo::BSONObj elto;
  mongo::BSONObjBuilder query;

  // Fiware-Service
  query.append(iota::store::types::SERVICE_ID, service);
  // Fiware-ServicePath
  query.append(iota::store::types::SERVICE_PATH, service_path);

  table->find(query.obj());
  if (table->more()) {
    elto = table->next();
    res << elto.jsonString();
  }
  return res.str();
}

//TODO duda sobre los permisos y relacion entre service header y borrado
int iota::AdminService::delete_service_json(
  const boost::shared_ptr<iota::ServiceCollection>& table,
  const std::string& service,
  const std::string& service_path,
  const std::string& id,
  const std::string& apikey,
  const std::string& resource,
  bool remove_devices,
  pion::http::response& http_response,
  std::string& response,
  std::string x_auth_token,
  std::string request_identifier) {
  int code = pion::http::types::RESPONSE_CODE_NO_CONTENT;
  std::string param_request("delete_service_json:service=" + service +
                            " service_path=" +
                            service_path + " apikey=" + apikey + " resource="
                            + resource + " r_devices=" +
                            boost::lexical_cast<std::string>(remove_devices));
  IOTA_LOG_DEBUG(m_log, param_request);

  std::string reason;
  std::string error_details;
  std::string b_query_service_path;

  mongo::BSONObjBuilder b_delete;
  mongo::BSONObjBuilder b_count;
  if (!id.empty()) {
    b_delete.append(iota::store::types::SERVICE_ID, id);
    b_count.append(iota::store::types::SERVICE_ID, id);
    if (!apikey.empty()) {
      b_delete.append(iota::store::types::APIKEY, apikey);
    }
    if (!resource.empty()) {
      b_delete.append(iota::store::types::RESOURCE, resource);
    }
  }
  else {
    std::string text_error = " service:";
    text_error.append(id);
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST, text_error,
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  if (!service_path.empty() && service_path[0] == '/') {
    if (service_path.compare("/*") != 0 &&
        service_path.compare("/#") != 0) {
      b_delete.append(iota::store::types::SERVICE_PATH, service_path);
      b_count.append(iota::store::types::SERVICE_PATH, service_path);
      b_query_service_path = service_path;
    }
  }
  else {
    std::string text_error = "service_path:";
    text_error.append(service_path);
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST, text_error,
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  mongo::BSONObj obj_delete = b_delete.obj();
  mongo::BSONObj obj_count = b_count.obj();
  int num_services = table->count(obj_count);
  int num_removed_services = table->remove(obj_delete);

  if ((num_services - num_removed_services) == 0 &&
      remove_devices) {

    mongo::BSONObjBuilder b_dev;
    b_dev.append(iota::store::types::SERVICE_ID, id);
    b_dev.append(iota::store::types::SERVICE_PATH, b_query_service_path);
    mongo::BSONObj obj_device = b_dev.obj();
    Collection devTable(iota::store::types::DEVICE_TABLE);
    devTable.remove(obj_device);
  }
  register_iota_manager();
  return create_response(code, reason, error_details, http_response,
                         response);
}

void iota::AdminService::deploy_device(Device& device) {

  boost::mutex::scoped_lock lock(iota::AdminService::m_sm);
  IOTA_LOG_DEBUG(m_log, "deploy_device " << device._protocol);
  std::string protocol_name = device._protocol;

  std::map<std::string, iota::RestHandle*>::const_iterator it =
    _service_manager.begin();
  while (it != _service_manager.end()) {
    try {

      iota::CommandHandle* cmd_handle = dynamic_cast<iota::CommandHandle*>
                                        (it->second);
      if (cmd_handle != NULL){
          iota::ProtocolData pro = cmd_handle->get_protocol_data();
          if (protocol_name.compare(pro.protocol) ==0) {
            cmd_handle->send_register_device(device);
          }
      }
    }
    catch (std::exception& e) {
      IOTA_LOG_DEBUG(m_log, "deploy_device error: " <<  e.what());
    }
    ++it;
  }
}

void iota::AdminService::register_iota_manager() {

  boost::mutex::scoped_lock lock(iota::AdminService::m_sm);
  std::map<std::string, iota::RestHandle*>::const_iterator it =
    _service_manager.begin();
  while (it != _service_manager.end()) {
    try {
      iota::RestHandle* rest_handle = dynamic_cast<iota::RestHandle*>
                                      (it->second);
      if (rest_handle != NULL) {
        rest_handle->register_iota_manager();
      }
    }
    catch (std::exception& e) {
      IOTA_LOG_DEBUG(m_log, e.what());
    }
    ++it;
  }
}

void iota::AdminService::timeout_check_logs(const boost::system::error_code&
    ec) {
  if (!ec || ec != boost::asio::error::operation_aborted) {
    check_logs();
  }
}

void iota::AdminService::check_logs() {
  // Check log file writen a INFO log to force activity
  if (_log_file.empty()) {
    return;
  }
  std::multimap<std::string, std::string> q_p;
  JsonDocument get_agents;
  get_agents.SetObject();

  boost::mutex::scoped_lock lock(iota::AdminService::m_sm);
  std::map<std::string, iota::RestHandle*>::const_iterator it =
    _service_manager.begin();
  while (it != _service_manager.end()) {
    iota::RestHandle* agent = it->second;
    if (agent != NULL) {

      // para no crearlo de nuevo
      JsonValue statistics;
      statistics.SetArray();
      if (!get_agents.HasMember("statistics")) {
        get_agents.AddMember("statistics", statistics, get_agents.GetAllocator());
      }
      get_info_agent(agent, q_p, get_agents);
      //agent->reset_counters();
    }
    ++it;
  }
  rapidjson::StringBuffer buffer;
  std::string response;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  get_agents.Accept(writer);
  response.assign(buffer.GetString());
  IOTA_LOG_INFO(m_log, "Agent Activity " << response);
  bool error_log = false;
  FILE* pFile;
  pFile = fopen(_log_file.c_str(),"a");
  if (pFile == NULL) {
    error_log = true;
  }
  else {
    if (fputs("",pFile) < 0) {
      error_log = true;
    }
    fclose(pFile);
  }
  std::time_t time_for_log = boost::filesystem::last_write_time(_log_file);
  unsigned long time_last_log = std::time(NULL) - time_for_log;
  if (error_log == false) {
    _timer->expires_from_now(boost::posix_time::seconds(TIME_TO_LOG));
    _timer->async_wait(boost::bind(&iota::AdminService::timeout_check_logs,
                                   this,
                                   boost::asio::placeholders::error));
  }
  else {
    perror("Server shutdown for log file errors ");
    //get_web_server()->shutdown();
    pion::process::shutdown();
  }
}

void iota::AdminService::set_log_file(std::string& log_file) {
  _log_file = log_file;
}

bool iota::AdminService::check_device_protocol(const std::string& protocol_name,
    const std::string& service_name,
    const std::string& service_path,
    const boost::shared_ptr<iota::ServiceCollection>& table) {
  IOTA_LOG_DEBUG(m_log, "check_device_protocol " << protocol_name);

  if (protocol_name.empty()) {
    return false;
  }

  std::vector<std::string> resources;
  std::string resource;
  iota::RestHandle* plugin;
  table->fill_all_resources(service_name, service_path, resources);
  IOTA_LOG_DEBUG(m_log, "Number of elements in resources:" << resources.size());
  for (std::vector<std::string>::iterator it = resources.begin();
       it != resources.end(); ++it) {
    resource = *it;
    plugin = get_service(resource);
    IOTA_LOG_DEBUG(m_log, "Checking resource " << resource);
    if (plugin != NULL) {
      iota::ProtocolData pro = plugin->get_protocol_data();
      IOTA_LOG_DEBUG(m_log, "pro.protocol plugin " << pro.protocol);
      if (protocol_name.compare(pro.protocol) ==0) {
        IOTA_LOG_DEBUG(m_log, "exists plugin:" << protocol_name);
        return true;
      }
    }
    else {
      IOTA_LOG_ERROR(m_log, "Plugin NULL checking resource " << resource);
    }
  }

  IOTA_LOG_DEBUG(m_log, "no exists plugin:" << protocol_name);
  return false;
}

std::string iota::AdminService::get_class_name() {
  return _class_name;
}

