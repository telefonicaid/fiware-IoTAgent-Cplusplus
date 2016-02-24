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
#include "ContextElement.h"
#include "ngsi_const.h"
#include "util/store_const.h"
#include "util/json_parser.hpp"
#include "util/RiotISO8601.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <boost/foreach.hpp>
#include <stdexcept>

iota::ContextElement::ContextElement(
    const std::istringstream& str_context_element) {
  rapidjson::Document document;
  char buffer[str_context_element.str().length()];
  strcpy(buffer, str_context_element.str().c_str());
  if (document.Parse<0>(buffer).HasParseError()) {
    std::ostringstream what;
    what << "ContextElement: ";
    what << document.GetParseError();
    what << "[";
    what << document.GetErrorOffset();
    what << "]";
    throw std::runtime_error(what.str());
  }

  if (document.HasMember(iota::ngsi::NGSI_ID.c_str()) == false) {
    std::ostringstream what;
    what << "ContextElement: ";
    what << "missing field [";
    what << "id]";
    throw std::runtime_error(what.str());
  }

  if (document[iota::ngsi::NGSI_ID.c_str()].IsString()) {
    _id.assign(iota::render_identifier(document[iota::ngsi::NGSI_ID.c_str()].GetString()));
  }
  if ((document.HasMember(iota::ngsi::NGSI_TYPE.c_str())) &&
      (document[iota::ngsi::NGSI_TYPE.c_str()].IsString())) {
    _type.assign(iota::render_identifier(document[iota::ngsi::NGSI_TYPE.c_str()].GetString()));
  }
  if ((document.HasMember(iota::ngsi::NGSI_ISPATTERN.c_str())) &&
      (document[iota::ngsi::NGSI_ISPATTERN.c_str()].IsString())) {
    _is_pattern.assign(
        document[iota::ngsi::NGSI_ISPATTERN.c_str()].GetString());
  }

  if (document.HasMember(iota::ngsi::NGSI_ATTRIBUTES.c_str()) &&
      document[iota::ngsi::NGSI_ATTRIBUTES.c_str()].IsArray()) {
    const rapidjson::Value& attributes =
        document[iota::ngsi::NGSI_ATTRIBUTES.c_str()];
    if (attributes.IsArray()) {
      for (rapidjson::SizeType i = 0; i < attributes.Size(); i++) {
        iota::Attribute attribute(attributes[i]);
        add_attribute(attribute);
      }
    } else {
      std::ostringstream what;
      what << "ContextElement: ";
      what << "invalid type field [";
      what << "attributes]";
      throw std::runtime_error(what.str());
    }
  }
}

iota::ContextElement::ContextElement(const rapidjson::Value& context_element) {
  if (!context_element.IsObject()) {
    throw std::runtime_error("Invalid Object");
  }
  if (context_element.HasMember(iota::ngsi::NGSI_ID.c_str()) == false) {
    std::ostringstream what;
    what << "ContextElement: ";
    what << "missing field [";
    what << "id]";
    throw std::runtime_error(what.str());
  }

  if (context_element[iota::ngsi::NGSI_ID.c_str()].IsString()) {
    _id.assign(iota::render_identifier(context_element[iota::ngsi::NGSI_ID.c_str()].GetString()));
  }
  if ((context_element.HasMember(iota::ngsi::NGSI_TYPE.c_str())) &&
      (context_element[iota::ngsi::NGSI_TYPE.c_str()].IsString())) {
    _type.assign(iota::render_identifier(context_element[iota::ngsi::NGSI_TYPE.c_str()].GetString()));
  }
  if ((context_element.HasMember(iota::ngsi::NGSI_ISPATTERN.c_str())) &&
      (context_element[iota::ngsi::NGSI_ISPATTERN.c_str()].IsString())) {
    _is_pattern.assign(
        context_element[iota::ngsi::NGSI_ISPATTERN.c_str()].GetString());
  }

  if (context_element.HasMember(iota::ngsi::NGSI_ATTRIBUTES.c_str()) &&
      context_element[iota::ngsi::NGSI_ATTRIBUTES.c_str()].IsArray()) {
    const rapidjson::Value& attributes =
        context_element[iota::ngsi::NGSI_ATTRIBUTES.c_str()];
    if (attributes.IsArray()) {
      for (rapidjson::SizeType i = 0; i < attributes.Size(); i++) {
        iota::Attribute attribute(attributes[i]);
        add_attribute(attribute);
      }
    }
  }
};

