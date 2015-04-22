#include "admin_mgmt_service.h"
#include "util/iot_url.h"
#include "util/http_client.h"
#include "rest/rest_handle.h"
#include "util/iot_url.h"
#include "util/iota_exception.h"
#include <boost/property_tree/ptree.hpp>

namespace iota {
extern std::string logger;
}

iota::AdminManagerService::AdminManagerService(pion::http::plugin_server_ptr web_server):
            _timeout(5),
            m_log(PION_GET_LOGGER(
            iota::logger)),
            _class_name("iota::AdminManagerService")
             {

}

iota::AdminManagerService::AdminManagerService() :
  _class_name("iota::AdminManagerService"), _timeout(5),
  m_log(PION_GET_LOGGER(iota::logger)) {
}

iota::AdminManagerService::~AdminManagerService() {

}

void iota::AdminManagerService::set_timeout(unsigned short timeout) {
  _timeout = timeout;
}

void iota::AdminManagerService::start(){

  std::map<std::string, std::string> filters;


  add_common_urls(filters);

    // Manager manages protocols
  add_url(ADMIN_SERVICE_PROTOCOLS, filters,
            REST_HANDLE(&iota::AdminManagerService::protocols), this);

  add_oauth_media_filters();
  check_for_logs();

}

void iota::AdminManagerService::create_collection( boost::shared_ptr<iota::ServiceCollection>& col){
  col.reset(new ServiceMgmtCollection());
}

void iota::AdminManagerService::resolve_endpoints(std::vector<DeviceToBeAdded>&
    v_devices_endpoint_out, const std::string& devices_protocols_in,
    std::string service, std::string sub_service) {

  /*
   devices_protocol_in: it brings the whole list of devices with their protocol.
   "devices": [
  { "protocol": <identifier>, "device_id": "device_id" <other fields in IoTAgent API> },
  */
  rapidjson::Document document;
  char buffer[devices_protocols_in.length()];
  //memcpy(buffer, str_attribute.c_str(), str_attribute.length());
  strcpy(buffer, devices_protocols_in.c_str());
  if (document.ParseInsitu<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "AdminManagerService: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());

  }

  std::map <std::string, std::vector<IotagentType> > map_endpoints;

  if (document.HasMember("devices")) {
    PION_LOG_DEBUG(m_log, "resolve_endpoints: devices found, parsing individually");
    const rapidjson::Value& devices = document["devices"];
    if (!devices.IsArray()) {

      throw std::runtime_error("AdminManagerService: not proper formatted [devices]: not an array");

    }
    PION_LOG_DEBUG(m_log, "resolve_endpoints: size of elements [" << devices.Size()
                   <<
                   "]");
    for (rapidjson::SizeType i = 0; i < devices.Size(); i++) {

      const std::string protocol(devices[i]["protocol"].GetString());
      PION_LOG_DEBUG(m_log, "resolve_endpoints: Processing first Device: protocol ["
                     <<
                     protocol << "]");
      std::vector <IotagentType> v_endpoint = map_endpoints[protocol];

      if (v_endpoint.size() == 0) {
        PION_LOG_DEBUG(m_log, "resolve_endpoints: getting endpoints for protocol [" <<
                       protocol << "]");
        v_endpoint = _service_mgmt.get_iotagents_by_service(service, sub_service,
                     protocol);
        map_endpoints[protocol] =  v_endpoint;
        PION_LOG_DEBUG(m_log, "resolve_endpoints: endpoints [" << v_endpoint.size()
                       << "] found for device");
      }

      //Now link endpoints with device.
      for (int j = 0; j < v_endpoint.size(); j++) {

        rapidjson::StringBuffer string_buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(string_buffer);
        devices[i].Accept(writer);

        iota::DeviceToBeAdded dev_add(string_buffer.GetString(), v_endpoint[j]);
        v_devices_endpoint_out.push_back(dev_add);
        PION_LOG_DEBUG(m_log, "resolve_endpoints: adding endpoint [" << v_endpoint[j] <<
                       "] for device [" << string_buffer.GetString() << "]");
      }

    }
  }
  else {
    throw std::runtime_error("AmdinManagerService: not proper formatted [devices] : not found");
  }


}


