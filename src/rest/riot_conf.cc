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
#include "riot_conf.h"
#include "rest/process.h"
#include "util/json_util.h"
#include "util/store_const.h"

#include <fstream>
#include <iostream>

#include <pion/http/types.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filestream.h>
#include <rapidjson/prettywriter.h>

iota::Configurator* iota::Configurator::pinstance =
    0;  // Inicializar el puntero

iota::Configurator::Configurator()
    : m_log(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  _error = "error, no config file was initializated";
  http_message = createMessagesMap();
}

std::string iota::Configurator::getHttpMessage(const unsigned int resource) {
  return http_message[resource];
}

std::string iota::Configurator::getError() { return _error; }

bool iota::Configurator::hasError() { return !_error.empty(); }

void iota::Configurator::release() {
  if (pinstance != 0) {
    delete pinstance;
    pinstance = 0;
  }
}

int iota::Configurator::get_listen_port() { return listen_port; }

void iota::Configurator::set_listen_port(int p) { listen_port = p; }

std::string iota::Configurator::get_listen_ip() { return listen_ip; }

void iota::Configurator::set_listen_ip(std::string ip) { listen_ip = ip; }

std::string iota::Configurator::get_iotagent_name() { return iotagent_name; }

void iota::Configurator::set_iotagent_name(std::string name) {
  iotagent_name = name;
}

std::string iota::Configurator::get_iotagent_identifier() {
  if (iotagent_identifier.empty()) {
    try {
      const iota::JsonValue& iot_id = iota::Configurator::instance()->get(
          iota::store::types::IOTAGENT_ID.c_str());
      if (iot_id.IsString()) {
        iotagent_identifier.assign(iot_id.GetString());
      }
    } catch (std::exception& e) {
      IOTA_LOG_DEBUG(m_log, "iotagent identifier does not exist in config.json "
                                << e.what());
    }
  }
  return iotagent_identifier;
}

void iota::Configurator::set_iotagent_identifier(std::string id) {
  iotagent_identifier = id;
}

/**
   flush all data from istream to ptree in memmory

   @param[in]     _is istream json with all data(in json format).
   @return   true if it runs well.
 */

std::string iota::Configurator::read_file(std::stringstream& _is) {
  JsonDocument tmp_doc;
  if (tmp_doc.Parse<0>(_is.str().c_str()).HasParseError()) {
    std::ostringstream what;
    what << "Configurator: ";
    what << tmp_doc.GetParseError();
    what << "[";
    what << tmp_doc.GetErrorOffset();
    what << "]";
    _error = what.str();
  } else {
    _error = check_configuration_json(tmp_doc);
    _document.SetNull();
    // Building document
    if (_error.empty()) {
      _document.Parse<0>(_is.str().c_str());
    }
  }

  // Ejecutamos una peticion para ver si se puede dar por bueno
  return _error;
}

void iota::Configurator::reload() {
  std::ifstream f;
  ;
  f.open(_filename.c_str(), std::ios::binary);
  if (f.good()) {
    f.rdbuf();
    std::stringstream ss;
    ss << f.rdbuf();
    read_file(ss);
  } else {
    IOTA_LOG_ERROR(m_log, "does not exists " << _filename);
  }
  f.close();
}

std::string iota::Configurator::update_conf(std::stringstream& is) {
  IOTA_LOG_INFO(m_log, "Updating configuration " << _filename << " "
                                                 << is.str());
  boost::mutex::scoped_lock lock(m_mutex);
  std::string resp_read_file;
  try {
    resp_read_file = read_file(is);
    if (resp_read_file.empty()) {
      write();
    } else {
      reload();
    }

  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_log, e.what());
    std::ifstream f;
    ;
    f.open(_filename.c_str(), std::ios::binary);
    if (f.good()) {
      f.rdbuf();
      std::stringstream ss;
      ss << f.rdbuf();
      read_file(ss);
    } else {
      IOTA_LOG_INFO(m_log, "File error " << _filename);
    }
    f.close();
  }
  return resp_read_file;
}

iota::Configurator* iota::Configurator::instance() {
  if (pinstance == 0) {
    pinstance = new Configurator();
  }
  return pinstance;
}

