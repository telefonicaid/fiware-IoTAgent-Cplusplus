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
#include "dev_file.h"

#include "store_const.h"
#include "json_util.h"
#include <fstream>
#include <iostream>
#include <rest/process.h>
#include <pion/http/types.hpp>
#include <boost/thread/mutex.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filestream.h>
#include <rapidjson/prettywriter.h>

iota::DevicesFile* iota::DevicesFile::pinstance = 0;  // Inicializar el puntero

iota::DevicesFile::DevicesFile()
    : m_log(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  _error = "error, no config file was initializated";
}

std::string iota::DevicesFile::read_file(std::stringstream& _is) {
  IOTA_LOG_INFO(m_log, "Reading file  " << _filename);

  if (_document.Parse<0>(_is.str().c_str()).HasParseError()) {
    std::ostringstream what;
    what << "DevicesFile: ";
    what << _document.GetParseError();
    what << "[";
    what << _document.GetErrorOffset();
    what << "]";
    _error = what.str();
  } else {
    _error = "";
  }

  return "";
}

void iota::DevicesFile::set_filename(std::string filename) {
  _filename = filename;
};

iota::DevicesFile* iota::DevicesFile::instance() {
  if (pinstance == 0) {
    pinstance = new DevicesFile();
  }
  return pinstance;
}

iota::DevicesFile* iota::DevicesFile::initialize(const std::string& filename) {
  iota::DevicesFile* my_instance = instance();
  my_instance->set_filename(filename);
  my_instance->init();
  return my_instance;
}

void iota::DevicesFile::release() {
  if (pinstance != 0) {
    delete pinstance;
    pinstance = 0;
  }
}

void iota::DevicesFile::init() {
  std::string init_error;
  try {
    std::ifstream f;
    f.open(_filename.c_str(), std::ios::binary);
    if (f.good()) {
      f.rdbuf();
      std::stringstream ss;
      ss << f.rdbuf();
      read_file(ss);
    } else {
      _error = "File does not exists: ";
      _error.append(_filename.c_str());
      init_error = _error;
    }
    f.close();
  } catch (std::exception& e) {
    init_error = e.what();
  }
  if (!init_error.empty()) {
    IOTA_LOG_ERROR(m_log, init_error);
    throw iota::IotaException(
        iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER + " [" + _filename +
            "]",
        init_error, iota::types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR);
  }
}