int iota::AdminManagerService::add_device_iotagent(std::string url_iotagent,
    const std::string& body,std::string service,std::string sub_service,
    std::string x_auth_token) {

  boost::shared_ptr<iota::HttpClient> http_client;
  pion::http::response_ptr response;
  // IoTAgent trust token
  int code_res = 404;
  try {
    iota::IoTUrl                 dest(url_iotagent);
    std::string resource = dest.getPath();
    std::string query    = dest.getQuery();
    std::string server   = dest.getHost();
    std::string compound_server(server);

    //iot/device

    compound_server.append(":");
    compound_server.append(boost::lexical_cast<std::string>(dest.getPort()));
    PION_LOG_DEBUG(m_log, "Server " << server);


    pion::http::request_ptr request(new pion::http::request());
    request->set_method(pion::http::types::REQUEST_METHOD_POST);
    request->set_resource(resource);

    std::string content("{\"devices\":[");
    content.append(body);
    content.append("]}");

    request->set_content(content);
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


    response = http_client->send(request, _timeout, "");
  }
  catch (std::exception& e) {
    PION_LOG_ERROR(m_log, e.what());
    iota::Alarm::error(iota::types::ALARM_CODE_NO_IOTA, url_iotagent,
                       iota::types::ERROR, e.what());
  }
  // TODO check remove (sync)
  //remove_connection(http_client);
  if (response.get() != NULL) {
    code_res = response->get_status_code();
    PION_LOG_DEBUG(m_log,"Response: CODE: "<<code_res);
  }
  return code_res;


}

