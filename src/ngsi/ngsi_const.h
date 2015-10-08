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
#ifndef SRC_NGSI_NGSI_CONST_H_
#define SRC_NGSI_NGSI_CONST_H_

#include <string>
namespace iota {
struct ngsi {
  virtual ~ngsi(){};
  static const std::string NGSI_NAME;
  static const std::string NGSI_ID;
  static const std::string NGSI_VALUE;
  static const std::string NGSI_TYPE;
  static const std::string NGSI_ISPATTERN;
  static const std::string NGSI_ISDOMAIN;
  static const std::string NGSI_ATTRIBUTES;
  static const std::string NGSI_METADATAS;
  static const std::string NGSI_ENTITIES;
  static const std::string NGSI_PROVIDINGAPPLICATION;
  static const std::string NGSI_CONTEXTELEMENT;
  static const std::string NGSI_STATUSCODE;
  static const std::string NGSI_CODE;
  static const std::string NGSI_REASONPHRASE;
  static const std::string NGSI_DETAILS;
  static const std::string NGSI_CONTEXTRESPONSES;
  static const std::string NGSI_CONDVALUES;
  static const std::string NGSI_CONTEXTREGISTRATIONS;
  static const std::string NGSI_DURATION;
  static const std::string NGSI_REGISTRATIONID;
  static const std::string NGSI_REGISTERRESPONSE;
  static const std::string NGSI_REFERENCE;
  static const std::string NGSI_THROTTLING;
  static const std::string NGSI_NOTIFYCONDITIONS;
  static const std::string NGSI_SUBSCRIBERESPONSE;
  static const std::string NGSI_SUBSCRIPTIONID;
  static const std::string NGSI_UPDATEACTION;
  static const std::string NGSI_CONTEXTELEMENTS;
};
};

#endif
