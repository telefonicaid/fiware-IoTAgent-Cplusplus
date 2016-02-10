/**
* Copyright 2015 Telefonica Investigación y Desarrollo, S.A.U
*
* This file is part of iotagent project.
*
* iotagent is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published
* by the Free Software Foundation, either version 3 of the License,
* or (at your option) any later version.
*
* iotagent is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with iotagent. If not, see http://www.gnu.org/licenses/.
*
* For those usages not covered by the GNU Affero General Public License
* please contact with iot_support at tid dot es
*/

#include "rest/process.h"
#include "command_handle.h"

#include <algorithm>
#include <string>

#include <ngsi/ContextElement.h>
#include <ngsi/UpdateContext.h>
#include "rest/iot_cb_comm.h"
#include "util/RiotISO8601.h"
#include "util/service.h"
#include "rest/types.h"
#include "util/iota_exception.h"
#include "util/alarm.h"
#include "boost/format.hpp"
#include <boost/uuid/uuid.hpp>             // uuid class
#include <boost/uuid/uuid_generators.hpp>  // generators
#include <boost/uuid/uuid_io.hpp>          // streaming operators etc.
#include <boost/algorithm/string.hpp>

#include "ngsi/ContextRegistration.h"
#include "ngsi/RegisterContext.h"
#include "ngsi/RegisterResponse.h"

#include "util/service_collection.h"
#include "util/store_const.h"
#include "util/command_collection.h"

#include "services/admin_service.h"

#include "util/dev_file.h"
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "util/device_collection.h"

extern "C" void* _begin_registrations(void* arg) {
  iota::CommandHandle* usrv = (iota::CommandHandle*)arg;
  usrv->make_registrations();
}

boost::shared_ptr<iota::Command> command_from_mongo(
    boost::shared_ptr<iota::Command> item) {
  boost::shared_ptr<iota::Command> resu;

  try {
    iota::CommandCollection table;
    table.find(*item);

    if (table.more()) {
      iota::Command aux = table.next();
      resu.reset(new iota::Command(aux));
    }
  } catch (...) {
    // ERROR
  }
  return resu;
}

iota::CommandHandle::CommandHandle()
    : m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())),
      m_asyncCommands(iota::types::MAX_SIZE_CACHE, false) {
  IOTA_LOG_DEBUG(m_logger, "iota::CommandHandle::CommandHandle");
  m_asyncCommands.set_timeout_function(
      boost::bind(&iota::CommandHandle::timeout_f, this, _1));

  // ctor
  _reg_timeout = DEFAULT_REG_TIMEOUT;
  _myProvidingApp = UNKOWN_PROVIDING_APP;
  _callback = NULL;

  try {
    const iota::JsonValue& storage =
        iota::Configurator::instance()->get(iota::store::types::STORAGE);
    if (storage.HasMember(iota::store::types::TYPE.c_str())) {
      _storage_type.assign(
          storage[iota::store::types::TYPE.c_str()].GetString());
      IOTA_LOG_INFO(m_logger, "type_store:" << _storage_type);
      if (_storage_type.compare(iota::store::types::MONGODB) == 0) {
        IOTA_LOG_DEBUG(m_logger,
                       "Setting function get in cache to find in mongo");
        m_asyncCommands.set_function(boost::bind(command_from_mongo, _1));
        m_asyncCommands.set_entity_function(
            boost::bind(command_from_mongo, _1));
        m_asyncCommands.set_id_function(boost::bind(command_from_mongo, _1));
      }
    } else {
      IOTA_LOG_ERROR(m_logger, "Config file has not got storage");
    }
  } catch (...) {
    IOTA_LOG_DEBUG(m_logger, " Problem with devices config file");
  }
}

iota::CommandHandle::~CommandHandle() {
  // dtor
}

void iota::CommandHandle::set_async_commands() { _callback = true; }

void iota::CommandHandle::handle_updateContext(const std::string& url,
                                               std::string response,
                                               int status) {
}

boost::shared_ptr<iota::Command> iota::CommandHandle::timeout_f(
    boost::shared_ptr<Command> item) {
  boost::property_tree::ptree service_ptree;

  IOTA_LOG_INFO(m_logger, "timeout command: entity:"
                              << item->get_entity()
                              << " service:" << item->get_service()
                              << " service_path:" << item->get_service_path()
                              << " command_id" << item->get_id());
  try {
    int status = item->get_status();
    std::string statusSTR;
    if (iota::types::READY_FOR_READ == status) {
      statusSTR = iota::types::EXPIRED_READ_MESSAGE;
    } else if (iota::types::DELIVERED == status) {
      statusSTR = iota::types::EXPIRED_DELIVERY_MESSAGE;
    } else {
      statusSTR = iota::types::ERROR;
    }
    boost::shared_ptr<Device> dev(new Device("", item->get_service()));
    dev->_entity_type = item->get_entity_type();
    dev->_entity_name = item->get_entity();

    get_service_by_name(service_ptree, item->get_service(),
                        item->get_service_path());

    //for HA, it is necessary to check if the new state is bigger
    // then don't do anything, because the other iotagent has done things
    iota::CommandPtr itemBBDD= get_command_from_mongo(item->get_id(), item->get_service(),
                           item->get_service_path());
    IOTA_LOG_DEBUG(m_logger, "coompare status" << status << "  <  " << itemBBDD->get_status());
    if (itemBBDD.get() == NULL || status < itemBBDD->get_status()) {
        IOTA_LOG_INFO(m_logger, "HA, CommandHandle::timeout_f refused because command status is bigger, other iotagent has incremented");
    }else{
        // look for command in cache
        int c = remove_command(item->get_id(), item->get_service(),
                               item->get_service_path());

        if (c > 0) {
          // ha saltado algun timeout, hay que enviar el nuevo estado del comando
          send_updateContext(item->get_name(), iota::types::STATUS,
                             iota::types::STATUS_TYPE, statusSTR, dev,
                             service_ptree, iota::types::STATUS_OP);
        } else {
          IOTA_LOG_ERROR(
              m_logger,
              "timeout command:command_id"
                  << item->get_id()
                  << " timeout but no command in cache, no sended data to CB");
        }
    }
  } catch (iota::IotaException& e) {
    IOTA_LOG_ERROR(m_logger, "IotaException in timeout function: " << e.what());
  } catch (...) {
    IOTA_LOG_ERROR(m_logger, "Other Exception in timeout function");
  }

  return item;
}

int iota::CommandHandle::get_duration_seconds(std::string data) {
  int result = DEFAULT_REG_TIMEOUT;
  std::string saux;
  std::string sresu;
  int tseg = 3600;
  size_t pos = std::string::npos;

  if ((pos = data.find("PT")) != std::string::npos) {
    // daypart duration example  PT24H   (S, M, H)
    saux = data.substr(pos + 2);

    pos = std::string::npos;

    if ((pos = saux.find("H")) != std::string::npos) {
      sresu = saux.substr(0, pos);
      tseg = 3600;
    } else if ((pos = saux.find("M")) != std::string::npos) {
      sresu = saux.substr(0, pos);
      tseg = 60;
    } else if ((pos = saux.find("S")) != std::string::npos) {
      sresu = saux.substr(0, pos);
      tseg = 1;
    }
  } else if ((pos = data.find("P")) != std::string::npos) {
    // No daypart duration example  P1Y  (D, W, M, Y)
    saux = data.substr(pos + 1);

    pos = std::string::npos;

    if ((pos = saux.find("Y")) != std::string::npos) {
      sresu = saux.substr(0, pos);
      tseg = 365 * 24 * 3600;
    } else if ((pos = saux.find("M")) != std::string::npos) {
      sresu = saux.substr(0, pos);
      tseg = 30 * 24 * 3600;
    } else if ((pos = saux.find("W")) != std::string::npos) {
      sresu = saux.substr(0, pos);
      tseg = 7 * 24 * 3600;
    } else if ((pos = saux.find("D")) != std::string::npos) {
      sresu = saux.substr(0, pos);
      tseg = 24 * 3600;
    }
  }

  if (!sresu.empty()) {
    IOTA_LOG_DEBUG(m_logger, "sresu:  " << sresu);
    int naux = atoi(sresu.c_str());
    if (naux > 0) {
      result = naux * tseg;
    }
  }

  return result;
}

void iota::CommandHandle::init_services_by_resource() {
  try {
    IOTA_LOG_INFO(m_logger, "Init vector of services names by resource: "
                                << get_resource());
    const JsonValue& res =
        iota::Configurator::instance()->getResourceObject(get_resource());

    if (res.IsObject() && res.HasMember("services")) {
      const JsonValue& services = res["services"];
      if (services.IsArray()) {
        for (rapidjson::SizeType i = 0; i < services.Size(); i++) {
          std::string service = services[i]["service"].GetString();
          _services_names.push_back(service);
          IOTA_LOG_DEBUG(m_logger, "Found service: " << service);
        }
      }
    }
  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, "Configuration error " << e.what());
  }
}