int iota::AdminManagerService::get_all_devices_json(
      const std::string& service,
      const std::string& service_path,
      int limit,
      int offset,
      const std::string& detailed,
      const std::string& entity,
      pion::http::response& http_response,
      std::string& response,
      std::string request_identifier,
      std::string x_auth_token,
      std::string protocol_filter) {


  /*std::string request_identifier = http_request_ptr.get()->get_header(
                                     iota::types::HEADER_TRACE_MESSAGES);
*/
  iota::ServiceMgmtCollection manager_service_collection;

  PION_LOG_DEBUG(m_log,"AdminManagerService: get_all_devices_json, starting...");

  std::string log_message("|id-request=" + request_identifier);
  PION_LOG_DEBUG(m_log, log_message);

  std::vector<iota::IotagentType> all_dest =
    manager_service_collection.get_iotagents_by_service(service, service_path,
        protocol_filter);

  std::map<std::string, std::string> response_from_iotagent;

  /*std::string x_auth_token = http_request_ptr.get()->get_header(
                               iota::types::IOT_HTTP_HEADER_AUTH);
*/


  for (int i = 0; i < all_dest.size(); i++) {

    iota::IoTUrl dest(all_dest.at(i));
    // Build request
    pion::http::request_ptr request(new pion::http::request());
    request->set_method(pion::http::types::REQUEST_METHOD_GET);
    request->set_resource(dest.getPath() + iota::ADMIN_SERVICE_DEVICES);

    // Query parameters
    request->add_query(iota::store::types::PROTOCOL, protocol_filter);
    request->add_query(iota::store::types::LIMIT,
                       boost::lexical_cast<std::string>(limit));
    request->add_query(iota::store::types::OFFSET,
                       boost::lexical_cast<std::string>(offset));
    request->add_query(iota::store::types::ENTITY, entity);
    request->add_query(iota::store::types::DETAILED, detailed);

    request->add_header(iota::types::IOT_HTTP_HEADER_ACCEPT,
                        iota::types::IOT_CONTENT_TYPE_JSON);

    if (!x_auth_token.empty()) {
      request->add_header(iota::types::IOT_HTTP_HEADER_AUTH, x_auth_token);
    }
    request->add_header(iota::types::HEADER_TRACE_MESSAGES, request_identifier);
    request->add_header(iota::types::FIWARE_SERVICE, service);
    request->add_header(iota::types::FIWARE_SERVICEPATH, service_path);

    std::string server(dest.getHost());
    server.append(":");
    server.append(boost::lexical_cast<std::string>(dest.getPort()));
    request->change_header(pion::http::types::HEADER_HOST, server);
    boost::shared_ptr<iota::HttpClient> http_client(
      new iota::HttpClient(dest.getHost(),
                           dest.getPort()));

    boost::property_tree::ptree additional_info;


    log_message.append("|endpoint=" + http_client->getRemoteEndpoint());
    // Asynch send
    /*
    http_client->async_send(request, _timeout, "",
                            boost::bind(&iota::AdminManagerService::receive_get_devices,
                                        this, request_identifier, boost::ref(http_response), _1, _2, _3));
    */
    pion::http::response_ptr resp_http = http_client->send(request, _timeout, "");
    int code = -1;

    if (resp_http.get() != NULL && !http_client->get_error()) {
      code = resp_http->get_status_code();
      iota::Alarm::info(iota::types::ALARM_CODE_NO_IOTA, all_dest.at(i),
                        iota::types::ERROR, http_client->get_error().message());
    }
    else {
      iota::Alarm::error(iota::types::ALARM_CODE_NO_IOTA, all_dest.at(i),
                         iota::types::ERROR, http_client->get_error().message());
    }

    log_message.append("|error-conn=" + http_client->get_error().message());
    log_message.append("|status-code=" + boost::lexical_cast<std::string>(code));

    // If no successful response, nothing
    if (code == pion::http::types::RESPONSE_CODE_OK) {
      response_from_iotagent.insert(std::pair<std::string, std::string>(all_dest.at(
                                      i), resp_http->get_content()));
    }
    PION_LOG_INFO(m_log, log_message);
  }

  http_response.add_header(pion::http::types::HEADER_CONTENT_TYPE,
                           iota::types::IOT_CONTENT_TYPE_JSON);
  http_response.set_status_code(pion::http::types::RESPONSE_CODE_OK);
  http_response.set_status_message(iota::Configurator::instance()->getHttpMessage(
                                     pion::http::types::RESPONSE_CODE_OK));

  int total_count = 0;
  mongo::BSONObjBuilder builder_json;
  mongo::BSONArrayBuilder builder_array;
  std::map<std::string, std::string>::iterator it_resp =
    response_from_iotagent.begin();
  while (it_resp != response_from_iotagent.end()) {
    try {

      mongo::BSONObj obj_mongo = mongo::fromjson(it_resp->second);
      int tmp_count = obj_mongo.getIntField("count");
      total_count += tmp_count;
      std::vector<mongo::BSONElement> devices = obj_mongo.getField(
            iota::store::types::DEVICES).Array();
      for (int j = 0; j < devices.size(); j++) {
        builder_array.append(devices.at(j));
      }

    }
    catch (mongo::MsgAssertionException& e) {
      log_message.append("|endpoint=" + it_resp->first + "|error-json=" + std::string(
                           e.what()));
      PION_LOG_ERROR(m_log, log_message);
    }
    catch (std::exception& e) {
      log_message.append("|endpoint=" + it_resp->first + "|error=" + std::string(
                           e.what()));
      PION_LOG_ERROR(m_log, log_message);
    }
    ++it_resp;
  }
  builder_json.append("count", total_count);
  builder_json.appendArray(iota::store::types::DEVICES, builder_array.obj());
  mongo::BSONObj result = builder_json.obj();
  response = result.jsonString();
  PION_LOG_DEBUG(m_log, log_message + "|content=" + response);
  return pion::http::types::RESPONSE_CODE_OK;
}

