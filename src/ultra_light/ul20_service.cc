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
#include "ul20_service.h"
#include "services/admin_service.h"
#include <pion/http/response_writer.hpp>
#include <sstream>
#include <rest/riot_conf.h>
#include <util/iot_url.h>
#include "util/iota_exception.h"
#include "util/KVP.h"
#include "util/FuncUtil.h"
#include "util/alarm.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include "ngsi/UpdateContext.h"
#include "rest/iot_cb_comm.h"
#include <pion/http/response_writer.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
#include "ultra_light/ULInsertObservation.h"
#include "ngsi/ContextRegistration.h"
#include "ngsi/RegisterContext.h"
#include "ngsi/RegisterResponse.h"

#include "util/dev_file.h"
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#include <pion/tcp/server.hpp>
#include "boost/format.hpp"

iota::UL20Service::UL20Service()
    : m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  IOTA_LOG_DEBUG(m_logger, "iota::UL20Service::UL20Service");
  _protocol_data.description = "UL2";
  _protocol_data.protocol = "PDI-IoTA-UltraLight";
  //_reg_timeout = DEFAULT_REG_TIMEOUT;
  //_myProvidingApp = UNKOWN_PROVIDING_APP;
}

iota::UL20Service::~UL20Service() {
  IOTA_LOG_DEBUG(m_logger, "Destructor iota::UL20Service::UL20Service");
}

void iota::UL20Service::start() {
  IOTA_LOG_DEBUG(m_logger, "START PLUGIN UL2.0");
  std::map<std::string, std::string> filters;
  add_url("", filters, REST_HANDLE(&iota::UL20Service::service), this);

  enable_ngsi_service(filters, REST_HANDLE(&iota::UL20Service::op_ngsi), this);
  set_async_commands();

  try {
    _reg_timeout = iota::Configurator::instance()->get("timeout").GetInt64();
  } catch (...) {
    IOTA_LOG_DEBUG(m_logger, " Problem reading timeout");
  }
}

void iota::UL20Service::op_ngsi(
    pion::http::request_ptr& http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response, std::string& response) {
  IOTA_LOG_DEBUG(m_logger, "UL20Service op_ngsi");
  default_op_ngsi(http_request_ptr, url_args, query_parameters, http_response,
                  response);
}

