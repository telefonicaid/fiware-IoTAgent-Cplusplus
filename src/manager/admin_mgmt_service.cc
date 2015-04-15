#include "admin_mgmt_service.h"
#include "util/iot_url.h"
#include "util/http_client.h"

namespace iota {
extern std::string logger;
}

iota::AdminManagerService::AdminManagerService(): m_log(PION_GET_LOGGER(
        iota::logger)) {

}

iota::AdminManagerService::~AdminManagerService() {

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
      PION_LOG_DEBUG(m_log,"resolve_endpoints: devices found, parsing individually");
      const rapidjson::Value& devices = document["devices"];
      if (!devices.IsArray()) {

          throw std::runtime_error("AdminManagerService: not proper formatted [devices]: not an array");

      }
      PION_LOG_DEBUG(m_log,"resolve_endpoints: size of elements ["<< devices.Size() << "]");
        for (rapidjson::SizeType i = 0; i < devices.Size(); i++) {

          const std::string protocol(devices[i]["protocol"].GetString());
          PION_LOG_DEBUG(m_log,"resolve_endpoints: Processing first Device: protocol ["<< protocol << "]");
          std::vector <IotagentType> v_endpoint = map_endpoints[protocol];

          if (v_endpoint.size()==0){
            PION_LOG_DEBUG(m_log,"resolve_endpoints: getting endpoints for protocol ["<< protocol << "]");
            v_endpoint = _service_mgmt.get_iotagents_by_service(service,sub_service,protocol);
            map_endpoints[protocol] =  v_endpoint;
            PION_LOG_DEBUG(m_log,"resolve_endpoints: endpoints ["<< v_endpoint.size() <<"] found for device");
          }

          //Now link endpoints with device.
          for (int j = 0; j < v_endpoint.size(); j++){

	          rapidjson::StringBuffer string_buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(string_buffer);
            devices[i].Accept(writer);

            iota::DeviceToBeAdded dev_add(string_buffer.GetString(),v_endpoint[j]);
            v_devices_endpoint_out.push_back(dev_add);
            PION_LOG_DEBUG(m_log,"resolve_endpoints: adding endpoint ["<< v_endpoint[j] << "] for device [" << string_buffer.GetString() << "]");
          }

        }
  }
  else {
    throw std::runtime_error("AmdinManagerService: not proper formatted [devices] : not found");
  }


}


int iota::AdminManagerService::add_device_iotagent(std::string url_iotagent,
    const std::string& body,std::string service,std::string sub_service,std::string x_auth_token) {

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
  request->set_content(body);
  request->set_content_type(iota::types::IOT_CONTENT_TYPE_JSON);


  request->add_header(iota::types::IOT_HTTP_HEADER_ACCEPT,
                        iota::types::IOT_CONTENT_TYPE_JSON);
  request->add_header(pion::http::types::HEADER_HOST, server);


  request->add_header(iota::types::FIWARE_SERVICE, service);
  request->add_header(iota::types::FIWARE_SERVICEPATH, sub_service);

  if (!x_auth_token.empty()) {
    request->add_header(iota::types::IOT_HTTP_HEADER_AUTH,x_auth_token);
  }

  http_client.reset(new iota::HttpClient(server, dest.getPort()));

  //TODO: set timeout and proxy well.
  response = http_client->send(request, 10, "");

  }
  catch (std::exception& e) {
    PION_LOG_ERROR(m_log, e.what());
    iota::Alarm::error(iota::types::ALARM_CODE_NO_CB, url_iotagent,
                       iota::types::ERROR, e.what());
  }
  // TODO check remove (sync)
  //remove_connection(http_client);
  if (response.get() != NULL){
    code_res = response->get_status_code();
  }

  return code_res;


}
