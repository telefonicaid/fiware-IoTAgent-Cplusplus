#include "admin_mgmt_service.h"

namespace iota {
extern std::string logger;
}

iota::AdminManagerService::AdminManagerService(): m_log(PION_GET_LOGGER(iota::logger)){

}

iota::AdminManagerService::~AdminManagerService(){

}

void iota::AdminManagerService::extract_protocol_and_device(std::vector<std::string> v_protocols_out, std::vector<DeviceToBeAdded> v_devices_out,const std::string& device_protocol_in){

  rapidjson::Document document;
  char buffer[device_protocol_in.length()];
  //memcpy(buffer, str_attribute.c_str(), str_attribute.length());
  strcpy(buffer, device_protocol_in.c_str());
  if (document.ParseInsitu<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "ContextElement: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());

  }



}

/**
CHECK CAREFULLY!!!!

Typical value:
ip_iotagent: 'http://127.0.0.1:2000'
*/
int iota::AdminManagerService::add_device_iotagent(std::string url_iotagent,const std::string& body){

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

  std::string service("");
  std::string service_path("");


  pion::http::request http_request(uri);
  http_request.set_method(method);
  http_request.set_content_type(content_type);

  if (!service.empty()) {
    http_request.add_header(iota::types::FIWARE_SERVICE, service);
  }
  if (!service_path.empty()) {
    http_request.add_header(iota::types::FIWARE_SERVICEPATH, service_path);
  }

  http_request.add_header("Accept", "application/json"); //Needed?

 /* if (!query_string.empty()) {
    http_request.set_query_string(query_string);
  }



  std::map<std::string, std::string>::const_iterator iter;
  for (iter = headers.begin(); iter != headers.end(); ++iter) {
    std::cout << "header: " << iter->first << iter->second << std::endl;
    http_request.add_header(iter->first, iter->second);
  }
*/



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