void iota::UL20Service::service(
    pion::http::request_ptr& http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response, std::string& response) {
  std::string trace_message =
      http_request_ptr->get_header(iota::types::HEADER_TRACE_MESSAGES);
  std::string method = http_request_ptr->get_method();
  IOTA_LOG_INFO(m_logger, "iota::UL20Service::service method:" + method +
                              " trace_message:" + trace_message);

  // HTTPResponse http_response(*http_request_ptr);
  std::string respuesta;
  boost::system::error_code error_code;
  int code_resp = pion::http::types::RESPONSE_CODE_OK;

  bool url_update = false;

  iota::ULInsertObservation io_ul_idas;

  // std::auto_ptr<IdasCommandResponseType> c_http_response;

  // Adaptamos a nuevas PION cambio   getMethod   -->   get_method
  // Mirar :  /home/develop/iot/pion-5.0.6/include/pion/http/request.hpp
  // std::string method = http_request_ptr->getMethod();

  std::string resource_o = http_request_ptr->get_original_resource();

  std::string ip = http_request_ptr->get_remote_ip().to_string();

  std::string resource = http_request_ptr->get_resource();

  IOTA_LOG_DEBUG(m_logger, method << " " << resource_o);

  std::string content;

  std::vector<std::string> str_peticiones;

  std::string s_query = http_request_ptr->get_query_string();
  std::vector<KVP> query;
  bool hay_p = riot_getQueryParams(s_query, query);
  std::vector<KVP> querySBC;
  std::string query_params;
  std::string apikey;
  std::string device;
  bool get_cmd = false;
  bool command_resp = false;
  boost::property_tree::ptree service_ptree;
  std::string entity_type, new_endpoint;
  boost::shared_ptr<Device> dev;

  try {
    int i = 0;
    for (i = 0; i < query.size(); i++) {
      IOTA_LOG_DEBUG(m_logger, "QUERY " << query[i].getKey()
                                        << query[i].getValue());
      if (query[i].getKey().compare("i") == 0) {
        KVP id("ID", query[i].getValue());
        querySBC.push_back(id);
        device = query[i].getValue();
      } else if (query[i].getKey().compare("ip") == 0) {
        new_endpoint = query[i].getValue();
        if (!new_endpoint.empty()) {
          KVP ip("URL", new_endpoint);
          querySBC.push_back(ip);
          url_update = true;
        }
      } else if (query[i].getKey().compare("k") == 0) {
        KVP k("apikey", query[i].getValue());
        querySBC.push_back(k);

        apikey.append(query[i].getValue());
      } else if ((query[i].getKey().compare("d") == 0) &&
                 (method.compare("GET") == 0)) {
        if (!content.empty()) {
          content.append("#");
        }
        content.append(query[i].getValue());
      } else if (query[i].getKey().compare("getCmd") == 0) {
        get_cmd = true;
      } else {
        KVP p(query[i].getKey(), query[i].getValue());
        querySBC.push_back(p);
      }
    }

    for (i = 0; i < querySBC.size(); i++) {
      if (i != 0) {
        query_params.append("&");
      }
      query_params.append(querySBC[i].getKey());
      query_params.append("=");
      query_params.append(querySBC[i].getValue());
    }

    std::string version = http_request_ptr->get_version_string();
    IOTA_LOG_DEBUG(m_logger, "apikey:" << apikey << "body:" << content);
    // check if this key exists
    get_service_by_apiKey(service_ptree, apikey);

    // check if device exists
    dev = get_device(
        device, service_ptree.get<std::string>(iota::store::types::SERVICE, ""),
        service_ptree.get<std::string>(
            iota::store::types::SERVICE_PATH,
            iota::types::FIWARE_SERVICEPATH_DEFAULT));
    if (dev.get() == NULL) {
      dev.reset(new Device(device, service_ptree.get<std::string>(
                                       iota::store::types::SERVICE, "")));
      dev->_service_path =
          service_ptree.get<std::string>(iota::store::types::SERVICE_PATH, "");

      IOTA_LOG_DEBUG(m_logger, "Device "
                                   << device << " is not registered;"
                                                " apikey: "
                                   << apikey << " service: " << dev->_service
                                   << " service_path: " << dev->_service_path);
    } else if (url_update) {
      if (iota::check_forbidden_characters(
              iota::types::IOTA_FORBIDDEN_CHARACTERS, new_endpoint)) {
        throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                                  "Invalid Characters",
                                  iota::types::RESPONSE_CODE_BAD_REQUEST);
      }
      iota::IoTUrl endpoint = iota::IoTUrl(new_endpoint);
      if (endpoint.getHost().empty() || endpoint.getProtocol().empty()) {
        throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                                  "Not a valid IP",
                                  iota::types::RESPONSE_CODE_BAD_REQUEST);
      }

      std::string old_endpoint = dev->_endpoint;
      if (!old_endpoint.empty() && old_endpoint.compare(new_endpoint) != 0) {
        IOTA_LOG_DEBUG(m_logger, " Device " << device << " has a new endpoint "
                                            << new_endpoint);
        dev->_endpoint = new_endpoint;
        if (_storage_type.compare(iota::store::types::MONGODB) == 0) {
          update_endpoint_device(dev, new_endpoint);
        }
      }
    }

    std::string content_type;
    int cl = 0;
    if ((http_request_ptr->get_content_length() != 0) &&
        (http_request_ptr->get_content() != NULL)) {
      cl = http_request_ptr->get_content_length();

      std::string content_aux;
      std::string content_encoding = http_request_ptr->get_header(
          pion::http::types::HEADER_CONTENT_ENCODING);
      std::string gzip_str("gzip");
      std::string deflate_str("deflate");

      if ((!content_encoding.empty()) &&
          ((boost::iequals(gzip_str, content_encoding)) ||
           (boost::iequals(deflate_str, content_encoding)))) {
        IOTA_LOG_DEBUG(m_logger, "Hay que descomprimir " << content_encoding);
        std::string method(content_encoding);
        std::string mi_content = http_request_ptr->get_content();
      } else {
        content_aux = http_request_ptr->get_content();
      }

      content = content_aux;

      IOTA_LOG_DEBUG(m_logger, "Message from " << ip << " to " << resource
                                               << " CONTENT " << content);
      IOTA_LOG_DEBUG(m_logger, "CONTENT " << content);
    }

    std::vector<iota::ContextElement> cb_eltos;

    if (content.empty() == false) {
      std::string command_http_response_translate, id_command;
      if (isCommandResp(content, 2000, command_http_response_translate,
                        id_command) == -1) {
        std::string srv = service_ptree.get<std::string>("service", "");

        io_ul_idas.translate(content, dev, service_ptree, querySBC, cb_eltos,
                             0);
        code_resp = iota::types::RESPONSE_CODE_OK;
        for (int ii = 0; ii < cb_eltos.size(); ii++) {
          std::string cb_response;
          // NO cb_eltos[ii].set_env_info(service_ptree, dev);
          send(cb_eltos[ii], "APPEND", service_ptree, cb_response);
        }
      } else {
        IOTA_LOG_DEBUG(m_logger, "han mandado una respuesta a un comando "
                                     << id_command << "->"
                                     << command_http_response_translate);
        command_resp = true;

        std::string srv =
            service_ptree.get<std::string>(iota::store::types::SERVICE, "");
        std::string srv_path = service_ptree.get<std::string>(
            iota::store::types::SERVICE_PATH, "");
        std::string command;
        CommandPtr commandPtr = get_command(id_command, srv, srv_path);
        if (commandPtr.get() == NULL) {
          IOTA_LOG_ERROR(m_logger,
                         "already responsed, command not in cache id_command:"
                             << id_command << " service:" << srv << " "
                             << srv_path);
        } else {
          IOTA_LOG_DEBUG(m_logger, "command in cache id_command:"
                                       << id_command << " service:" << srv
                                       << " " << srv_path);
          command = commandPtr->get_name();
          commandPtr->cancel();
          remove_command(id_command, srv, srv_path);

          response_command(command, command_http_response_translate, dev,
                           service_ptree);
        }
      }
    }
  } catch (iota::IotaException& e) {
    boost::property_tree::ptree additional_info;
    std::string content("{\"apikey\":\"");
    content.append(apikey);
    content.append("\",\"device\":\"");
    content.append(device);
    content.append("\"}");
    std::string reason = e.reason();
    if (reason.compare(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR) == 0) {
      iota::Alarm::error(iota::types::ALARM_CODE_NO_CB,
                         MongoConnection::instance()->get_endpoint(), content,
                         additional_info, iota::types::ERROR, e.reason());
    }
    code_resp = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
  } catch (std::runtime_error& e) {
    IOTA_LOG_ERROR(m_logger, "translate error runtime_error" << e.what());
    code_resp = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, "translate error exception" << e.what());
    code_resp = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
  } catch (...) {
    IOTA_LOG_ERROR(m_logger, "translate error ");
    code_resp = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
  }

  /// vamos a ver si hay que ir a por comandos
  CommandVect cmdPtes;
  if ((method.compare("GET") == 0) || (get_cmd == true)) {
    cmdPtes = get_all_command(dev, service_ptree);
    IOTA_LOG_DEBUG(m_logger, "N. commands " << cmdPtes.size());
  }

  if (code_resp != pion::http::types::RESPONSE_CODE_OK) {
    if (code_resp == 503) {
      // io_ul_idas->release_cache();
      http_response.set_status_message("Service Unavailable");
      http_response.set_status_code(503);
    } else if (code_resp == pion::http::types::RESPONSE_CODE_SERVER_ERROR) {
      http_response.set_status_message(
          pion::http::types::RESPONSE_MESSAGE_SERVER_ERROR);
      http_response.set_status_code(
          pion::http::types::RESPONSE_CODE_SERVER_ERROR);
    } else {
      http_response.set_status_message(
          pion::http::types::RESPONSE_MESSAGE_BAD_REQUEST);
      http_response.set_status_code(
          pion::http::types::RESPONSE_CODE_BAD_REQUEST);
    }
  }

  if ((!response.empty()) || (cmdPtes.size() > 0)) {
    std::string c_t("text/json; charset=UTF-8");
    http_response.set_content_type(c_t);

    std::string commandsSTR, body;
    CommandVect::const_iterator i;
    CommandPtr ptr;
    for (CommandVect::iterator it_v = cmdPtes.begin(); it_v != cmdPtes.end();
         ++it_v) {
      ptr = *it_v;
      body = ptr->get_command().get(iota::store::types::BODY, "");
      commandsSTR.append(body);
      if (it_v != cmdPtes.end() - 1) {
        commandsSTR.append("#");
      }
    }

    response.append(commandsSTR);
  }

  IOTA_LOG_INFO(m_logger, "iota::UL20Service::service method:" + method +
                              " trace_message:" + trace_message + " code: " +
                              boost::lexical_cast<std::string>(code_resp) +
                              " response:" + response + " ");
}

