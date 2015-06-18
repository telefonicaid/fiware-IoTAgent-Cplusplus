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
#include "sp_service.h"
#include "services/admin_service.h"
#include <pion/http/response_writer.hpp>
#include <sstream>
#include <rest/riot_conf.h>
#include <util/iot_url.h>
#include "util/KVP.h"
#include "util/FuncUtil.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include "ngsi/UpdateContext.h"
#include "rest/iot_cb_comm.h"
#include <pion/http/response_writer.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
#include "ultra_light/ULInsertObservation.h"
#include "ngsi/ContextRegistration.h"
#include "ngsi/RegisterContext.h"
#include "ngsi/RegisterResponse.h"

#include "util/dev_file.h"
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#include <pion/tcp/server.hpp>
#include "boost/format.hpp"

namespace iota {
extern std::string logger;
}
extern iota::AdminService* AdminService_ptr;


iota::SPService::SPService(): m_logger(PION_GET_LOGGER(iota::logger)) {
  IOTA_LOG_DEBUG(m_logger, "iota::SPService::SPService");
}

iota::SPService::~SPService() {
  IOTA_LOG_DEBUG(m_logger, "Destructor iota::SPService::SPService");
}

void iota::SPService::start() {
  IOTA_LOG_DEBUG(m_logger, "START PLUGIN SPService");

  std::map<std::string, std::string> filters;
  add_url("", filters, REST_HANDLE(&iota::SPService::service), this);
  // Get ngsi service.
  iota::RestHandle* ngsi_service = NULL;
  if (AdminService_ptr != NULL) {
    ngsi_service = AdminService_ptr->get_service("/iot/ngsi");
  }
  if (ngsi_service != NULL) {
    std::string ngsi_operation(get_resource());
    ngsi_operation.append("/<operation>");
    ngsi_service->add_url(ngsi_operation,
                          filters,
                          REST_HANDLE(&iota::SPService::op_ngsi), this);

    std::string my_resource = ngsi_service->get_my_url_base();
    std::string saux = ngsi_service->get_my_url_base();
    size_t pos = my_resource.find("/<operation>");
    if (pos != std::string::npos) {
      my_resource = saux.substr(0,pos);
    }
  }
}

iota::ProtocolData iota::SPService::get_protocol_data() {
  iota::ProtocolData p_d;
  p_d.protocol = "HOLA";
  p_d.description = "ADIOS";
  return p_d;
}

void iota::SPService::op_ngsi(pion::http::request_ptr& http_request_ptr,
                              std::map<std::string, std::string>& url_args,
                              std::multimap<std::string, std::string>& query_parameters,
                              pion::http::response& http_response, std::string& response) {

  default_op_ngsi(http_request_ptr, url_args, query_parameters,
                  http_response, response);

}

