#include "admin_mgmt_service.h"
#include "util/iot_url.h"
#include "util/http_client.h"
#include "rest/rest_handle.h"
#include "util/iot_url.h"
#include "util/iota_exception.h"
#include "util/protocol_collection.h"
#include "util/FuncUtil.h"
#include <boost/property_tree/ptree.hpp>
#include "util/device_collection.h"

std::string iota::AdminManagerService::_POST_PROTOCOL_SCHEMA;
std::string iota::AdminManagerService::_PUT_PROTOCOL_SCHEMA;

std::string iota::AdminManagerService::_POST_SERVICE_SCHEMA;
std::string iota::AdminManagerService::_PUT_SERVICE_SCHEMA;

iota::AdminManagerService::AdminManagerService()
    : _class_name("iota::AdminManagerService"),
      _timeout(5),
      m_log(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  read_schema("post_protocol.schema",
              iota::AdminManagerService::_POST_PROTOCOL_SCHEMA);
  read_schema("put_protocol.schema",
              iota::AdminManagerService::_PUT_PROTOCOL_SCHEMA);
  read_schema("post_service_manager.schema",
              iota::AdminManagerService::_POST_SERVICE_SCHEMA);
  read_schema("put_service_manager.schema",
              iota::AdminManagerService::_PUT_SERVICE_SCHEMA);
}

iota::AdminManagerService::~AdminManagerService() {}

void iota::AdminManagerService::set_timeout(unsigned short timeout) {
  _timeout = timeout;
}

void iota::AdminManagerService::start() {
  std::map<std::string, std::string> filters;

  add_common_urls(filters);

  // Manager manages protocols
  add_url(ADMIN_SERVICE_PROTOCOLS, filters,
          REST_HANDLE(&iota::AdminManagerService::protocols), this);

  add_oauth_media_filters();
  check_for_logs();
}

void iota::AdminManagerService::create_collection(
    boost::shared_ptr<iota::ServiceCollection>& col) {
  col.reset(new ServiceMgmtCollection());
}

void iota::AdminManagerService::resolve_endpoints(
    std::vector<DeviceToBeAdded>& v_devices_endpoint_out,
    const std::string& devices_protocols_in, std::string service,
    std::string sub_service) {
  /*
   devices_protocol_in: it brings the whole list of devices with their protocol.
   "devices": [
  { "protocol": <identifier>, "device_id": "device_id" <other fields in IoTAgent
  API> },
  */
  rapidjson::Document document;
  char buffer[devices_protocols_in.length()];
  // memcpy(buffer, str_attribute.c_str(), str_attribute.length());
  strcpy(buffer, devices_protocols_in.c_str());
  if (document.ParseInsitu<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "AdminManagerService: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";

    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                              what.str(),
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  std::map<std::string, std::vector<IotagentType> > map_endpoints;

  if (document.HasMember("devices")) {
    IOTA_LOG_DEBUG(m_log,
                   "resolve_endpoints: devices found, parsing individually");
    const rapidjson::Value& devices = document["devices"];
    if (!devices.IsArray()) {
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                                "not proper formatted [devices]: not an array",
                                iota::types::RESPONSE_CODE_BAD_REQUEST);
    }
    IOTA_LOG_DEBUG(m_log, "resolve_endpoints: size of elements ["
                              << devices.Size() << "]");
    try {
      for (rapidjson::SizeType i = 0; i < devices.Size(); i++) {
        if (devices[i].HasMember("protocol")) {
          const std::string protocol(devices[i]["protocol"].GetString());
          IOTA_LOG_DEBUG(
              m_log, "resolve_endpoints: Processing first Device: protocol ["
                         << protocol << "]");
          std::vector<IotagentType> v_endpoint = map_endpoints[protocol];

          if (v_endpoint.size() == 0) {
            IOTA_LOG_DEBUG(m_log,
                           "resolve_endpoints: getting endpoints for protocol ["
                               << protocol << "]");
            v_endpoint = _service_mgmt.get_iotagents_by_service(
                service, sub_service, protocol);
            map_endpoints[protocol] = v_endpoint;
            IOTA_LOG_DEBUG(m_log, "resolve_endpoints: endpoints ["
                                      << v_endpoint.size()
                                      << "] found for device");
          }

          // Now link endpoints with device.
          for (int j = 0; j < v_endpoint.size(); j++) {
            rapidjson::StringBuffer string_buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(
                string_buffer);
            devices[i].Accept(writer);

            iota::DeviceToBeAdded dev_add(string_buffer.GetString(),
                                          v_endpoint[j]);
            v_devices_endpoint_out.push_back(dev_add);
            IOTA_LOG_DEBUG(m_log, "resolve_endpoints: adding endpoint ["
                                      << v_endpoint[j] << "] for device ["
                                      << string_buffer.GetString() << "]");
          }

        } else {
          IOTA_LOG_ERROR(m_log, "protocol is empty");
        }
      }
    } catch (iota::IotaException& ie) {
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                                ie.what(),
                                iota::types::RESPONSE_CODE_BAD_REQUEST);
    }

    if (v_devices_endpoint_out.size() == 0) {
      std::string error_details(
          iota::types::RESPONSE_MESSAGE_MISSING_IOTAGENTS);
      error_details.append("[service ");
      error_details.append(service);
      error_details.append(" sub-service ");
      error_details.append(sub_service);
      error_details.append("]");
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                                error_details,
                                iota::types::RESPONSE_CODE_BAD_REQUEST);
    }
  } else {
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                              "not proper formatted [devices] : not found",
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }
}

int iota::AdminManagerService::operation_device_iotagent(
    std::string url_iotagent, const std::string& body, std::string service,
    std::string sub_service, std::string x_auth_token,
    const std::string& method, std::string& response) {
  boost::shared_ptr<iota::HttpClient> http_client;
  pion::http::response_ptr http_response;

  response.assign("");

  int code_i = -1;
  try {
    iota::IoTUrl dest(url_iotagent);
    std::string resource = dest.getPath();
    std::string query = dest.getQuery();
    std::string server = dest.getHost();
    std::string compound_server(server);

    compound_server.append(":");
    compound_server.append(boost::lexical_cast<std::string>(dest.getPort()));
    IOTA_LOG_DEBUG(m_log, "Server " << server);

    pion::http::request_ptr request(new pion::http::request());
    request->set_method(method);
    request->set_resource(resource);
    request->set_query_string(query);
    request->set_content(body);
    request->set_content_type(iota::types::IOT_CONTENT_TYPE_JSON);

    request->add_header(iota::types::IOT_HTTP_HEADER_ACCEPT,
                        iota::types::IOT_CONTENT_TYPE_JSON);
    request->add_header(pion::http::types::HEADER_HOST, server);

    request->add_header(iota::types::FIWARE_SERVICE, service);
    request->add_header(iota::types::FIWARE_SERVICEPATH, sub_service);

    if (!x_auth_token.empty()) {
      request->add_header(iota::types::IOT_HTTP_HEADER_AUTH, x_auth_token);
    }

    http_client.reset(new iota::HttpClient(server, dest.getPort()));

    http_response = http_client->send(request, _timeout, "");
    code_i = check_alarm(http_response, http_client);
    if (code_i != -1) {
      response.assign(http_response->get_content());
    } else {
      response.assign(http_client->get_error().message());
    }
  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_log, e.what());
    iota::Alarm::error(iota::types::ALARM_CODE_NO_IOTA, url_iotagent,
                       iota::types::ERROR, e.what());
  }
  return code_i;
}

