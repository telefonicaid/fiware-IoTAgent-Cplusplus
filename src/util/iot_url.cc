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
#include "iot_url.h"
#include "iota_exception.h"
#include "rest/types.h"
#include <sstream>
#include <string>
#include <stdlib.h>

iota::IoTUrl::IoTUrl(std::string url) : _port(-1) {
  if (url.empty()) {
    throw iota::IotaException(
        iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER + "[ empty url ]",
        "empty url", iota::types::RESPONSE_CODE_BAD_REQUEST);
  }

  _url = url;

  try {
    // Protocol
    std::string prot(URL_PROTOCOL_DELIMITER);

    size_t pos_prot = _url.find(prot);

    if (pos_prot != std::string::npos) {
      _protocol = _url.substr(0, pos_prot);
      pos_prot += prot.length();
    }

    // host+puerto
    std::string hp;

    size_t pos_path = std::string::npos;

    pos_path = _url.find(URL_PATH_DELIMITER, pos_prot);

    if (pos_path != std::string::npos) {
      hp = _url.substr(pos_prot, pos_path - pos_prot);
    }

    size_t pos_query = _url.find('?', pos_prot);

    if (pos_query != std::string::npos) {
      _query = _url.substr(pos_query + 1);

      // Si no hay path
      if (pos_path == std::string::npos) {
        hp = _url.substr(pos_prot, pos_query - pos_prot);
      } else {
        _path = _url.substr(pos_path, pos_query - pos_path);
      }
    } else {
      if (pos_path == std::string::npos) {
        hp = _url.substr(pos_prot);
      } else {
        _path = _url.substr(pos_path);
      }
    }

    size_t pos_port = hp.find(':');

    if (pos_port != std::string::npos) {
      _host = hp.substr(0, pos_port);
      _port = atoi((hp.substr(pos_port + 1)).c_str());
    } else {
      _host = hp;
    }

    // Sanity
    if (_port == -1) {
      if (_protocol.compare(URL_PROTOCOL_HTTP) == 0) {
        _port = 80;
      } else if (_protocol.compare(URL_PROTOCOL_HTTPS) == 0) {
        _port = 443;
      }
    }

    if (_path.empty() == true) {
      _path = "/";
    }
  } catch (std::exception& e) {
    throw iota::IotaException(
        iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER + "[" + _url + "]",
        url + " is not a correct uri", iota::types::RESPONSE_CODE_BAD_REQUEST);
  }
}

iota::IoTUrl::~IoTUrl(void) {}

std::string iota::IoTUrl::getProtocol(void) { return (_protocol); }

std::string iota::IoTUrl::getHost(void) { return (_host); }

int iota::IoTUrl::getPort(void) { return (_port); }

std::string iota::IoTUrl::getPath(void) { return (_path); }

std::string iota::IoTUrl::getQuery(void) { return (_query); }

bool iota::IoTUrl::getSSL(void) {
  // En funcion del protocolo se sabe si es
  // con ssl o no.
  return _protocol.compare(URL_PROTOCOL_HTTPS) == 0;
};

//~ Formatted by Jindent --- http://www.jindent.com
