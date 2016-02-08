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
#ifndef SRC_NGSI_CONTEXTELEMENT_H_
#define SRC_NGSI_CONTEXTELEMENT_H_

#include <ngsi/Attribute.h>
#include <rapidjson/rapidjson.h>
#include <boost/property_tree/ptree.hpp>
#include "util/device.h"
#include "util/FuncUtil.h"
#include <vector>
#include <string>
namespace iota {
class ContextElement {
 public:
  ContextElement(const std::string& id, const std::string& type,
                 const std::string& is_pattern)
      : _id(iota::render_identifier(id)), _type(iota::render_identifier(type)), _is_pattern(is_pattern){};
  ContextElement(const std::istringstream& str_context_element);
  ContextElement(const rapidjson::Value& context_element);
  ContextElement(){};
  ~ContextElement(){};
  std::string get_string();
  void add_attribute(Attribute& attribute);
  std::string& get_id() { return _id; };
  void set_id(const std::string& a_id) { _id = iota::render_identifier(a_id); };
  std::string& get_type() { return _type; };
  void set_type(const std::string& data) { _type = iota::render_identifier(data); };
  std::string& get_is_pattern() { return _is_pattern; };
  void set_is_pattern(const std::string& data) { _is_pattern = data; };
  std::vector<Attribute>& get_attributes() { return _attributes; };

  // Convenience function to set device properties and service properties
  // This information has precedence over user info.
  void set_env_info(boost::property_tree::ptree service_info,
                    boost::shared_ptr<Device> device);

  template <typename Writer>
  void Serialize(Writer& writer) const {
    // format();
    writer.StartObject();
    writer.String("id");
    writer.String(_id.c_str(), (rapidjson::SizeType)_id.length());
    writer.String("type");
    writer.String(_type.c_str(), (rapidjson::SizeType)_type.length());
    writer.String("isPattern");
    writer.String(_is_pattern.c_str(),
                  (rapidjson::SizeType)_is_pattern.length());
    if (_attributes.size() > 0) {
      writer.String("attributes");
      writer.StartArray();
      for (std::vector<Attribute>::const_iterator it = _attributes.begin();
           it != _attributes.end(); ++it) {
        it->Serialize(writer);
      }
      writer.EndArray();
    }
    writer.EndObject();
  };

 private:
  mutable std::string _id;
  mutable std::string _type;
  std::string _is_pattern;
  std::vector<Attribute> _attributes;
  boost::property_tree::ptree _service_info;
  boost::shared_ptr<Device> _device_info;
  bool exists(std::string& name);

  void format() const;
};
}
#endif
