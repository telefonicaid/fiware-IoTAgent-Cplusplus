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
#include <boost/algorithm/string.hpp>

iota::Service::Service() : Timer() {
  _document.reset(new JsonDocument());
  _document->SetObject();
}

iota::Service::Service(const std::string& service) : Timer() {
  _document.reset(new JsonDocument());
  _document->SetObject();
  set_service(service);
}

iota::Service::~Service(void) {}

boost::shared_ptr<iota::JsonDocument> iota::Service::get_document() {
  return _document;
}

void iota::Service::put_attribute(const std::string& object_id,
                                  const std::string& type,
                                  const std::string& name) {
  iota::JsonValue nameH(rapidjson::kObjectType);
  nameH.SetString("name");
  iota::JsonValue nameV(rapidjson::kObjectType);
  nameV.SetString(name.c_str(), name.size(), _document->GetAllocator());
  iota::JsonValue typeH(rapidjson::kObjectType);
  typeH.SetString("type");
  iota::JsonValue typeV(rapidjson::kObjectType);
  typeV.SetString(type.c_str(), type.size(), _document->GetAllocator());
  iota::JsonValue object_idH(rapidjson::kObjectType);
  object_idH.SetString("object_id");
  iota::JsonValue object_idV(rapidjson::kObjectType);
  object_idV.SetString(object_id.c_str(), object_id.size(),
                       _document->GetAllocator());

  iota::JsonValue tupla(rapidjson::kObjectType);
  tupla.AddMember(nameH, nameV, _document->GetAllocator());
  tupla.AddMember(typeH, typeV, _document->GetAllocator());
  tupla.AddMember(object_idH, object_idV, _document->GetAllocator());

  if (_document->HasMember("attributes")) {
    (*_document)["attributes"].PushBack(tupla, _document->GetAllocator());
  } else {
    iota::JsonValue arr(rapidjson::kArrayType);
    arr.PushBack(tupla, _document->GetAllocator());
    iota::JsonValue attH(rapidjson::kObjectType);
    attH.SetString("attributes");
    _document->AddMember(attH, arr, _document->GetAllocator());
  }
}

void iota::Service::put_static_attribute(const std::string& name,
                                         const std::string& type,
                                         const std::string& value) {
  iota::JsonValue nameH(rapidjson::kObjectType);
  nameH.SetString("name");
  iota::JsonValue nameV(rapidjson::kObjectType);
  nameV.SetString(name.c_str(), name.size(), _document->GetAllocator());
  iota::JsonValue typeH(rapidjson::kObjectType);
  typeH.SetString("type");
  iota::JsonValue typeV(rapidjson::kObjectType);
  typeV.SetString(type.c_str(), type.size(), _document->GetAllocator());
  iota::JsonValue valueH(rapidjson::kObjectType);
  valueH.SetString("value");
  iota::JsonValue valueV(rapidjson::kObjectType);
  valueV.SetString(value.c_str(), value.size(), _document->GetAllocator());

  iota::JsonValue tupla(rapidjson::kObjectType);
  tupla.AddMember(nameH, nameV, _document->GetAllocator());
  tupla.AddMember(typeH, typeV, _document->GetAllocator());
  tupla.AddMember(valueH, valueV, _document->GetAllocator());

  if (_document->HasMember("static_attributes")) {
    (*_document)["static_attributes"].PushBack(tupla,
                                               _document->GetAllocator());
  } else {
    iota::JsonValue arr(rapidjson::kArrayType);
    arr.PushBack(tupla, _document->GetAllocator());
    iota::JsonValue attH(rapidjson::kObjectType);
    attH.SetString("static_attributes");
    _document->AddMember(attH, arr, _document->GetAllocator());
  }
}

void iota::Service::put_metadata(const std::string& name,
                                 const std::string& type,
                                 const std::string& value) {
  iota::JsonValue nameH(rapidjson::kObjectType);
  nameH.SetString("name");
  iota::JsonValue nameV(rapidjson::kObjectType);
  nameV.SetString(name.c_str(), name.size(), _document->GetAllocator());
  iota::JsonValue typeH(rapidjson::kObjectType);
  typeH.SetString("type");
  iota::JsonValue typeV(rapidjson::kObjectType);
  typeV.SetString(type.c_str(), type.size(), _document->GetAllocator());
  iota::JsonValue valueH(rapidjson::kObjectType);
  valueH.SetString("value");
  iota::JsonValue valueV(rapidjson::kObjectType);
  valueV.SetString(value.c_str(), value.size(), _document->GetAllocator());

  iota::JsonValue tupla(rapidjson::kObjectType);
  tupla.AddMember(nameH, nameV, _document->GetAllocator());
  tupla.AddMember(typeH, typeV, _document->GetAllocator());
  tupla.AddMember(valueH, valueV, _document->GetAllocator());

  if (_document->HasMember("metadatas")) {
    (*_document)["metadatas"].PushBack(tupla, _document->GetAllocator());
  } else {
    iota::JsonValue arr(rapidjson::kArrayType);
    arr.PushBack(tupla, _document->GetAllocator());
    iota::JsonValue attH(rapidjson::kObjectType);
    attH.SetString("metadatas");
    _document->AddMember(attH, arr, _document->GetAllocator());
  }
}