/**
  * en ul20 las respuesta de comandos son device@commando|resultado  by example
 * dev1@ping|pingOK
  * si es un comando separa id_command y command response
  */
int iota::UL20Service::isCommandResp(const std::string& str_command_resp,
                                     const int& cmd_code,
                                     std::string& command_response,
                                     std::string& id_command) {
  int resp = -1;
  std::string result;

  id_command.assign(get_id_command(str_command_resp));
  IOTA_LOG_DEBUG(m_logger, "isCommandResp" << cmd_code << ":"
                                           << str_command_resp << ":"
                                           << id_command);
  if (id_command.empty() == false) {
    size_t p_id = str_command_resp.find_first_of("|");
    if (p_id != std::string::npos) {
      result = str_command_resp.substr(p_id + 1);
    } else {
      // if there is not response, we put all body (id_command)
      result = str_command_resp;
    }
    resp = 200;

    // now, in command_response we put the entire command_response
    // if you want to separate only de response youmust assign result
    command_response.assign(str_command_resp);
  }

  return resp;
};

int iota::UL20Service::sendHTTP(
    const std::string& endpoint, int port, const std::string& operation,
    const std::string& path, const std::string& proxy_server, int timeout,
    bool ssl, const std::string& query, const std::string& message,
    const std::string& outgoing_route, std::string& response,
    iota::HttpClient::application_callback_t callback) {
  IOTA_LOG_DEBUG(m_logger, "UL20Plugin sendHTTP(): "
                               << endpoint << ":" << port << "//" << path << " "
                               << operation << " proxy " << proxy_server
                               << " timeout " << timeout << " query " << query
                               << " body:" << message
                               << " outgoing_route:" << outgoing_route);

  int ret = pion::http::types::RESPONSE_CODE_OK;
  boost::shared_ptr<iota::HttpClient> http_client;
  try {
    pion::http::request_ptr request(new pion::http::request());
    request->set_method(operation);
    request->set_resource(path);
    request->set_content(message);
    // request->set_content_type(IOT_CONTENT_TYPE_JSON);
    // request->setContentLength(message.size());

    if (!outgoing_route.empty()) {
      request->add_header(iota::types::HEADER_OUTGOING_ROUTE, outgoing_route);
    }

    if (query.empty() == false) {
      request->set_query_string(query);
    }

    if (callback) {
      http_client.reset(new iota::HttpClient(
          iota::Process::get_process().get_io_service(), endpoint, port));
      // pion::http::response_ptr response_ptr
      http_client->async_send(request, timeout, proxy_server, callback);
    } else {
      http_client.reset(new iota::HttpClient(endpoint, port));
      pion::http::response_ptr response_ptr =
          http_client->send(request, timeout, proxy_server);
      if (response_ptr.get() != NULL) {
        ret = response_ptr->get_status_code();
        IOTA_LOG_DEBUG(m_logger, "sendHTTP responses "
                                     << ret << ":"
                                     << response_ptr->get_content());
        if ((response_ptr->get_content_length() != 0) &&
            (response_ptr->get_content() != NULL)) {
          response.assign(
              iota::url_decode(response_ptr->get_content()));
          boost::trim(response);
          boost::erase_all(response, "\n");
          boost::erase_all(response, "\r");
        }
      } else {
        std::string errSTR = "This seems to be like an error connection to ";
        errSTR.append(endpoint);
        errSTR.append(":");
        errSTR.append(boost::lexical_cast<std::string>(port));
        errSTR.append("//");
        errSTR.append("path");

        IOTA_LOG_ERROR(m_logger, errSTR);
        ret = types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR;
        response.assign(errSTR);
      }
    }
  } catch (std::exception const& ex) {
    IOTA_LOG_ERROR(m_logger, "ul20Plugin sendHTTP(): Exception: " << ex.what());
    ret = types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR;
    response.assign(ex.what());
  } catch (...) {
    IOTA_LOG_ERROR(m_logger, "ul20Plugin sendHTTP(): Envio error ");
    ret = types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR;
    response.assign("document instance parsing failed");
  }

  // http_client->stop();

  IOTA_LOG_DEBUG(m_logger, "UL20Plugin sendHTTP response(): " << ret << ":"
                                                              << response);
  return ret;
}

