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
#include "TTService.h"

#include <TDA.h>

#include <ESP_SBC_Command.h>
#include "services/admin_service.h"
#include <pion/http/response_writer.hpp>
#include <pion/algorithm.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "QueryContextWrapper.h"
#include "SearchResponse.h"
#include "DecodeTTJSON.h"

#include "util/iota_exception.h"

ESPLib* iota::esp::TTService::esplib_instance = NULL;

ESPLib* iota::esp::TTService::getESPLib() {
  if (TTService::esplib_instance == NULL) {
    esplib_instance = new ESPLib();
  }

  return esplib_instance;
}

iota::esp::TTService::TTService()
    : m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  std::cout << "esp::TTService " << std::endl;
  IOTA_LOG_DEBUG(m_logger, "iota::esp::TTService Running...  ");

  _protocol_data.description = "Thinking Things Protocol";
  _protocol_data.protocol = "PDI-IoTA-ThinkingThings";
  // iota::tt::TTCBPublisher* cbPubImpl = new iota::tt::TTCBPublisher();

  contextBrokerPub.reset(new iota::tt::TTCBPublisher());
  idSensor = -1;
  tt_post_ptr_ = new ESP_Postprocessor_TT();
  tt_post_ptr_->load_default_TTModules();
}

iota::esp::TTService::TTService(
    boost::shared_ptr<iota::tt::TTCBPublisher> cbPub)
    : m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  _protocol_data.description = "Thinking Things Protocol";
  _protocol_data.protocol = "PDI-IoTA-ThinkingThings";
  contextBrokerPub = cbPub;
  idSensor = -1;
  tt_post_ptr_ = new ESP_Postprocessor_TT();
  tt_post_ptr_->load_default_TTModules();
}

iota::esp::TTService::~TTService() {
  // dtor

  // contextBrokerPub.reset();
  iota::esp::TTService::getESPLib()->stopSensor(idSensor);

  iota::esp::TTService::getESPLib()->destroySensor(idSensor);
}

int iota::esp::TTService::initESPLib(std::string& pathToLog,
                                     std::string& sensorFile) {
  iota::esp::TTService::getESPLib()->setLoggerPath(pathToLog);

  if (iota::esp::TTService::getESPLib()->sensors.size() == 0) {
    int sensorid = iota::esp::TTService::getESPLib()->createSensor(sensorFile);
    if (sensorid >= 0) {
      return sensorid;
    }
  } else {
    IOTA_LOG_ERROR(
        m_logger,
        "Unexpected call to initESPLib. There are some sensors already in use: "
            << iota::esp::TTService::getESPLib()->sensors.size());

    return -1;
  }
}

void iota::esp::TTService::set_option(const std::string& name,
                                      const std::string& value) {
  if (name.compare("ConfigFile") == 0) {
    IOTA_LOG_DEBUG(m_logger, "Reading Config File: " << value);

    try {
      read_xml(value, _service_configuration,
               boost::property_tree::xml_parser::no_comments);
      IOTA_LOG_DEBUG(m_logger, "XML READ");

      std::string sensors = _service_configuration.get<std::string>("Sensors");
      IOTA_LOG_DEBUG(m_logger, "Sensors: " << sensors);

      // Set LogPath
      std::string logpath = _service_configuration.get<std::string>("LogPath");

      idSensor = initESPLib(logpath, sensors);

    } catch (boost::exception& e) {
      IOTA_LOG_DEBUG(m_logger, "boost::exception while parsing Config File ");
    } catch (std::exception& e) {
      IOTA_LOG_DEBUG(m_logger, "std::exception: " << e.what());
    }
  } else {
    IOTA_LOG_DEBUG(m_logger, "OPTION:  " << name << " IS UNKNOWN");
    iota::RestHandle::set_option(name, value);
  }
}

void iota::esp::TTService::start() {
  std::cout << "START PLUGINESP" << std::endl;

  std::map<std::string, std::string> filters;

  add_url("", filters, REST_HANDLE(&iota::esp::TTService::service), this);

  // enable_ngsi_service(filters, REST_HANDLE(&iota::esp::TTService::service),
  // this);
}