iota::Configurator* iota::Configurator::initialize(
    const std::string& filename) {
  // Check if configuration file exists.
  if (!boost::filesystem::exists(filename)) {
    // IOTA_LOG_ERROR(m_log, "Configuration file " +  filename + "does not
    // exist");
    return NULL;
  }
  iota::Configurator* my_instance = instance();
  my_instance->set_filename(filename);
  my_instance->init();
  return my_instance;
}

void iota::Configurator::init() {
  std::ifstream f;
  f.open(_filename.c_str(), std::ios::binary);
  if (f.good()) {
    IOTA_LOG_DEBUG(m_log, "Configurator::init:" << _filename);
    f.rdbuf();
    std::stringstream ss;
    ss << f.rdbuf();
    read_file(ss);
  } else {
    _error = "File does not exists: ";
    _error.append(_filename.c_str());
    IOTA_LOG_ERROR(m_log, _error);
  }
  f.close();
}

const iota::JsonValue& iota::Configurator::getServicebyApiKey(
    const std::string& resource, const std::string& apiKey) {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex_document);
  std::map<std::string,
           boost::shared_ptr<iota::Configurator::Resource> >::iterator it_r =
      _resources.begin();
  it_r = _resources.find(resource);

  if (it_r != _resources.end()) {
    return it_r->second->get_service_by_apikey(apiKey);
  }

  std::string text_error;
  text_error.append("Service by ApiKey: not found");
  text_error.append("[");
  text_error.append(resource);
  text_error.append(",");
  text_error.append(apiKey);
  text_error.append("]");
  throw iota::IotaException(iota::types::RESPONSE_MESSAGE_NO_SERVICE,
                            text_error, iota::types::RESPONSE_CODE_NO_SERVICE);
}

const iota::JsonValue& iota::Configurator::getService(
    const std::string& resource, const std::string& service,
    const std::string& service_path) {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex_document);
  std::map<std::string,
           boost::shared_ptr<iota::Configurator::Resource> >::iterator it_r =
      _resources.begin();
  it_r = _resources.find(resource);

  if (it_r != _resources.end()) {
    return it_r->second->get_service(service, service_path);
  }

  std::string text_error;
  text_error.append("Service by Service: not found");
  text_error.append("[");
  text_error.append(resource);
  text_error.append(",");
  text_error.append(service);
  text_error.append("]");
  throw iota::IotaException(iota::types::RESPONSE_MESSAGE_NO_SERVICE,
                            text_error, iota::types::RESPONSE_CODE_NO_SERVICE);
}

void iota::Configurator::getAllServicePath(const std::string& resource,
                                           const std::string& service,
                                           std::vector<std::string>& vsrvpath) {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex_document);
  std::map<std::string,
           boost::shared_ptr<iota::Configurator::Resource> >::iterator it_r =
      _resources.begin();
  it_r = _resources.find(resource);

  if (it_r != _resources.end()) {
    it_r->second->get_all_servicepath(service, vsrvpath);
    return;
  }

  std::ostringstream what;
  what << "Service get all Service Path: not found";
  what << "[";
  what << resource;
  what << ",";
  what << service;
  what << "]";
  // throw std::runtime_error(what.str());
  throw iota::IotaException(iota::types::CONF_FILE_SERVICE_PATH, what.str(),
                            iota::types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR);
}

std::string iota::Configurator::getResource(const std::string& resource) {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex_document);
  std::map<std::string,
           boost::shared_ptr<iota::Configurator::Resource> >::iterator it_r =
      _resources.begin();
  it_r = _resources.find(resource);

  if (it_r != _resources.end()) {
    return it_r->second->get_string();
  }

  std::string text_error;
  text_error.append("Resource: not found");
  text_error.append("[");
  text_error.append(resource);
  text_error.append("]");
  throw iota::IotaException(iota::types::RESPONSE_MESSAGE_NO_SERVICE,
                            text_error, iota::types::RESPONSE_CODE_NO_SERVICE);
}