std::string iota::ContextElement::get_string() {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  Serialize(writer);
  return buffer.GetString();
};

void iota::ContextElement::format() const {
  // Before stringfy, check environment info and modify if needed

  // Check service info
  // If device info, this information is taken.
  // If device info has entity_type, this info is taken.
  // If device info has not entity_type, service entity_type is taken.
  // If _type is defined in constructor, does not follow default.
  std::string service_entity_type = _service_info.get<std::string>(
      iota::store::types::ENTITY + "_" + iota::store::types::TYPE, "");

  std::string entity_type("thing");
  std::string entity_id(_id);
  if (_device_info.get() != NULL) {
    if (!_device_info->_entity_type.empty() &&
        _device_info->_entity_type.compare(iota::store::types::DEFAULT) != 0) {
      entity_type.assign(_device_info->_entity_type);
    } else {
      // Device has not entity type, default is used if context is not created
      // with specific entity_type
      if (!_type.empty()) {
        entity_type = _type;
      } else if (!service_entity_type.empty()) {
        entity_type = service_entity_type;
      }
    }
    // Entity name (default is entity_type:device_id)
    if (!_device_info->_entity_name.empty() &&
        _device_info->_entity_name.compare(iota::store::types::DEFAULT) != 0) {
      entity_id.assign(_device_info->_entity_name);
    } else {
      // Device has not entity_name, default is used if _type is not defined
      // in constructor
      if (_type.empty()) {
        entity_id.assign(entity_type + ":" + _device_info->_name);
      }
    }

  } else {
    // No device info
    if (!_type.empty()) {
      entity_type = _type;
    } else if (!service_entity_type.empty()) {
      entity_type = service_entity_type;
    }
    if (_type.empty()) {
      entity_id.assign(entity_type + ":" + _id);
    }
  }
  _type.assign(entity_type);
  _id.assign(entity_id);
};

void iota::ContextElement::add_attribute(iota::Attribute& attribute) {
  // If value is empty, this attribute is not published
  // Type compound use other method (to review)
  bool empty_value = false;

  // Sanity. If type is empty, default is string
  if (attribute.get_type().empty()) {
    std::string default_type("string");
    attribute.set_type(default_type);
  }

  if (attribute.get_type().compare("compound") == 0) {
    if (attribute.compound_object() == false) {
      empty_value = attribute.get_compound_value().empty();
    } else {
      empty_value = attribute.get_value().empty();
    }
  } else {
    empty_value = attribute.get_value().empty();
  }
  if (empty_value == false) {
    // Check if attribute has user mapping. This mapping could be in service
    // configuration
    std::string attr_mapping;
    if (_device_info.get() != NULL) {
      attr_mapping = _device_info->get_attribute(attribute.get_name());
    }
    if (attr_mapping.empty()) {
      try {
        boost::property_tree::ptree::const_iterator it =
            _service_info.get_child(iota::store::types::ATTRIBUTES).begin();
        while (
            attr_mapping.empty() &&
            it !=
                _service_info.get_child(iota::store::types::ATTRIBUTES).end()) {
          if (it->second.get<std::string>(iota::store::types::ATTRIBUTE_ID, "")
                  .compare(attribute.get_name()) == 0) {
            std::stringstream os_json;
            iota::property_tree::json_parser::write_json(os_json, it->second);
            attr_mapping = os_json.str();
          }
          ++it;
        }
      } catch (boost::property_tree::ptree_bad_path& e) {
        // Nothing
      }
    }
    if (!attr_mapping.empty()) {
      rapidjson::Document d;

      if (!d.Parse<0>(attr_mapping.c_str()).HasParseError()) {
        iota::Attribute att(d);
        std::string type = att.get_type();
        // Compound: special type with value no string
        if (type.empty() || attribute.get_type().compare("compound") == 0) {
          type = attribute.get_type();
        }
        if (type.empty()) {
          // default value
          type = "string";
        }

        iota::Attribute mapped_attr(att.get_name(), type, "");
        if (type.compare("compound") == 0) {
          if (attribute.compound_object() == false) {
            std::vector<iota::Attribute> compound_value =
              attribute.get_compound_value();
            for (int i = 0; i < compound_value.size(); i++) {
              mapped_attr.add_value_compound(compound_value[i]);
            }
          } else {
            mapped_attr.set_value(attribute.get_value());
          }
 
        } else if (type.compare(iota::store::types::COORDS) == 0) {
          std::string valueSTR = attribute.get_value();
          std::replace(valueSTR.begin(), valueSTR.end(), '/', ',');
          mapped_attr.set_value(valueSTR);
          // add metadata
          iota::Attribute metadata_loc("location", "string", "WGS84");
          mapped_attr.add_metadata(metadata_loc);
        } else {
          mapped_attr.set_value(attribute.get_value());
        }
        std::vector<iota::Attribute> metadata = attribute.get_metadatas();
        int i = 0;
        for (i = 0; i < metadata.size(); i++) {
          mapped_attr.add_metadata(metadata[i]);
        }
        for (i = 0; i < att.get_metadatas().size(); i++) {
          mapped_attr.add_metadata(att.get_metadatas()[i]);
        }
        add_attribute(mapped_attr);
      } else {
        // TODO Only log, exception, what??????
        // std::cout << "ERROR PARSE ATTR" << std::endl;
      }
    } else {
      _attributes.push_back(attribute);
    }
  }
};