void iota::esp::TTService::service(
    pion::http::request_ptr& http_request_ptr,
    std::map<std::string, std::string>& url_args,
    std::multimap<std::string, std::string>& query_parameters,
    pion::http::response& http_response, std::string& response) {
  // Here is where I am going to place the code for doing three main things:
  // 1. Parse input and get parameters,
  // 2. Query the CB to get what I need to response to the device (HTTP
  // Response)
  // 3. Create NGSI publication.

  IOTA_LOG_DEBUG(m_logger, "TT: service ");
  std::string keyword("cadena");

  std::string apikey;
  boost::property_tree::ptree pt_cb;
  std::string cb_url;

  std::string strResponse;
  boost::system::error_code error_code;

  int code_resp = pion::http::types::RESPONSE_CODE_OK;

  std::string strBuffer;
  int contentLength;

  std::string method = http_request_ptr->get_method();
  std::string orig_resource = http_request_ptr->get_original_resource();

  IOTA_LOG_DEBUG(m_logger, method << ":" << orig_resource);

  try {
    // strBuffer.assign(http_request_ptr->get_content());

    strBuffer.assign(
        pion::algorithm::url_decode(http_request_ptr->get_content()));

    IOTA_LOG_DEBUG(m_logger, "TT Request received: [" << strBuffer << "]");

    http_response.set_status_code(pion::http::types::RESPONSE_CODE_OK);

    response.assign(strBuffer);  // Just to returns something in case of
                                 // failure.

    // Now, ESPlib takes over:
    if (idSensor <= 0) {
      IOTA_LOG_ERROR(m_logger,
                     "ESPlib was not started properly, nothing will be done.");
      http_response.set_status_code(500);
      http_response.set_status_message("IoTAgent not properly initialized");
      response.assign("Error on ESPlib");
      return;
    }
    std::map<std::string, std::string> params;
    params.clear();

    ESP_Result mainResult = getESPLib()->executeRunnerFromBuffer(
        idSensor, "main", params, (char*)strBuffer.c_str(), strBuffer.length());

    // get device-id.

    std::string device_id = mainResult.findInputAttributeAsString(
        "request",
        true);  // The keyword "request" is defined somewhere in the ESP Xml
                // file, how could I get it?

    if (device_id != "") {
      boost::property_tree::ptree pt_cb;

      // add_info(pt_cb, get_resource(), "");
      IOTA_LOG_DEBUG(m_logger, "TTService: Getting searching device : ["
                                   << device_id << "]");
      boost::shared_ptr<Device> dev = get_device(device_id);

      if (dev.get() == NULL) {
        IOTA_LOG_ERROR(m_logger, "TTService:  device : [" << device_id
                                                          << "] Not FOUND");
        std::string reason("Error: Device ");
        reason.append(device_id);
        reason.append(" not found");
        http_response.set_status_code(404);
        // response.assign(reason);
        throw iota::IotaException("Not Found", reason, 404);
      }

      std::string service_tt(dev->_service);
      std::string sub_service(dev->_service_path);

      IOTA_LOG_DEBUG(m_logger, "TTService: Getting info: Service: ["
                                   << service_tt << "] sub service ["
                                   << sub_service << "]");

      get_service_by_name(
          pt_cb, service_tt,
          sub_service);  // this replaces old "add_info" call to populate ptree.

      iota::esp::tt::SearchResponse seeker = iota::esp::tt::SearchResponse();
      iota::esp::tt::QueryContextWrapper* queryC =
          new iota::esp::tt::QueryContextWrapper(&pt_cb);

      IOTA_LOG_DEBUG(m_logger,
                     "TTService: Creating entity to be published: Service: ["
                         << service_tt << "]");

      // This values will be overwritten later on.
      ::iota::ContextElement cElem(device_id, "", "false");

      cElem.set_env_info(pt_cb, dev);

      IOTA_LOG_DEBUG(m_logger,
                     "TTService: Creating entity : ["
                         << cElem.get_string()
                         << "]");  // call to populate internal fields from
                                   // cache, or by default, etc... not
                                   // interesated in result

      // TWO vectors.
      std::vector<CC_AttributesType> attributes_result;
      std::string temp;
      std::vector<std::string> vJsonTT_Plain;  // This would be published to CB
                                               // if no previous entries are
                                               // found.
      std::vector<std::string> vJsonTT_Processed;  // This is the "normal"
                                                   // processed (with some
                                                   // prefixes) JSON of TT
                                                   // Attributes.
      iota::esp::tt::DecodeTTJSON* decodeTT = new iota::esp::tt::DecodeTTJSON();
      std::string module;

      for (int i = 0; i < mainResult.attresults.size(); i++) {
        CC_AttributesType::iterator itModule =
            mainResult.attresults[i].find("module");

        if (itModule == mainResult.attresults[i].end()) {
          IOTA_LOG_WARN(m_logger,
                        "TTService:  Missing keyword: [module] in parser's "
                        "output device: ["
                            << device_id << "]");
          continue;
        }
        tt_post_ptr_->initialize();
        tt_post_ptr_->execute(&mainResult.attresults[i]);

        module.assign(itModule->second.getValueAsString());
        if (module == "K1") {
          attributes_result.push_back(mainResult.attresults[i]);

        } else if (!tt_post_ptr_->isResultValid()) {
          IOTA_LOG_ERROR(m_logger,
                         "TTService: ERROR-TT while processing module ["
                             << module << "] for id_device [" << device_id
                             << "] ");
        } else {
          IOTA_LOG_DEBUG(m_logger,
                         "TTService: adding module to response processing : ["
                             << module << "]");
          attributes_result.push_back(mainResult.attresults[i]);
          temp.assign(tt_post_ptr_->getResultData());
          decodeTT->parse(temp);
          vJsonTT_Processed.push_back(decodeTT->getProcessedJSON());
          vJsonTT_Plain.push_back(decodeTT->getPlainJSON());
          IOTA_LOG_DEBUG(m_logger, "TTService: JSONS: ["
                                       << decodeTT->getPlainJSON() << "]");
        }
        tt_post_ptr_->terminate();
      }
      delete decodeTT;

      if (attributes_result.size() == 0) {
        IOTA_LOG_WARN(m_logger,
                      "TTService: ignoring CB operations due to no valid "
                      "modules found on the request for device:["
                          << device_id << "]");
        response.assign("");
        return;
      }

      response.assign(seeker.searchTTResponse(attributes_result, cElem.get_id(),
                                              cElem.get_type(), queryC));
      IOTA_LOG_DEBUG(m_logger, "TTService: Response after QueryContext: ["
                                   << response << "]");

      iota::RiotISO8601 timeInstant;

      std::string responseCB;

      // no Response was found for the TT request.
      if (response == "") {
        // Need to publish new Attribute
        IOTA_LOG_DEBUG(m_logger,
                       "TTService: No previous entries were found for these "
                       "attributes, so publishing for the first time.");

        ::iota::ContextElement cElemNew(
            device_id, "",
            "false");  // This values will be overwritten later on.

        cElemNew.set_env_info(pt_cb, dev);

        cElemNew.get_string();  // call to populate internal fields from cache,
                                // or by default, etc... not interesated in
                                // result
        IOTA_LOG_DEBUG(m_logger,
                       "TTService: About to publish on CB for the First Time");

        responseCB.assign(contextBrokerPub->publishContextBroker(
            cElemNew, vJsonTT_Plain, pt_cb, timeInstant));
        IOTA_LOG_DEBUG(
            m_logger,
            "TTService: Response from CB after Publishing FIRST TIME TT Events "
            "[" << responseCB
                << "]");
        // Now quering the CB again... although I should get the same I just
        // posted, just in case a real change happens in between.
        response.assign(seeker.searchTTResponse(
            attributes_result, cElemNew.get_id(), cElemNew.get_type(), queryC));
        // response is what needs to be sent to the device.
        // What should I do if I get another 404 or empty response after
        // publishing? is this even possible?
        if (response == "") {
          IOTA_LOG_ERROR(
              m_logger,
              "TTService: unexpected empty response from ContextBroker.");
          http_response.set_status_code(500);
          throw iota::IotaException("Empty response from CB",
                                    "An unexpected empty response received "
                                    "from ContextBroker. Entity was not "
                                    "published",
                                    500);
        }
      }

      // else, response is sent to device.

      // In anycase, as I have some message from the TT Device, I have to
      // populate the ContextElement and send it to CB.

      // Get the response, and log it.
      responseCB.assign(contextBrokerPub->publishContextBroker(
          cElem, vJsonTT_Processed, pt_cb, timeInstant));
      IOTA_LOG_DEBUG(
          m_logger,
          "TTService: Response from CB after Publishing received TT Events ["
              << responseCB << "]");

    } else {
      IOTA_LOG_ERROR(m_logger,
                     "[request] keyword not found in incoming TT message.");
      http_response.set_status_code(400);
      http_response.set_status_message("Bad Request");
      response.assign("Error: TT message not properly formatted");
      // throw new std::runtime_error("TT message not processed.");
    }

  } catch (iota::IotaException& ex) {
    IOTA_LOG_ERROR(m_logger, "Error: [" << ex.what() << "]");
    http_response.set_status_message("IoTAgent Error");
    response.assign(ex.what());
  } catch (std::runtime_error& excep) {
    IOTA_LOG_ERROR(
        m_logger,
        "An error has occured and the message couldn't get processed.");
    http_response.set_status_code(500);
    http_response.set_status_message("IoTAgent Internal Error");
    response.assign(excep.what());
  }
}