const iota::JsonValue& iota::Configurator::getResourceObject(
    const std::string& resource) {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex_document);
  std::map<std::string,
           boost::shared_ptr<iota::Configurator::Resource> >::iterator it_r =
      _resources.begin();
  it_r = _resources.find(resource);

  if (it_r != _resources.end()) {
    return it_r->second->get_value();
  }

  std::string text_error;
  text_error.append("Resource: not found");
  text_error.append("[");
  text_error.append(resource);
  text_error.append("]");
  throw iota::IotaException(iota::types::RESPONSE_MESSAGE_NO_SERVICE,
                            text_error, iota::types::RESPONSE_CODE_NO_SERVICE);
}

void iota::Configurator::add_resource(std::string resource_name,
                                      std::string json, std::string& error) {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex_document);
  boost::shared_ptr<JsonDocument> d(new JsonDocument());
  d->SetObject();
  (*d).Parse<0>(json.c_str());
  if ((*d).HasParseError()) {
    error.assign((*d).GetParseError());
    return;
  }
  if (!(*d).HasMember(iota::types::CONF_FILE_RESOURCE.c_str())) {
    iota::JsonValue v;
    v.SetString(resource_name.c_str(), resource_name.size(),
                (*d).GetAllocator());
    (*d).AddMember(iota::types::CONF_FILE_RESOURCE.c_str(), v,
                   (*d).GetAllocator());
  }

  boost::shared_ptr<iota::Configurator::Resource> res =
      check_resource(d, error);

  if (error.empty() == true) {
    remove_resource(resource_name);
    add_resource(res);
    _document.SetNull();
    std::stringstream ss(getAll());
    update_conf(ss);
  }
}

void iota::Configurator::remove_resource(std::string resource_name) {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex_document);
  _resources.erase(resource_name);
}

void iota::Configurator::add_resource(boost::shared_ptr<Resource> resource) {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex_document);
  const iota::JsonValue& rn = resource->get(iota::types::CONF_FILE_RESOURCE);
  std::string resource_name(rn.GetString());
  _resources.insert(
      std::pair<std::string, boost::shared_ptr<iota::Configurator::Resource> >(
          resource_name, resource));
}

std::string iota::Configurator::getAll() {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex_document);
  if (!_document.IsNull()) {
    rapidjson::StringBuffer buffer_doc;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer_doc(buffer_doc);
    _document.Accept(writer_doc);
    return buffer_doc.GetString();
  }

  _document.SetObject();
  // First-level fields;
  if (_root_fields.get() != NULL) {
    rapidjson::StringBuffer buffer_root;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer_root(buffer_root);
    (*_root_fields).Accept(writer_root);
    JsonDocument d_root;
    d_root.SetObject();
    d_root.Parse<0>(buffer_root.GetString());
    for (iota::JsonValue::MemberIterator itr1 = d_root.MemberBegin();
         itr1 != d_root.MemberEnd(); itr1++) {
      std::string name(itr1->name.GetString());
      iota::JsonValue v(rapidjson::kObjectType);
      v.SetString(itr1->name.GetString(), itr1->name.GetStringLength(),
                  _document.GetAllocator());
      _document.AddMember(v, itr1->value, _document.GetAllocator());
    }
  }
  // Resources
  iota::JsonValue res_array(rapidjson::kArrayType);
  std::map<std::string,
           boost::shared_ptr<iota::Configurator::Resource> >::iterator it =
      _resources.begin();
  while (it != _resources.end()) {
    JsonDocument dr;
    dr.SetObject();
    dr.Parse<0>(it->second->get_string().c_str());
    iota::JsonValue object;
    object.SetObject();
    for (iota::JsonValue::MemberIterator itr2 = dr.MemberBegin();
         itr2 != dr.MemberEnd(); itr2++) {
      iota::JsonValue v(rapidjson::kObjectType);
      v.SetString(itr2->name.GetString(), itr2->name.GetStringLength(),
                  _document.GetAllocator());
      object.AddMember(v, itr2->value, _document.GetAllocator());
    }

    res_array.PushBack(object, _document.GetAllocator());
    ++it;
  }

  _document.AddMember(iota::types::CONF_FILE_RESOURCES.c_str(), res_array,
                      _document.GetAllocator());

  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  _document.Accept(writer);
  return buffer.GetString();
}

