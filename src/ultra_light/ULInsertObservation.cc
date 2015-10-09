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
/*
 * File: ULInsertObservation.cc
 * Author: desgw (Telefonica Investigacion y Desarrollo)
 *
 * Created on jue sep 29 09:45:56 CEST 2011
 *
 */

#include <ULInsertObservation.h>
#include <boost/algorithm/string/trim.hpp>
#include "rest/riot_conf.h"
#include "ngsi/UpdateContext.h"
#include "util/iota_exception.h"

#include "util/RiotISO8601.h"

iota::ULInsertObservation::ULInsertObservation()
    : m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {}

void iota::ULInsertObservation::translate(
    std::string str_io, const boost::shared_ptr<Device>& dev,
    const boost::property_tree::ptree& service_ptree, std::vector<KVP>& query,
    std::vector<iota::ContextElement>& cb_eltos, unsigned short protocol) {
  // Es necesario disponer de dos separadores, uno a nivel de medida y otro
  // a nivel de campo dentro de una medida, para que sea posible recibir en un
  // mensaje
  // varias de estas medidas.
  // el retorno de carro sera el separador entre medidas

  // Analisis de la query
  IOTA_LOG_DEBUG(m_logger, "ULInsertObservation::translate " << str_io);
  boost::trim(str_io);
  boost::erase_all(str_io, "\n");
  boost::erase_all(str_io, "\r");
  std::string device_name;
  std::string device_type;
  std::string uc_query;
  std::string uri_query, samplingTime;

  int i = 0;
  int num_medidas = 0;
  for (i = 0; i < query.size(); i++) {
    if (query[i].getKey().compare("UC") == 0) {
      uc_query = query[i].getValue();
    } else if (query[i].getKey().compare("ID") == 0) {
      device_name = query[i].getValue();
    } else if (query[i].getKey().compare("URL") == 0) {
      uri_query = query[i].getValue();
    } else if (query[i].getKey().compare("t") == 0) {
      samplingTime = query[i].getValue();
    }
  }

  if (dev.get() != NULL) {
    device_name = dev->get_real_name(service_ptree);
    device_type = dev->_entity_type;
  }

  std::string sep_medidas = UL20_MEASURE_SEPARATOR;

  std::vector<std::string> tokens_msgs = riot_tokenizer(str_io, sep_medidas);
  num_medidas = tokens_msgs.size();
  IOTA_LOG_DEBUG(m_logger, "Numero de medidas "
                               << num_medidas << "device:" << device_name << ":"
                               << device_type);
  if (tokens_msgs.size() == 0) {
    // Lanzar excepcion error de protocolo.
    //
    std::ostringstream what;
    what << "Protocol error, no measurements";
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER,
                              what.str(),
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  std::string date_to_cb;
  for (i = 0; i < tokens_msgs.size(); i++) {
    // Cada medida esta en tokens_msg, por si viene por SBC
    if (protocol == 0) {  // SENSOR_PROTOCOL_SML = 0
      tokens_msgs[i] =
          contentForSBCProtocol(tokens_msgs[i], samplingTime, device_name);
    }

    std::string separador = UL20_SEPARATOR;

    std::vector<std::string> tokens_io =
        riot_tokenizer(tokens_msgs[i], separador);
    IOTA_LOG_DEBUG(m_logger, "Analisis de la medida " << i << ":"
                                                      << tokens_msgs[i]);
    if (tokens_io.size() < NUM_ELEMENTS_ULPROTOCOL) {
      // Lanzar excepcion error de protocolo.
      // Seguramente no se trata de UltraLight Protocol
      //
      // Aseguramos que eliminamos los recursos introducidos en la cache
      // por este mensaje
      std::ostringstream what;
      what << "Protocol error, a measure is not complete ";
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER,
                                what.str(),
                                iota::types::RESPONSE_CODE_BAD_REQUEST);
    }

    if (tokens_io[UL_UNIVERSAL_CONCENTRATOR].empty()) {
      tokens_io[UL_UNIVERSAL_CONCENTRATOR] = uc_query;
    }
    if (tokens_io[UL_ID_RESOURCE].empty()) {
      tokens_io[UL_ID_RESOURCE] = device_name;
    } else {
      // Si el recurso no esta vacio, puede ocurrir que
      // haya que componerlo con el ID de la request uri.
      // Ya no es obligatoria la presencia de UC.
      if (!device_name.empty()) {
        tokens_io[UL_ID_RESOURCE].insert(0, ".");
        tokens_io[UL_ID_RESOURCE].insert(0, device_name);
      }
    }

    IOTA_LOG_DEBUG(m_logger, "UL_UNIVERSAL_CONCENTRATOR "
                                 << tokens_io[UL_UNIVERSAL_CONCENTRATOR]);
    IOTA_LOG_DEBUG(m_logger, "ID RECUrSO " << tokens_io[UL_ID_RESOURCE]);
    IOTA_LOG_DEBUG(m_logger, "SAMPLING TIME " << tokens_io[UL_SAMPLING_TIME]);
    IOTA_LOG_DEBUG(m_logger, "OBSERVED PROPERTY "
                                 << tokens_io[UL_OBSERVED_PROPERTY]);

    // Si no tiene concentrador/recurso se coge el de la UrI
    // si no viene error
    // Se elimina la obligatoriedad de UNIVERSAL_CONCENTRATOR

    if (  //(tokens_io[UL_UNIVERSAL_CONCENTRATOR].empty()) ||
        (tokens_io[UL_ID_RESOURCE].empty()) ||
        (tokens_io[UL_OBSERVED_PROPERTY].empty())) {
      // Lanzar excepcion error protocolo
      IOTA_LOG_ERROR(m_logger, "Falta informacion de medida");
      // Aseguramos que eliminamos los recursos introducidos en la cache
      // por este mensaje
      std::ostringstream what;
      what << "Protocol error, measurement has not got property or idResource ";
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER,
                                what.str(),
                                iota::types::RESPONSE_CODE_BAD_REQUEST);
    }

    // Creacion de la entidad contextBroker
    //

    // AssignedSensorId y procedure
    // El recurso puede ser un componente de un sistema. El sistema sera
    // el AssignedSensorId y el procedure el componente.
    //

    // Debe extraer el system y el procedure
    std::string sep(".");
    std::vector<std::string> v_tokens =
        riot_tokenizer(tokens_io[UL_ID_RESOURCE], sep);
    std::string procedure;
    std::string assigned_sensor_id;
    if (v_tokens.size() == 1) {
      // El recurso no esta asociado a un sistema
      // El procedure y el AssignedSensorId coinciden
      procedure = tokens_io[UL_ID_RESOURCE];
      assigned_sensor_id = procedure;
    } else {
      // El formato del identificador de recurso incluye
      // sistema
      for (int i = 1; i < v_tokens.size(); i++) {
        if (!procedure.empty()) {
          procedure.append(".");
        }
        procedure.append(v_tokens[i]);
      }
      if (procedure.empty()) {
        procedure = tokens_io[UL_ID_RESOURCE];
      }
      assigned_sensor_id = v_tokens[0];
    }

    IOTA_LOG_DEBUG(m_logger, "Sistema/AssignedSensorId " << assigned_sensor_id);
    IOTA_LOG_DEBUG(m_logger, "Procedure " << procedure);

    std::string nombre_recurso(tokens_io[UL_UNIVERSAL_CONCENTRATOR]);
    if (nombre_recurso.empty() == false) {
      nombre_recurso.append(".");
    }
    nombre_recurso.append(assigned_sensor_id);
    if (assigned_sensor_id.compare(procedure) != 0) {
      nombre_recurso.append(".");
      nombre_recurso.append(procedure);
    }

    try {
      // observacion->setSensorId(assigned_sensor_id);
      // observacion->procedure(procedure);

      // SamplingTime
      //
      std::string sep_stime(" ");  // Puede ser un rango
      std::vector<std::string> stime_tokens =
          riot_tokenizer(tokens_io[UL_SAMPLING_TIME], sep_stime);
      int n_dates = stime_tokens.size();
      IOTA_LOG_DEBUG(m_logger, "Numero de elementos " << n_dates);
      std::string frame("urn:x-ogc:def:trs:IDAS:1.0:ISO8601");

      if (n_dates == 0) {
        // Se pone el samplingTime local
        iota::RiotISO8601 mi_hora;
        date_to_cb = mi_hora.toUTC().toString();
        // observacion->samplingTime(frame, mi_hora.toUTC().toString(), end);
      } else if (n_dates == 1) {
        // Corresponde a TimeInstant
        std::string end;

        // IDAS-1928 tratamiento de tiempos epoch
        std::string s_time;
        try {
          IOTA_LOG_DEBUG(m_logger, "Tratando formato epoch");
          iota::RiotISO8601 iso(stime_tokens[0]);
          s_time = iso.toString();

        } catch (std::runtime_error& e) {
          // Se trata como formato iso (string).
          IOTA_LOG_ERROR(
              m_logger,
              "using now time, because error in date format: " << e.what());
          // ponemos la hora actual
          iota::RiotISO8601 mi_hora;
          s_time = mi_hora.toUTC().toString();
        }

        date_to_cb = s_time;
        // observacion->samplingTime(frame, stime_tokens[0], end);
        // observacion->samplingTime(frame, s_time, end);
      } else if (n_dates >= 2) {
        // IDAS-1928 tratamiento de tiempos epoch
        std::string s_time_b;
        std::string s_time_e;
        try {
          long s_time_number = boost::numeric_cast<long>(
              boost::lexical_cast<double>(stime_tokens[0]));
          iota::RiotISO8601 iso_b(s_time_number);
          s_time_b = iso_b.toString();
          s_time_number += boost::numeric_cast<long>(
              boost::lexical_cast<double>(stime_tokens[1]));
          iota::RiotISO8601 iso_e(s_time_number);
          s_time_e = iso_e.toString();

        } catch (boost::bad_lexical_cast&) {
          // Se trata como formato iso (string).
          IOTA_LOG_DEBUG(m_logger, "Tratando formato ISO");
          s_time_b = stime_tokens[0];
          s_time_e = stime_tokens[1];
        }
        // Corresponde a TimePeriod
        date_to_cb = s_time_b;
        // observacion->samplingTime(frame, stime_tokens[0], stime_tokens[1]);
        // observacion->samplingTime(frame, s_time_b, s_time_e);
      }

      // UC (UniversalIdentifierOfLogicalHub)
      //
      if (tokens_io[UL_UNIVERSAL_CONCENTRATOR].empty() == false) {
        IOTA_LOG_DEBUG(m_logger, "Existe UniversalConcentrator");
        std::string definition_uc(
            "urn:x-ogc:def:identifier:IDAS:1.0:"
            "UniversalIdentifierOfLogicalHub");
        std::string gml_name;
        // swe::Text txt = createText("", gml_name,
        // tokens_io[UL_UNIVERSAL_CONCENTRATOR]);
        // observacion->parameter(definition_uc, txt);
        IOTA_LOG_DEBUG(m_logger, "Insertado parametro UniversalConcentrator");
      }

      // Propiedad observada
      //
      // Se intenta primero resolver como identificativo corto
      //

      IOTA_LOG_DEBUG(m_logger, "observedProperty "
                                   << tokens_io[UL_OBSERVED_PROPERTY]);

      int j = NUM_ELEMENTS_ULPROTOCOL;
      int num_tokens = tokens_io.size();

      while (j < num_tokens) {
        IOTA_LOG_DEBUG(m_logger, "Elementos variables de la medida "
                                     << tokens_io[j]);

        // Parametros de la medida
        // Puede que no existan parametros

        if (!tokens_io[j].empty()) {
          // parametro = recurso->getParameterById(tokens_io[j]);
          IOTA_LOG_DEBUG(m_logger, "recurso->getParameterById "
                                       << tokens_io[j]);
        } else {
          j++;
        }

        std::string valueSTR = tokens_io[j + 1];
        std::string attr_name = tokens_io[j];

        IOTA_LOG_DEBUG(m_logger, "Creacion de la medida " << attr_name << ":"
                                                          << valueSTR);
        if (valueSTR.empty()) {
          IOTA_LOG_ERROR(m_logger, "Empty values is not allowed");
          std::ostringstream what;
          what << "Protocol error, Empty values is not allowed ";
          throw iota::IotaException(
              iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER, what.str(),
              iota::types::RESPONSE_CODE_BAD_REQUEST);
        }

        iota::Attribute att(attr_name, "", valueSTR);
        j += 2;
        IOTA_LOG_DEBUG(m_logger, "TimeInstant:" << date_to_cb);
        iota::Attribute metadata_ts("TimeInstant", "ISO8601", date_to_cb);
        att.add_metadata(metadata_ts);
        // iota::Attribute metadata_uom("uom", "string", uom);
        // att.add_metadata(metadata_uom);

        iota::ContextElement cb_elto(device_name, "", "false");
        cb_elto.set_env_info(service_ptree, dev);
        cb_elto.add_attribute(att);

        // attribute with time
        iota::Attribute timeAT("TimeInstant", "ISO8601", date_to_cb);
        cb_elto.add_attribute(timeAT);

        cb_eltos.push_back(cb_elto);
      }
    } catch (std::runtime_error& e) {
      IOTA_LOG_ERROR(m_logger, "Error de formato2 " << e.what());
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER,
                                e.what(),
                                iota::types::RESPONSE_CODE_BAD_REQUEST);
    }

  }  // end for

  if (num_medidas != cb_eltos.size()) {
    // Lanzar excepcion error de protocolo.
    //
    std::ostringstream what;
    what << "Protocol error, ";
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER,
                              what.str(),
                              iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  IOTA_LOG_DEBUG(m_logger, "Salida del constructor ULInsertObservation");
};