std::string iota::Service::get(const std::string& field,
                               const std::string& default_value) {
  iota::JsonValue& doc = *_document;
  return get(field, default_value, doc);
}

int iota::Service::get(const std::string& field, int default_value) {
  iota::JsonValue& doc = *_document;
  return get(field, default_value, doc);
}

std::string iota::Service::get(const std::string& field,
                               const std::string& default_value,
                               const iota::JsonValue& obj) {
  std::string result;
  if (!obj.IsNull() && obj.IsObject() && obj.HasMember(field.c_str())) {
    if (obj[field.c_str()].IsString()){
       result.assign(obj[field.c_str()].GetString());
    }else{
       result = default_value;
    }
  } else {
    result.assign(default_value);
  }

  return result;
}

int iota::Service::get(const std::string& field, int default_value,
                       const iota::JsonValue& obj) {
  int result;
  if (!obj.IsNull() && obj.IsObject() && obj.HasMember(field.c_str())) {
    if (obj[field.c_str()].IsNumber()){
       result = obj[field.c_str()].GetInt();
    }else{
       result = default_value;
    }
  } else {
    result = default_value;
  }

  return result;
}

iota::JsonValue& iota::Service::getObject(const std::string& field) {
  int result;
  if (_document->HasMember(field.c_str())) {
    return (*_document)[field.c_str()];
  } else {
    static iota::JsonValue nullValue;
    return nullValue;
  }
}

iota::JsonValue& iota::Service::getObject(const std::string& field,
                                          iota::JsonValue& obj) {
  int result;
  if (!obj.IsNull() && obj.HasMember(field.c_str())) {
    return obj[field.c_str()];
  } else {
    static iota::JsonValue nullValue;
    return nullValue;
  }
}

void iota::Service::put(const std::string& resource_name,
                        const std::string& value, iota::JsonValue& obj) {
  iota::JsonValue v;
  v.SetString(value.c_str(), value.size(), _document->GetAllocator());
  iota::JsonValue k;
  k.SetString(resource_name.c_str(), resource_name.size(),
              _document->GetAllocator());
  if (!obj.IsNull() && obj.IsObject()) {
    obj.AddMember(k, v, _document->GetAllocator());
  }
}

void iota::Service::put(const std::string& field,
                        const std::string& value) {

  iota::JsonValue& doc = *_document;
  return put(field, value, doc);

}

void iota::Service::put(const std::string& field, int value) {
  iota::JsonValue& doc = *_document;
  return put(field, value, doc);
}

void iota::Service::put(const std::string& field, int value,
                        iota::JsonValue& obj) {
  iota::JsonValue v;
  v.SetInt(value);
  iota::JsonValue k;
  k.SetString(field.c_str(), field.size(),
              _document->GetAllocator());
  if (!obj.IsNull() && obj.IsObject()) {
    obj.AddMember(k, v, _document->GetAllocator());
  }
}

void iota::Service::putObject(const std::string& field) {

  iota::JsonValue& doc = *_document;
  return putObject(field, doc);

}

void iota::Service::putObject(const std::string& resource_name,
                              iota::JsonValue& obj) {
  iota::JsonValue v;
  v.SetObject();
  iota::JsonValue k;
  k.SetString(resource_name.c_str(), resource_name.size(),
              _document->GetAllocator());
  if (!obj.IsNull() && obj.IsObject()) {
    obj.AddMember(k, v, _document->GetAllocator());
  }
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
  if (!_document->IsNull()) {
    rapidjson::StringBuffer buffer_doc;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer_doc(buffer_doc);
    _document->Accept(writer_doc);
    return buffer_doc.GetString();
  }
}

std::string iota::Service::toString(const iota::JsonValue& obj) const {
  if (!obj.IsNull()) {
    rapidjson::StringBuffer buffer_doc;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer_doc(buffer_doc);
    obj.Accept(writer_doc);
    return buffer_doc.GetString();
  }
}

std::string iota::Service::read_json(std::stringstream& _is) {
  std::string error;
  if (_document->Parse<0>(_is.str().c_str()).HasParseError()) {
    std::ostringstream what;
    what << "Configurator: ";
    what << _document->GetParseError();
    what << "[";
    what << _document->GetErrorOffset();
    what << "]";
    error = what.str();
    _document->SetObject();
  }

  return error;
}