const iota::JsonValue& iota::Configurator::get(const std::string& field) {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex_document);

  if ((_root_fields.get() != NULL) && (*_root_fields).IsObject() &&
      (*_root_fields).HasMember(field.c_str())) {
    return (*_root_fields)[field.c_str()];
  }

  // Resources
  if ((field.compare(iota::types::CONF_FILE_RESOURCES) == 0) &&
      _document.HasMember(iota::types::CONF_FILE_RESOURCES.c_str())) {
    return _document[iota::types::CONF_FILE_RESOURCES.c_str()];
  }

  std::ostringstream what;
  what << "Not found";
  what << "[";
  what << field;
  what << "]";
  throw std::runtime_error(what.str());
}

void iota::Configurator::get(const std::string& field,
                             std::map<std::string, std::string>& to_map) {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex_document);

  if ((_root_fields.get() != NULL) && (*_root_fields).IsObject() &&
      (*_root_fields).HasMember(field.c_str())) {
    return get_map_from_rapidjson((*_root_fields)[field.c_str()], to_map);
  }

  std::string text_error;
  text_error.append("Not found");
  text_error.append("[");
  text_error.append(field);
  text_error.append("]");
  throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATA_NOT_FOUND,
                            text_error,
                            iota::types::RESPONSE_CODE_DATA_NOT_FOUND);
}

void iota::Configurator::write() {
  FILE* p_file = fopen(_filename.c_str(), "w+");
  rapidjson::FileStream buffer(p_file);
  rapidjson::PrettyWriter<rapidjson::FileStream> writer(buffer);
  _document.Accept(writer);
  fclose(p_file);
}

std::string iota::Configurator::check_configuration_json(
    JsonDocument& config_doc) {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex_document);
  std::string error;

  if (!config_doc.IsObject()) {
    error.assign(iota::types::CONF_FILE_ERROR + " [No JSON object]");
  } else {
    // First-level fields
    boost::shared_ptr<JsonDocument> first_level_fields_d(new JsonDocument());
    first_level_fields_d->SetObject();
    for (iota::JsonValue::MemberIterator itr1 = config_doc.MemberBegin();
         itr1 != config_doc.MemberEnd(); itr1++) {
      std::string name(itr1->name.GetString());
      if (name.compare(iota::types::CONF_FILE_RESOURCES) != 0) {
        iota::JsonValue v;
        v.SetString(itr1->name.GetString(), itr1->name.GetStringLength(),
                    (*first_level_fields_d).GetAllocator());
        (*first_level_fields_d)
            .AddMember(v, itr1->value, (*first_level_fields_d).GetAllocator());
      }
    }
    _root_fields = first_level_fields_d;

    if (!config_doc.HasMember(iota::types::CONF_FILE_RESOURCES.c_str()) ||
        !config_doc[iota::types::CONF_FILE_RESOURCES.c_str()].IsArray()) {
      error.assign(iota::types::CONF_FILE_ERROR +
                   " [resources is not a JSON array]");
      return error;
    }

    iota::JsonValue& resources =
        config_doc[iota::types::CONF_FILE_RESOURCES.c_str()];
    for (rapidjson::SizeType i = 0; i < resources.Size(); i++) {
      // Check resource field
      if (!resources[i].IsObject()) {
        error.assign(iota::types::CONF_FILE_ERROR +
                     " [resource is not a JSON object]");
      } else {
        boost::shared_ptr<JsonDocument> resource_d(new JsonDocument());
        resource_d->SetObject();
        rapidjson::StringBuffer b_resource;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> w_resource(b_resource);
        resources[i].Accept(w_resource);
        if ((*resource_d).Parse<0>(b_resource.GetString()).HasParseError()) {
          error.assign(iota::types::CONF_FILE_ERROR + " [" +
                       (*resource_d).GetParseError() + "]");

        } else {
          std::string local_error;
          boost::shared_ptr<iota::Configurator::Resource> res =
              check_resource(resource_d, local_error);
          if (local_error.empty() == true) {
            add_resource(res);
          } else {
            error = local_error;
          }
        }
      }
    }

    // Oauth parameters & PEP rules
    if ((*_root_fields).HasMember(iota::types::CONF_FILE_PEP_RULES.c_str()) &&
        (*_root_fields)[iota::types::CONF_FILE_PEP_RULES.c_str()].IsArray()) {
      const iota::JsonValue& pep_rules =
          (*_root_fields)[iota::types::CONF_FILE_PEP_RULES.c_str()];
      for (rapidjson::SizeType i = 0; i < pep_rules.Size(); i++) {
        if (pep_rules[i].HasMember(
                iota::types::CONF_FILE_PEP_RULES_ACTION.c_str()) &&
            pep_rules[i][iota::types::CONF_FILE_PEP_RULES_ACTION.c_str()]
                .IsString() &&
            pep_rules[i].HasMember(
                iota::types::CONF_FILE_PEP_RULES_VERB.c_str()) &&
            pep_rules[i][iota::types::CONF_FILE_PEP_RULES_VERB.c_str()]
                .IsString() &&
            pep_rules[i].HasMember(
                iota::types::CONF_FILE_PEP_RULES_URI.c_str()) &&
            pep_rules[i][iota::types::CONF_FILE_PEP_RULES_URI.c_str()]
                .IsString()) {
          std::string action =
              pep_rules[i][iota::types::CONF_FILE_PEP_RULES_ACTION.c_str()]
                  .GetString();
          iota::PepRule rule;
          rule.verb =
              pep_rules[i][iota::types::CONF_FILE_PEP_RULES_VERB.c_str()]
                  .GetString();
          rule.uri = pep_rules[i][iota::types::CONF_FILE_PEP_RULES_URI.c_str()]
                         .GetString();
          _pep_rules.insert(
              std::pair<std::string, iota::PepRule>(action, rule));
          std::string r(rule.uri);
          r += rule.verb;
          r += action;
        }
      }
    }
  }

  return error;
}

