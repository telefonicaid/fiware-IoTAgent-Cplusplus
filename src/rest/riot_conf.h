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
#ifndef SRC_REST_RIOT_CONF_H_
#define SRC_REST_RIOT_CONF_H_

#include "rest/types.h"
#include <string>
#include <map>
#include <rapidjson/document.h>
#include "util/iota_logger.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <util/common.h>

namespace iota {

class Configurator {
 public:
  static Configurator* instance();
  static Configurator* initialize(const std::string& filename);
  static void release();

  std::string getHttpMessage(const unsigned int resource);

  // Get service object by apikey
  const JsonValue& getServicebyApiKey(const std::string& resource,
                                      const std::string& apikey);

  // Get service object by its name
  const JsonValue& getService(const std::string& resource,
                              const std::string& service,
                              const std::string& service_path);

  const JsonValue& get_service(const std::string& service);

  void getAllServicePath(const std::string& resource,
                         const std::string& service,
                         std::vector<std::string>& vsrvpath);

  // Get string with resource object
  std::string getResource(const std::string& resource);

  std::multimap<std::string, iota::PepRule>& get_pep_rules();

  // Gen resource object by its name
  const JsonValue& getResourceObject(const std::string& resource);

  // Only for first-level fields
  const JsonValue& get(const std::string& field);

  /**
    * Transform a Json object in first-level configuration into a std::string
   * map
    * @param Field in first level. This field only can contain simple fields
   * (string, boolean, number).
    * @param map<std::string, std::string> where transformation is stored. You
   * can get a value by to_map[<field>].
    */
  void get(const std::string& field,
           std::map<std::string, std::string>& to_map);

  std::string getAll();
  std::string read_file(std::stringstream& is);
  std::string update_conf(std::stringstream& is);
  std::string getError();
  bool hasError();

  void add_resource(std::string resource_name, std::string json,
                    std::string& error);

  int get_listen_port();
  void set_listen_port(int p);

  std::string get_listen_ip();
  void set_listen_ip(std::string ip);

  std::string get_iotagent_name();
  void set_iotagent_name(std::string name);

  std::string get_iotagent_identifier();
  void set_iotagent_identifier(std::string id);

 protected:
 private:
  class Resource {
   public:
    Resource(){};
    virtual ~Resource(){};
    void add_service(boost::shared_ptr<JsonDocument> service);
    void add_fields(boost::shared_ptr<JsonDocument> fields);
    const JsonValue& get_service_by_apikey(const std::string& apikey);

    void get_all_servicepath(const std::string& service,
                             std::vector<std::string>& vsrvpath);

    const JsonValue& get_service(const std::string& service);
    const JsonValue& get_service(const std::string& service,
                                 const std::string& service_path);
    const JsonValue& get(const std::string& field);
    std::string get_string();
    const JsonValue& get_value();

   protected:
   private:
    boost::shared_ptr<JsonDocument> _root_fields;
    std::map<std::string, boost::shared_ptr<JsonDocument> > _services;
    JsonDocument _doc;
  };
  boost::mutex m_mutex;
  boost::recursive_mutex m_mutex_document;
  pion::logger m_log;
  std::string _filename;
  std::string _error;
  int listen_port;
  std::string listen_ip;

  std::string iotagent_name;
  std::string iotagent_identifier;

  static Configurator* pinstance;

  JsonDocument _document;

  // Root fields
  boost::shared_ptr<JsonDocument> _root_fields;
  // Resources
  std::map<std::string, boost::shared_ptr<Resource> > _resources;

  // Pep Rules
  std::multimap<std::string, iota::PepRule> _pep_rules;

  std::map<unsigned int, std::string> http_message;

  Configurator();
  void set_filename(std::string filename) { _filename = filename; };

  std::map<unsigned int, std::string> createMessagesMap();
  void write();
  void reload();
  void init();

  std::string check_configuration_json(JsonDocument& config_doc);
  boost::shared_ptr<Resource> check_resource(
      boost::shared_ptr<JsonDocument> resource_obj, std::string& error);
  void check_service(boost::shared_ptr<JsonDocument> service_obj,
                     std::string& error);
  void remove_resource(std::string resource_name);
  void add_resource(boost::shared_ptr<Resource> res);
};
};
#endif
