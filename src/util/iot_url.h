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
#ifndef SRC_UTIL_IOT_URL_H_
#define SRC_UTIL_IOT_URL_H_

#include <string>
#include <vector>

#define URL_PROTOCOL_DELIMITER "://"
#define URL_PATH_DELIMITER "/"
#define URL_PROTOCOL_HTTP "http"
#define URL_PROTOCOL_HTTPS "https"
#define URL_PROTOCOL_TCP "tcp"

namespace iota {

class IoTUrl {
 public:
  IoTUrl(std::string url);

  ~IoTUrl(void);

  std::string getProtocol(void);

  std::string getHost(void);

  int getPort(void);

  std::string getPath(void);

  std::string getQuery(void);

  bool getSSL(void);

 protected:
 private:
  std::string _url;
  std::string _protocol;
  std::string _host;
  int _port;
  std::string _path;
  std::string _query;
};
};
#endif