boost::shared_ptr<iota::Configurator::Resource>
iota::Configurator::check_resource(boost::shared_ptr<JsonDocument> resource_obj,
                                   std::string& error) {
  boost::shared_ptr<iota::Configurator::Resource> resource(
      new iota::Configurator::Resource());

  if (!(*resource_obj).HasMember(iota::types::CONF_FILE_RESOURCE.c_str())) {
    error.assign(iota::types::CONF_FILE_ERROR + " [resource field not found]");
  } else {
    // First-level fields
    boost::shared_ptr<JsonDocument> first_level_fields_d(new JsonDocument());
    first_level_fields_d->SetObject();
    for (iota::JsonValue::MemberIterator itr1 = (*resource_obj).MemberBegin();
         itr1 != (*resource_obj).MemberEnd(); itr1++) {
      std::string name(itr1->name.GetString());
      if (name.compare(iota::types::CONF_FILE_SERVICES) != 0) {
        iota::JsonValue v(rapidjson::kObjectType);
        v.SetString(itr1->name.GetString(), itr1->name.GetStringLength(),
                    (*first_level_fields_d).GetAllocator());
        (*first_level_fields_d)
            .AddMember(v, itr1->value, (*first_level_fields_d).GetAllocator());
      }
    }
    resource->add_fields(first_level_fields_d);

    // Services
    if ((*resource_obj).HasMember(iota::types::CONF_FILE_SERVICES.c_str()) &&
        (*resource_obj)[iota::types::CONF_FILE_SERVICES.c_str()].IsArray()) {
      iota::JsonValue& services =
          (*resource_obj)[iota::types::CONF_FILE_SERVICES.c_str()];
      for (rapidjson::SizeType i = 0; i < services.Size(); i++) {
        boost::shared_ptr<JsonDocument> service_d(new JsonDocument());
        service_d->SetObject();
        rapidjson::StringBuffer b_service;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> w_service(b_service);
        services[i].Accept(w_service);

        if ((*service_d).Parse<0>(b_service.GetString()).HasParseError()) {
          error.assign(iota::types::CONF_FILE_ERROR + " [" +
                       (*service_d).GetParseError() + "]");
        } else {
          std::string local_error;
          check_service(service_d, local_error);

          if (local_error.empty() == true) {
            resource->add_service(service_d);
          } else {
            error = local_error;
          }
        }
      }
    }
  }
  return resource;
}

