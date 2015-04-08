#ifndef SRC_SERVICES_ADMIN_MGMT_SERVICE_H_
#define SRC_SERVICES_ADMIN_MGMT_SERVICE_H_

#include <string>
#include "util/device.h"
#include <pion/http/plugin_service.hpp>
#include <pion/http/plugin_server.hpp>


namespace iota{

class AdminManagerService{

  public:

    AdminManagerService();
    virtual ~AdminManagerService();

    void add_device_iotagent(std::string ip_iotagent,const iota::Device& device);

};


}// end namespace iota

#endif