int iota::AdminManagerService::get_all_devices_json(
    const std::string& service, const std::string& service_path, int limit,
    int offset, const std::string& detailed, const std::string& entity,
    pion::http::response& http_response, std::string& response,
    std::string request_identifier, std::string x_auth_token,
    std::string protocol_filter) {
  iota::ServiceMgmtCollection manager_service_collection;
  int limitI = limit;
  int offsetI = offset;
  int total_count = 0;
  mongo::BSONArrayBuilder builder_array;
  mongo::BSONObjBuilder builder_json;
  int code = pion::http::types::RESPONSE_CODE_OK;
  int code_i = pion::http::types::RESPONSE_CODE_OK;

  IOTA_LOG_DEBUG(m_log,
                 "AdminManagerService: get_all_devices_json, starting...");

  if ((!detailed.empty()) && (detailed.compare(iota::store::types::ON) != 0) &&
      (detailed.compare(iota::store::types::OFF) != 0)) {
    IOTA_LOG_DEBUG(m_log,
                   "status=" << pion::http::types::RESPONSE_CODE_BAD_REQUEST);
    return create_response(pion::http::types::RESPONSE_CODE_BAD_REQUEST,
                           types::RESPONSE_MESSAGE_BAD_REQUEST,
                           "parameter detailed must be on or off",
                           http_response, response);
  }

  std::string log_message(" id-request=" + request_identifier);
  IOTA_LOG_DEBUG(m_log, log_message);

  std::vector<iota::IotagentType> all_dest;
  try {
    all_dest = manager_service_collection.get_iotagents_by_service(
        service, service_path, protocol_filter);
  } catch (iota::IotaException& e) {
    IOTA_LOG_ERROR(m_log, log_message << e.what());
  }

  std::map<std::string, std::string> response_from_iotagent_nok;
  for (int i = 0; i < all_dest.size(); i++) {
    try {
      iota::IoTUrl dest(all_dest.at(i));

      // Query Parameters
      std::multimap<std::string, std::string> query_parameters;
      query_parameters.insert(std::pair<std::string, std::string>(
          iota::store::types::PROTOCOL, protocol_filter));
      query_parameters.insert(std::pair<std::string, std::string>(
          iota::store::types::LIMIT, boost::lexical_cast<std::string>(limitI)));
      query_parameters.insert(std::pair<std::string, std::string>(
          iota::store::types::OFFSET,
          boost::lexical_cast<std::string>(offsetI)));
      query_parameters.insert(std::pair<std::string, std::string>(
          iota::store::types::ENTITY, entity));
      query_parameters.insert(std::pair<std::string, std::string>(
          iota::store::types::DETAILED, detailed));
      // Build request
      pion::http::request_ptr request =
          create_request(pion::http::types::REQUEST_METHOD_GET,
                         dest.getPath() + iota::ADMIN_SERVICE_DEVICES,
                         iota::make_query_string(query_parameters), "",
                         x_auth_token, request_identifier, service,
                         service_path, dest.getHost(), dest.getPort());

      boost::shared_ptr<iota::HttpClient> http_client(
          new iota::HttpClient(dest.getHost(), dest.getPort()));

      boost::property_tree::ptree additional_info;

      log_message.append(" endpoint=" + http_client->getRemoteEndpoint());

      pion::http::response_ptr resp_http =
          http_client->send(request, _timeout, "");
      code_i = check_alarm(resp_http, http_client);

      log_message.append(" error-conn=" + http_client->get_error().message());
      log_message.append(" status-code=" +
                         boost::lexical_cast<std::string>(code_i));

      if (code_i == pion::http::types::RESPONSE_CODE_OK) {
        std::string response_agent = resp_http->get_content();
        mongo::BSONObj obj_mongo = mongo::fromjson(response_agent);
        int countI = obj_mongo.getIntField(iota::store::types::COUNT);
        total_count += countI;
        PION_LOG_DEBUG(m_log, "get device count = " << countI);
        std::vector<mongo::BSONElement> devices =
            obj_mongo.getField(iota::store::types::DEVICES).Array();
        for (int j = 0; j < devices.size(); j++) {
          builder_array.append(devices.at(j));
        }
        if (offsetI > 0) {
          if (offsetI > countI) {
            // hay m�s offset que elementos
            offsetI = offsetI - countI;
          } else {
            // en countI dejamos los elemtos que se han devuelto en device
            countI = countI - offsetI;
            // se ha consumido todo el offset
            offsetI = 0;
          }
        }

        if (limitI > 0) {
          // restamos los elementos devueltos al limit
          if (limitI > countI) {
            limitI = limitI - countI;
          } else {
            limitI = -1;
          }
        }
        IOTA_LOG_DEBUG(m_log, "get device new limitI "
                                  << limitI << " new offset " << offsetI);

      } else {
        std::string message;
        if (code_i != -1) {
          message = resp_http->get_content();
        } else {
          message = http_client->get_error().message();
        }
        response_from_iotagent_nok.insert(std::pair<std::string, std::string>(
            all_dest.at(i), error_manager(code_i, message)));
      }
    } catch (std::exception& e) {
      code_i = -1;
      std::string message(e.what());
      response_from_iotagent_nok.insert(std::pair<std::string, std::string>(
          all_dest.at(i), error_manager(code_i, message)));
      IOTA_LOG_ERROR(m_log, "Processing endpoint " << all_dest.at(i) << " "
                                                   << message);
    }
  }
  IOTA_LOG_DEBUG(m_log, "code" + boost::lexical_cast<std::string>(code));

  builder_json.append("count", total_count);
  builder_json.appendArray(iota::store::types::DEVICES, builder_array.obj());
  mongo::BSONObj result = builder_json.obj();
  response = result.jsonString();
  add_errors_to_response(response, response_from_iotagent_nok);
  http_response.set_status_code(pion::http::types::RESPONSE_CODE_OK);
  http_response.set_status_message(
      iota::Configurator::instance()->getHttpMessage(
          pion::http::types::RESPONSE_CODE_OK));
  IOTA_LOG_DEBUG(
      m_log, "content=" + response + boost::lexical_cast<std::string>(code));

  return create_response(code, "", "", http_response, response);
  // return code;
}

int iota::AdminManagerService::get_a_device_json(
    const std::string& service, const std::string& service_path,
    const std::string& device_id, pion::http::response& http_response,
    std::string& response, std::string request_identifier,
    std::string x_auth_token, std::string protocol_filter) {
  int code = pion::http::types::RESPONSE_CODE_NOT_FOUND;
  int code_i = pion::http::types::RESPONSE_CODE_NOT_FOUND;

  iota::ServiceMgmtCollection manager_service_collection;

  IOTA_LOG_DEBUG(m_log, "AdminManagerService: get_a_device_json, starting...");

  std::vector<iota::IotagentType> all_dest =
      manager_service_collection.get_iotagents_by_service(
          service, service_path, protocol_filter, iota::types::LIMIT_MAX, 0);

  std::map<std::string, std::string> response_from_iotagent;
  std::map<std::string, std::string> response_from_iotagent_nok;

  std::string log_message;
  for (int i = 0; i < all_dest.size(); i++) {
    try {
      log_message.clear();
      log_message.append(" id-request=" + request_identifier);

      iota::IoTUrl dest(all_dest.at(i));
      // Build request
      std::multimap<std::string, std::string> query_parameters;
      pion::http::request_ptr request = create_request(
          pion::http::types::REQUEST_METHOD_GET,
          dest.getPath() + iota::ADMIN_SERVICE_DEVICES + "/" + device_id,
          iota::make_query_string(query_parameters), "", x_auth_token,
          request_identifier, service, service_path, dest.getHost(),
          dest.getPort());

      boost::shared_ptr<iota::HttpClient> http_client(
          new iota::HttpClient(dest.getHost(), dest.getPort()));

      boost::property_tree::ptree additional_info;

      log_message.append(" endpoint=" + http_client->getRemoteEndpoint());
      pion::http::response_ptr resp_http =
          http_client->send(request, _timeout, "");
      code_i = check_alarm(resp_http, http_client);

      log_message.append(" error-conn=" + http_client->get_error().message());
      log_message.append(" status-code=" +
                         boost::lexical_cast<std::string>(code_i));

      if (code_i == pion::http::types::RESPONSE_CODE_OK) {
        response_from_iotagent.insert(std::pair<std::string, std::string>(
            all_dest.at(i), resp_http->get_content()));
      } else {
        std::string message;
        if (code_i != -1) {
          message = resp_http->get_content();
        } else {
          message = http_client->get_error().message();
        }
        response_from_iotagent_nok.insert(std::pair<std::string, std::string>(
            all_dest.at(i), error_manager(code_i, message)));
      }

      IOTA_LOG_INFO(m_log, log_message);
    } catch (std::exception& e) {
      code_i = -1;
      std::string message(e.what());
      response_from_iotagent_nok.insert(std::pair<std::string, std::string>(
          all_dest.at(i), error_manager(-1, message)));
      IOTA_LOG_ERROR(m_log, "Processing endpoint " << all_dest.at(i));
    }
  }

  int total_count = 0;
  mongo::BSONObjBuilder builder_json;
  mongo::BSONArrayBuilder builder_array;
  std::map<std::string, std::string>::iterator it_resp =
      response_from_iotagent.begin();
  while (it_resp != response_from_iotagent.end()) {
    try {
      boost::algorithm::trim(it_resp->second);
      if (!it_resp->second.empty()) {
        IOTA_LOG_DEBUG(m_log, "Response " << it_resp->first << " "
                                          << it_resp->second);
        mongo::BSONObj obj_mongo = mongo::fromjson(it_resp->second);
        total_count++;
        builder_array.append(obj_mongo);
      }
    } catch (mongo::MsgAssertionException& e) {
      log_message.append(" endpoint=" + it_resp->first + " error-json=" +
                         std::string(e.what()));
      IOTA_LOG_ERROR(m_log, log_message);
    } catch (std::exception& e) {
      log_message.append(" endpoint=" + it_resp->first + " error=" +
                         std::string(e.what()));
      IOTA_LOG_ERROR(m_log, log_message);
    }
    ++it_resp;
  }
  builder_json.append("count", total_count);
  builder_json.appendArray(iota::store::types::DEVICES, builder_array.obj());
  mongo::BSONObj result = builder_json.obj();

  response = result.jsonString();
  add_errors_to_response(response, response_from_iotagent_nok);
  IOTA_LOG_DEBUG(m_log, "content=" + response);
  http_response.set_status_code(pion::http::types::RESPONSE_CODE_OK);
  http_response.set_status_message(
      iota::Configurator::instance()->getHttpMessage(
          pion::http::types::RESPONSE_CODE_OK));
  code = pion::http::types::RESPONSE_CODE_OK;
  return create_response(code, "", "", http_response, response);
  // return code;
}

