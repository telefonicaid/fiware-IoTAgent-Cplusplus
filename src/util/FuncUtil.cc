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
 * File: FuncUtil.cc
 * Author: desgw (Telefonica Investigacion y Desarrollo)
 *
 * Created on jue sep 29 09:45:56 CEST 2011
 *
 */

// TODO#include <RiotSOSDef.h>
#include "FuncUtil.h"
#include "RiotISO8601.h"
#include "rest/types.h"
#include "util/iota_exception.h"
#include <boost/tokenizer.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <sstream>
#include <algorithm>
#include <pion/algorithm.hpp>

namespace iota {
boost::uuids::random_generator RandomGenerator;
boost::mutex _mRandomGen;
unsigned long counter = 0;
boost::mt19937 ran(time(NULL));
boost::uuids::basic_random_generator<boost::mt19937> gen(&ran);
}

std::vector<std::string> iota::riot_tokenizer(std::string& str,
                                              std::string& sep) {
  // Aplica el tokenizer boost
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> separator(sep.c_str(), "",
                                        boost::keep_empty_tokens);
  tokenizer tok(str, separator);
  boost::tokenizer<boost::char_separator<char> >::iterator b;

  // Vector con los resultados
  std::vector<std::string> v;
  for (b = tok.begin(); b != tok.end(); ++b) {
    v.push_back(*b);
  }
  return (v);
};

std::string iota::riot_uuid(std::string str_key) {
  return str_key + "-" + riot_uuid();
};

std::string iota::riot_id(std::string& str_key) {
  iota::RiotISO8601 marca_tiempo;
  std::string id("IDAS");
  id.append(marca_tiempo.toIdString());
  id.append("-");
  id.append(str_key);
  return (id);
};

std::string iota::riot_uuid(void) {
  boost::mutex::scoped_lock lck(iota::_mRandomGen);

  // This is the way boost says avoid a false positive in valgrind
  /*
  boost::mt19937 ran;
  ran.seed(time(NULL));
  boost::uuids::basic_random_generator<boost::mt19937> gen(&ran);
  */
  boost::uuids::uuid u = iota::gen();
  if (counter >= ULONG_MAX) {
    counter = 0;
  }

  std::ostringstream ss;
  ss << u;
  ss << "-";
  ss << counter++;
  return ss.str();
};

bool iota::riot_getQueryParams(std::string q_str,
                               std::vector<iota::KVP>& q_param) {
  if (q_str.empty()) {
    return false;
  }

  std::string m_query = iota::url_decode(q_str);
  enum QueryParseState {
    QUERY_PARSE_NAME,
    QUERY_PARSE_VALUE
  } parse_state = QUERY_PARSE_NAME;
  const boost::uint32_t QUERY_NAME_MAX = 1024;
  const boost::uint32_t QUERY_VALUE_MAX = 1024 * 1024;

  const char* ptr = m_query.c_str();
  const size_t len = m_query.length();
  const char* const end = ptr + len;
  std::string query_name;
  std::string query_value;

  // iterate through each encoded character
  while (ptr < end) {
    switch (parse_state) {
      case QUERY_PARSE_NAME:
        // parsing query name
        if (*ptr == '=') {
          // end of name found
          if (query_name.empty()) {
            return false;
          }
          parse_state = QUERY_PARSE_VALUE;
        } else if (*ptr == '&') {
          // if query name is empty, just skip it (i.e. "&&")
          if (!query_name.empty()) {
            // assume that "=" is missing -- it's OK if the value is empty
            iota::KVP kvp(query_name, query_value);
            q_param.push_back(kvp);
            query_name.erase();
          }
        } else if (*ptr == '\r' || *ptr == '\n') {
          // ignore linefeeds and carriage returns (normally within POST
          // content)
        } else if (isControl(*ptr) || query_name.size() >= QUERY_NAME_MAX) {
          // control character detected, or max sized exceeded
          return false;
        } else {
          // character is part of the name
          query_name.push_back(*ptr);
        }
        break;
      case QUERY_PARSE_VALUE:
        // parsing query value
        if (*ptr == '&') {
          // end of value found (OK if empty)
          // q_param.insert( std::make_pair(query_name, query_value) );
          iota::KVP kvp(query_name, query_value);
          q_param.push_back(kvp);
          query_name.erase();
          query_value.erase();
          parse_state = QUERY_PARSE_NAME;
        } else if (*ptr == '\r' || *ptr == '\n') {
          // ignore linefeeds and carriage returns (normally within POST
          // content)
        } else if (isControl(*ptr) || query_value.size() >= QUERY_VALUE_MAX) {
          // control character detected, or max sized exceeded
          return false;
        } else {
          // character is part of the value
          query_value.push_back(*ptr);
        }
        break;
    }

    ++ptr;
  }

  // handle last pair in string
  if (!query_name.empty()) {
    iota::KVP kvp(query_name, query_value);
    q_param.push_back(kvp);
    // q_param.insert( std::make_pair(query_name, query_value) );
  }

  return true;
};

