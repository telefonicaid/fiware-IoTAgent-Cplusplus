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

namespace iota{

 class DeviceToBeAdded{

    public:
      DeviceToBeAdded(std::string device_json, std::string protocol) : _device_json(device_json) , _protocol(protocol)
      {};

    bool operator==(DeviceToBeAdded& a) const {
      if ((_device_json.compare(a._device_json) == 0) &&
          (_protocol.compare(a._protocol) == 0)) {
        return true;
      }
      return false;
    }

  DeviceToBeAdded(const DeviceToBeAdded& a){
    DeviceToBeAdded(a._protocol,a._device_json);
  };

  void swap(DeviceToBeAdded& a){
    _protocol.assign(a.get_protocol());
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

      std::string & get_protocol() {
        return _protocol;
      };

    private:
      std::string _device_json;
      std::string _protocol;
  };


class AdminManagerService{

  public:

    AdminManagerService();
    virtual ~AdminManagerService();

    int add_device_iotagent(std::string ip_iotagent,const std::string& device);

    void extract_protocol_and_device(std::vector<std::string> v_protocols_out, std::vector<DeviceToBeAdded> v_devices_out, const std::string& devices_protocol_in);
//TODO: more methods.
  private:
   pion::logger m_log;

   pion::one_to_one_scheduler _scheduler;
};


}// end namespace iota

#endif