void iota::esp::TTService::add_info(boost::property_tree::ptree& pt,
                                    const std::string& iotService,
                                    const std::string& apiKey) {
  try {
    get_service_by_apiKey(pt,
                          apiKey);  // TODO: this has to change, as it's not
                                    // going to work anymore.
    std::string timeoutSTR = pt.get<std::string>("timeout", "0");
    int timeout = boost::lexical_cast<int>(timeoutSTR);
    std::string service = pt.get<std::string>("service", "");
    std::string service_path = pt.get<std::string>("service_path", "");
    std::string token = pt.get<std::string>("token", "");

    IOTA_LOG_DEBUG(m_logger, "Config retrieved: token: "
                                 << token << " service_path: " << service_path);

    pt.put("timeout", timeout);
    pt.put("service", service);
    pt.put("service_path", service_path);

  } catch (std::exception& e) {
    IOTA_LOG_ERROR(m_logger, "Configuration error for service: "
                                 << iotService << " [" << e.what() << "] ");
    throw e;
  }
}

// creates new miplugin objects
//
extern "C" PION_PLUGIN iota::esp::TTService* pion_create_TTService(void) {
  return new iota::esp::TTService();
}

/// destroys miplugin objects
extern "C" PION_PLUGIN void pion_destroy_TTService(
    iota::esp::TTService* service_ptr) {
  delete service_ptr;
}