int iota::AdminManagerService::get_a_device_json(
  const std::string& service,
  const std::string& service_path,
  const std::string& device_id,
  pion::http::response& http_response,
  std::string& response,
  std::string request_identifier,
  std::string x_auth_token,
  std::string protocol_filter) {


  int code = pion::http::types::RESPONSE_CODE_NOT_FOUND;
  /*std::string request_identifier = http_request_ptr.get()->get_header(
                                     iota::types::HEADER_TRACE_MESSAGES);
*/
  iota::ServiceMgmtCollection manager_service_collection;

  PION_LOG_DEBUG(m_log,"AdminManagerService: get_a_device_json, starting...");

  std::vector<iota::IotagentType> all_dest =
    manager_service_collection.get_iotagents_by_service(service, service_path,
        protocol_filter);

  std::map<std::string, std::string> response_from_iotagent;
  /*std::string x_auth_token = http_request_ptr.get()->get_header(
                               iota::types::IOT_HTTP_HEADER_AUTH);
*/
   std::string log_message;
  for (int i = 0; i < all_dest.size(); i++) {
    log_message.clear();
    log_message.append("|id-request=" + request_identifier);
    iota::IoTUrl dest(all_dest.at(i));
    // Build request
    pion::http::request_ptr request(new pion::http::request());
    request->set_method(pion::http::types::REQUEST_METHOD_GET);
    request->set_resource(dest.getPath() + iota::ADMIN_SERVICE_DEVICES + "/" +
                          device_id);

    request->add_header(iota::types::IOT_HTTP_HEADER_ACCEPT,
                        iota::types::IOT_CONTENT_TYPE_JSON);

    if (!x_auth_token.empty()) {
      request->add_header(iota::types::IOT_HTTP_HEADER_AUTH, x_auth_token);
    }
    request->add_header(iota::types::HEADER_TRACE_MESSAGES, request_identifier);
    request->add_header(iota::types::FIWARE_SERVICE, service);
    request->add_header(iota::types::FIWARE_SERVICEPATH, service_path);

    std::string server(dest.getHost());
    server.append(":");
    server.append(boost::lexical_cast<std::string>(dest.getPort()));
    request->change_header(pion::http::types::HEADER_HOST, server);
    boost::shared_ptr<iota::HttpClient> http_client(
      new iota::HttpClient(dest.getHost(),
                           dest.getPort()));

    boost::property_tree::ptree additional_info;


    log_message.append("|endpoint=" + http_client->getRemoteEndpoint());
    pion::http::response_ptr resp_http = http_client->send(request, _timeout, "");

    if (resp_http.get() != NULL && !http_client->get_error()) {
      code = resp_http->get_status_code();
      iota::Alarm::info(iota::types::ALARM_CODE_NO_IOTA, all_dest.at(i),
                        iota::types::ERROR, http_client->get_error().message());
    }
    else {
      iota::Alarm::error(iota::types::ALARM_CODE_NO_IOTA, all_dest.at(i),
                         iota::types::ERROR, http_client->get_error().message());
    }

    log_message.append("|error-conn=" + http_client->get_error().message());
    log_message.append("|status-code=" + boost::lexical_cast<std::string>(code));
    log_message.append("|content=" + std::string(resp_http->get_content()));
    // If no successful response, nothing
    if (code == pion::http::types::RESPONSE_CODE_OK) {
      response_from_iotagent.insert(std::pair<std::string, std::string>(all_dest.at(
                                      i), resp_http->get_content()));
    }
    PION_LOG_INFO(m_log, log_message);
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
        mongo::BSONObj obj_mongo = mongo::fromjson(it_resp->second);
        total_count++;
        builder_array.append(obj_mongo);
      }
    }
    catch (mongo::MsgAssertionException& e) {
      log_message.append("|endpoint=" + it_resp->first + "|error-json=" + std::string(
                           e.what()));
      PION_LOG_ERROR(m_log, log_message);
    }
    catch (std::exception& e) {
      log_message.append("|endpoint=" + it_resp->first + "|error=" + std::string(
                           e.what()));
      PION_LOG_ERROR(m_log, log_message);
    }
    ++it_resp;
  }
  builder_json.append("count", total_count);
  builder_json.appendArray(iota::store::types::DEVICES, builder_array.obj());
  mongo::BSONObj result = builder_json.obj();
  response = result.jsonString();
  PION_LOG_DEBUG(m_log, log_message + "|content=" + response);
  return pion::http::types::RESPONSE_CODE_OK;

}

