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
 * File:   ULInsertObservation.h
 * Author: desgw
 *
 * Created on August 9, 2011, 12:13 PM
 *
 */

#ifndef SRC_ULTRA_LIGHT_ULINSERTOBSERVATION_H_
#define SRC_ULTRA_LIGHT_ULINSERTOBSERVATION_H_

#include "util/FuncUtil.h"
#include "util/device.h"
#include "rest/process.h"

#include <vector>
#include <ngsi/ContextElement.h>

#define NUM_ELEMENTS_ULPROTOCOL 4

#define UL20_SEPARATOR "|"
#define UL20_MEASURE_SEPARATOR "#"

namespace iota {

class ULInsertObservation {
 public:
  // Datos fijos de la cadena
  typedef enum {
    UL_UNIVERSAL_CONCENTRATOR = 0,
    UL_ID_RESOURCE,
    UL_SAMPLING_TIME,
    UL_OBSERVED_PROPERTY
  } ULProtocol;
  // Constructor
  ULInsertObservation();
  // Destructor
  ~ULInsertObservation(void);

  void translate(std::string str_io, const boost::shared_ptr<Device>& dev,
                 const boost::property_tree::ptree& service_ptree,
                 std::vector<KVP>& query,
                 std::vector<iota::ContextElement>& cb_eltos,
                 unsigned short protocol = 1);

  const std::string send(const std::string& deviceIdStr,
                         const std::string& entity_type,
                         const std::string& timestampStr, const std::string& at,
                         const std::string& value);

 protected:
 private:
  pion::logger m_logger;

  // Variante SBC o UL 2.0
  std::string contentForSBCProtocol(std::string io,
                                    const std::string& sampling_time,
                                    const std::string& a_res_query);
};
}

#endif /* __PAIDULINSERTOBSERVATION_H__ */