bool iota::CommandHandle::find_service_name(std::string srv) {
  int i = 0;
  for (i = 0; i < _services_names.size(); i++) {
    if (_services_names[i] == srv) {
      return true;
    }
  }

  return false;
}

void iota::CommandHandle::make_registrations(void) {
  IOTA_LOG_INFO(m_logger, "Start thread make_registrations");

  _reg_timeout = DEFAULT_REG_TIMEOUT;

  while (true) {
    // Timer with select
    struct timeval time_to_wait;
    time_to_wait.tv_sec = _reg_timeout;
    time_to_wait.tv_usec = 0;
    select(0, NULL, NULL, NULL, &time_to_wait);

    if (_reg_timeout == DEFAULT_REG_TIMEOUT) {
      _reg_timeout = DEFAULT_REG_TIMEOUT * 600;
    }

    std::string type_store;
    try {
      const JsonValue& storage =
          iota::Configurator::instance()->get(iota::store::types::STORAGE);
      if (storage.HasMember(iota::store::types::TYPE.c_str())) {
        type_store.assign(
            storage[iota::store::types::TYPE.c_str()].GetString());

        if (type_store == "mongodb") {
          send_all_registrations_from_mongo();
        } else if (type_store == "file") {
          send_all_registrations();
        } else {
          IOTA_LOG_ERROR(m_logger, "incorrect type: " << type_store);
        }
      } else {
        IOTA_LOG_ERROR(m_logger, "in config devices store: no type defined");
      }
    } catch (std::exception& e) {
      IOTA_LOG_ERROR(m_logger, "ERROR in conf devices_store");
    }
  }

  pthread_exit(NULL);
}

void iota::CommandHandle::start_thread_registrations(void) {
  pthread_t thrid;
  pthread_create(&thrid, NULL, _begin_registrations, this);
}

void iota::CommandHandle::send_all_registrations() {
  IOTA_LOG_DEBUG(m_logger, "Sending all register");

  if (_myProvidingApp == UNKOWN_PROVIDING_APP) {
    IOTA_LOG_DEBUG(m_logger,
                   "Registrations are not sent because a valid ProvidingApp "
                   "can not be obtained");
    return;
  }

  try {
    iota::DevicesFile* devFile = iota::DevicesFile::instance();

    const JsonValue& devices = devFile->getDevicesObject();
    if (devices.IsArray()) {
      for (rapidjson::SizeType i = 0; i < devices.Size(); i++) {
        std::vector<iota::ContextRegistration> context_registrations;
        iota::ContextRegistration cr;
        std::string cb_response;
        std::string reg_id;
        std::string reg_time;
        std::string service, service_path, dev_id;

        if (devices[i].HasMember(iota::store::types::DEVICE_ID.c_str())) {
          dev_id =
              devices[i][iota::store::types::DEVICE_ID.c_str()].GetString();
        }
        if (devices[i].HasMember(iota::store::types::SERVICE.c_str())) {
          service = devices[i]["service"].GetString();
        }
        if (devices[i].HasMember(iota::store::types::SERVICE_PATH.c_str())) {
          service_path = devices[i]["service_path"].GetString();
        }

        IOTA_LOG_DEBUG(m_logger, "Search dev: " << dev_id);

        boost::shared_ptr<Device> item_find(new Device(dev_id, service));
        item_find->_service_path = service_path;

        boost::shared_ptr<Device> item_dev = registeredDevices.get(item_find);

        if (!find_service_name(service)) {
          IOTA_LOG_DEBUG(m_logger, "Not service of this resource: " << service);
        } else if (item_dev != NULL) {
          IOTA_LOG_DEBUG(m_logger, "Found Device Entitiy: " << item_dev->_name);

          std::string original_entity_name = item_dev->_entity_name;
          std::string original_entity_type = item_dev->_entity_type;

          // check service
          boost::property_tree::ptree service_ptree;
          get_service_by_name(service_ptree, service);

          IOTA_LOG_DEBUG(m_logger, "setting env info");
          cr.set_env_info(service_ptree, item_dev);

          cr.add_provider(_myProvidingApp);

          if (!item_dev->_registration_id.empty()) {
            reg_id = item_dev->_registration_id;
            IOTA_LOG_DEBUG(m_logger, "Setting registrationId: " << reg_id);
          }

          std::map<std::string, std::string>::iterator p;
          for (p = item_dev->_commands.begin(); p != item_dev->_commands.end();
               ++p) {
            std::string attr_name = p->first;
            std::string attr_type = p->second;

            if ((attr_type.compare(iota::types::STATUS) != 0) &&
                (attr_type.compare(iota::types::INFO) != 0)) {
              attr_type = iota::types::COMMAND_TYPE;
            }

            iota::AttributeRegister attribute(attr_name, attr_type, "false");
            cr.add_attribute(attribute);
          }

          context_registrations.push_back(cr);

          IOTA_LOG_DEBUG(m_logger, "sending to CB");
          send_register(context_registrations, service_ptree, item_dev, reg_id,
                        cb_response);

          IOTA_LOG_DEBUG(m_logger, "cb_response: " << cb_response);

          std::istringstream str_reg_response;
          str_reg_response.str(cb_response);
          iota::RegisterResponse resp(str_reg_response);

          reg_id = resp.get_registration_id();
          item_dev->_registration_id = reg_id;

          reg_time = resp.get_duration();
          _reg_timeout = get_duration_seconds(reg_time);

          IOTA_LOG_DEBUG(m_logger, "registrationId: " << reg_id);
          IOTA_LOG_DEBUG(m_logger, "duration: " << reg_time);

          if ((original_entity_name != item_dev->_entity_name) ||
              (original_entity_type != item_dev->_entity_type)) {
            IOTA_LOG_DEBUG(m_logger, "Calculated entity name or type");
            boost::shared_ptr<Device> item_new(new Device(*item_dev));
            registeredDevices.remove(*item_dev);
            registeredDevices.insert(item_new);
            IOTA_LOG_DEBUG(
                m_logger, "Calculated entity_name: " << item_new->_entity_name);
            IOTA_LOG_DEBUG(
                m_logger, "Calculated entity_type: " << item_new->_entity_type);
          }
        }
      }
    } else {
      IOTA_LOG_ERROR(m_logger, "devices is not array");
    }
  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, "Error sending registrations " << e.what());
  } catch (...) {
    IOTA_LOG_ERROR(m_logger, "Other exception");
  }
}

void iota::CommandHandle::send_all_registrations_from_mongo() {
  IOTA_LOG_DEBUG(m_logger, "Sending all register from devices collection");

  try {
    iota::DeviceCollection dev_table;
    std::string srv, service_path, protocol;

    iota::ServiceCollection srv_table;

    IOTA_LOG_DEBUG(m_logger, "Resource: " << get_resource());
    mongo::BSONObj srv_find = BSON("resource" << get_resource());
    int code_res = srv_table.find(srv_find);

    while (srv_table.more()) {
      mongo::BSONObj srv_resu = srv_table.next();
      boost::property_tree::ptree service_ptree;
      fill_service_with_bson(srv_resu, service_ptree);

      srv = srv_resu.getStringField(iota::store::types::SERVICE);
      service_path = srv_resu.getStringField(iota::store::types::SERVICE_PATH);
      iota::ProtocolData protocol_data = get_protocol_data();
      protocol = protocol_data.protocol;
      IOTA_LOG_DEBUG(m_logger, "service:" << srv
                                          << " service_path:" << service_path
                                          << " protocol:" << protocol);

      find_devices_with_commands(dev_table, srv, service_path, protocol);

      while (dev_table.more()) {
        Device dev_resu = dev_table.nextd();
        IOTA_LOG_DEBUG(m_logger, "Found device: " << dev_resu._name);
        // If no commands and no internal attributes, register is not needed.
        IOTA_LOG_DEBUG(
            m_logger,
            "commands=" << boost::lexical_cast<std::string>(
                               dev_resu._commands.size())
                        << " lazy=" << boost::lexical_cast<std::string>(
                                           dev_resu._lazy.size()));

        if (dev_resu._commands.size() != 0 || dev_resu._lazy.size() != 0) {
          std::vector<iota::ContextRegistration> context_registrations;
          iota::ContextRegistration cr;
          std::string cb_response;
          std::string reg_id;
          std::string reg_time;

          boost::shared_ptr<Device> item_dev(new Device(dev_resu));

          IOTA_LOG_DEBUG(m_logger, "setting env info");
          cr.set_env_info(service_ptree, item_dev);

          iota::Entity entity(item_dev->_entity_name, item_dev->_entity_type,
                              "false");
          cr.add_provider(_myProvidingApp);

          if (!dev_resu._registration_id.empty()) {
            reg_id = dev_resu._registration_id;
            IOTA_LOG_DEBUG(m_logger, "Setting registrationId: " << reg_id);
          }

          std::map<std::string, std::string>::iterator p;
          for (p = dev_resu._commands.begin(); p != dev_resu._commands.end();
               ++p) {
            std::string attr_name = p->first;
            std::string attr_type = p->second;

            if ((attr_type.compare(iota::types::STATUS) != 0) &&
                (attr_type.compare(iota::types::INFO) != 0)) {
              attr_type = iota::types::COMMAND_TYPE;
            }

            iota::AttributeRegister attribute(attr_name, attr_type, "false");
            cr.add_attribute(attribute);
          }

          context_registrations.push_back(cr);

          send_register(context_registrations, service_ptree, item_dev, reg_id,
                        cb_response);

          IOTA_LOG_DEBUG(m_logger, "cb_response: " << cb_response);

          std::istringstream str_reg_response;
          str_reg_response.str(cb_response);
          iota::RegisterResponse resp(str_reg_response);

          reg_id = resp.get_registration_id();
          dev_resu._registration_id = reg_id;

          reg_time = resp.get_duration();
          _reg_timeout = get_duration_seconds(reg_time);
          dev_resu._duration_cb = _reg_timeout;

          Device dev_update("", "");
          dev_update._registration_id = reg_id;
          dev_update._duration_cb = _reg_timeout;

          Device dev_query(dev_resu._name, dev_resu._service);
          dev_table.updated(dev_query, dev_update);

          IOTA_LOG_DEBUG(m_logger, "registrationId: " << reg_id);
          IOTA_LOG_DEBUG(m_logger, "duration: " << reg_time);
        }
      }
    }
  } catch (...) {
    IOTA_LOG_ERROR(m_logger, "Error sending registrations");
  }
  IOTA_LOG_DEBUG(m_logger, "END Registrations ");
}

