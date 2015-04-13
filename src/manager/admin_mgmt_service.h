#ifndef SRC_SERVICES_ADMIN_MGMT_SERVICE_H_
#define SRC_SERVICES_ADMIN_MGMT_SERVICE_H_

#include <string>
#include "util/device.h"
#include <pion/http/plugin_service.hpp>
#include <pion/http/plugin_server.hpp>
#include <pion/http/response_writer.hpp>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <boost/foreach.hpp>
#include <iostream>
#include "util/service_mgmt_collection.h"

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


class AdminManagerService{

  public:

    AdminManagerService();
    virtual ~AdminManagerService();

    /**
    @name add_device_iotagent
    @brief it adds the device_json to the endpoint represented by iotagent_endpoint in a POST request. The result
    is returned in HTTP code.
    */
    int add_device_iotagent(std::string iotagent_endpoint,const std::string& device_json,std::string service, std::string sub_service);

    /**
    @name resolve_endpoints
    @brief Based on the information given in devices_protocols_in, the method will produce as result an array of objects with the
    endpoint where the JSON will be posted. This JSON is the same coming in the original post but linked to the endpoint. The relationship
    is given by what IoTManager knows about  endpoints - protocols - services.
    */
    void resolve_endpoints (std::vector<DeviceToBeAdded>& v_devices_endpoint_out, const std::string& devices_protocols_in,std::string service,std::string sub_service);



  private:
   pion::logger m_log;

   pion::one_to_one_scheduler _scheduler;

   iota::ServiceMgmtCollection _service_mgmt;

};


}// end namespace iota

#endif