void iota::DevicesFile::parse_to_cache(Cache* pcache) {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex_document);

  IOTA_LOG_DEBUG(m_log, "Parsing file to cache");

  if (pcache == NULL) {
    IOTA_LOG_ERROR(m_log, "Empty cache");
    return;
  }

  try {
    if (_document.IsObject() &&
        _document.HasMember(iota::store::types::DEVICES.c_str()) &&
        _document[iota::store::types::DEVICES.c_str()].IsArray()) {
      const iota::JsonValue& devices =
          _document[iota::store::types::DEVICES.c_str()];
      for (rapidjson::SizeType i = 0; i < devices.Size(); i++) {
        std::string id;
        std::string type;
        std::string endpoint;
        std::string timezone;
        std::string entity;
        std::string service;
        std::string service_path;
        //
        //  mandatory fields

        id = get_value_from_rapidjson<std::string>(
            devices[i][iota::store::types::DEVICE_ID.c_str()]);

        if (devices[i].HasMember(iota::store::types::ENTITY_TYPE.c_str())) {
          type = get_value_from_rapidjson<std::string>(
              devices[i][iota::store::types::ENTITY_TYPE.c_str()]);
        }

        if (devices[i].HasMember(iota::store::types::ENDPOINT.c_str())) {
          endpoint = get_value_from_rapidjson<std::string>(
              devices[i][iota::store::types::ENDPOINT.c_str()]);
        }
        if (devices[i].HasMember(iota::store::types::TIMEZONE.c_str())) {
          timezone = get_value_from_rapidjson<std::string>(
              devices[i][iota::store::types::TIMEZONE.c_str()]);
        }
        if (devices[i].HasMember(iota::store::types::ENTITY_NAME.c_str())) {
          entity = get_value_from_rapidjson<std::string>(
              devices[i][iota::store::types::ENTITY_NAME.c_str()]);
        }
        if (devices[i].HasMember(iota::store::types::SERVICE.c_str())) {
          service = get_value_from_rapidjson<std::string>(
              devices[i][iota::store::types::SERVICE.c_str()]);
        }
        if (devices[i].HasMember(iota::store::types::SERVICE_PATH.c_str())) {
          service_path = get_value_from_rapidjson<std::string>(
              devices[i][iota::store::types::SERVICE_PATH.c_str()]);
        }

        if (service_path.empty()) {
          service_path = iota::types::FIWARE_SERVICEPATH_DEFAULT;
        }

        boost::shared_ptr<Device> item(new Device(entity, type, service));

        item->_name = id;
        item->_timezone = timezone;
        item->_endpoint = endpoint;
        item->_service_path = service_path;

        std::string status;
        if (devices[i].HasMember(iota::store::types::STATUS.c_str())) {
          status = get_value_from_rapidjson<std::string>(
              devices[i][iota::store::types::STATUS.c_str()]);
        }

        if (!status.empty() &&
            (status.compare(iota::store::types::INACTIVE.c_str()) == 0)) {
          item->_active = false;
        } else {
          item->_active = true;
        }

        // Commands
        if (devices[i].HasMember(iota::store::types::COMMANDS.c_str())) {
          const iota::JsonValue& commands =
              devices[i][iota::store::types::COMMANDS.c_str()];
          for (rapidjson::SizeType j = 0; j < commands.Size(); j++) {
            std::string attr_name = get_value_from_rapidjson<std::string>(
                commands[j][iota::store::types::NAME.c_str()]);
            std::string attr_cmd = get_value_from_rapidjson<std::string>(
                commands[j][iota::store::types::COMMAND.c_str()]);
            item->_commands.insert(
                std::pair<std::string, std::string>(attr_name, attr_cmd));
          }
        }

        // Attributes
        if (devices[i].HasMember(iota::store::types::ATTRIBUTES.c_str())) {
          const iota::JsonValue& attributes =
              devices[i][iota::store::types::ATTRIBUTES.c_str()];
          for (rapidjson::SizeType j = 0; j < attributes.Size(); j++) {
            std::string attr_name, attr_attr, attr_type;
            if (attributes[j].HasMember(
                    iota::store::types::ATTRIBUTE_ID.c_str())) {
              attr_name = get_value_from_rapidjson<std::string>(
                  attributes[j][iota::store::types::ATTRIBUTE_ID.c_str()]);
            }
            if (attributes[j].HasMember(iota::store::types::NAME.c_str())) {
              attr_attr = get_value_from_rapidjson<std::string>(
                  attributes[j][iota::store::types::NAME.c_str()]);
            }
            if (attributes[j].HasMember(iota::store::types::TYPE.c_str())) {
              attr_type = get_value_from_rapidjson<std::string>(
                  attributes[j][iota::store::types::TYPE.c_str()]);
            }

            std::string json = "{";
            if (!attr_type.empty()) {
              json.append("\"type\":\"");
              json.append(attr_type);
              json.append("\",");
            }
            if (!attr_attr.empty()) {
              json.append("\"name\":\"");
              json.append(attr_attr);
              json.append("\"");
            }

            json.append("}");

            item->_attributes.insert(
                std::pair<std::string, std::string>(attr_name, json));
          }
        }

        // Static attributes
        if (devices[i].HasMember(
                iota::store::types::STATIC_ATTRIBUTES.c_str())) {
          const iota::JsonValue& static_attributes =
              devices[i][iota::store::types::STATIC_ATTRIBUTES.c_str()];
          for (rapidjson::SizeType j = 0; j < static_attributes.Size(); j++) {
            std::string attr_name = get_value_from_rapidjson<std::string>(
                static_attributes[j][iota::store::types::NAME.c_str()]);

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            static_attributes[j].Accept(writer);

            item->_static_attributes.insert(std::pair<std::string, std::string>(
                attr_name, buffer.GetString()));
          }
        }

        pcache->insert(item);
      }
    }
  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_log, e.what());
  }
  IOTA_LOG_DEBUG(m_log, "END Parsing file to cache");
}

const iota::JsonValue& iota::DevicesFile::getDevicesObject() {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex_document);

  try {
    if (_document.IsObject() &&
        _document.HasMember(iota::store::types::DEVICES.c_str())) {
      return _document[iota::store::types::DEVICES.c_str()];
    }
  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_log, e.what());
  }
}