void iota::CommandHandle::send_register_device(Device& device) {
  std::string p_request(" module=" + get_resource() + " device=" +
                        device._name + " service=" + device._service +
                        " service_path=" + device._service_path);
  IOTA_LOG_DEBUG(m_logger, p_request);

  Device& register_device = device;
  try {
    std::string srv, service_path;
    iota::DeviceCollection dev_table;
    iota::Collection srv_table(iota::store::types::SERVICE_TABLE);

    IOTA_LOG_DEBUG(m_logger, "Resource: " << get_resource());
    mongo::BSONObj srv_find =
        BSON(iota::store::types::SERVICE
             << device._service << iota::store::types::SERVICE_PATH
             << device._service_path << iota::store::types::RESOURCE
             << get_resource());
    int code_res = srv_table.find(srv_find);

    while (srv_table.more()) {
      mongo::BSONObj srv_resu = srv_table.next();
      srv = srv_resu.getStringField(iota::store::types::SERVICE);
      service_path = srv_resu.getStringField(iota::store::types::SERVICE_PATH);

      IOTA_LOG_DEBUG(m_logger, " service=" << srv
                                           << " service_path=" << service_path);

      dev_table.findd(device);

      while (dev_table.more()) {
        register_device = dev_table.nextd();
        IOTA_LOG_DEBUG(m_logger, "Found device: " << register_device._name);

        boost::property_tree::ptree service_ptree;
        get_service_by_name(service_ptree, srv, service_path);
        boost::shared_ptr<Device> item_dev(new Device(register_device));

        IOTA_LOG_DEBUG(m_logger, "updatContext Device::" << item_dev->_name);
        send_updateContext("", "", "", "", item_dev, service_ptree,
                           iota::types::STATUS_OP);

        // If no commands and no internal attributes, register is not needed.
        IOTA_LOG_DEBUG(
            m_logger,
            p_request << " commands=" << boost::lexical_cast<std::string>(
                                             register_device._commands.size())
                      << " lazy=" << boost::lexical_cast<std::string>(
                                         register_device._lazy.size()));

        if (register_device._commands.size() != 0 ||
            register_device._lazy.size() != 0) {
          std::vector<iota::ContextRegistration> context_registrations;
          iota::ContextRegistration cr;
          std::string cb_response;
          std::string reg_id;
          std::string reg_time;

          IOTA_LOG_DEBUG(m_logger, "setting env info");
          cr.set_env_info(service_ptree, item_dev);

          iota::Entity entity(item_dev->_entity_name, item_dev->_entity_type,
                              "false");
          cr.add_provider(_myProvidingApp);

          if (!register_device._registration_id.empty()) {
            reg_id = register_device._registration_id;
            IOTA_LOG_DEBUG(m_logger, "Setting registrationId: " << reg_id);
          }

          std::map<std::string, std::string>::iterator p;
          for (p = register_device._commands.begin();
               p != register_device._commands.end(); ++p) {
            std::string attr_name = p->first;
            std::string attr_type = p->second;

            if ((attr_type.compare(iota::types::STATUS) != 0) &&
                (attr_type.compare(iota::types::INFO) != 0)) {
              attr_type = iota::types::COMMAND_TYPE;
            }

            iota::AttributeRegister attribute(attr_name, attr_type, "false");
            cr.add_attribute(attribute);
          }

          context_registrations.push_back(cr);

          send_register(context_registrations, service_ptree, item_dev, reg_id,
                        cb_response);

          IOTA_LOG_DEBUG(m_logger, "cb_response: " << cb_response);

          std::istringstream str_reg_response;
          str_reg_response.str(cb_response);
          iota::RegisterResponse resp(str_reg_response);

          reg_id = resp.get_registration_id();
          register_device._registration_id = reg_id;

          reg_time = resp.get_duration();
          _reg_timeout = get_duration_seconds(reg_time);

          Device dev_update("", "");
          dev_update._registration_id = reg_id;
          dev_update._duration_cb = _reg_timeout;

          Device dev_query(register_device._name, register_device._service);
          dev_table.updated(dev_query, dev_update);

          IOTA_LOG_DEBUG(m_logger, p_request << " registrationId=" << reg_id
                                             << " duration=" << reg_time);
        }
      }
    }

  } catch (...) {
    IOTA_LOG_ERROR(m_logger, "Error sending registrations");
  }
}

int iota::CommandHandle::updateContext(
    iota::UpdateContext& updateContext,
    const boost::property_tree::ptree& service_ptree,
    const std::string& sequence, iota::ContextResponses& context_responses) {
  IOTA_LOG_DEBUG(m_logger, "updateContext");
  int iresponse = 200;

  std::vector<iota::ContextElement>::const_iterator i;
  std::vector<iota::ContextElement> contextElemts =
      updateContext.get_context_elements();
  std::string service =
      service_ptree.get<std::string>(iota::store::types::SERVICE, "");

  std::string service_path =
      service_ptree.get<std::string>(iota::store::types::SERVICE_PATH, "");

  for (i = contextElemts.begin(); i != contextElemts.end(); ++i) {
    iota::ContextResponse res;
    iota::ContextElement entity = *i;
    res.add_context_element(entity);

    IOTA_LOG_DEBUG(m_logger, registeredDevices.size()
                                 << " getDevice:" << entity.get_id() << " "
                                 << entity.get_type()
                                 << " of service:" << service
                                 << " with service_path: " << service_path);
    try {
      const boost::shared_ptr<Device> item_dev = get_device_by_entity(
          entity.get_id(), entity.get_type(), service, service_path);
      if (item_dev.get() == NULL) {
        std::string errSTR = "CommandHandle::updateContext NO exists item_dev ";
        errSTR.append(entity.get_id());
        errSTR.append(" of service:");
        errSTR.append(service);
        IOTA_LOG_ERROR(m_logger, errSTR);
        res.set_code(iota::types::RESPONSE_CODE_NO_REGISTERED_DEVICE);
        res.set_reason(iota::types::RESPONSE_MESSAGE_NO_REGISTERED_DEVICE);
        res.set_details(errSTR);
        iresponse = iota::types::RESPONSE_CODE_NO_REGISTERED_DEVICE;
      } else {
        std::string entity_type = item_dev->_entity_type;
        if (entity_type.empty()) {
          IOTA_LOG_DEBUG(m_logger, "dev " << item_dev->_name
                                          << " has not got entity_type");
          entity_type = service_ptree.get<std::string>("entity_type", "");
        }
        IOTA_LOG_DEBUG(m_logger, "devvvv " << item_dev->_name << " entity_type "
                                           << entity_type << " "
                                           << item_dev->_service << " "
                                           << item_dev->_service_path);
        std::vector<iota::Attribute>::const_iterator j;
        std::vector<iota::Attribute> atts = entity.get_attributes();
        for (j = atts.begin(); j != atts.end(); ++j) {
          iota::Attribute att = *j;
          if (att.get_type().compare(iota::types::COMMAND_TYPE) == 0) {
            try {
              IOTA_LOG_DEBUG(m_logger, "Command VALUE:  " << att.get_value());
              updateCommand(att.get_name(), att.get_value(), item_dev,
                            entity_type, sequence, service_ptree);
            } catch (iota::IotaException& e) {
              iresponse = e.status();
              res.set_code(e.status());
              res.set_reason(e.reason());
              res.set_details(e.what());
            }
          } else {
            iresponse = iota::types::RESPONSE_CODE_BAD_REQUEST;
            res.set_code(iota::types::RESPONSE_CODE_BAD_REQUEST);
            res.set_reason(iota::types::RESPONSE_MESSAGE_NO_COMMAND);
            res.set_details(att.get_name());
          }
        }
      }
    } catch (iota::IotaException& e) {
      IOTA_LOG_DEBUG(m_logger,
                     "CommandHandle::updateContext capturada IotaException "
                         << e.what());
      iresponse = e.status();
      res.set_code(e.status());
      res.set_reason(e.reason());
      res.set_details(e.what());
    } catch (std::exception& e) {
      IOTA_LOG_DEBUG(m_logger,
                     "CommandHandle::updateContext capturada std::exception"
                         << e.what());
      iresponse = iota::types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR;
      res.set_reason(iota::types::RESPONSE_MESSAGE_INTERNAL_ERROR);
      res.set_details(e.what());
      res.set_code(pion::http::types::RESPONSE_CODE_SERVER_ERROR);
    }

    context_responses.add_context_response(res);
  }  // end for

  return iresponse;
}