void iota::AdminManagerService::receive_get_devices(
    std::string request_identifier, pion::http::response& http_response_request,
    boost::shared_ptr<iota::HttpClient> connection,
    pion::http::response_ptr response_ptr,
    const boost::system::error_code& error) {
  std::string log_message(" id-request=" + request_identifier + " endpoint=" +
                          connection->getRemoteEndpoint() + " error-conn=" +
                          error.message());

  int code = -1;
  if (response_ptr.get() != NULL) {
    code = response_ptr->get_status_code();
  }
  log_message.append(" status-code=" + boost::lexical_cast<std::string>(code));
  IOTA_LOG_INFO(m_log, log_message);
  // If no successful response, nothing
  if (code != pion::http::types::RESPONSE_CODE_OK) {
    return;
  }
}

int iota::AdminManagerService::post_multiple_devices(
    std::vector<DeviceToBeAdded>& v_devices_endpoint_in, std::string service,
    std::string sub_service, std::string x_auth_token, std::string& response) {
  // For each protocol, endpoints
  std::map<std::string, std::string> response_from_iotagent;
  std::map<std::string, std::string> response_from_iotagent_nok;

  std::string log_message(" service=" + service + " sub_service=" +
                          sub_service);
  IOTA_LOG_DEBUG(m_log, log_message);

  response.assign("");
  int code = pion::http::types::RESPONSE_CODE_CREATED;
  for (int i = 0; i < v_devices_endpoint_in.size(); i++) {
    DeviceToBeAdded& dev = v_devices_endpoint_in[i];
    std::string temp_res;
    std::string url_endpoint = dev.get_endpoint();
    // url_endpoint.append("/");
    url_endpoint.append(iota::ADMIN_SERVICE_DEVICES);

    std::string content("{\"devices\":[");
    content.append(dev.get_device_json());
    content.append("]}");

    int code_i = operation_device_iotagent(
        url_endpoint, content, service, sub_service, x_auth_token,
        pion::http::types::REQUEST_METHOD_POST, temp_res);
    // If no successful response, nothing
    if (code_i == pion::http::types::RESPONSE_CODE_CREATED) {
      response_from_iotagent.insert(
          std::pair<std::string, std::string>(url_endpoint, temp_res));
    } else {
      std::string message(temp_res);
      response_from_iotagent_nok.insert(std::pair<std::string, std::string>(
          url_endpoint, error_manager(code_i, message)));
    }

    IOTA_LOG_DEBUG(
        m_log, "Endpoint: ["
                   << url_endpoint
                   << "] Result: " + boost::lexical_cast<std::string>(code_i));
  }

  add_errors_to_response(response, response_from_iotagent_nok);
  if (response_from_iotagent.size() == 0) {
    code = pion::http::types::RESPONSE_CODE_SERVER_ERROR;
  } else if (!response.empty()) {
    code = pion::http::types::RESPONSE_CODE_OK;
  }

  return code;
}

int iota::AdminManagerService::delete_multiple_devices(
    std::vector<DeviceToBeAdded>& v_devices_endpoint_in,
    const std::string& device_id, std::string service, std::string sub_service,
    std::string x_auth_token, std::string& response) {
  // For each protocol, endpoints
  std::map<std::string, std::string> response_from_iotagent;
  std::map<std::string, std::string> response_from_iotagent_nok;
  std::string log_message(" service=" + service + " sub_service=" +
                          sub_service);
  IOTA_LOG_DEBUG(m_log, log_message);

  int code = pion::http::types::RESPONSE_CODE_NO_CONTENT;
  for (int i = 0; i < v_devices_endpoint_in.size(); i++) {
    DeviceToBeAdded& dev = v_devices_endpoint_in[i];

    std::string url_endpoint = dev.get_endpoint();
    // url_endpoint.append("/");
    url_endpoint.append(iota::ADMIN_SERVICE_DEVICES);
    url_endpoint.append("/");
    url_endpoint.append(pion::algorithm::url_encode(device_id));

    std::string content;
    std::string temp_res;

    int code_i = operation_device_iotagent(
        url_endpoint, content, service, sub_service, x_auth_token,
        pion::http::types::REQUEST_METHOD_DELETE, temp_res);

    // If no successful response, nothing
    if (code_i == pion::http::types::RESPONSE_CODE_NO_CONTENT) {
      response_from_iotagent.insert(
          std::pair<std::string, std::string>(url_endpoint, temp_res));
    } else {
      std::string message(temp_res);
      response_from_iotagent_nok.insert(std::pair<std::string, std::string>(
          url_endpoint, error_manager(code_i, message)));
    }

    IOTA_LOG_DEBUG(
        m_log, "Endpoint: ["
                   << url_endpoint
                   << "] Result: " + boost::lexical_cast<std::string>(code_i));
  }
  add_errors_to_response(response, response_from_iotagent_nok);
  if (response_from_iotagent.size() == 0) {
    code = pion::http::types::RESPONSE_CODE_NOT_FOUND;
  } else if (!response.empty()) {
    code = pion::http::types::RESPONSE_CODE_OK;
  }
  return code;
}