bool iota::isControl(int c) { return ((c >= 0 && c <= 31) || c == 127); };

bool iota::isEqualString(bool case_sensitive, std::string str1,
                         std::string str2) {
  if (str1.size() == str2.size()) {
    if (case_sensitive == true) {
      if (str1.compare(str2) == 0) {
        return true;
      }
    } else {
      std::string str1_m = boost::to_lower_copy(str1);
      std::string str2_m = boost::to_lower_copy(str2);
      if (str1_m.compare(str2_m) == 0) {
        return true;
      }
    }
  }
  return false;
};

std::string iota::str_to_hex(std::string& fr) {
  std::string trama_hex;
  for (int i = 0; i < fr.size(); i++) {
    trama_hex.append(
        boost::str(boost::format("%02X ") %
                   static_cast<unsigned short>((unsigned char)(fr.at(i)))));
  }
  boost::trim(trama_hex);
  return (trama_hex);
};

std::string str_to_hex(boost::asio::mutable_buffer& fr) {
  std::string trama_hex;
  char* p = boost::asio::buffer_cast<char*>(fr);
  int b_size = boost::asio::buffer_size(fr);
  for (int i = 0; i < b_size; i++) {
    trama_hex.append(
        boost::str(boost::format("%02X ") %
                   static_cast<unsigned short>((unsigned char)(p[i]))));
  }

  return (trama_hex);
};

std::string iota::str_to_hex(const std::vector<unsigned char>& fr) {
  std::string trama_hex;
  int b_size = fr.size();
  for (int i = 0; i < b_size; i++) {
    trama_hex.append(
        boost::str(boost::format("%02X ") %
                   static_cast<unsigned short>((unsigned char)(fr[i]))));
  }
  boost::trim(trama_hex);
  return (trama_hex);
}

std::vector<unsigned char> iota::hex_str_to_vector(std::string& str) {
  std::vector<unsigned char> v;
  std::string str_wit_no_whitspaces(str);
  boost::erase_all(str_wit_no_whitspaces, " ");
  std::size_t l = str_wit_no_whitspaces.size();
  for (int i = 0; i < l; i += 2) {
    std::istringstream ss(str_wit_no_whitspaces.substr(i, 2));
    unsigned short x;
    if (!(ss >> std::hex >> x)) {
      // Conversion error.
      v.clear();
      break;
    }
    v.push_back(x);
  }
  return v;
}

std::string iota::writeDictionaryTerm(pion::http::response& resp_http) {
  pion::ihash_multimap h = resp_http.get_headers();
  std::string echo;
  for (pion::ihash_multimap::const_iterator val = h.begin(); val != h.end();
       ++val) {
    echo.append(val->first + ":" + val->second + "\n");
  }
  return echo;
}

std::string iota::make_query_string(
    std::multimap<std::string, std::string>& query_parameters) {
  std::string query_string;
  std::map<std::string, std::string>::iterator it = query_parameters.begin();
  int i = 0;
  while (it != query_parameters.end()) {
    if (!it->second.empty()) {
      if (i != 0) {
        query_string += '&';
      }
      query_string += pion::algorithm::url_encode(it->first);
      query_string += '=';
      query_string += pion::algorithm::url_encode(it->second);
      ++i;
    }
    ++it;
  }
  return query_string;
}

void iota::check_fiware_service_name(std::string& header_fiware_service) {
  // Length
  bool error = false;
  std::string reason = iota::types::REASON_MALFORMED_HEADER;
  std::string details = iota::types::DETAILS_HEADER_FIWARE_SERVICE;
  int code = iota::types::RESPONSE_CODE_FIWARE_SERVICE_ERROR;
  if (header_fiware_service.empty() ||
      header_fiware_service.size() > iota::FIWARE_SERVICE_LENGTH) {
    error = true;
  }

  // aphanum and '_' and lowercase
  int i = 0;
  for (i = 0; (i < header_fiware_service.size() && !error); i++) {
    int c = header_fiware_service[i];
    if ((!isalnum(c) && c != '_') || (isalpha(c) && !islower(c))) {
      error = true;
    }
  }

  if (error) {
    throw iota::IotaException(reason, details, code);
  }
};
void iota::check_fiware_service_path_name(
    std::string& header_fiware_service_path) {
  if (header_fiware_service_path.empty() == true) {
    return;
  }
  bool error = false;
  std::string reason = iota::types::REASON_MALFORMED_HEADER;
  std::string details = iota::types::DETAILS_HEADER_FIWARE_SERVICE_PATH;
  int code = iota::types::RESPONSE_CODE_FIWARE_SERVICE_PATH_ERROR;
  // aphanum and '_' and start '/'
  int i = 0;
  int start = 1;
  if (header_fiware_service_path[0] != '/') {
    error = true;
  }
  bool wild_card = false;
  if (header_fiware_service_path[1] == '*' ||
      header_fiware_service_path[1] == '#') {
    wild_card = true;
    start = 2;
    if (header_fiware_service_path.size() > 2) {
      error = true;
    }
  }

  // Length
  if (header_fiware_service_path.size() > iota::FIWARE_SERVICE_LENGTH + 1) {
    error = true;
  }

  for (i = start; (i < header_fiware_service_path.size() && !error); i++) {
    int c = header_fiware_service_path[i];
    if (!isalnum(c) && c != '_') {
      error = true;
    }
  }

  if (error) {
    throw iota::IotaException(reason, details, code);
  }
};