void iota::CommandHandle::response_command(
    const std::string& id_command, const std::string& response,
    const boost::shared_ptr<Device>& device,
    const boost::property_tree::ptree& service_ptree) {

  send_updateContext(id_command, iota::types::STATUS, iota::types::STATUS_TYPE,
                     iota::types::EXECUTED_MESSAGE, iota::types::INFO,
                     iota::types::INFO_TYPE, response, device, service_ptree,
                     iota::types::STATUS_OP);
}

/**
  *  check if Device has an specified command
  *  fill the command with the parameters
  *  return the command ready to send to item_dev
 **/
void iota::CommandHandle::getCommandLine(
    const std::string& command_name, const std::string& updateCommand_value,
    const std::string& sequence_id, const boost::shared_ptr<Device>& item_dev,
    const boost::property_tree::ptree& service, std::string& command_id,
    boost::property_tree::ptree& command_line) {
  if (item_dev.get() == NULL) {
    std::string err = "no Device for command ";
    err.append(command_name);
    IOTA_LOG_ERROR(m_logger, err);
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER,
                              err, iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  if (command_name.empty()) {
    std::string err = "need specify a command ";
    IOTA_LOG_ERROR(m_logger, "need specify a command");
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER,
                              err, iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  std::string strValueCommand;
  std::map<std::string, std::string>::const_iterator it =
      item_dev->_commands.find(command_name);
  if (it == item_dev->_commands.end()) {
    std::string err = "Device ";
    err.append(item_dev->_name);
    err.append(" has not a command called ");
    err.append(command_name);
    IOTA_LOG_ERROR(m_logger, err);
    std::map<std::string, std::string>::iterator iter;
    std::string strCommands;
    strCommands.append("commands of device:{ ");
    for (iter = item_dev->_commands.begin(); iter != item_dev->_commands.end();
         ++iter) {
      strCommands.append(iter->first);  // Not a method call
      strCommands.append(":");
      strCommands.append(iter->second);
      strCommands.append(",");
    }
    IOTA_LOG_DEBUG(m_logger, strCommands);
    throw iota::IotaException(
        iota::types::RESPONSE_MESSAGE_NO_DEVICE_NO_COMMAND, err,
        iota::types::RESPONSE_CODE_CONTEXT_ELEMENT_NOT_FOUND);
  } else {
    strValueCommand = item_dev->_commands[command_name];
  }

  transform_command(command_name, strValueCommand, updateCommand_value,
                    sequence_id, item_dev, service, command_id, command_line);
}

/**
 * receive an updateContext
 *
 **/
void iota::CommandHandle::updateCommand(
    const std::string& command_name, const std::string& parameters,
    boost::shared_ptr<iota::Device> item_dev, const std::string& entity_type,
    const std::string& sequence, const boost::property_tree::ptree& service) {
  IOTA_LOG_DEBUG(m_logger, "updateCommand: " << command_name << "->"
                                             << parameters);

  std::string command_id;
  boost::property_tree::ptree command_to_send;

  getCommandLine(command_name, parameters, sequence, item_dev, service,
                 command_id, command_to_send);

  IOTA_LOG_DEBUG(m_logger, "command_id: " << command_id);

  int timeout = get_timeout_commands();

  std::string resp_cmd;
  CommandData cmd_data;
  cmd_data.command_name = command_name;
  cmd_data.command_id = command_id;
  cmd_data.command_to_send = command_to_send;
  cmd_data.sequence = sequence;
  cmd_data.timeout = timeout;
  cmd_data.item_dev = item_dev;
  cmd_data.entity_type = entity_type;
  cmd_data.service = service;

  if (is_push_type_of_command(item_dev)) {
    send_updateContext(command_name, iota::types::STATUS,
                       iota::types::STATUS_TYPE, iota::types::PENDING, item_dev,
                       service, iota::types::STATUS_OP);
    // always we save command in cache
    save_command(cmd_data.command_name, cmd_data.command_id, cmd_data.timeout,
                 cmd_data.command_to_send, cmd_data.item_dev,
                 cmd_data.entity_type, cmd_data.item_dev->_endpoint,
                 cmd_data.service, cmd_data.sequence,
                 iota::types::READY_FOR_READ);
    IOTA_LOG_DEBUG(m_logger, "Device has endpoint, send command to "
                                 << item_dev->_endpoint);
    int res_code = -1;
    try {
      if (_callback) {
        res_code = execute_command(
            item_dev->_endpoint, command_id, command_to_send, timeout, item_dev,
            service, resp_cmd,
            boost::bind(
                &iota::CommandHandle::receive_command_response,
                this, cmd_data, _1, _2, _3));
      } else {
        res_code =
            execute_command(item_dev->_endpoint, command_id, command_to_send,
                            timeout, item_dev, service, resp_cmd);

        IOTA_LOG_DEBUG(m_logger, "response:" << res_code << "->" << resp_cmd);
        std::string service_name =
            service.get<std::string>(iota::store::types::SERVICE, "");
        std::string service_path =
            service.get<std::string>(iota::store::types::SERVICE_PATH, "");
        if (resp_cmd.empty()) {
          IOTA_LOG_DEBUG(m_logger,
                         "command response from plugin_terceros is empty");
          // empty is a good response, do not add anything
        }
        process_command_response(cmd_data, res_code, resp_cmd);
      }

    } catch (std::runtime_error& e) {
      IOTA_LOG_ERROR(m_logger, "ERROR in execute_command:" << e.what());
      res_code = types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR;
      resp_cmd.assign(e.what());
    }
  } else {
    send_updateContext(command_name, iota::types::STATUS,
                       iota::types::STATUS_TYPE,
                       iota::types::READY_FOR_READ_MESSAGE, item_dev, service,
                       iota::types::STATUS_OP);
    IOTA_LOG_DEBUG(m_logger, " POLLING command, item_dev has not got endpoint");
    save_command(command_name, command_id, timeout, command_to_send, item_dev,
                 entity_type, item_dev->_endpoint, service, sequence,
                 iota::types::READY_FOR_READ);
    IOTA_LOG_DEBUG(m_logger, " PUSH command id," << command_id);
  }
}

bool iota::CommandHandle::is_push_type_of_command(
    boost::shared_ptr<Device> device) {
  return !device->_endpoint.empty();
}

void iota::CommandHandle::transform_command(
    const std::string& command_name, const std::string& command_value,
    const std::string& updateCommand_value, const std::string& sequence_id,
    const boost::shared_ptr<Device>& item_dev,
    const boost::property_tree::ptree& service, std::string& command_id,
    boost::property_tree::ptree& command_line) {
  IOTA_LOG_DEBUG(m_logger, "transform_command:: " << command_value
                                                  << " updateCommand_value:"
                                                  << updateCommand_value);
  std::string result;

  if (!command_value.empty()) {
    if (command_value.compare(iota::types::RAW) == 0) {
      result = updateCommand_value;
      command_line.put(iota::store::types::BODY, updateCommand_value);
    } else {
      std::vector<std::string> params;
      boost::split(params, updateCommand_value, boost::is_any_of("|"));
      int count = std::count(command_value.begin(), command_value.end(), '%');
      IOTA_LOG_DEBUG(m_logger, "count:" << count
                                        << " size params:" << params.size());
      if (count == 0) {
        command_line.put(iota::store::types::BODY, command_value);
      } else if (count > params.size()) {
        std::string errSTR = "malformed command ";
        errSTR.append(command_value);
        errSTR.append(" in relation with ");
        errSTR.append(updateCommand_value);
        errSTR.append(".%s is replaced with  | value");
        IOTA_LOG_ERROR(m_logger, errSTR);
        throw iota::IotaException(
            iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER, errSTR,
            iota::types::RESPONSE_CODE_BAD_REQUEST);
      } else {
        std::size_t found1 = 0;
        std::size_t found2 = command_value.find("%s");
        int i = 0;
        while (found2 != std::string::npos) {
          result.append(command_value.substr(found1, found2 - found1));
          result.append(params[i++]);
          found1 = found2 + 2;
          found2 = command_value.find("%s", found1);
        }
        result.append(
            command_value.substr(found1, command_value.length() - found1));

        command_line.put(iota::store::types::BODY, result);
      }
    }
  } else {
    // by default the command is raw, and return the parameter
    result = updateCommand_value;
    command_line.put(iota::store::types::BODY, result);
  }

  if (sequence_id.empty()) {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    command_id.assign(boost::lexical_cast<std::string>(uuid));
  } else {
    command_id.assign(sequence_id);
  }

  IOTA_LOG_DEBUG(m_logger, "resultcommand " << command_id << " " << result);
};

void iota::CommandHandle::default_op_ngsi(
    pion::http::request_ptr& http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response, std::string& response) {
  std::string trace_message =
      http_request_ptr->get_header(iota::types::HEADER_TRACE_MESSAGES);
  IOTA_LOG_INFO(
      m_logger,
      "iota::CommandHandle::default_op_ngsi trace_message:" + trace_message);

  int iresponse = 200;
  response = "OK";
  iota::ContextResponses context_response;
  try {
    // read updateContext
    std::string method = http_request_ptr->get_method();
    IOTA_LOG_DEBUG(m_logger, "method:" << method);
    std::string resource_o = http_request_ptr->get_original_resource();
    IOTA_LOG_DEBUG(m_logger, "resource_o:" << resource_o);
    std::string resource = http_request_ptr->get_resource();
    IOTA_LOG_DEBUG(m_logger, "resource:" << resource);

    std::string item_dev_ip = http_request_ptr->get_remote_ip().to_string();
    IOTA_LOG_DEBUG(m_logger, "item_dev_ip:" << item_dev_ip);
    std::string s_query = http_request_ptr->get_query_string();
    IOTA_LOG_DEBUG(m_logger, "s_query:" << s_query);
    std::string content = http_request_ptr->get_content();
    IOTA_LOG_DEBUG(m_logger, "content:" << content);

    if (http_request_ptr->has_header(iota::types::FIWARE_SERVICE)) {
      std::string service =
          http_request_ptr->get_header(iota::types::FIWARE_SERVICE);
      IOTA_LOG_DEBUG(m_logger, "service:" << service);
      std::string service_path =
          http_request_ptr->get_header(iota::types::FIWARE_SERVICEPATH);

      // check service
      boost::property_tree::ptree service_ptree;
      get_service_by_name(service_ptree, service, service_path);

      // Add proxy

      std::istringstream ss(content);
      iota::UpdateContext op_updateContext(ss);

      // TODO  leer el sequence del updateContext, si deciden que haya sequence
      boost::uuids::uuid uuid = boost::uuids::random_generator()();
      std::string sequence(boost::lexical_cast<std::string>(uuid));
      updateContext(op_updateContext, service_ptree, sequence, context_response);

      response = context_response.get_string();
      boost::erase_all(response, "\\\"");

    } else {
      IOTA_LOG_ERROR(m_logger, "you need a header with "
                                   << iota::types::FIWARE_SERVICEPATH);

      response =
          create_ngsi_response(types::RESPONSE_CODE_BAD_REQUEST,
                               "you need a header with Fiware-Service", "");
    }

  } catch (iota::IotaException& e) {
    IOTA_LOG_ERROR(m_logger, "Capturada: IotaException in default_op_ngsi");
    IOTA_LOG_ERROR(m_logger, e.what());

    response = create_ngsi_response(e.status(), e.reason(), e.what());
  } catch (std::runtime_error& e) {

    response = create_ngsi_response(500, e.what(), e.what());
  } catch (std::exception& e) {

    response = create_ngsi_response(500, e.what(), e.what());
  } catch (...) {

    response = create_ngsi_response(
        types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR,
        iota::types::RESPONSE_MESSAGE_INTERNAL_ERROR, " ---");
  }

  // write response

  IOTA_LOG_INFO(m_logger,
                "iota::CommandHandle::default_op_ngsi trace_message:" +
                    trace_message + " code: " +
                    boost::lexical_cast<std::string>(iresponse) + " response:" +
                    response);
  http_response.set_status_code(iresponse);

  if (!response.empty()) {
    std::string c_t(iota::types::IOT_CONTENT_TYPE_JSON + "; charset=UTF-8");
    http_response.set_content_type(c_t);
  }
}

void iota::CommandHandle::default_queryContext_ngsi(
    pion::http::request_ptr& http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response, std::string& response) {
  int iresponse = 200;
  std::string trace_message =
      http_request_ptr->get_header(iota::types::HEADER_TRACE_MESSAGES);
  IOTA_LOG_INFO(
      m_logger,
      "iota::CommandHandle::default_queryContext_ngsi trace_message:" +
          trace_message);
  try {
    response = "OK";
    iota::ContextResponses context_response;

    // read updateContext
    std::string method = http_request_ptr->get_method();
    IOTA_LOG_DEBUG(m_logger, "method:" << method);
    std::string resource_o = http_request_ptr->get_original_resource();
    IOTA_LOG_DEBUG(m_logger, "resource_o:" << resource_o);
    std::string resource = http_request_ptr->get_resource();
    IOTA_LOG_DEBUG(m_logger, "resource:" << resource);

    std::string item_dev_ip = http_request_ptr->get_remote_ip().to_string();
    IOTA_LOG_DEBUG(m_logger, "item_dev_ip:" << item_dev_ip);
    std::string s_query = http_request_ptr->get_query_string();
    IOTA_LOG_DEBUG(m_logger, "s_query:" << s_query);
    std::string content = http_request_ptr->get_content();
    IOTA_LOG_DEBUG(m_logger, "content:" << content);

    if (http_request_ptr->has_header(iota::types::FIWARE_SERVICE)) {
      std::string service =
          http_request_ptr->get_header(iota::types::FIWARE_SERVICE);
      IOTA_LOG_DEBUG(m_logger, "service:" << service);
      std::string service_path =
          http_request_ptr->get_header(iota::types::FIWARE_SERVICEPATH);

      // check service
      boost::property_tree::ptree service_ptree;
      get_service_by_name(service_ptree, service, service_path);

      // Add proxy

      std::istringstream ss(content);
      iota::QueryContext op_queryContext(ss);
      IOTA_LOG_DEBUG(m_logger, "queryContext:" << op_queryContext.get_string());

      iresponse =
          queryContext(op_queryContext, service_ptree, context_response);

      IOTA_LOG_DEBUG(m_logger,
                     "response queryContext:" << iresponse << " "
                                              << context_response.get_string());
      response = context_response.get_string();
      iresponse = 200;
    } else {
      IOTA_LOG_ERROR(m_logger, "you need a header with "
                                   << iota::types::FIWARE_SERVICEPATH);
      iresponse = 200;
      response =
          create_ngsi_response(types::RESPONSE_CODE_BAD_REQUEST,
                               "you need a header with Fiware-Service", "");
    }

    // write response
  } catch (iota::IotaException& e) {
    IOTA_LOG_ERROR(m_logger, "Capturada: IotaException in default_op_ngsi");
    IOTA_LOG_ERROR(m_logger, e.what());
    iresponse = 200;
    response = create_ngsi_response(e.status(), e.reason(), e.what());
  } catch (std::runtime_error& e) {
    iresponse = 200;
    response = create_ngsi_response(500, e.what(), e.what());
  } catch (std::exception& e) {
    iresponse = 200;
    response = create_ngsi_response(500, e.what(), e.what());
  } catch (...) {
    iresponse = 200;
    response = create_ngsi_response(
        types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR,
        iota::types::RESPONSE_MESSAGE_INTERNAL_ERROR, " ---");
  }

  IOTA_LOG_INFO(m_logger,
                "iota::CommandHandle::default_query_ngsi trace_message:" +
                    trace_message + " code: " +
                    boost::lexical_cast<std::string>(iresponse) + " response:" +
                    response);
  http_response.set_status_code(iresponse);

  if (!response.empty()) {
    std::string c_t(iota::types::IOT_CONTENT_TYPE_JSON + "; charset=UTF-8");
    http_response.set_content_type(c_t);
  }
}

std::string iota::CommandHandle::create_ngsi_response(
    int code, const std::string& reason, const std::string& details) {
  std::string result;

  if (code == 200) {
    result.append("{ \"statusCode\" : ");
  } else {
    result.append("{ \"errorCode\" : ");
  }

  result.append("{ \"code\":");
  result.append(boost::lexical_cast<std::string>(code));
  result.append(", \"reasonPhrase\":\"");
  result.append(reason);
  result.append("\"}}");

  return result;
}

int iota::CommandHandle::send(iota::ContextElement ngsi_context_element,
                              const std::string& opSTR,
                              const boost::property_tree::ptree& service,
                              std::string& cb_response) {
  boost::shared_ptr<iota::ContextBrokerCommunicator> cb_comm(
      new iota::ContextBrokerCommunicator());

  std::string cb_url;

  std::string cbrokerURL = service.get<std::string>("cbroker", "");
  if (!cbrokerURL.empty()) {
    cb_url.assign(cbrokerURL);
    cb_url.append(get_ngsi_operation("updateContext"));
  }

  std::string updateAction(opSTR);
  iota::UpdateContext op(updateAction);
  op.add_context_element(ngsi_context_element);

  std::string toCV = op.get_string();
  IOTA_LOG_DEBUG(m_logger, ":send2CB:" << cb_url << ":body:" << toCV);
  return cb_comm->async_send(
      cb_url, toCV, service,
      boost::bind(&iota::CommandHandle::handle_updateContext, this, cb_url, _1,
                  _2));
}

std::string iota::CommandHandle::get_ngsi_operation(
    const std::string& operation) {
  std::string op("/NGSI10/");
  op.append(operation);
  IOTA_LOG_DEBUG(m_logger, "Search for " + operation + " uri");
  try {
    const JsonValue& ngsi_url = iota::Configurator::instance()->get("ngsi_url");
    if (ngsi_url.HasMember(operation.c_str())) {
      std::string op_url = ngsi_url[operation.c_str()].GetString();
      op.assign(op_url);
    }
  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, "Configuration error " << e.what());
  }
  return op;
}

int iota::CommandHandle::send_unregister(boost::property_tree::ptree& pt_cb,
                                         const boost::shared_ptr<Device> device,
                                         const std::string& regId,
                                         std::string& cb_response) {
  iota::RegisterContext reg;
  std::string cb_url;
  std::string entity_type("thing");
  std::string entity_name = device->get_real_name(pt_cb);

  try {
    std::string cbrokerSTR = pt_cb.get<std::string>("cbroker", "");
    if (!cbrokerSTR.empty()) {
      cb_url.assign(cbrokerSTR);
      cb_url.append(get_ngsi_operation("registerContext"));
    }
    std::string entity_typeSTR = pt_cb.get<std::string>("entity_type", "");
    if (!entity_typeSTR.empty()) {
      entity_type.assign(entity_typeSTR);
    }

    // Setting Accept to "application/json,text/json"
    pt_cb.put<std::string>(iota::types::IOT_HTTP_HEADER_ACCEPT,
                           iota::types::IOT_CONTENT_TYPE_JSON);

  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, "Configuration error " << e.what());
  }

  iota::ContextRegistration cr;
  iota::Entity entity(entity_name, entity_type, "false");
  cr.add_entity(entity);

  if (_myProvidingApp == UNKOWN_PROVIDING_APP) {
    IOTA_LOG_DEBUG(m_logger,
                   "Registrations are not sent because a valid ProvidingApp "
                   "can not be obtained");
    return -1;
  } else {
    cr.add_provider(_myProvidingApp);
  }

  IOTA_LOG_DEBUG(m_logger, "send_unregister: " << regId);
  reg.add_registrationId(regId);
  reg.add_duration("PT1S");
  reg.add_context_registration(cr);
  ContextBrokerCommunicator cb_communicator_unreg;
  cb_communicator_unreg.send(cb_url, reg.get_string(), pt_cb);
}

