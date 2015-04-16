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

iota::AdminManagerService::AdminManagerService(boost::asio::io_service&
    io_service): _io_service(io_service), _timeout(5), m_log(PION_GET_LOGGER(
            iota::logger)) {

}

iota::AdminManagerService::~AdminManagerService() {

}

void iota::AdminManagerService::set_timeout(unsigned short timeout) {
  _timeout = timeout;
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
    const std::string& body, std::string service, std::string sub_service,
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
    iota::Alarm::error(iota::types::ALARM_CODE_NO_CB, url_iotagent,
                       iota::types::ERROR, e.what());
  }
  // TODO check remove (sync)
  //remove_connection(http_client);
  if (response.get() != NULL) {
    code_res = response->get_status_code();
  }

  return code_res;


}

void iota::AdminManagerService::get_devices(pion::http::request_ptr&
    http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response,
    std::string& response) {



  std::string request_identifier = http_request_ptr->get_header(
                                     iota::types::HEADER_TRACE_MESSAGES);

  iota::ServiceMgmtCollection manager_service_collection;

  // Protocol filter
  std::multimap<std::string, std::string>::iterator it;
  std::string protocol_filter;
  it = query_parameters.find(iota::store::types::PROTOCOL);
  if (it != query_parameters.end()) {
    protocol_filter = it->second;
  }

  std::string log_message("|id-request=" + request_identifier);
  PION_LOG_DEBUG(m_log, log_message);
  // Headers services
  std::string service(http_request_ptr->get_header(
                        iota::types::FIWARE_SERVICE));
  std::string service_path(http_request_ptr->get_header(
                             iota::types::FIWARE_SERVICEPATH));

  // Check mandatory headers
  if (service.empty() || (!service_path.empty() && service_path[0] != '/')) {
    std::string reason(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER);
    std::string error_details(iota::types::FIWARE_SERVICE + "/" +
                              iota::types::FIWARE_SERVICEPATH + " not present or invalid");
    throw iota::IotaException(reason, error_details,
                              iota::types::RESPONSE_CODE_BAD_REQUEST);

  }
  if (service_path.empty()) {
    service_path = iota::types::FIWARE_SERVICEPATH_DEFAULT;
  }

  std::vector<iota::IotagentType> all_dest =
    manager_service_collection.get_iotagents_by_service(service, service_path,
        protocol_filter);

  int i = 0;
  for (i = 0; i < all_dest.size(); i++) {

    iota::IoTUrl dest(all_dest.at(i));
    log_message.append("|endpoint=" +  dest.getHost());
    boost::shared_ptr<iota::HttpClient> http_client(
      new iota::HttpClient(_io_service, dest.getHost(),
                           dest.getPort()));
    boost::property_tree::ptree additional_info;

    // Build request
    pion::http::request_ptr request(new pion::http::request());
    request->set_method(pion::http::types::REQUEST_METHOD_GET);
    request->set_resource(dest.getPath() + iota::ADMIN_SERVICE_DEVICES);
    request->add_header(iota::types::IOT_HTTP_HEADER_ACCEPT,
                        iota::types::IOT_CONTENT_TYPE_JSON);
    std::string server(dest.getHost());
    server.append(":");
    server.append(boost::lexical_cast<std::string>(dest.getPort()));
    request->add_header(pion::http::types::HEADER_HOST, server);
    request->add_header(iota::types::HEADER_TRACE_MESSAGES, request_identifier);

    // Asynch send
    /*
    http_client->async_send(request, _timeout, "",
                            boost::bind(&iota::AdminManagerService::receive_get_devices,
                                        this, request_identifier, boost::ref(http_response), _1, _2, _3));
    */
    pion::http::response_ptr resp_http = http_client->send(request, _timeout, "");
    int code = -1;
    if (resp_http.get() != NULL) {
      code = resp_http->get_status_code();
    }
    log_message.append("|error-conn=" + http_client->get_error().message());
    log_message.append("|status-code=" + boost::lexical_cast<std::string>(code));

    // If no successful response, nothing
    if (code != pion::http::types::RESPONSE_CODE_OK) {
      return;
    }
    PION_LOG_INFO(m_log, log_message);
  }
  //http_request_ptr->add_header(iota::types::HEADER_INTERNAL_TYPE, "true");
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

  for (int i=0;i<v_devices_endpoint_in.size();i++){
    DeviceToBeAdded& dev = v_devices_endpoint_in[i];

    std::string url_endpoint = dev.get_endpoint();
    url_endpoint.append("/iot/");
    url_endpoint.append(iota::ADMIN_SERVICE_DEVICES);

    int res = add_device_iotagent(url_endpoint,dev.get_device_json(),service,sub_service,x_auth_token);

    PION_LOG_DEBUG(m_log,"Endpoint: ["<< url_endpoint<< "] Result: "+boost::lexical_cast<std::string>(res));

    if ( i > 0)
      response.append(",");

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
