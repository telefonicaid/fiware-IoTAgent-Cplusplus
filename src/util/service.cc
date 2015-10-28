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
#include "service.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filestream.h>
#include <rapidjson/prettywriter.h>

iota::Service::Service() : Timer() { _document.SetObject(); }

iota::Service::Service(const std::string& service) : Timer() {
  _document.SetObject();
  set_service(service);
}

iota::Service::~Service(void) {}

std::string iota::Service::get(const std::string& field,
                               const std::string& default_value) {
  std::string result;
  if (_document.HasMember(field.c_str())) {
    result.assign(_document[field.c_str()].GetString());
  } else {
    result.assign(default_value);
  }

  return result;
}

// TODO
// añade el json a un campo array indicado por resource_name
void iota::Service::add_json(std::string resource_name, std::string json,
                             std::string& error) {
  /* iota::JsonValue contact(kObject);
   contact.AddMember("name", "Milo", document.GetAllocator());

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
   }*/
}

void iota::Service::put(const std::string& resource_name,
                        const std::string& value) {
  iota::JsonValue v;
  v.SetString(value.c_str(), value.size(), _document.GetAllocator());
  _document.AddMember(resource_name.c_str(), v, _document.GetAllocator());
}

int iota::Service::get(const std::string& field, int default_value) {
  int result;
  if (_document.HasMember(field.c_str())) {
    result = _document[field.c_str()].GetInt();
  } else {
    result = default_value;
  }

  return result;
}

void iota::Service::put(const std::string& resource_name, int value) {
  iota::JsonValue v;
  v.SetInt(value);
  _document.AddMember(resource_name.c_str(), v, _document.GetAllocator());
}

std::string iota::Service::get_resource() { return _resource; };

void iota::Service::set_resource(const std::string& resource) {
  _resource = resource;
}

std::string iota::Service::get_token() { return _token; };

void iota::Service::set_token(const std::string& token) { _token = token; }

std::string iota::Service::get_service_path() { return _service_path; };

void iota::Service::set_service_path(const std::string& service_path) {
  _service_path = service_path;
  put("service_path", service_path);
}

int iota::Service::get_timeout() { return _timeout; };

void iota::Service::set_timeout(int timeout) { _timeout = timeout; }

std::string iota::Service::get_service() { return _service; };

void iota::Service::set_service(const std::string& service) {
  _service = service;
  put("service", service);
}

std::string iota::Service::get_cbroker() { return _cbroker; };

void iota::Service::set_cbroker(const std::string& cbroker) {
  _cbroker = cbroker;
}

std::string iota::Service::get_real_name() const { return _service; }

void iota::Service::read_xml_file(const std::string& file_path) {}

std::string iota::Service::toString() const {
  if (!_document.IsNull()) {
    rapidjson::StringBuffer buffer_doc;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer_doc(buffer_doc);
    _document.Accept(writer_doc);
    return buffer_doc.GetString();
  }
}

std::string iota::Service::read_json(std::stringstream& _is) {
  std::string error;
  if (_document.Parse<0>(_is.str().c_str()).HasParseError()) {
    std::ostringstream what;
    what << "Configurator: ";
    what << _document.GetParseError();
    what << "[";
    what << _document.GetErrorOffset();
    what << "]";
    error = what.str();
  }

  return error;
}