int iota::CommandHandle::send_register(
    std::vector<iota::ContextRegistration> context_registrations,
    boost::property_tree::ptree& pt_cb, const boost::shared_ptr<Device> device,
    const std::string& regId, std::string& cb_response) {
  iota::RegisterContext reg;

  std::string cb_url;
  std::string entity_type("thing");
  try {
    std::string cbrokerSTR = pt_cb.get<std::string>("cbroker", "");
    if (!cbrokerSTR.empty()) {
      cb_url.assign(cbrokerSTR);
      cb_url.append(get_ngsi_operation("registerContext"));
    }
    std::string entity_typeSTR = pt_cb.get<std::string>("entity_type", "");
    if (!entity_typeSTR.empty()) {
      entity_type.assign(entity_typeSTR);
    }

    // Setting Accept to "application/json,text/json"
    pt_cb.put<std::string>(iota::types::IOT_HTTP_HEADER_ACCEPT,
                           iota::types::IOT_CONTENT_TYPE_JSON);

  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, "Configuration error " << e.what());
  }

  int i = 0;
  for (i = 0; i < context_registrations.size(); i++) {
    reg.add_context_registration(context_registrations[i]);
  }

  if (!regId.empty()) {
    IOTA_LOG_DEBUG(m_logger, "Adding registrationId: " << regId);
    reg.add_registrationId(regId);
    reg.add_duration("PT1S");
    ContextBrokerCommunicator cb_communicator_unreg;
    cb_communicator_unreg.send(cb_url, reg.get_string(), pt_cb);
  }
  reg.add_duration("");
  reg.add_registrationId("");
  IOTA_LOG_DEBUG(m_logger, "send2CB :" << cb_url);
  IOTA_LOG_DEBUG(m_logger, "RegisterContext : " << reg.get_string());

  ContextBrokerCommunicator cb_communicator;
  cb_response.append(cb_communicator.send(cb_url, reg.get_string(), pt_cb));

  return pion::http::types::RESPONSE_CODE_OK;
}