void iota::AdminManagerService::receive_get_devices(
  std::string request_identifier,
  pion::http::response& http_response_request,
  boost::shared_ptr<iota::HttpClient> connection,
  pion::http::response_ptr response_ptr,
  const boost::system::error_code& error) {
  std::string log_message("|id-request=" + request_identifier + "|endpoint=" +
                          connection->getRemoteEndpoint() + "|error-conn=" + error.message());

  int code = -1;
  if (response_ptr.get() != NULL) {
    code = response_ptr->get_status_code();
  }
  log_message.append("|status-code=" + boost::lexical_cast<std::string>(code));
  PION_LOG_INFO(m_log, log_message);
  // If no successful response, nothing
  if (code != pion::http::types::RESPONSE_CODE_OK) {
    return;
  }

}


std::string iota::AdminManagerService::post_multiple_devices(
  std::vector<DeviceToBeAdded>& v_devices_endpoint_in, std::string service,
  std::string sub_service, std::string x_auth_token) {

  std::string response("{");

  std::string log_message("|service=" + service+"|sub_service="+sub_service);
  PION_LOG_DEBUG(m_log, log_message);

  response.append("\"responses\":[");

  for (int i=0; i<v_devices_endpoint_in.size(); i++) {
    DeviceToBeAdded& dev = v_devices_endpoint_in[i];

    std::string url_endpoint = dev.get_endpoint();
    url_endpoint.append("/iot");
    url_endpoint.append(iota::ADMIN_SERVICE_DEVICES);

    int res = add_device_iotagent(url_endpoint,dev.get_device_json(),service,
                                  sub_service,x_auth_token);

    PION_LOG_DEBUG(m_log,"Endpoint: ["<< url_endpoint<< "] Result: "
                   +boost::lexical_cast<std::string>(res));

    if (i > 0) {
      response.append(",");
    }

    response.append("{\"device\":[");
    response.append(dev.get_device_json());
    response.append("],");
    response.append("\"endpoint\":\"");
    response.append(url_endpoint);
    response.append("\",");
    response.append("\"result\":\"");
    response.append(boost::lexical_cast<std::string>(res));
    response.append("\"}");


  }

  response.append("]}");
  PION_LOG_DEBUG(m_log,"Final response: "+response);
  return response;

}


int iota::AdminManagerService::post_device_json(
      const std::string& service,
      const std::string& service_path,
      const std::string& body,
      pion::http::response& http_response,
      std::string& response,
      std::string token ) {

  PION_LOG_DEBUG(m_log,"AdminManagerService: post_devices_json: validating input");

  if (service.empty() || service_path.empty()) {
    PION_LOG_ERROR(m_log,"");
    http_response.set_status_code(400);
  }

  std::vector<iota::DeviceToBeAdded> v_endpoints;

  resolve_endpoints(v_endpoints,body,service,service_path);
  PION_LOG_DEBUG(m_log,
                 "do_post_json_devices: endpoints found ["<<v_endpoints.size()<<"]");
  response.assign(post_multiple_devices(v_endpoints,service,service_path,
                                        token));
  PION_LOG_DEBUG(m_log,"do_post_json_devices: POSTs processed: ["<<response<<"]");
  if (!response.empty()) {
    http_response.set_content(response);
    http_response.set_status_code(200);
  }
  else {
    http_response.set_status_code(404);
  }

  return http_response.get_status_code();

}