void iota::Configurator::check_service(
    boost::shared_ptr<JsonDocument> service_obj, std::string& error) {
  // Apikey defined (even "")
  if (!(*service_obj).HasMember(iota::types::CONF_FILE_APIKEY.c_str()) ||
      !(*service_obj)[iota::types::CONF_FILE_APIKEY.c_str()].IsString()) {
    error.assign(iota::types::CONF_FILE_ERROR + " [" +
                 iota::types::CONF_FILE_APIKEY + "]");
    return;
  }
  if (!(*service_obj).HasMember(iota::types::CONF_FILE_SERVICE.c_str()) ||
      !(*service_obj)[iota::types::CONF_FILE_SERVICE.c_str()].IsString()) {
    error.assign(iota::types::CONF_FILE_ERROR + " [" +
                 iota::types::CONF_FILE_SERVICE + "]");
    return;
  }

  if (!(*service_obj).HasMember(iota::types::CONF_FILE_ENTITY_TYPE.c_str()) ||
      !(*service_obj)[iota::types::CONF_FILE_ENTITY_TYPE.c_str()].IsString()) {
    error.assign(iota::types::CONF_FILE_ERROR + " [" +
                 iota::types::CONF_FILE_ENTITY_TYPE + "]");
    return;
  }
  if (!(*service_obj).HasMember(iota::types::CONF_FILE_SERVICE_PATH.c_str()) ||
      !(*service_obj)[iota::types::CONF_FILE_SERVICE_PATH.c_str()].IsString()) {
    error.assign(iota::types::CONF_FILE_ERROR + " [" +
                 iota::types::CONF_FILE_SERVICE_PATH + "]");
    return;
  }
  if (!(*service_obj).HasMember(iota::types::CONF_FILE_CBROKER.c_str()) ||
      !(*service_obj)[iota::types::CONF_FILE_CBROKER.c_str()].IsString()) {
    error.assign(iota::types::CONF_FILE_ERROR + " [" +
                 iota::types::CONF_FILE_CBROKER + "]");
    return;
  }
  if (!(*service_obj).HasMember(iota::types::CONF_FILE_TOKEN.c_str()) ||
      !(*service_obj)[iota::types::CONF_FILE_TOKEN.c_str()].IsString()) {
    error.assign(iota::types::CONF_FILE_ERROR + " [" +
                 iota::types::CONF_FILE_TOKEN + "]");
    return;
  }
}

const iota::JsonValue& iota::Configurator::Resource::get(
    const std::string& field) {
  if ((_root_fields.get() != NULL) &&
      (*_root_fields).HasMember(field.c_str())) {
    return (*_root_fields)[field.c_str()];
  }
  std::string text_error;
  text_error.append("Field : not found");
  text_error.append("[");
  text_error.append(field);
  text_error.append("]");
  throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATA_NOT_FOUND,
                            text_error,
                            iota::types::RESPONSE_CODE_DATA_NOT_FOUND);
}
void iota::Configurator::Resource::add_service(
    boost::shared_ptr<JsonDocument> service) {
  std::string name(
      (*service)[iota::types::CONF_FILE_SERVICE.c_str()].GetString());
  name.append(":");
  name.append(
      (*service)[iota::types::CONF_FILE_SERVICE_PATH.c_str()].GetString());

  _services.insert(
      std::pair<std::string, boost::shared_ptr<JsonDocument> >(name, service));
}

const iota::JsonValue& iota::Configurator::Resource::get_service_by_apikey(
    const std::string& apikey) {
  std::map<std::string, boost::shared_ptr<JsonDocument> >::iterator it =
      _services.begin();
  while (it != _services.end()) {
    boost::shared_ptr<JsonDocument> d = it->second;
    if ((*d).HasMember(iota::types::CONF_FILE_APIKEY.c_str()) &&
        (*d)[iota::types::CONF_FILE_APIKEY.c_str()].IsString()) {
      std::string apk((*d)[iota::types::CONF_FILE_APIKEY.c_str()].GetString());
      if (apk.compare(apikey) == 0) {
        return *(it->second);
      }
    }
    ++it;
  }

  std::string text_error;
  text_error.append("Service by ApiKey: not found");
  text_error.append("[");
  text_error.append(apikey);
  text_error.append("]");
  throw iota::IotaException(iota::types::RESPONSE_MESSAGE_NO_SERVICE,
                            text_error, iota::types::RESPONSE_CODE_NO_SERVICE);
}