int iota::AdminManagerService::delete_device_json(
    const std::string& service, const std::string& service_path,
    const std::string& id_device, pion::http::response& http_response,
    std::string& response, std::string token, const std::string& protocol) {
  IOTA_LOG_DEBUG(m_log,
                 "AdminManagerService: delete_device_json: validating input");

  if (protocol.empty()) {
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_MISSING_PARAMETER,
                              "protocol parameter is mandatory",
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  if (service.empty() || service_path.empty()) {
    IOTA_LOG_ERROR(m_log, "Missing Service or Service_path");
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                              "Service or Service path are missing",
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  std::vector<iota::DeviceToBeAdded> v_endpoints;
  std::vector<IotagentType> v_endpoint;

  IOTA_LOG_DEBUG(m_log, "delete_device_json: [" << protocol << "]");
  v_endpoint =
      _service_mgmt.get_iotagents_by_service(service, service_path, protocol);

  for (int j = 0; j < v_endpoint.size(); j++) {
    iota::DeviceToBeAdded dev_add("", v_endpoint[j]);
    v_endpoints.push_back(dev_add);
  }

  IOTA_LOG_DEBUG(m_log, "delete_device_json: endpoints found ["
                            << v_endpoints.size() << "]");
  int code = delete_multiple_devices(v_endpoints, id_device, service,
                                     service_path, token, response);
  http_response.set_status_code(code);

  IOTA_LOG_DEBUG(m_log, "delete_device_json: POSTs processed: [" << code
                                                                 << "]");

  // return http_response.get_status_code();
  return create_response(code, "", "", http_response, response);
}

int iota::AdminManagerService::post_device_json(
    const std::string& service, const std::string& service_path,
    const std::string& body, pion::http::response& http_response,
    std::string& response, std::string token) {
  IOTA_LOG_DEBUG(m_log,
                 "AdminManagerService: post_devices_json: validating input");

  if (service.empty() || service_path.empty()) {
    IOTA_LOG_ERROR(m_log, "");
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                              "Service or Service path are missing",
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  std::vector<iota::DeviceToBeAdded> v_endpoints;

  resolve_endpoints(v_endpoints, body, service, service_path);
  IOTA_LOG_DEBUG(m_log, "post_device_json: endpoints found ["
                            << v_endpoints.size() << "]");
  int code = post_multiple_devices(v_endpoints, service, service_path, token,
                                   response);
  http_response.set_status_code(code);

  IOTA_LOG_DEBUG(m_log, "post_device_json: POSTs processed: [" << code << "]");
  // return http_response.get_status_code();
  return create_response(code, "", "", http_response, response);
}

int iota::AdminManagerService::put_device_json(
    const std::string& service, const std::string& service_path,
    const std::string& device_id, const std::string& body,
    pion::http::response& http_response, std::string& response,
    const std::string& token, const std::string& protocol) {
  IOTA_LOG_DEBUG(m_log,
                 get_class_name() + ": put_device_json: validating input");

  std::string error_details;
  int code = pion::http::types::RESPONSE_CODE_NO_CONTENT;

  if (service.empty() || service_path.empty()) {
    IOTA_LOG_ERROR(m_log, "Missing Service or Service_path");
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                              "Service or Service path are missing",
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  if (body.empty()) {
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                              "Body is missing",
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  if (protocol.empty()) {
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_MISSING_PARAMETER,
                              "protocol parameter is mandatory",
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  std::vector<iota::DeviceToBeAdded> v_endpoints_put;

  std::vector<IotagentType> v_endpoint;

  if (validate_json_schema(body, iota::AdminService::_PUT_DEVICE_SCHEMA,
                           error_details)) {
    IOTA_LOG_DEBUG(
        m_log,
        "put_device_json: SCHEMA validated, getting endpoints for protocol ["
            << protocol << "]");
    v_endpoint =
        _service_mgmt.get_iotagents_by_service(service, service_path, protocol);

    IOTA_LOG_DEBUG(m_log, "put_device_json: endpoints ["
                              << v_endpoint.size() << "] found for device");

    // Now link endpoints with device.
    for (int j = 0; j < v_endpoint.size(); j++) {
      iota::DeviceToBeAdded dev_add(body, v_endpoint[j]);
      v_endpoints_put.push_back(dev_add);
    }

    IOTA_LOG_DEBUG(m_log, "put_device_json:  sending request to "
                              << v_endpoints_put.size() << " endpoints");

    code = put_multiple_devices(v_endpoints_put, device_id, service,
                                service_path, protocol, token, response);
    http_response.set_status_code(code);

  } else {
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                              error_details,
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }
  // return http_response.get_status_code();
  return create_response(code, "", "", http_response, response);
}

int iota::AdminManagerService::put_multiple_devices(
    std::vector<DeviceToBeAdded>& v_devices_endpoint_in,
    const std::string& device_id, std::string service, std::string sub_service,
    const std::string& protocol, std::string x_auth_token,
    std::string& response) {
  std::string log_message(" service=" + service + " sub_service=" +
                          sub_service);
  IOTA_LOG_DEBUG(m_log, log_message);

  // For each protocol, endpoints
  std::map<std::string, std::string> response_from_iotagent;
  std::map<std::string, std::string> response_from_iotagent_nok;
  int code = pion::http::types::RESPONSE_CODE_NO_CONTENT;
  for (int i = 0; i < v_devices_endpoint_in.size(); i++) {
    DeviceToBeAdded& dev = v_devices_endpoint_in[i];
    std::string temp_res;
    std::string url_endpoint = dev.get_endpoint();
    // url_endpoint.append("/");
    url_endpoint.append(iota::ADMIN_SERVICE_DEVICES);
    url_endpoint.append("/");
    url_endpoint.append(pion::algorithm::url_encode(device_id));
    if (!protocol.empty()) {
      url_endpoint.append("?protocol=" + pion::algorithm::url_encode(protocol));
    }

    int code_i = operation_device_iotagent(
        url_endpoint, dev.get_device_json(), service, sub_service, x_auth_token,
        pion::http::types::REQUEST_METHOD_PUT, temp_res);
    // If no successful response, nothing
    if (code_i == pion::http::types::RESPONSE_CODE_NO_CONTENT) {
      response_from_iotagent.insert(
          std::pair<std::string, std::string>(url_endpoint, temp_res));
    } else {
      std::string message(temp_res);
      response_from_iotagent_nok.insert(std::pair<std::string, std::string>(
          url_endpoint, error_manager(code_i, message)));
    }

    IOTA_LOG_DEBUG(
        m_log, "Endpoint: ["
                   << url_endpoint
                   << "] Result: " + boost::lexical_cast<std::string>(code_i));
  }

  add_errors_to_response(response, response_from_iotagent_nok);
  if (response_from_iotagent.size() == 0) {
    code = pion::http::types::RESPONSE_CODE_NOT_FOUND;
  } else if (!response.empty()) {
    code = pion::http::types::RESPONSE_CODE_OK;
  }
  return code;
}

std::string iota::AdminManagerService::get_role() {
  return " [working as manager] ";
}

std::string iota::AdminManagerService::get_class_name() { return _class_name; }

std::string iota::AdminManagerService::get_param_resource(
    const std::multimap<std::string, std::string>& query_parameters,
    bool mandatory) {
  std::string result;
  std::multimap<std::string, std::string>::const_iterator it;

  it = query_parameters.find(iota::store::types::PROTOCOL);
  if (it != query_parameters.end()) {
    result = it->second;
  }

  if (mandatory && result.empty()) {
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_MISSING_PARAMETER,
                              "protocol parameter is mandatory",
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  return result;
}

void iota::AdminManagerService::protocols(
    pion::http::request_ptr& http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response, std::string& response) {
  // TODO protocols,  POST y PUT don't need authentication (it is a register)
  std::string trace_message =
      http_request_ptr->get_header(iota::types::HEADER_TRACE_MESSAGES);
  std::string method = http_request_ptr->get_method();
  IOTA_LOG_INFO(
      m_log, " protocols method:" + method + " trace_message:" + trace_message);

  std::string reason;
  std::string error_details;
  int code = pion::http::types::RESPONSE_CODE_OK;

  std::string temp, op;
  try {
    check_mongo_config();

    std::string service(
        http_request_ptr->get_header(iota::types::FIWARE_SERVICE));
    std::string service_path(
        http_request_ptr->get_header(iota::types::FIWARE_SERVICEPATH));

    if (method.compare(pion::http::types::REQUEST_METHOD_POST) == 0) {
      std::string content = http_request_ptr->get_content();
      boost::trim(content);
      code = post_protocol_json(service, service_path, content, http_response,
                                response);
    } else if (method.compare(pion::http::types::REQUEST_METHOD_GET) == 0) {
      std::string detailed, protocol;
      int limit = types::LIMIT_DEFAULT, offset = 0;
      std::multimap<std::string, std::string>::iterator it;
      it = query_parameters.find(iota::store::types::LIMIT);
      if (it != query_parameters.end()) {
        temp = it->second;
        op = "limit";
        if (!temp.empty()) {
          limit = boost::lexical_cast<int>(temp);
        } else {
          limit = types::LIMIT_DEFAULT;
        }
        if (limit < 0) {
          IOTA_LOG_ERROR(m_log, " bad limit using default");
          limit = types::LIMIT_DEFAULT;
        } else if (limit > types::LIMIT_MAX) {
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
        } else {
          offset = 0;
        }
      }
      it = query_parameters.find(iota::store::types::DETAILED);
      if (it != query_parameters.end()) {
        detailed = it->second;
      }
      it = query_parameters.find(iota::store::types::RESOURCE);
      if (it != query_parameters.end()) {
        protocol = it->second;
      }

      code = get_protocols_json(limit, offset, "on", protocol, http_response,
                                response);

    } else {
      code = pion::http::types::RESPONSE_CODE_NOT_FOUND;
      create_response(code, reason, error_details, http_response, response);
    }

  } catch (const boost::bad_lexical_cast& e) {
    IOTA_LOG_ERROR(m_log, "Capturada boost::bad_lexical_cast en services");
    IOTA_LOG_ERROR(m_log, e.what());
    reason.assign(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER);
    error_details.assign(op + " must be a number but it is " + temp);
    code = 400;
    create_response(code, reason, error_details, http_response, response);
  } catch (iota::IotaException& e) {
    IOTA_LOG_ERROR(m_log, "Capturada: Exception en services");
    IOTA_LOG_ERROR(m_log, e.what());
    reason.assign(e.reason());
    error_details.assign(e.what());
    code = e.status();
    create_response(code, reason, error_details, http_response, response);
  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_log, "Excepcion en services");
    reason.assign(iota::types::RESPONSE_MESSAGE_INTERNAL_ERROR);
    error_details.assign(e.what());
    code = pion::http::types::RESPONSE_CODE_SERVER_ERROR;
    create_response(code, reason, error_details, http_response, response);
  }

  IOTA_LOG_INFO(m_log, " method:" + method + " trace_message:" + trace_message +
                           " code: " + boost::lexical_cast<std::string>(code) +
                           " response:" + response);
}

int iota::AdminManagerService::delete_all_protocol_json(
    pion::http::response& http_response, const std::string& protocol_name,
    std::string& response) {
  int code = pion::http::types::RESPONSE_CODE_NO_CONTENT;
  std::string param_request("delete_all_protocol_json");
  IOTA_LOG_DEBUG(m_log, param_request);

  std::string reason;
  std::string error_details;
  std::string b_query_service_path;

  Collection table(iota::store::types::PROTOCOL_TABLE);
  mongo::BSONObj all;
  if (!protocol_name.empty()) {
    all = BSON(iota::store::types::PROTOCOL_NAME << protocol_name);
  }
  table.remove(all, 0);

  Collection tableSM(iota::store::types::MANAGER_SERVICE_TABLE);
  tableSM.remove(all, 0);

  return create_response(code, reason, error_details, http_response, response);
}

int iota::AdminManagerService::post_protocol_json(
    const std::string& service, const std::string& service_path,
    const std::string& body, pion::http::response& http_response,
    std::string& response) {
  std::string param_request("post_protocol_json service=" + service +
                            " service_path=" + service_path);
  IOTA_LOG_DEBUG(m_log, param_request);
  int code = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
  std::string reason, srv, srv_path;
  std::multimap<std::string, mongo::BSONObj>::iterator it;
  std::string error_details;
  ServiceMgmtCollection service_table;
  boost::shared_ptr<iota::ProtocolCollection> protocol_table(
      new iota::ProtocolCollection());

  if (body.empty()) {
    error_details.assign("empty body");
    reason.assign(types::RESPONSE_MESSAGE_BAD_REQUEST);
    code = types::RESPONSE_CODE_BAD_REQUEST;
  } else if (validate_json_schema(
                 body, iota::AdminManagerService::_POST_PROTOCOL_SCHEMA,
                 error_details)) {
    mongo::BSONObj obj = mongo::fromjson(body);
    mongo::BSONObj insObj;

    // Resource and description define a protocol
    std::string endpoint = obj.getStringField(iota::store::types::IOTAGENT);
    std::string endpoint_id =
        obj.getStringField(iota::store::types::ENDPOINT_ID);
    if (endpoint_id.empty()) {
      IOTA_LOG_DEBUG(
          m_log, "in update protocol, there is an empty endpoint identifier:" +
                     endpoint +
                     ", check this iotagent to update, using ip as identifier");

      endpoint_id = endpoint;
    }
    std::string resource = obj.getStringField(iota::store::types::RESOURCE);
    std::string description =
        obj.getStringField(iota::store::types::PROTOCOL_DESCRIPTION);
    std::string protocol_name =
        obj.getStringField(iota::store::types::PROTOCOL_NAME);

    IOTA_LOG_DEBUG(m_log, "update protocol :" + protocol_name + " iotagent:" +
                              endpoint + " resource:" + resource);

    int num_ups = 0;
    try {
      // db.PROTOCOL.update( { protocol: "PDI-IoTA-UltraLight", endpoints: {
      // $elemMatch: { identifier: "KK1:8080" } } },
      //        { $set: { "endpoints.$.endpoint": "http://127.0.0.2:8080/iot",
      //        "endpoints.$.resource": "/iot/d" } }  )
      //   update endpoint and resource if exists identifier, example change ip
      num_ups = protocol_table->update_r(
          BSON(iota::store::types::PROTOCOL_NAME
               << protocol_name << iota::store::types::ENDPOINTS
               << BSON("$elemMatch" << BSON(iota::store::types::ENDPOINT_ID
                                            << endpoint_id))),
          BSON("$set" << BSON("endpoints.$.endpoint" << endpoint
                                                     << "endpoints.$.resource"
                                                     << resource)),
          true, 0);

    } catch (iota::IotaException e) {
      IOTA_LOG_DEBUG(m_log, "update protocol : no exists this protocol"
                                << e.what());
    }

    if (num_ups == 0) {
      try {
        // db.PROTOCOL.update( { protocol: "PDI-IoTA-UltraLight", endpoints: {
        // $elemMatch: { endpoint: "http://127.0.0.2:8080/iot" } } },
        //        { $set: { "endpoints.$.identifier": "KK1:8080",
        //        "endpoints.$.resource": "/iot/d" } }  )
        //   update identifier and resource if exists endpoint, example change
        //   identifier
        num_ups = protocol_table->update_r(
            BSON(iota::store::types::PROTOCOL_NAME
                 << protocol_name << iota::store::types::ENDPOINTS
                 << BSON("$elemMatch"
                         << BSON(iota::store::types::ENDPOINT << endpoint))),
            BSON("$set" << BSON("endpoints.$.identifier"
                                << endpoint_id << "endpoints.$.resource"
                                << resource)),
            true, 0);

      } catch (iota::IotaException e) {
        IOTA_LOG_DEBUG(m_log, "update protocol : no exists this identifier"
                                  << e.what());
      }
    }

    if (num_ups == 0) {
      // query only protocol name (identifier)
      // update description because it can be different
      // if there are several iotagents with different description
      // with every register, the description would change
      num_ups = protocol_table->update_r(
          BSON(iota::store::types::PROTOCOL_NAME << protocol_name),
          BSON("$set" << BSON(iota::store::types::PROTOCOL_DESCRIPTION
                              << description)
                      << "$addToSet"
                      << BSON(iota::store::types::ENDPOINTS << BSON(
                                  iota::store::types::ENDPOINT
                                  << endpoint << iota::store::types::RESOURCE
                                  << resource << iota::store::types::ENDPOINT_ID
                                  << endpoint_id))),
          true, 0);
    }

    mongo::BSONElement element = obj.getField(iota::store::types::SERVICES);
    std::map<std::string, mongo::BSONObj> services_in_mongo;
    service_table.fillServices(endpoint, protocol_name, services_in_mongo);

    if (element.eoo()) {
      IOTA_LOG_DEBUG(m_log,
                     "Protocol: " + protocol_name + " has not got services");
    } else {
      std::vector<mongo::BSONElement> be = element.Array();
      IOTA_LOG_DEBUG(m_log, "insert services ");
      for (unsigned int i = 0; i < be.size(); i++) {
        mongo::BSONObj srvObj = be[i].embeddedObject();
        srv = srvObj.getStringField(iota::store::types::SERVICE);
        srv_path = srvObj.getStringField(iota::store::types::SERVICE_PATH);
        it = services_in_mongo.find(srv + "|" + srv_path);
        if (it == services_in_mongo.end() ||
            srvObj.woCompare(it->second) != 0) {
          IOTA_LOG_DEBUG(m_log,
                         "services changed, update:" + srv + " " + srv_path);
          mongo::BSONObjBuilder query;
          query.append(iota::store::types::SERVICE,
                       srvObj.getStringField(iota::store::types::SERVICE));
          query.append(iota::store::types::SERVICE_PATH,
                       srvObj.getStringField(iota::store::types::SERVICE_PATH));

          query.append(iota::store::types::IOTAGENT, endpoint);
          query.append(iota::store::types::PROTOCOL, protocol_name);

          srvObj = srvObj.removeField(iota::store::types::SERVICE);
          srvObj = srvObj.removeField(iota::store::types::SERVICE_PATH);
          srvObj = srvObj.removeField(iota::store::types::RESOURCE);

          mongo::BSONObjBuilder data2set;
          data2set.appendElements(srvObj);
          data2set.append(iota::store::types::PROTOCOL_DESCRIPTION,
                          description);

          service_table.update(query.obj(), data2set.obj(), true, 0);
        } else {
          IOTA_LOG_DEBUG(m_log,
                         "services no changed service:" + srv + " " + srv_path);
        }
        if (it != services_in_mongo.end()) {
          // lo borramos para quedarno solo con los que hay que borrar
          services_in_mongo.erase(it);
          IOTA_LOG_DEBUG(m_log, "erase mappp service:" + srv + " " + srv_path);
        } else {
          IOTA_LOG_DEBUG(m_log, "no in mappp service:" + srv + " " + srv_path);
        }
      }
    }

    if (services_in_mongo.size() > 0) {
      IOTA_LOG_DEBUG(m_log, "there are services to delete");
      std::map<std::string, mongo::BSONObj>::iterator iter;
      for (iter = services_in_mongo.begin(); iter != services_in_mongo.end();
           ++iter) {
        service_table.remove(iter->second);
      }
    }

    code = pion::http::types::RESPONSE_CODE_CREATED;
  } else {
    reason.assign(types::RESPONSE_MESSAGE_BAD_REQUEST);
    code = pion::http::types::RESPONSE_CODE_BAD_REQUEST;
  }

  return create_response(code, reason, error_details, http_response, response);
}

int iota::AdminManagerService::post_service_json(
    const boost::shared_ptr<iota::ServiceCollection>& table,
    const std::string& service, const std::string& service_path,
    const std::string& body, pion::http::response& http_response,
    std::string& response, std::string x_auth_token,
    std::string request_identifier) {
  // Table is not used. This post makes a POST towards manager/protocols
  std::string param_request("service=" + service + " service_path=" +
                            service_path);
  std::string error_details;

  IOTA_LOG_DEBUG(m_log, param_request);
  int code = pion::http::types::RESPONSE_CODE_CREATED;
  boost::shared_ptr<iota::ServiceMgmtCollection> manager_service_collection(
      new iota::ServiceMgmtCollection());
  iota::ProtocolCollection proto_collection;

  if (body.empty()) {
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                              "empty body",
                              pion::http::types::RESPONSE_CODE_BAD_REQUEST);
  } else if (validate_json_schema(
                 body, iota::AdminManagerService::_POST_SERVICE_SCHEMA,
                 error_details)) {
    IOTA_LOG_DEBUG(m_log, "Search protocol of service");
    mongo::BSONObj obj = mongo::fromjson(body);
    std::vector<mongo::BSONObj> obj_protocols;
    manager_service_collection->getElementsFromBSON(obj, obj_protocols);

    // For each protocol, endpoints
    std::map<std::string, std::string> response_from_iotagent;
    std::map<std::string, std::string> response_from_iotagent_nok;
    for (int j = 0; j < obj_protocols.size(); j++) {
      std::string protocol_filter =
          obj_protocols[j].getStringField(iota::store::types::PROTOCOL_NAME);
      IOTA_LOG_DEBUG(m_log, "Search IoTA for " << protocol_filter);
      std::vector<iota::Protocol> protocols =
          proto_collection.get_endpoint_by_protocol(protocol_filter);
      iota::Protocol::resource_endpoint_vector all_dest;
      for (int k = 0; k < protocols.size(); k++) {
        all_dest = protocols[k].get_endpoints();
      }

      // Send to agents
      IOTA_LOG_DEBUG(m_log, "Number of endpoints to deploy service "
                                << all_dest.size());
      for (int i = 0; i < all_dest.size(); i++) {
        try {
          mongo::BSONObj trim_obj =
              obj_protocols[j]
                  .removeField(iota::store::types::PROTOCOL_DESCRIPTION)
                  .removeField(iota::store::types::PROTOCOL);

          mongo::BSONObjBuilder obj_to_send;
          mongo::BSONArrayBuilder services;
          mongo::BSONObjBuilder srv_to_send;
          srv_to_send.appendElements(trim_obj);
          srv_to_send.append(iota::store::types::RESOURCE,
                             all_dest.at(i).resource);
          services.append(srv_to_send.obj());
          obj_to_send.appendArray(iota::store::types::SERVICES, services.obj());
          mongo::BSONObj to_send = obj_to_send.obj();
          iota::IoTUrl dest(all_dest.at(i).endpoint);
          // Query Parameters
          std::multimap<std::string, std::string> query_parameters;

          // Build request
          pion::http::request_ptr request = create_request(
              pion::http::types::REQUEST_METHOD_POST,
              dest.getPath() + iota::ADMIN_SERVICE_SERVICES,
              iota::make_query_string(query_parameters), to_send.jsonString(),
              x_auth_token, request_identifier, service, service_path,
              dest.getHost(), dest.getPort());

          boost::shared_ptr<iota::HttpClient> http_client(
              new iota::HttpClient(dest.getHost(), dest.getPort()));

          boost::property_tree::ptree additional_info;

          param_request.append(" endpoint=" + http_client->getRemoteEndpoint());
          pion::http::response_ptr resp_http =
              http_client->send(request, _timeout, "");
          int code_i = check_alarm(resp_http, http_client);

          param_request.append(" error-conn=" +
                               http_client->get_error().message());
          param_request.append(" status-code=" +
                               boost::lexical_cast<std::string>(code_i));
          IOTA_LOG_DEBUG(m_log, "response from "
                                    << http_client->getRemoteEndpoint() << ":"
                                    << code_i << ":"
                                    << http_client->get_error().message());

          // If no successful response, nothing
          if (code_i == pion::http::types::RESPONSE_CODE_CREATED) {
            response_from_iotagent.insert(std::pair<std::string, std::string>(
                all_dest.at(i).endpoint, resp_http->get_content()));
          } else {
            std::string message;
            if (code_i != -1) {
              message = resp_http->get_content();
            } else {
              message = http_client->get_error().message();
            }
            response_from_iotagent_nok.insert(
                std::pair<std::string, std::string>(
                    all_dest.at(i).endpoint, error_manager(code_i, message)));
          }
        } catch (std::exception& e) {
          iota::Alarm::error(iota::types::ALARM_CODE_NO_IOTA,
                             all_dest.at(i).endpoint, iota::types::ERROR,
                             e.what());
        }
      }
    }

    add_errors_to_response(response, response_from_iotagent_nok);
    if (response_from_iotagent.size() == 0) {
      code = pion::http::types::RESPONSE_CODE_SERVER_ERROR;
    } else if (!response.empty()) {
      code = pion::http::types::RESPONSE_CODE_OK;
    }

  } else {
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                              error_details,
                              pion::http::types::RESPONSE_CODE_BAD_REQUEST);
  }
  IOTA_LOG_DEBUG(m_log, param_request << " status=" << code
                                      << " reason=" << response);
  http_response.set_status_code(code);
  http_response.set_status_message(
      iota::Configurator::instance()->getHttpMessage(code));
  return code;
}

int iota::AdminManagerService::put_service_json(
    const boost::shared_ptr<iota::ServiceCollection>& table,
    const std::string& service, const std::string& service_path,
    const std::string& id, const std::string& apikey,
    const std::string& resource, const std::string& body,
    pion::http::response& http_response, std::string& response,
    std::string x_auth_token, std::string request_identifier) {
  std::string param_request("service=" + service + " service_path=" +
                            service_path);
  std::string error_details;

  IOTA_LOG_DEBUG(m_log, param_request);
  int code = pion::http::types::RESPONSE_CODE_NO_CONTENT;
  int code_from_iota = pion::http::types::RESPONSE_CODE_NO_CONTENT;

  boost::shared_ptr<iota::ServiceMgmtCollection> manager_service_collection(
      new iota::ServiceMgmtCollection());
  iota::ProtocolCollection proto_collection;

  if (body.empty()) {
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                              "empty body",
                              pion::http::types::RESPONSE_CODE_BAD_REQUEST);
  } else if (validate_json_schema(
                 body, iota::AdminManagerService::_PUT_SERVICE_SCHEMA,
                 error_details)) {
    IOTA_LOG_DEBUG(m_log, "Search protocol of service");
    mongo::BSONObj obj = mongo::fromjson(body);
    std::vector<mongo::BSONObj> obj_protocols;
    manager_service_collection->getElementsFromBSON(obj, obj_protocols);

    // For each protocol, endpoints
    std::map<std::string, std::string> response_from_iotagent;
    std::map<std::string, std::string> response_from_iotagent_nok;
    for (int j = 0; j < obj_protocols.size(); j++) {
      std::string protocol_filter =
          obj_protocols[j].getStringField(iota::store::types::PROTOCOL_NAME);
      IOTA_LOG_DEBUG(m_log, "Search IoTA for " << protocol_filter);
      std::vector<iota::Protocol> protocols =
          proto_collection.get_endpoint_by_protocol(protocol_filter);
      iota::Protocol::resource_endpoint_vector all_dest;
      for (int k = 0; k < protocols.size(); k++) {
        all_dest = protocols[k].get_endpoints();
      }

      // Send to agents
      IOTA_LOG_DEBUG(m_log, "Number of endpoints to deploy service "
                                << all_dest.size());
      for (int i = 0; i < all_dest.size(); i++) {
        try {
          mongo::BSONObj trim_obj =
              obj_protocols[j]
                  .removeField(iota::store::types::PROTOCOL_DESCRIPTION)
                  .removeField(iota::store::types::PROTOCOL);
          mongo::BSONObjBuilder obj_to_send;
          mongo::BSONObjBuilder srv_to_send;
          obj_to_send.appendElements(trim_obj);
          // obj_to_send.append(iota::store::types::RESOURCE,
          // all_dest.at(i).resource);
          mongo::BSONObj to_send = obj_to_send.obj();
          iota::IoTUrl dest(all_dest.at(i).endpoint);

          // Query Parameters
          std::multimap<std::string, std::string> query_parameters;
          query_parameters.insert(std::pair<std::string, std::string>(
              iota::store::types::RESOURCE, all_dest.at(i).resource));
          std::string api_key =
              obj_protocols[j].getStringField(iota::store::types::APIKEY);
          if (!apikey.empty()) {
            query_parameters.insert(std::pair<std::string, std::string>(
                iota::store::types::APIKEY, apikey));
          }
          // Build request
          pion::http::request_ptr request = create_request(
              pion::http::types::REQUEST_METHOD_PUT,
              dest.getPath() + iota::ADMIN_SERVICE_SERVICES,
              iota::make_query_string(query_parameters), to_send.jsonString(),
              x_auth_token, request_identifier, service, service_path,
              dest.getHost(), dest.getPort());

          boost::shared_ptr<iota::HttpClient> http_client(
              new iota::HttpClient(dest.getHost(), dest.getPort()));

          boost::property_tree::ptree additional_info;

          param_request.append(" endpoint=" + http_client->getRemoteEndpoint());
          pion::http::response_ptr resp_http =
              http_client->send(request, _timeout, "");
          int code_i = check_alarm(resp_http, http_client);

          param_request.append(" error-conn=" +
                               http_client->get_error().message());
          param_request.append(" status-code=" +
                               boost::lexical_cast<std::string>(code_i));

          // If no successful response, nothing
          if (code_i == pion::http::types::RESPONSE_CODE_NO_CONTENT) {
            response_from_iotagent.insert(std::pair<std::string, std::string>(
                all_dest.at(i).endpoint, resp_http->get_content()));
          } else {
            std::string message;
            if (code_i != -1) {
              message = resp_http->get_content();
            } else {
              message = http_client->get_error().message();
            }
            response_from_iotagent_nok.insert(
                std::pair<std::string, std::string>(
                    all_dest.at(i).endpoint, error_manager(code_i, message)));
          }
        } catch (std::exception& e) {
          iota::Alarm::error(iota::types::ALARM_CODE_NO_IOTA,
                             all_dest.at(i).endpoint, iota::types::ERROR,
                             e.what());
        }
      }
    }

    add_errors_to_response(response, response_from_iotagent_nok);
    if (response_from_iotagent.size() == 0) {
      code = pion::http::types::RESPONSE_CODE_NOT_FOUND;
    } else if (!response.empty()) {
      code = pion::http::types::RESPONSE_CODE_OK;
    }
  } else {
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                              error_details,
                              pion::http::types::RESPONSE_CODE_BAD_REQUEST);
  }
  IOTA_LOG_DEBUG(m_log, param_request << " status=" << code
                                      << " reason=" << response);
  http_response.set_status_code(code);
  http_response.set_status_message(
      iota::Configurator::instance()->getHttpMessage(code));
  return code;
}

int iota::AdminManagerService::delete_service_json(
    const boost::shared_ptr<iota::ServiceCollection>& table,
    const std::string& service, const std::string& service_path,
    const std::string& id_service, const std::string& apikey,
    const std::string& resource, bool remove_devices,
    pion::http::response& http_response, std::string& response,
    std::string x_auth_token, std::string request_identifier) {
  std::string param_request("delete_service_json=" + service +
                            " service_path=" + service_path);
  IOTA_LOG_DEBUG(m_log, param_request);
  int code = pion::http::types::RESPONSE_CODE_NO_CONTENT;
  boost::shared_ptr<iota::ServiceMgmtCollection> manager_service_collection(
      new iota::ServiceMgmtCollection());
  iota::ProtocolCollection proto_collection;
  std::map<std::string, std::string> response_from_iotagent;
  std::map<std::string, std::string> response_from_iotagent_nok;
  if (resource.empty()) {
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                              "protocol is madatory",
                              pion::http::types::RESPONSE_CODE_BAD_REQUEST);
  } else {
    std::string protocol_filter = resource;
    IOTA_LOG_DEBUG(m_log, "Search IoTA for " << protocol_filter);
    std::vector<iota::Protocol> protocols =
        proto_collection.get_endpoint_by_protocol(protocol_filter);
    iota::Protocol::resource_endpoint_vector all_dest;
    if (protocols.size() == 0) {
      throw iota::IotaException(
          iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
          "Protocol does not exists [" + protocol_filter + "]",
          iota::types::RESPONSE_CODE_CONTEXT_ELEMENT_NOT_FOUND);
    }

    for (int k = 0; k < protocols.size(); k++) {
      all_dest = protocols[k].get_endpoints();
    }

    // Send to agents
    IOTA_LOG_DEBUG(m_log, "Number of endpoints to deploy service "
                              << all_dest.size());
    for (int i = 0; i < all_dest.size(); i++) {
      try {
        iota::IoTUrl dest(all_dest.at(i).endpoint);

        // Query Parameters
        std::multimap<std::string, std::string> query_parameters;
        // resource is mandatory in iotagents
        query_parameters.insert(std::pair<std::string, std::string>(
            store::types::RESOURCE, all_dest.at(i).resource));

        // Build request
        pion::http::request_ptr request =
            create_request(pion::http::types::REQUEST_METHOD_DELETE,
                           dest.getPath() + iota::ADMIN_SERVICE_SERVICES,
                           iota::make_query_string(query_parameters), "",
                           x_auth_token, request_identifier, service,
                           service_path, dest.getHost(), dest.getPort());

        boost::shared_ptr<iota::HttpClient> http_client(
            new iota::HttpClient(dest.getHost(), dest.getPort()));

        boost::property_tree::ptree additional_info;

        param_request.append(" endpoint=" + http_client->getRemoteEndpoint());
        IOTA_LOG_DEBUG(m_log, "sending delete " + param_request);
        pion::http::response_ptr resp_http =
            http_client->send(request, _timeout, "");
        int code_i = check_alarm(resp_http, http_client);
        IOTA_LOG_DEBUG(m_log, "response delete code:" << code_i);

        param_request.append(" error-conn=" +
                             http_client->get_error().message());
        param_request.append(" status-code=" +
                             boost::lexical_cast<std::string>(code_i));

        // If no successful response, nothing
        if (code_i == pion::http::types::RESPONSE_CODE_NO_CONTENT) {
          response_from_iotagent.insert(std::pair<std::string, std::string>(
              all_dest.at(i).endpoint, resp_http->get_content()));
        } else {
          std::string message;
          if (code_i != -1) {
            message = resp_http->get_content();
          } else {
            message = http_client->get_error().message();
          }
          response_from_iotagent_nok.insert(std::pair<std::string, std::string>(
              all_dest.at(i).endpoint, error_manager(code_i, message)));
        }
      } catch (std::exception& e) {
        iota::Alarm::error(iota::types::ALARM_CODE_NO_IOTA,
                           all_dest.at(i).endpoint, iota::types::ERROR,
                           e.what());
      }
    }  // end for

    add_errors_to_response(response, response_from_iotagent_nok);
    if (response_from_iotagent.size() == 0) {
      code = pion::http::types::RESPONSE_CODE_NOT_FOUND;
    } else if (!response.empty()) {
      code = pion::http::types::RESPONSE_CODE_OK;
    }
  }

  IOTA_LOG_DEBUG(m_log, param_request << " status=" << code
                                      << " reason=" << response);
  http_response.set_status_code(code);
  http_response.set_status_message(
      iota::Configurator::instance()->getHttpMessage(code));
  return code;
}

pion::http::request_ptr iota::AdminManagerService::create_request(
    std::string method, std::string resource, std::string query,
    std::string content, std::string x_auth_token,
    std::string request_identifier, std::string service,
    std::string service_path, std::string host, unsigned short port) {
  pion::http::request_ptr request(new pion::http::request());
  request->set_method(method);
  request->set_resource(resource);
  request->set_query_string(query);
  request->add_header(iota::types::IOT_HTTP_HEADER_ACCEPT,
                      iota::types::IOT_CONTENT_TYPE_JSON);

  if (!x_auth_token.empty()) {
    request->add_header(iota::types::IOT_HTTP_HEADER_AUTH, x_auth_token);
  }
  request->add_header(iota::types::HEADER_TRACE_MESSAGES, request_identifier);
  request->add_header(iota::types::FIWARE_SERVICE, service);
  request->add_header(iota::types::FIWARE_SERVICEPATH, service_path);
  if (!content.empty()) {
    request->set_content(content);
    request->set_content_type(iota::types::IOT_CONTENT_TYPE_JSON);
  }
  std::string server(host);
  server.append(":");
  server.append(boost::lexical_cast<std::string>(port));
  request->change_header(pion::http::types::HEADER_HOST, server);
  return request;
}

int iota::AdminManagerService::check_alarm(
    pion::http::response_ptr& http_resp,
    boost::shared_ptr<iota::HttpClient>& http_client) {
  int code_i = -1;
  if (http_resp.get() != NULL && !http_client->get_error()) {
    code_i = http_resp->get_status_code();
    iota::Alarm::info(iota::types::ALARM_CODE_NO_IOTA,
                      http_client->getRemoteEndpoint(), iota::types::ERROR,
                      http_client->get_error().message());
  } else {
    iota::Alarm::error(iota::types::ALARM_CODE_NO_IOTA,
                       http_client->getRemoteEndpoint(), iota::types::ERROR,
                       http_client->get_error().message());
  }
  return code_i;
}

int iota::AdminManagerService::get_protocols_json(
    int limit, int offset, const std::string& detailed,
    const std::string& resource, pion::http::response& http_response,
    std::string& response) {
  int code = pion::http::types::RESPONSE_CODE_OK;
  std::ostringstream res;
  mongo::BSONObj* fieldsToReturn = NULL;
  mongo::BSONObjBuilder bson_fields;
  std::string param_request("get_protocols_json detailed=" + detailed);
  IOTA_LOG_DEBUG(m_log, param_request);
  Collection table(iota::store::types::PROTOCOL_TABLE);
  mongo::BSONObj elto;

  mongo::BSONObjBuilder bson_sort;
  // se ordena de manera ascendente por nombre device
  bson_sort.append(store::types::PROTOCOL_NAME, 1);

  mongo::BSONObj obj_query;

  int count = table.count(obj_query);
  res << "{ \"count\": " << count << ",";
  res << "\"protocols\": [";

  if (detailed.empty() || (detailed.compare(iota::store::types::OFF) == 0)) {
    bson_fields.append(iota::store::types::PROTOCOL_NAME, 1);
    bson_fields.append(iota::store::types::PROTOCOL_DESCRIPTION, 1);
  } else if (detailed.compare(iota::store::types::ON) != 0) {
    IOTA_LOG_DEBUG(m_log, param_request
                              << " status="
                              << pion::http::types::RESPONSE_CODE_BAD_REQUEST);
    return create_response(pion::http::types::RESPONSE_CODE_BAD_REQUEST,
                           types::RESPONSE_MESSAGE_BAD_REQUEST,
                           "parameter detailed must be on or off",
                           http_response, response);
  }
  table.find(INT_MIN, obj_query, limit, offset, bson_sort.obj(), bson_fields);
  while (table.more()) {
    elto = table.next();
    res << elto.jsonString();
    if (table.more()) {
      res << ",";
    }
  }
  res << "]}";

  return create_response(code, res.str(), "", http_response, response);
}

void iota::AdminManagerService::add_errors_to_response(
    std::string& response, std::map<std::string, std::string>& nok) {
  std::map<std::string, std::string>::iterator it_nok = nok.begin();
  std::string errors;
  if (nok.size() > 0) {
    errors.append("\"" + iota::types::IOT_MANAGER_ERRORS + "\": [");
    while (it_nok != nok.end()) {
      errors.append("{");
      errors.append("\"" + iota::types::IOT_MANAGER_ERRORS_ENDPOINT + "\": \"" +
                    it_nok->first + "\",");
      errors.append(it_nok->second);
      errors.append("}");
      ++it_nok;
      if (it_nok != nok.end()) {
        errors.append(",");
      }
    }
    errors.append("]");
  }
  boost::erase_all(errors, "\n");
  if (response.empty() && !errors.empty()) {
    response.append("{" + errors + "}");
  } else if (!errors.empty()) {
    // Add errors field
    std::size_t pos_prepend = response.find_last_of('}');
    if (pos_prepend != std::string::npos) {
      response.insert(pos_prepend, ",");
    }
    response.insert(++pos_prepend, errors);
  }
}

std::string iota::AdminManagerService::error_manager(int code,
                                                     std::string& message) {
  std::stringstream ss;
  ss << "\"";
  ss << iota::types::IOT_MANAGER_ERRORS_CODE;
  ss << "\": \"";
  ss << code;
  ss << "\",";
  ss << "\"";
  ss << iota::types::IOT_MANAGER_ERRORS_DETAILS;
  ss << "\": ";
  rapidjson::Document document;
  if (document.Parse<0>(message.c_str()).HasParseError()) {
    ss << "\"";
    ss << message;
    ss << "\"";
  } else {
    ss << message;
  }
  return ss.str();
}