std::string iota::AdminManagerService::get_role() {
  return " [working as manager] ";
}

std::string iota::AdminManagerService::get_class_name(){
  return _class_name;
}


std::string iota::AdminManagerService::get_param_resource(
            const std::multimap<std::string, std::string>& query_parameters,
            bool mandatory)
{
  std::string result;
  std::multimap<std::string,std::string>::const_iterator it;

  it = query_parameters.find(iota::store::types::PROTOCOL);
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


void iota::AdminManagerService::protocols(pion::http::request_ptr& http_request_ptr,
                                   std::map<std::string, std::string>& url_args,
                                   std::multimap<std::string, std::string>& query_parameters,
                                   pion::http::response& http_response,
                                   std::string& response) {

  // TODO protocols,  POST y PUT don't need authentication (it is a register)
  std::string trace_message = http_request_ptr->get_header(
                                iota::types::HEADER_TRACE_MESSAGES);
  std::string method = http_request_ptr->get_method();
  PION_LOG_INFO(m_log, "|protocols|method:" +method +
                "|trace_message:" + trace_message);

  std::string reason;
  std::string error_details;
  int code = pion::http::types::RESPONSE_CODE_OK;
  std::string service_in_url;
  std::string temp, op;
  try {
    check_mongo_config();
    std::string content_type(http_request_ptr->get_header(
                               pion::http::types::HEADER_CONTENT_TYPE));
    std::string service(http_request_ptr->get_header(
                          iota::types::FIWARE_SERVICE));
    std::string service_path(http_request_ptr->get_header(
                               iota::types::FIWARE_SERVICEPATH));


    if (method.compare(pion::http::types::REQUEST_METHOD_POST) == 0) {
      std::string content = http_request_ptr->get_content();
      boost::trim(content);
      code = post_protocol_json(service,  service_path,
                                content, http_response, response);
    }
    else if (method.compare(pion::http::types::REQUEST_METHOD_GET) == 0) {

      std::string detailed, protocol;
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
        if (limit < 0) {
          PION_LOG_ERROR(m_log, " bad limit using default");
          limit = types::LIMIT_DEFAULT;
        }
        else if (limit > types::LIMIT_MAX) {
          PION_LOG_ERROR(m_log, " bad limit using maximun");
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
      it = query_parameters.find(iota::store::types::RESOURCE);
      if (it != query_parameters.end()) {
        protocol = it->second;
      }

      code = get_protocols_json(limit,
                               offset, "on", protocol, http_response, response);

    }
    else {
      code = pion::http::types::RESPONSE_CODE_NOT_FOUND;
      create_response(code, reason, error_details, http_response, response);
    }

  }
  catch (const boost::bad_lexical_cast& e) {
    PION_LOG_ERROR(m_log,"Capturada boost::bad_lexical_cast en services");
    PION_LOG_ERROR(m_log,e.what());
    reason.assign(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER);
    error_details.assign(op + " must be a number but it is " + temp);
    code = 400;
    create_response(code, reason, error_details, http_response, response);
  }
  catch (iota::IotaException& e) {
    PION_LOG_ERROR(m_log,"Capturada: Exception en services");
    PION_LOG_ERROR(m_log,e.what());
    reason.assign(e.reason());
    error_details.assign(e.what());
    code = e.status();
    create_response(code, reason, error_details, http_response, response);
  }
  catch (std::exception& e) {
    PION_LOG_ERROR(m_log,"Excepcion en services");
    reason.assign(iota::types::RESPONSE_MESSAGE_INTERNAL_ERROR);
    error_details.assign(e.what());
    code = pion::http::types::RESPONSE_CODE_SERVER_ERROR;
    create_response(code, reason, error_details, http_response, response);
  }

  PION_LOG_INFO(m_log, "|method:" +method +
                "|trace_message:" + trace_message+
                "|code: " + boost::lexical_cast<std::string>(code)+
                "|response:" + response);
}