void iota::SPService::service(pion::http::request_ptr& http_request_ptr,
                              std::map<std::string, std::string>& url_args,
                              std::multimap<std::string, std::string>& query_parameters,
                              pion::http::response& http_response, std::string& response) {

  IOTA_LOG_DEBUG(m_logger, "service");
  int code_resp = pion::http::types::RESPONSE_CODE_OK;

  std::string apikey = "apikey1";
  std::string device = "dev1";
  std::string content = "body";
  std::string method = "GET";
  boost::property_tree::ptree service_ptree;
  std::string entity_type;


  //check if this key exists
  get_service_by_apiKey(service_ptree, apikey);

  //check if device exists
  boost::shared_ptr<Device> dev = get_device(device,
                                  service_ptree.get<std::string>("service", ""));
  if (dev.get() == NULL) {
    IOTA_LOG_DEBUG(m_logger, "Device "  << device << "is not registered;"
                   << apikey << ":" << service_ptree.get<std::string>("service", ""));
    entity_type = service_ptree.get<std::string>("entity_type", "");
  }
  else {
    device = dev->get_real_name(service_ptree);
    entity_type = dev->_entity_type;
    if (entity_type.empty()) {
      entity_type = service_ptree.get<std::string>("entity_type", "");
    }
  }


  std::vector<iota::ContextElement> cb_eltos;

  if (content.empty() == false) {

    std::string command_http_response_translate, id_command;
    if (isCommandResp(content, 2000, command_http_response_translate,
                      id_command) == -1) {
      std::string srv = service_ptree.get<std::string>("service", "");

      //fill context_elements wuth the device information

      for (int ii=0; ii < cb_eltos.size(); ii++) {
        std::string cb_response;
        code_resp = send(cb_eltos[ii], "APPEND", service_ptree, cb_response);
        IOTA_LOG_DEBUG(m_logger, "response cb "<< cb_response);
      }
    }
    else {
      IOTA_LOG_DEBUG(m_logger, "han mandado una respuesta a un comando " <<
                     id_command << "->" <<command_http_response_translate);


      response_command(id_command, command_http_response_translate, dev,
                       service_ptree);

    }


  }


  ///vamos a ver si hay que ir a por comandos
  CommandVect cmdPtes;
  if (method.compare("GET") == 0) {
    cmdPtes = get_all_command(dev, service_ptree);
    IOTA_LOG_DEBUG(m_logger,  "N. commands " << cmdPtes.size());
  }


  if (code_resp != pion::http::types::RESPONSE_CODE_OK) {
    if (code_resp == 503) {
      //io_ul_idas->release_cache();
      http_response.set_status_message("Service Unavailable");
      http_response.set_status_code(503);
    }
    else if (code_resp == pion::http::types::RESPONSE_CODE_SERVER_ERROR) {
      http_response.set_status_message(
        pion::http::types::RESPONSE_MESSAGE_SERVER_ERROR);
      http_response.set_status_code(pion::http::types::RESPONSE_CODE_SERVER_ERROR);
    }
    else {
      http_response.set_status_message(
        pion::http::types::RESPONSE_MESSAGE_BAD_REQUEST);
      http_response.set_status_code(pion::http::types::RESPONSE_CODE_BAD_REQUEST);
    }
  }

  if ((!response.empty()) || (cmdPtes.size() > 0)) {
    std::string c_t("text/json; charset=UTF-8");
    http_response.set_content_type(c_t);

    std::string commandsSTR;
    CommandVect::const_iterator i;
    CommandPtr ptr;
    std::string body;
    for (CommandVect::iterator it_v=cmdPtes.begin(); it_v!=cmdPtes.end(); ++it_v) {
      ptr = *it_v;
      body = ptr->get_command().get(iota::store::types::BODY, "");
      commandsSTR.append(body);
      if (it_v != cmdPtes.end() - 1) {
        commandsSTR.append("#");
      }
    }

    response.append(commandsSTR);
  }
}


/**
  * en ul20 las respuesta de comandos son device@commando|resultado  by example  dev1@ping|pingOK
  * si es un comando separa id_command y command response
  */
int iota::SPService::isCommandResp(const std::string& str_command_resp,
                                   const int& cmd_code,
                                   std::string& command_response,
                                   std::string& id_command) {
  int resp = -1;
  std::string result;

  id_command.assign(get_id_command(str_command_resp));
  IOTA_LOG_DEBUG(m_logger,
                 "isCommandResp" << cmd_code << ":" << str_command_resp<< ":" <<id_command);
  if (id_command.empty() == false) {
    size_t p_id = str_command_resp.find_first_of("|");
    if (p_id!=std::string::npos) {
      result = str_command_resp.substr(p_id + 1);
    }
    else {
      // if there is not response, we put all body (id_command)
      result = str_command_resp;
    }
    resp = 200;

    command_response.append(result);
  }

  return resp;
};


int iota::SPService::execute_command(const std::string& destino,
                                const std::string& command_id,
                                const boost::property_tree::ptree& command_to_send,
                                int timeout,
                                const boost::shared_ptr<Device> &item_dev,
                                const boost::property_tree::ptree& service,
                                std::string& resp_cmd,
                                iota::HttpClient::application_callback_t callback){

  int codigo_respuesta = pion::http::types::RESPONSE_CODE_OK;
  int size_respuesta = 0;


  IOTA_LOG_DEBUG(m_logger, "execute the command");
  std::string response_from_device;

  std::string command_response;
  std::string id_command;
  std::string content_resp;
  int code_resp=200;
  isCommandResp(content_resp, code_resp, command_response, id_command);
  resp_cmd.assign(command_response);

  return (code_resp);
};



/**
 *  se queda con el primer campo que es el nombre del comando
 **/
std::string iota::SPService::get_id_command(std::string str_command) {
  size_t p_id = str_command.find_first_of("|");
  std::string id_comando = str_command.substr(0, p_id);
  if (id_comando.find_first_of("@") == std::string::npos) {
    id_comando.clear();
  }
  return (id_comando);
};



// creates new SPService objects
//

extern "C" PION_PLUGIN iota::SPService* pion_create_SPService(void) {
  return new iota::SPService();
}

/// destroys SPService objects

extern "C" PION_PLUGIN void pion_destroy_SPService(iota::SPService*
    service_ptr) {
  std::cout << "DESTROY" << std::endl;
  delete service_ptr;
}