int iota::UL20Service::execute_command(
    const std::string& destino, const std::string& command_id,
    const boost::property_tree::ptree& command_ptree, int timeout,
    const boost::shared_ptr<Device>& item_dev,
    const boost::property_tree::ptree& service, std::string& response,
    iota::HttpClient::application_callback_t callback) {
  int codigo_respuesta = pion::http::types::RESPONSE_CODE_OK;
  int size_respuesta = 0;

  std::string command_to_send = command_ptree.get(iota::store::types::BODY, "");
  IoTUrl url(destino);
  std::string port;
  bool ssl = false;
  if (url.getPort() != -1) {
    port = boost::lexical_cast<std::string>(url.getPort());
  }
  if (url.getSSL() == false) {
    IOTA_LOG_DEBUG(m_logger, "NO SSL");
  }

  int timeout_plugin;

  // Por defecto 5 segundos
  // timeout_plugin = _service_configuration.get<int> ("Timeout", timeout);
  timeout_plugin = timeout;

  // Proxy and outgoing_route
  std::string outgoing_route =
      service.get<std::string>(iota::store::types::OUTGOING_ROUTE, "");
  std::string proxy =
      service.get<std::string>(iota::types::CONF_FILE_PROXY, "");
  ;

  std::string content_resp;
  int code_resp =
      sendHTTP(url.getHost(), url.getPort(), "POST", url.getPath(), proxy,
               timeout_plugin, url.getSSL(), url.getQuery(), command_to_send,
               outgoing_route, content_resp, callback);
  if (!callback) {
    std::string command_response;
    std::string id_command;
    int code =
        isCommandResp(content_resp, code_resp, command_response, id_command);
    if (code < 0) {
      std::string errSTR = "no ul20 response command:";
      errSTR.append(content_resp);
      IOTA_LOG_ERROR(m_logger, "command response from " << destino << " "
                                                        << errSTR);
      response.assign(errSTR);
    } else {
      response.assign(command_response);
    }
  }
  return (code_resp);
};

