#include "admin_mgmt_service.h"

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


/**
CHECK CAREFULLY!!!!

Typical value:
ip_iotagent: 'http://127.0.0.1:2000'
*/
int iota::AdminManagerService::add_device_iotagent(std::string url_iotagent,
    const std::string& body,std::string service,std::string sub_service) {

  pion::tcp::connection tcp_conn(_scheduler.get_io_service());
  boost::system::error_code error_code;


  int port;
  std::string ip_iotagent;
  std::string response;

  //TODO: split ip_iotagent and extract port.

  error_code = tcp_conn.connect(
                 boost::asio::ip::address::from_string(ip_iotagent), port);

  std::string uri("/iot/devices");
  std::string method("POST");
  std::string content_type("application/json");

  pion::http::request http_request(uri);
  http_request.set_method(method);
  http_request.set_content_type(content_type);

  if (!service.empty()) {
    http_request.add_header(iota::types::FIWARE_SERVICE, service);
  }
  if (!sub_service.empty()) {
    http_request.add_header(iota::types::FIWARE_SERVICEPATH, sub_service);
  }

  http_request.add_header("Accept", "application/json"); //Needed?

  if (!body.empty()) {
    http_request.set_content(body);
  }


  http_request.send(tcp_conn, error_code);
  pion::http::response http_response(http_request);
  http_response.receive(tcp_conn, error_code);
  tcp_conn.close();

  int code_res = http_response.get_status_code();
  response.assign(http_response.get_content());

  return code_res;


}
