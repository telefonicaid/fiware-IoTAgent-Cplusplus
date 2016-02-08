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
 * File: FuncUtil.h
 * Author: desgw (Telefonica Investigacion y Desarrollo)
 *
 * Created on jue sep 29 09:45:55 CEST 2011
 *
 */

#ifndef SRC_UTIL_FUNCUTIL_H_
#define SRC_UTIL_FUNCUTIL_H_

#include "KVP.h"
#include <vector>
#include <string>
#include <pion/http/response.hpp>
#include <pion/http/request.hpp>
#include <mongo/bson/bson.h>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>

namespace iota {

// static boost::uuids::random_generator RandomGenerator;
// static boost::mutex _mRandomGen;

// Namespace UUID
const boost::uuids::uuid namespace_uuid = {{0x6b, 0xa7, 0xb8, 0x10, 0x9d, 0xad,
                                            0x11, 0xd1, 0x80, 0xb4, 0x00, 0xc0,
                                            0x4f, 0xd4, 0x30, 0xc8}};
const int FIWARE_SERVICE_LENGTH = 50;

const std::string REGEX_IDENTIFIER_FORBIDDEN_CHARACTERS = "[<>();'=\"/?#& ]+";
const int IDENTIFIER_MAX_LENGTH = 256;

std::vector<std::string> riot_tokenizer(std::string& str, std::string& sep);
std::string riot_uuid(std::string str_key);
std::string riot_id(std::string& str_key);
std::string riot_uuid(void);
bool riot_getQueryParams(std::string q_str, std::vector<iota::KVP>& q_param);
bool isControl(int c);
bool isEqualString(bool case_sensitive, std::string str1, std::string str2);
std::string str_to_hex(std::string& fr);
std::string str_to_hex(boost::asio::mutable_buffer& fr);
std::string str_to_hex(const std::vector<unsigned char>& fr);
std::vector<unsigned char> hex_str_to_vector(std::string& str);
std::string writeDictionaryTerm(pion::http::response& resp_http);
std::string make_query_string(
    std::multimap<std::string, std::string>& query_parameters);
void check_fiware_service_name(std::string& header_fiware_service);
void check_fiware_service_path_name(std::string& header_fiware_service_path);
bool check_forbidden_characters(std::string forbidden, std::string& str);
bool check_forbidden_characters(mongo::BSONObjBuilder& in_bson_builder);
std::string http2string(pion::http::request& req);
std::string http2string(pion::http::response& req);
void writeDictionaryTerm(std::ostringstream& os,
                         const pion::ihash_multimap::value_type& val);

int number_of_decimals(const std::string& str);

 /**  escapes URL-encoded strings (a%20value+with%20spaces)
   *  same function in  escapes URL-encoded strings (a%20value+with%20spaces)
   *  without decode + to space, only decode %..
   *
   * @param str - a string
   * @return the same string with percent escapes
   */
std::string url_decode(const std::string& str);

std::string render_identifier(const std::string& str_to_render);

};

#endif