iota::ULInsertObservation::~ULInsertObservation(void) {
  IOTA_LOG_DEBUG(m_logger, "iota::ULInsertObservation::~ULInsertObservation");
};

std::string iota::ULInsertObservation::contentForSBCProtocol(
    std::string io, const std::string& a_sampling_time,
    const std::string& a_res_query) {
  IOTA_LOG_DEBUG(m_logger, "CONTENT SBC " << io);
  std::string sampling_time_measure;
  /*
     std::size_t found = io.find_last_not_of("\r\n");
     if (found != std::string::npos) {
        io.erase(found+1);
     }
     else {
        io.clear();
     }
  */
  std::string io_final;
  std::string alias;
  std::string data;
  // Se mira si el dato contiene el sampling time
  std::size_t p_pipe = io.find_first_of("|");
  std::size_t p_pipe_f = io.find_last_of("|");
  if ((p_pipe != std::string::npos) && (p_pipe_f != std::string::npos)) {
    if (p_pipe == p_pipe_f) {
      alias = io.substr(0, p_pipe);
      data = io;
      sampling_time_measure = a_sampling_time;
    } else {
      sampling_time_measure = io.substr(0, p_pipe);
      data = io.substr(p_pipe + 1);
      std::size_t p_alias = data.find_first_of("|");
      if (p_alias != std::string::npos) {
        alias = data.substr(0, p_alias);
      }
    }
  }
  io_final.append("|");
  io_final.append(a_res_query);
  io_final.append("|");
  io_final.append(sampling_time_measure);
  io_final.append("|");
  io_final.append(alias);
  io_final.append("||");
  io_final.append(data);
  return io_final;
};