void iota::ContextElement::set_env_info(
    boost::property_tree::ptree service_info,
    boost::shared_ptr<Device> device) {
  _service_info = service_info;
  _device_info = device;
  // Format type and id
  format();
  iota::RiotISO8601 mi_hora;
  std::string timestamp = mi_hora.toUTC().toString();
  // Static Attributes
  // Add static attributes. These attributes are either in device or in service
  if (_device_info.get() != NULL &&
      _device_info->_static_attributes.size() > 0) {
    std::map<std::string, std::string>::iterator it =
        _device_info->_static_attributes.begin();
    while (it != _device_info->_static_attributes.end()) {
      rapidjson::Document d;
      if (!d.Parse<0>(it->second.c_str()).HasParseError()) {
        iota::Attribute att(d);
        iota::Attribute metadata("TimeInstant", "ISO8601", timestamp);
        att.add_metadata(metadata);
        add_attribute(att);
      }
      ++it;
    }
  }

  try {
    BOOST_FOREACH (
        boost::property_tree::ptree::value_type& v,
        _service_info.get_child(iota::store::types::STATIC_ATTRIBUTES)) {
      std::string a_name = v.second.get<std::string>(iota::ngsi::NGSI_NAME, "");
      if (!a_name.empty() && !exists(a_name)) {
        // Add attribute.

        std::string a_type =
            v.second.get<std::string>(iota::ngsi::NGSI_TYPE, "string");
        std::string a_value =
            v.second.get<std::string>(iota::ngsi::NGSI_VALUE, "");
        if (!a_value.empty()) {
          std::stringstream os_json;
          iota::property_tree::json_parser::write_json(os_json, v.second);
          rapidjson::Document d;
          if (!d.Parse<0>(os_json.str().c_str()).HasParseError()) {
            iota::Attribute att(d);
            iota::Attribute metadata("TimeInstant", "ISO8601", timestamp);
            att.add_metadata(metadata);
            add_attribute(att);
          }
        }
      }
    }
  } catch (boost::property_tree::ptree_bad_path& e) {
    // Nothing
  }
}

bool iota::ContextElement::exists(std::string& name) {
  bool exists = false;
  std::vector<Attribute>::const_iterator it = _attributes.begin();
  while (!exists && it != _attributes.end()) {
    std::string n = ((iota::Attribute)*it).get_name();
    if (n.compare(name) == 0) {
      exists = true;
    }
    ++it;
  }
  return exists;
}