/**
  *  command_name,   the name of command, for example ping, set, ...
  *  command_att, specificied attribue command ( command, status, info)
  *  value, message save in attribute status of the command
  *  item_dev,  es el item_dev del comando
  *  opSTR es la operacion del context broker, por ejemplo APPEND
  **/
int iota::CommandHandle::send_updateContext(
    const std::string& command_name, const std::string& command_att,
    const std::string& type, const std::string& value,
    const boost::shared_ptr<Device>& item_dev,
    const boost::property_tree::ptree& service, const std::string& opSTR) {
  int code_resp = -1;
  iota::ContextElement ngsi_context_element;
  std::string cb_response;

  if (!command_name.empty()) {
    ContextBrokerCommunicator::add_updateContext(command_name, command_att,
                                                 type, value, item_dev, service,
                                                 ngsi_context_element);
  } else {
    ngsi_context_element.set_is_pattern("false");
  }

  iota::RiotISO8601 mi_hora;
  std::string date_to_cb = mi_hora.toUTC().toString();
  iota::Attribute timeAT("TimeInstant", "ISO8601", date_to_cb);
  ngsi_context_element.add_attribute(timeAT);

  if (item_dev.get() != NULL) {
    ngsi_context_element.set_env_info(service, item_dev);
    code_resp = send(ngsi_context_element, opSTR, service, cb_response);
    IOTA_LOG_DEBUG(m_logger, "<<<" << code_resp << ":" << cb_response);
  } else {
    IOTA_LOG_ERROR(m_logger, "no device in send_updateContext, so do nothing");
  }

  return code_resp;
}

int iota::CommandHandle::send_updateContext(
    const std::string& command_name, const std::string& command_att,
    const std::string& type, const std::string& value,
    const std::string& command_att2, const std::string& type2,
    const std::string& value2, const boost::shared_ptr<Device>& item_dev,
    const boost::property_tree::ptree& service, const std::string& opSTR) {
  iota::ContextElement ngsi_context_element;
  std::string cb_response;
  ContextBrokerCommunicator::add_updateContext(command_name, command_att, type,
                                               value, item_dev, service,
                                               ngsi_context_element);
  ContextBrokerCommunicator::add_updateContext(command_name, command_att2,
                                               type2, value2, item_dev, service,
                                               ngsi_context_element);

  iota::RiotISO8601 mi_hora;
  std::string date_to_cb = mi_hora.toUTC().toString();
  iota::Attribute timeAT("TimeInstant", "ISO8601", date_to_cb);
  ngsi_context_element.add_attribute(timeAT);

  ngsi_context_element.set_env_info(service, item_dev);

  int code_resp = send(ngsi_context_element, opSTR, service, cb_response);
  IOTA_LOG_DEBUG(m_logger, "<<<" << code_resp << ":" << cb_response);

  return code_resp;
}