/**
 *  se queda con el prier campo que es el nombre del comando
 **/
std::string iota::UL20Service::get_id_command(std::string str_command) {
  size_t p_id = str_command.find_first_of("|");
  std::string id_comando = str_command.substr(0, p_id);
  if (id_comando.find_first_of("@") == std::string::npos) {
    id_comando.clear();
  }
  return (id_comando);
};

void iota::UL20Service::send_optional_registration(std::string device,
                                                   std::string service) {
  IOTA_LOG_DEBUG(m_logger, "Sending one register if it is neccesary");

  if (_myProvidingApp == UNKOWN_PROVIDING_APP) {
    IOTA_LOG_DEBUG(m_logger,
                   "Registration is not sent because a valid ProvidingApp can "
                   "not be obtained");
    return;
  }

  try {
    std::vector<iota::ContextRegistration> context_registrations;
    iota::ContextRegistration cr;
    std::string cb_response;
    std::string reg_id;
    std::string reg_time;

    std::string entity_name;

    IOTA_LOG_DEBUG(m_logger, "Search Device: " << device
                                               << "with Service: " << service);

    boost::shared_ptr<Device> item_find(new Device(device, service));
    boost::shared_ptr<Device> item_dev = registeredDevices.get(item_find);

    if (item_dev != NULL) {
      IOTA_LOG_DEBUG(m_logger, "Found Device: " << item_dev->_name);

      if (((item_dev->_registration_id).empty()) &&
          ((item_dev->_commands.size() > 0))) {
        IOTA_LOG_DEBUG(m_logger,
                       "Must be registered Device: " << item_dev->_name);

        iota::Entity entity(item_dev->_entity_name, item_dev->_entity_type,
                            "false");
        cr.add_entity(entity);
        cr.add_provider(_myProvidingApp);

        if (!item_dev->_registration_id.empty()) {
          reg_id = item_dev->_registration_id;
          IOTA_LOG_DEBUG(m_logger, "Setting registrationId: " << reg_id);
        }

        std::map<std::string, std::string>::iterator p;
        for (p = item_dev->_commands.begin(); p != item_dev->_commands.end();
             ++p) {
          std::string attr_name = p->first;
          std::string attr_type = attr_type = "command";

          iota::AttributeRegister attribute(attr_name, attr_type, "false");
          cr.add_attribute(attribute);
        }

        context_registrations.push_back(cr);
        boost::property_tree::ptree service_ptree;
        get_service_by_name(service_ptree, service);

        // TODO
        send_register(context_registrations, service_ptree, item_dev, reg_id,
                      cb_response);

        IOTA_LOG_DEBUG(m_logger, "cb_response: " << cb_response);

        std::istringstream str_reg_response;
        str_reg_response.str(cb_response);
        iota::RegisterResponse resp(str_reg_response);

        reg_id = resp.get_registration_id();
        item_dev->_registration_id = reg_id;

        reg_time = resp.get_duration();

        IOTA_LOG_DEBUG(m_logger, "registrationId: " << reg_id);
        IOTA_LOG_DEBUG(m_logger, "duration: " << reg_time);
      }
    }
  } catch (...) {
    IOTA_LOG_ERROR(m_logger, "Error sending one registration");
  }
}