bool iota::check_forbidden_characters(std::string forbidden, std::string& str) {
  bool has_forbidden = false;
  if (str.find_first_of(forbidden) != std::string::npos) {
    has_forbidden = true;
  }
  return has_forbidden;
};

bool iota::check_forbidden_characters(mongo::BSONObjBuilder& in_bson_builder) {
  bool has_forbidden = false;
  mongo::BSONObj in_copy_bson = in_bson_builder.asTempObj();
  for (mongo::BSONObjIterator i_obj = in_copy_bson.begin(); i_obj.more();) {
    mongo::BSONElement e_obj = i_obj.next();
    std::string value_str = e_obj.valuestrsafe();
    if (e_obj.type() != mongo::EOO && e_obj.type() != mongo::Object &&
        e_obj.type() != mongo::Array) {
      has_forbidden = has_forbidden ||
                      check_forbidden_characters(
                          iota::types::IOTA_FORBIDDEN_CHARACTERS, value_str);
    } else if (e_obj.type() != mongo::EOO && e_obj.type() == mongo::Object) {
      mongo::BSONObj obj_obj = e_obj.Obj();
      mongo::BSONObjBuilder bb;
      bb.appendElements(obj_obj);
      has_forbidden = has_forbidden || check_forbidden_characters(bb);
    } else if (e_obj.type() != mongo::EOO && e_obj.type() == mongo::Array) {
      std::vector<mongo::BSONElement> array_obj = e_obj.Array();
      for (int i = 0; i < array_obj.size(); i++) {
        mongo::BSONObj obj_obj = array_obj[i].Obj();
        mongo::BSONObjBuilder bb;
        bb.appendElements(obj_obj);
        has_forbidden = has_forbidden || check_forbidden_characters(bb);
      }
    }
  }
  return has_forbidden;
}

void iota::writeDictionaryTerm(std::ostringstream& os,
                               const pion::ihash_multimap::value_type& val) {
  os << val.first << ": " << val.second << pion::http::types::STRING_CRLF;
}

std::string iota::http2string(pion::http::request& req) {
  std::ostringstream os;
  os << "Method: " << req.get_method() << pion::http::types::STRING_CRLF
     << "HTTP Version:  " << req.get_version_major() << "."
     << req.get_version_minor() << pion::http::types::STRING_CRLF
     << "Resource requested: " << req.get_original_resource()
     << pion::http::types::STRING_CRLF
     << "Resource delivered: " << req.get_resource()
     << pion::http::types::STRING_CRLF << "Query string "
     << req.get_query_string() << pion::http::types::STRING_CRLF
     << "Content length: " << (unsigned long)req.get_content_length()
     << pion::http::types::STRING_CRLF << "Headers: ";
  std::for_each(req.get_headers().begin(), req.get_headers().end(),
                boost::bind(&iota::writeDictionaryTerm, boost::ref(os), _1));
  if (req.get_content_length() != 0) {
    os << req.get_content();
  }
  return os.str();
}

std::string iota::http2string(pion::http::response& req) {
  std::ostringstream os;

  os << "Content length: " << (unsigned long)req.get_content_length()
     << pion::http::types::STRING_CRLF << "Headers: ";

  std::for_each(req.get_headers().begin(), req.get_headers().end(),
                boost::bind(&iota::writeDictionaryTerm, boost::ref(os), _1));
  if (req.get_content_length() != 0) {
    os << req.get_content();
  }
  return os.str();
}

int iota::number_of_decimals(const std::string& str) {
  std::string decimals;
  int pos = str.find(".");
  if (pos == std::string::npos) {
    return 0;
  } else {
    decimals = str.substr(pos + 1);
    return decimals.length();
  }
}

std::string iota::url_decode(const std::string& str) {
  char decode_buf[3];
  std::string result;
  result.reserve(str.size());

  for (std::string::size_type pos = 0; pos < str.size(); ++pos) {
    switch (str[pos]) {
      case '%':
        // decode hexidecimal value
        if (pos + 2 < str.size()) {
          decode_buf[0] = str[++pos];
          decode_buf[1] = str[++pos];
          decode_buf[2] = '\0';
          result += static_cast<char>(strtol(decode_buf, 0, 16));
        } else {
          // recover from error by not decoding character
          result += '%';
        }
        break;
      default:
        // character does not need to be escaped
        result += str[pos];
    }
  };

  return result;
}

std::string iota::render_identifier(const std::string& str_to_render) {
  return std::string(
      boost::erase_all_regex_copy(
          str_to_render,
          boost::regex(iota::REGEX_IDENTIFIER_FORBIDDEN_CHARACTERS)),
      0, iota::IDENTIFIER_MAX_LENGTH);
}