void iota::CommandHandle::save_command(
    const std::string& command_name, const std::string& command_id, int timeout,
    const boost::property_tree::ptree& command_to_send,
    const boost::shared_ptr<Device>& item_dev, const std::string& entity_type,
    const std::string& endpoint,
    const boost::property_tree::ptree& service_ptree,
    const std::string& sequence, int status) {
  std::string service =
      service_ptree.get<std::string>(iota::store::types::SERVICE, "");
  std::string service_path =
      service_ptree.get<std::string>(iota::store::types::SERVICE_PATH, "");
  if (command_id.empty()) {
    std::string errSTR = "command is is empty ";
    errSTR.append(command_name);
    errSTR.append(" of device ");
    errSTR.append(item_dev->_name);
    IOTA_LOG_ERROR(m_logger, errSTR);
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER,
                              errSTR, iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  IOTA_LOG_DEBUG(m_logger,
                 "save_command: "
                     << " id:" << command_id << " service:" << service
                     << " service_path:" << service_path
                     << " name:" << command_name << " sequence:" << sequence
                     << "device:" << item_dev->get_real_name(service_ptree)
                     << " endpoint:" << endpoint << "timeout:" << timeout);

  boost::shared_ptr<Command> item(
      new Command(command_id, command_name, service, service_path, sequence,
                  item_dev->get_real_name(service_ptree), entity_type, endpoint,
                  timeout, "", command_to_send));
  item->set_status(status);
  m_asyncCommands.insert(item);

  if (_storage_type.compare(iota::store::types::MONGODB) == 0) {
    IOTA_LOG_DEBUG(m_logger, "save_command in mongo");
    iota::CommandCollection table;
    table.insert(*(item.get()));
  }
}

iota::CommandVect iota::CommandHandle::get_all_command(
    const std::string& device_id, const std::string& apikey) {
  boost::property_tree::ptree pt_cb;
  boost::shared_ptr<Device> dev;

  get_service_by_apiKey(pt_cb, apikey);

  dev = get_device(
      device_id, pt_cb.get<std::string>(iota::store::types::SERVICE, ""),
      pt_cb.get<std::string>(iota::store::types::SERVICE_PATH, ""));

  return get_all_command(dev, pt_cb);
}

iota::CommandVect iota::CommandHandle::get_all_command(
    const boost::shared_ptr<Device>& dev,
    const boost::property_tree::ptree& service_ptree) {
  iota::CommandVect res;

  if (_storage_type.compare(iota::types::CONF_FILE_MONGO) != 0) {
    IOTA_LOG_INFO(m_logger,
                  "get_all_command not work if storage_type is not mongodb ");
    return res;
  }
  std::string service =
      service_ptree.get<std::string>(iota::store::types::SERVICE, "");
  std::string service_path =
      service_ptree.get<std::string>(iota::store::types::SERVICE_PATH, "");
  if (dev.get() == NULL) {
    IOTA_LOG_INFO(m_logger,
                  "get_all_command not work if storage_type is not mongodb ");
    return res;
  }

  std::string entity_name = dev->get_real_name(service_ptree);
  IOTA_LOG_DEBUG(m_logger, "get_all_command: " << entity_name
                                               << " service:" << service << "  "
                                               << service_path);

  boost::shared_ptr<Command> item(
      new Command(entity_name, service, service_path));
  item->set_status(iota::types::READY_FOR_READ);
  // change command status to DELIVERED
  res = m_asyncCommands.get_by_entityV(item, iota::types::DELIVERED);

  // send to CB status to DELIVERED
  iota::Collection table(iota::store::types::COMMAND_TABLE);
  for (CommandVect::iterator it = res.begin(); it != res.end(); ++it) {
    CommandPtr prt = *it;
    if (_storage_type.compare(iota::store::types::MONGODB) == 0) {
      IOTA_LOG_DEBUG(m_logger, "update command status to delivered "
                                   << prt->get_id());
      mongo::BSONObj no = BSON(
          iota::store::types::COMMAND_ID
          << prt->get_id() << iota::store::types::SERVICE << prt->get_service()
          << iota::store::types::SERVICE_PATH << prt->get_service_path());
      mongo::BSONObj ap =
          BSON(iota::store::types::STATUS << iota::types::DELIVERED);
      table.update(no, ap);
    }
    send_updateContext(prt->get_name(), iota::types::STATUS,
                       iota::types::STATUS_TYPE, iota::types::DELIVERED_MESSAGE,
                       dev, service_ptree, iota::types::STATUS_OP);
  }

  return res;
}

iota::CommandPtr iota::CommandHandle::get_command(
    const std::string& command_id, const std::string& service,
    const std::string& service_path) {
  IOTA_LOG_DEBUG(m_logger, "get_command: " << command_id << " service:"
                                           << service << " " << service_path);

  boost::shared_ptr<Command> item(new Command("", service, service_path));
  item->set_id(command_id);

  // change command status to DELIVERED
  CommandPtr res = m_asyncCommands.get_by_id(item);

  return res;
}

iota::CommandPtr iota::CommandHandle::get_command_from_mongo(
    const std::string& command_id, const std::string& service,
    const std::string& service_path) {
  IOTA_LOG_DEBUG(m_logger, "get_command: " << command_id << " service:"
                                           << service << " " << service_path);

  boost::shared_ptr<Command> item(new Command("", service, service_path));
  item->set_id(command_id);

  // change command status to DELIVERED
  CommandPtr res = m_asyncCommands.get_by_id_from_mongo(item);

  return res;
}

void iota::CommandHandle::remove_all_command() { m_asyncCommands.remove_all(); }

int iota::CommandHandle::remove_command(const std::string& command_id,
                                        const std::string& service,
                                        const std::string& service_path) {
  IOTA_LOG_DEBUG(m_logger, "remove_command: " << command_id
                                              << " service:" << service << " "
                                              << service_path);
  int removed_commands = 1;

  boost::shared_ptr<Command> item(new Command("", service, service_path));
  item->set_id(command_id);

  m_asyncCommands.remove(item);

  if (_storage_type.compare(iota::store::types::MONGODB) == 0) {
    IOTA_LOG_DEBUG(m_logger, "remove_command in mongo");
    iota::CommandCollection table;
    removed_commands = table.remove(*(item.get()));
  }

  return removed_commands;
}

int iota::CommandHandle::get_cache_size() { return registeredDevices.size(); }

void iota::CommandHandle::enable_ngsi_service(
    std::map<std::string, std::string>& filters,
    iota::RestHandle::HandleFunction_t handle_update, iota::RestHandle* context,
    iota::RestHandle::HandleFunction_t handle_query) {
  iota::RestHandle* ngsi_service = NULL;
  std::string url_ngsi_update;
  std::string url_ngsi_query;
  iota::AdminService* AdminService_ptr =
      iota::Process::get_process().get_admin_service();
  if (AdminService_ptr != NULL) {
    std::string ngsi_service_str(iota::Process::get_url_base());
    ngsi_service_str.append("/");
    ngsi_service_str.append(iota::NGSI_SERVICE);
    ngsi_service = AdminService_ptr->get_service(ngsi_service_str);
  }
  if (ngsi_service != NULL) {
    if (handle_update == NULL) {
      IOTA_LOG_DEBUG(m_logger, "default updateContext added");
      handle_query = REST_HANDLE(&iota::CommandHandle::default_op_ngsi);
    }

    std::string ngsi_update(get_resource());
    ngsi_update.append("/updateContext");
    url_ngsi_update =
        ngsi_service->add_url(ngsi_update, filters, handle_update, context);

    // add queryContext
    if (handle_query == NULL) {
      IOTA_LOG_DEBUG(m_logger, "default queryContext added");
      handle_query =
          REST_HANDLE(&iota::CommandHandle::default_queryContext_ngsi);
    }
    std::string ngsi_query(get_resource());
    ngsi_query.append("/queryContext");
    url_ngsi_query =
        ngsi_service->add_url(ngsi_query, filters, handle_query, context);

    init_services_by_resource();
    // Obtaining ip and port from pion

    std::string my_resource = url_ngsi_update;
    size_t pos = url_ngsi_update.find("/updateContext");
    if (pos != std::string::npos) {
      my_resource = url_ngsi_update.substr(0, pos);
    }

    std::string public_ip = get_public_ip();
    pos = public_ip.find("http");
    if (pos != 0) {
      _myProvidingApp = "http://" + public_ip;
    } else {
      _myProvidingApp = public_ip;
    }
    _myProvidingApp += my_resource;

    start_thread_registrations();
  }
}

void iota::CommandHandle::receive_command_response(
    CommandData cmd_data, boost::shared_ptr<iota::HttpClient> http_client,
    pion::http::response_ptr http_response,
    const boost::system::error_code& error) {
  IOTA_LOG_DEBUG(m_logger, "command response from "
                               << http_client->getRemoteEndpoint());
  int res_code;
  std::string command_response;
  if (error || http_response.get() == NULL) {
    command_response.assign(http_client->getRemoteEndpoint());
    command_response.append(" [");
    command_response.append(error.message());
    command_response.append("]");
    IOTA_LOG_ERROR(m_logger, command_response);
    res_code = types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR;
  } else {
    res_code = http_response->get_status_code();
    std::string resp_cmd = http_response->get_content();
    std::string id_command;
    if (transform_response(resp_cmd, res_code, command_response, id_command) <
        0) {
      IOTA_LOG_ERROR(m_logger, "command response from "
                                   << http_client->getRemoteEndpoint() << " "
                                   << resp_cmd);
    }
  }
  IOTA_LOG_DEBUG(m_logger, " response=" + command_response);
  process_command_response(cmd_data, res_code, command_response);
}

int iota::CommandHandle::transform_response(const std::string& str_command_resp,
                                            const int& cmd_status,
                                            std::string& command_response,
                                            std::string& id_command) {
  command_response.assign(str_command_resp);
  id_command.assign("");
}

void iota::CommandHandle::process_command_response(CommandData& cmd_data,
                                                   int& res_code,
                                                   std::string& resp_cmd) {
  std::string service_name =
      cmd_data.service.get<std::string>(iota::store::types::SERVICE, "");
  std::string service_path =
      cmd_data.service.get<std::string>(iota::store::types::SERVICE_PATH, "");

  if (res_code == pion::http::types::RESPONSE_CODE_OK) {
    remove_command(cmd_data.command_id, service_name, service_path);
    if (!(resp_cmd.empty())) {
      send_updateContext(cmd_data.command_name, iota::types::STATUS,
                         iota::types::STATUS_TYPE,
                         iota::types::EXECUTED_MESSAGE, iota::types::INFO,
                         iota::types::INFO_TYPE, resp_cmd, cmd_data.item_dev,
                         cmd_data.service, iota::types::INFO_OP);
    } else {
      send_updateContext(cmd_data.command_name, iota::types::STATUS,
                         iota::types::STATUS_TYPE,
                         iota::types::EXECUTED_MESSAGE, cmd_data.item_dev,
                         cmd_data.service, iota::types::STATUS_OP);
    }
  } else if (res_code == 202) {
    IOTA_LOG_DEBUG(m_logger, " accepted command, waiting for the result");
    int n = 1;
    if (_storage_type.compare(iota::store::types::MONGODB) == 0) {
      IOTA_LOG_DEBUG(m_logger, "update command status to delivered "
                                   << cmd_data.command_id);
      iota::Collection table(iota::store::types::COMMAND_TABLE);

      mongo::BSONObj no =
          BSON(iota::store::types::COMMAND_ID << cmd_data.command_id);
      mongo::BSONObj ap =
          BSON(iota::store::types::STATUS << iota::types::DELIVERED);
      n = table.update(no, ap);
    }

    iota::CommandPtr pt =
        get_command(cmd_data.command_id, service_name, service_path);
    if (pt.get() != NULL) {
      IOTA_LOG_DEBUG(m_logger, cmd_data.command_id
                                   << " change status command, delivered");
      pt->set_status(iota::types::DELIVERED);
    }

    if (n > 0) {
      send_updateContext(cmd_data.command_name, iota::types::STATUS,
                         iota::types::STATUS_TYPE,
                         iota::types::DELIVERED_MESSAGE, cmd_data.item_dev,
                         cmd_data.service, iota::types::STATUS_OP);
    } else {
      IOTA_LOG_ERROR(m_logger,
                     "no command in cache, timeout or response received ,"
                         << cmd_data.command_id);
    }

    IOTA_LOG_DEBUG(m_logger, " response 202, accepted command id ,"
                                 << cmd_data.command_id);
  } else {
    remove_command(cmd_data.command_id, service_name, service_path);
    send_updateContext(
        cmd_data.command_name, iota::types::STATUS, iota::types::STATUS_TYPE,
        iota::types::ERROR, iota::types::INFO, iota::types::INFO_TYPE, resp_cmd,
        cmd_data.item_dev, cmd_data.service, iota::types::STATUS_OP);
  }
}

int iota::CommandHandle::queryContext(
    iota::QueryContext& queryContext,
    const boost::property_tree::ptree& service_ptree,
    iota::ContextResponses& context_responses) {
  IOTA_LOG_DEBUG(m_logger, "queryContext");
  int iresponse = 200;

  std::string service =
      service_ptree.get<std::string>(iota::store::types::SERVICE, "");
  std::string service_path =
      service_ptree.get<std::string>(iota::store::types::SERVICE_PATH, "");

  if (service.empty() || service_path.empty()) {
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_REQUEST,
                              "Service or Sub_service are missing",
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  IOTA_LOG_DEBUG(m_logger, "queryContext: service ["
                               << service << "] sub-service [" << service_path
                               << "]");

  std::vector<iota::Entity> v_entities = queryContext.get_entities();

  if (v_entities.size() == 0) {
    IOTA_LOG_WARN(m_logger, "QueryContext has no entities");
  }

  // get entities one by one.
  int i = 0;
  for (i = 0; i < v_entities.size(); i++) {
    // get device, using a query to Mongo.
    iota::ContextResponse res;
    try {
      std::string id = v_entities[i].get_id();
      std::string type = v_entities[i].get_type();

      iota::ContextElement entity_context_element(
          id, type, v_entities[i].get_is_pattern());

      boost::shared_ptr<iota::Device> device =
          get_device_by_entity(id, type, service, service_path);

      if (device.get() != NULL) {
        IOTA_LOG_DEBUG(m_logger, "queryContext: Device retrieved id ["
                                     << id << "] type: [" << type << "]");
        populate_command_attributes(device, entity_context_element);

        if (entity_context_element.get_attributes().size() == 0) {
          IOTA_LOG_WARN(m_logger, "Device [" << id
                                             << "] has no commands to return");
        }
        IOTA_LOG_DEBUG(m_logger,
                       "Device ["
                           << id << "] returns ["
                           << entity_context_element.get_attributes().size()
                           << "] attributes");

        res.add_context_element(entity_context_element);
      }

    } catch (iota::IotaException& e) {
      IOTA_LOG_DEBUG(m_logger,
                     "CommandHandle::updateContext capturada IotaException "
                         << e.what());
      iresponse = e.status();
      res.set_code(e.status());
      res.set_reason(e.reason());
      res.set_details(e.what());
    } catch (std::exception& e) {
      IOTA_LOG_DEBUG(m_logger,
                     "CommandHandle::updateContext capturada std::exception"
                         << e.what());
      iresponse = iota::types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR;
      res.set_reason(iota::types::RESPONSE_MESSAGE_INTERNAL_ERROR);
      res.set_details(e.what());
      res.set_code(pion::http::types::RESPONSE_CODE_SERVER_ERROR);
    }

    context_responses.add_context_response(res);
  }  // end for
  IOTA_LOG_DEBUG(m_logger, "QueryContext returning  [" << i << "] entities");
  return iresponse;
}

void iota::CommandHandle::populate_command_attributes(
    const boost::shared_ptr<Device>& device,
    iota::ContextElement& entity_context_element) {
  std::map<std::string, std::string>::iterator it;
  for (it = device->_commands.begin(); it != device->_commands.end(); it++) {
    std::string value = it->second;
    if (value.empty()) {
      IOTA_LOG_DEBUG(m_logger, "command:" << it->first << " with empty value");
      value = " ";
    }
    iota::Attribute attribute(it->first, "command", value);
    entity_context_element.add_attribute(attribute);
  }
}

std::string iota::CommandHandle::json_value_to_ul(
    const std::string& json_value) {
  std::string new_value("");
  if (json_value != "") {
    rapidjson::Document document;
    char buffer[json_value.length()];

    strcpy(buffer, json_value.c_str());
    if (document.ParseInsitu<0>(buffer).HasParseError()) {
      // bypass json_value if it wasn't a proper json
      new_value.assign(json_value);
    } else {
      for (rapidjson::Value::ConstMemberIterator itr = document.MemberBegin();
           itr != document.MemberEnd(); ++itr) {
        new_value.append(itr->name.GetString());
        new_value.append("=");
        if (itr->value.IsString()) {
            new_value.append(itr->value.GetString());
        }else{
            throw iota::IotaException(iota::types::RESPONSE_MESSAGE_COMMAND_BAD,json_value,
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
        }
        if (itr + 1 != document.MemberEnd()) {
          new_value.append("|");  // TODO: replace with constants.
        }
      }
    }
  }
  return new_value;
}

void iota::CommandHandle::find_devices_with_commands(
    iota::DeviceCollection& dev_table, const std::string& srv,
    const std::string& service_path, const std::string& protocol) {
  mongo::BSONObj query =
      BSON(iota::store::types::SERVICE
           << srv << iota::store::types::SERVICE_PATH << service_path
           << iota::store::types::PROTOCOL << protocol
           << iota::store::types::COMMANDS << BSON("$exists"
                                                   << "true"));

  mongo::BSONObjBuilder fieldsToReturn;

  int res = dev_table.find(INT_MIN, query, fieldsToReturn);
  if (res == 0) {
    IOTA_LOG_DEBUG(
        m_logger,
        "find_devices_with_commands: Found some devices with commands");
  }
}