void iota::UL20Service::transform_command(
    const std::string& command_name, const std::string& a_command_value,
    const std::string& a_updateCommand_value, const std::string& sequence_id,
    const boost::shared_ptr<Device>& item_dev,
    const boost::property_tree::ptree& service, std::string& command_id,
    boost::property_tree::ptree& command_line) {
  std::string command_http_response_translate, result;
  std::string key = "|";

  // avoid " "
  std::string command_value(a_command_value);
  std::string updateCommand_value(a_updateCommand_value);
  boost::trim(command_value);
  boost::trim(updateCommand_value);

  updateCommand_value = json_value_to_ul(updateCommand_value);

  IOTA_LOG_DEBUG(m_logger, "transform_command:: " << command_value
                                                  << " updateCommand_value:"
                                                  << updateCommand_value);
  if (command_value.compare(iota::types::RAW) == 0) {
    result = updateCommand_value;
    command_line.put(iota::store::types::BODY, result);
  } else if (!command_value.empty()) {
    CommandHandle::transform_command(command_name, command_value,
                                     updateCommand_value, sequence_id, item_dev,
                                     service, command_id, command_line);
  } else {
    result.append(item_dev->_name);
    result.append("@");
    result.append(command_name);
    if (!updateCommand_value.empty()) {
      if (!boost::starts_with(key, updateCommand_value)) {
        result.append(key);
      }

      result.append(updateCommand_value);
    }
    command_line.put(iota::store::types::BODY, result);
  }

  // check if it is an ul20 command well formed
  std::string body = command_line.get(iota::store::types::BODY, "");
  if (isCommandResp(body, 2000, command_http_response_translate, command_id) ==
      -1) {
    std::string err =
        "UL20 Command is not correct, it must be  name_device@command_name, "
        "but it is ";
    err.append(body);
    IOTA_LOG_ERROR(m_logger, err);
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_COMMAND_BAD, err,
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  IOTA_LOG_DEBUG(m_logger, "UL20Service new command id  " << command_id);
}

int iota::UL20Service::transform_response(const std::string& str_command_resp,
                                          const int& cmd_status,
                                          std::string& command_response,
                                          std::string& id_command) {
  int code = pion::http::types::RESPONSE_CODE_OK;
  std::string response;
  response.assign(iota::url_decode(str_command_resp));
  boost::trim(response);
  boost::erase_all(response, "\n");
  boost::erase_all(response, "\r");
  code = isCommandResp(response, cmd_status, command_response, id_command);
  if (code < 0) {
    std::string errSTR = "no ul20 response command:";
    errSTR.append(response);
    command_response.assign(errSTR);
  }
  return code;
}

std::string iota::UL20Service::get_ngsi_operation(
    const std::string& operation) {
  std::string op("/NGSI10/");
  op.append(operation);

  try {
    const JsonValue& ngsi_url = iota::Configurator::instance()->get("ngsi_url");
    std::string op_url = ngsi_url[operation.c_str()].GetString();
    op.assign(op_url);
  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, "Configuration error " << e.what());
  }
  return op;
}

// creates new UL20Service objects
//

extern "C" PION_PLUGIN iota::UL20Service* pion_create_UL20Service(void) {
  return new iota::UL20Service();
}

/// destroys UL20Service objects

extern "C" PION_PLUGIN void pion_destroy_UL20Service(
    iota::UL20Service* service_ptr) {
  delete service_ptr;
}
