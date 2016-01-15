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
#ifndef SRC_REST_REST_FUNCTIONS_H_
#define SRC_REST_REST_FUNCTIONS_H_

#include <vector>
#include <boost/regex.hpp>

namespace iota {

bool restResourceParse(std::string& regex,
                       std::vector<std::string>& url_placeholders,
                       std::string& url_resource,
                       std::map<std::string, std::string>& resources);

void format_pattern(std::string& url,
                    std::map<std::string, std::string>& filters,
                    std::string& url_regex, std::vector<std::string>& url_args);

 /**  escapes URL-encoded strings (a%20value+with%20spaces)
   *  same function in  escapes URL-encoded strings (a%20value+with%20spaces)
   *  without decode + to space, only decode %..
   *
   * @param str - a string
   * @return the same string with percent escapes
   */
 static std::string url_decode(const std::string& str);
};

#endif
