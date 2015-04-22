#ifndef SRC_SERVICES_ADMIN_MGMT_SERVICE_H_
#define SRC_SERVICES_ADMIN_MGMT_SERVICE_H_

#include <string>
#include "util/device.h"
#include <pion/http/plugin_service.hpp>
#include <pion/http/plugin_server.hpp>
#include <pion/http/response_writer.hpp>
#include <pion/http/response_reader.hpp>
#include <pion/http/request.hpp>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>	// for stringify JSON
#include <rapidjson/filestream.h>	// wrapper of C stream for prettywriter as output
#include <rapidjson/stringbuffer.h>
#include <boost/foreach.hpp>
#include <iostream>
#include "util/service_mgmt_collection.h"
#include "util/alarm.h"
#include "rest/rest_handle.h"
#include "util/service_collection.h"
#include "admin_service.h"


namespace iota{

 class DeviceToBeAdded{

    public:
      DeviceToBeAdded(std::string device_json, std::string endpoint) : _device_json(device_json) , _endpoint(endpoint)
      {};

    bool operator==(DeviceToBeAdded& a) const {
      if ((_device_json.compare(a._device_json) == 0) &&
          (_endpoint.compare(a._endpoint) == 0)) {
        return true;
      }
      return false;
    }

  DeviceToBeAdded(const DeviceToBeAdded& a){
    _endpoint.assign(a._endpoint);
    _device_json.assign(a._device_json);
  };

  void swap(DeviceToBeAdded& a){
    _endpoint.assign(a.get_endpoint());
    _device_json.assign(a.get_device_json());
  };

  DeviceToBeAdded& operator=(const DeviceToBeAdded& a){
    DeviceToBeAdded tmp(a);
    swap(tmp);
    return *this;
  };

      std::string & get_device_json() {
        return _device_json;
      };

      std::string & get_endpoint() {
        return _endpoint;
      };

    private:
      std::string _device_json;
      std::string _endpoint;
  };


class AdminManagerService : public iota::AdminService {

  public:

    AdminManagerService();
    AdminManagerService(pion::http::plugin_server_ptr web_server);


    virtual ~AdminManagerService();

    /**
    @name add_device_iotagent
    @brief it adds the device_json to the endpoint represented by iotagent_endpoint in a POST request. The result
    is returned in HTTP code.
    */
    int add_device_iotagent(std::string iotagent_endpoint,const std::string& device_json,std::string service, std::string sub_service,std::string x_auth_token);

    /**
    @name resolve_endpoints
    @brief Based on the information given in devices_protocols_in, the method will produce as result an array of objects with the
    endpoint where the JSON will be posted. This JSON is the same coming in the original post but linked to the endpoint. The relationship
    is given by what IoTManager knows about  endpoints - protocols - services.
    */
    void resolve_endpoints (std::vector<DeviceToBeAdded>& v_devices_endpoint_out, const std::string& devices_protocols_in,std::string service,std::string sub_service);

    /**
    @name post_multiple_devices
    @brief This function will issue multiple posts request to add those devices contained in the v_devices_endpoint_in vector. Endpoints where posts should be sent are
    also included in the DeviceToBeAdded object. Extra information is needed like service, sub-service and x-auth-token for respective headers.
    @return JSON with results of all devices.
    */
    std::string post_multiple_devices(std::vector<DeviceToBeAdded>& v_devices_endpoint_in,std::string service,std::string sub_service,std::string x_auth_token);



     /**
    @name get_devices
    @brief it gets devices from iotagents based on enpoints IoTA Manager knows. Devices are filtered by service (Fiware-Service header)
    subservice (Fiware-ServicePath header). A optional query parameter (protocol) may be provided.

    */

    int get_all_devices_json(
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
      std::string protocol_filter
      );



    int get_a_device_json(
      const std::string& service,
      const std::string& service_path,
      const std::string& device_id,
      pion::http::response& http_response,
      std::string& response,
      std::string request_id,
      std::string token,
      std::string protocol_filter
      );



    void set_timeout(unsigned short timeout);

    virtual std::string get_param_resource(
                const std::multimap<std::string, std::string>& query_parameters,
                bool mandatory);

    /**
    @name post_json_devices
    @brief this method is intended to be called as part of AdminService::devices logic. The http_request contains an array of devices with their respective protocols. This
    method will take the service, x-auth-token header and content as inputs and then call to @see resolve_endpoints to get the vector of endpoints where devices will be posted.
    Then a call to @see post_multiple_devices will effectively do the individual posts.
    */
    virtual int post_device_json(
      const std::string& service,
      const std::string& service_path,
      const std::string& body,
      pion::http::response& http_response,
      std::string& response,
      std::string token = "");


  private:
    pion::logger m_log;

    pion::one_to_one_scheduler _scheduler;

    unsigned short _timeout;

    iota::ServiceMgmtCollection _service_mgmt;

    void receive_get_devices(
      std::string request_identifier,
      pion::http::response& http_response_request,
      boost::shared_ptr<iota::HttpClient> connection,
      pion::http::response_ptr response_ptr,
      const boost::system::error_code& error);


};


}// end namespace iota

#endif