/** return all service paths fron service
  */
void iota::Configurator::Resource::get_all_servicepath(
    const std::string& service, std::vector<std::string>& vsrvpath) {
  std::map<std::string, boost::shared_ptr<JsonDocument> >::iterator iter;

  std::string key;
  std::string strCompa = service;
  strCompa.append(":");
  std::string service_path;

  for (iter = _services.begin(); iter != _services.end(); ++iter) {
    key = iter->first;
    if (boost::starts_with(key, strCompa)) {
      boost::shared_ptr<JsonDocument> my_srv = iter->second;
      service_path =
          (*my_srv)[iota::store::types::SERVICE_PATH.c_str()].GetString();
      vsrvpath.push_back(service_path);
    }
  }
  return;
}

/** no service path
  * return a sevice with name if there is only one
  * if there are several, return error
  * if there is no, error
  */
const iota::JsonValue& iota::Configurator::Resource::get_service(
    const std::string& service) {
  std::map<std::string, boost::shared_ptr<JsonDocument> >::iterator iter;
  std::map<std::string, boost::shared_ptr<JsonDocument> >::iterator found =
      _services.end();

  std::string key;
  std::string strCompa = service;
  strCompa.append(":");

  for (iter = _services.begin(); iter != _services.end(); ++iter) {
    key = iter->first;
    if (boost::starts_with(key, strCompa)) {
      if (found != _services.end()) {
        std::string text_error;
        text_error.append("Service : more than one");
        text_error.append(" ");
        text_error.append(service);
        text_error.append(" ");
        throw iota::IotaException(
            iota::types::RESPONSE_MESSAGE_MORE_ONE_SERVICE, text_error,
            iota::types::RESPONSE_CODE_MORE_ONE_SERVICE);
      } else {
        found = iter;
      }
    }
  }

  if (found != _services.end()) {
    return *(found->second);
  }

  std::string text_error;
  text_error.append(" ");
  text_error.append(service);
  text_error.append(" ");
  text_error.append("/#");
  text_error.append(" ");
  throw iota::IotaException(iota::types::RESPONSE_MESSAGE_NO_SERVICE,
                            text_error, iota::types::RESPONSE_CODE_NO_SERVICE);
}

const iota::JsonValue& iota::Configurator::Resource::get_service(
    const std::string& service, const std::string& service_path) {
  if (service_path.empty()) {
    return get_service(service);
  }

  std::string name = service;
  name.append(":");
  name.append(service_path);
  std::map<std::string, boost::shared_ptr<JsonDocument> >::iterator it;
  it = _services.find(name);
  if (it != _services.end()) {
    return *(it->second);
  }

  std::string text_error;
  text_error.append("Service not found");
  text_error.append("[service:");
  text_error.append(service);
  text_error.append(" service_path:");
  text_error.append(service_path);
  text_error.append("]");
  throw iota::IotaException(iota::types::RESPONSE_MESSAGE_NO_SERVICE,
                            text_error, iota::types::RESPONSE_CODE_NO_SERVICE);
}

const iota::JsonValue& iota::Configurator::Resource::get_value() {
  // If already built
  if (!_doc.IsNull()) {
    return _doc;
  }

  // Not built
  _doc.SetObject();
  // First-level fields
  rapidjson::StringBuffer b_root;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> w_root(b_root);
  (*_root_fields).Accept(w_root);
  JsonDocument d;
  d.SetObject();
  d.Parse<0>(b_root.GetString());

  for (iota::JsonValue::MemberIterator itr1 = d.MemberBegin();
       itr1 != d.MemberEnd(); itr1++) {
    std::string name(itr1->name.GetString());
    iota::JsonValue v(rapidjson::kObjectType);
    v.SetString(itr1->name.GetString(), itr1->name.GetStringLength(),
                _doc.GetAllocator());
    _doc.AddMember(v, itr1->value, _doc.GetAllocator());
  }

  // Services
  std::map<std::string, boost::shared_ptr<JsonDocument> >::iterator it =
      _services.begin();
  iota::JsonValue va(rapidjson::kArrayType);
  while (it != _services.end()) {
    rapidjson::StringBuffer b_service;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w_service(b_service);
    (*(it->second)).Accept(w_service);
    JsonDocument d;
    d.SetObject();
    d.Parse<0>(b_service.GetString());
    iota::JsonValue obj;
    obj.SetObject();
    for (iota::JsonValue::MemberIterator itr2 = d.MemberBegin();
         itr2 != d.MemberEnd(); itr2++) {
      std::string name(itr2->name.GetString());
      iota::JsonValue v(rapidjson::kObjectType);
      v.SetString(itr2->name.GetString(), itr2->name.GetStringLength(),
                  _doc.GetAllocator());
      obj.AddMember(v, itr2->value, _doc.GetAllocator());
    }

    va.PushBack(obj, _doc.GetAllocator());
    ++it;
  }
  _doc.AddMember(iota::types::CONF_FILE_SERVICES.c_str(), va,
                 _doc.GetAllocator());
  return _doc;
}

void iota::Configurator::Resource::add_fields(
    boost::shared_ptr<JsonDocument> fl_fields) {
  _root_fields = fl_fields;
}

std::string iota::Configurator::Resource::get_string() {
  const iota::JsonValue& v = get_value();
  rapidjson::StringBuffer b_resource;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> w_resource(b_resource);
  v.Accept(w_resource);
  return b_resource.GetString();
}

std::multimap<std::string, iota::PepRule>& iota::Configurator::get_pep_rules() {
  return _pep_rules;
}

std::map<unsigned int, std::string> iota::Configurator::createMessagesMap() {
  std::map<unsigned int, std::string> messages;
  messages[pion::http::types::RESPONSE_CODE_OK] =
      pion::http::types::RESPONSE_MESSAGE_OK;
  messages[pion::http::types::RESPONSE_CODE_CREATED] =
      pion::http::types::RESPONSE_MESSAGE_CREATED;
  messages[pion::http::types::RESPONSE_CODE_ACCEPTED] =
      pion::http::types::RESPONSE_MESSAGE_ACCEPTED;
  messages[pion::http::types::RESPONSE_CODE_NO_CONTENT] =
      pion::http::types::RESPONSE_MESSAGE_NO_CONTENT;
  messages[pion::http::types::RESPONSE_CODE_FOUND] =
      pion::http::types::RESPONSE_MESSAGE_FOUND;
  messages[pion::http::types::RESPONSE_CODE_UNAUTHORIZED] =
      pion::http::types::RESPONSE_MESSAGE_UNAUTHORIZED;
  messages[pion::http::types::RESPONSE_CODE_FORBIDDEN] =
      pion::http::types::RESPONSE_MESSAGE_FORBIDDEN;
  messages[pion::http::types::RESPONSE_CODE_NOT_FOUND] =
      pion::http::types::RESPONSE_MESSAGE_NOT_FOUND;
  messages[pion::http::types::RESPONSE_CODE_METHOD_NOT_ALLOWED] =
      pion::http::types::RESPONSE_MESSAGE_METHOD_NOT_ALLOWED;
  messages[pion::http::types::RESPONSE_CODE_NOT_MODIFIED] =
      pion::http::types::RESPONSE_MESSAGE_NOT_MODIFIED;
  messages[pion::http::types::RESPONSE_CODE_BAD_REQUEST] =
      pion::http::types::RESPONSE_MESSAGE_BAD_REQUEST;
  messages[pion::http::types::RESPONSE_CODE_SERVER_ERROR] =
      pion::http::types::RESPONSE_MESSAGE_SERVER_ERROR;
  messages[pion::http::types::RESPONSE_CODE_NOT_IMPLEMENTED] =
      pion::http::types::RESPONSE_MESSAGE_NOT_IMPLEMENTED;
  messages[pion::http::types::RESPONSE_CODE_CONTINUE] =
      pion::http::types::RESPONSE_MESSAGE_CONTINUE;
  messages[409] = "Conflict";
  return messages;
}
