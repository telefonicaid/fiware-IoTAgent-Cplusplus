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

//TODO#include <RiotSOSDef.h>
#include "FuncUtil.h"
#include "RiotISO8601.h"
#include <boost/tokenizer.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <sstream>
#include <pion/algorithm.hpp>


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
  boost::mutex::scoped_lock lck(_mRandomGen);

  // This is the way boost says avoid a false positive in valgrind
  boost::mt19937 ran;
  ran.seed(time(NULL));
  boost::uuids::basic_random_generator<boost::mt19937> gen(&ran);
  boost::uuids::uuid u = gen();
  std::ostringstream ss;
  ss << u;
  return ss.str();
};

bool iota::riot_getQueryParams(std::string q_str,
                               std::vector<iota::KVP>& q_param) {
  if (q_str.empty()) {
    return false;
  }

  std::string m_query = pion::algorithm::url_decode(q_str);
  enum QueryParseState {
    QUERY_PARSE_NAME, QUERY_PARSE_VALUE
  } parse_state = QUERY_PARSE_NAME;
  const boost::uint32_t            QUERY_NAME_MAX = 1024;
  const boost::uint32_t            QUERY_VALUE_MAX = 1024*1024;

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
        }
        else if (*ptr == '&') {
          // if query name is empty, just skip it (i.e. "&&")
          if (! query_name.empty()) {
            // assume that "=" is missing -- it's OK if the value is empty
            iota::KVP kvp(query_name, query_value);
            q_param.push_back(kvp);
            query_name.erase();
          }
        }
        else if (*ptr == '\r' || *ptr == '\n') {
          // ignore linefeeds and carriage returns (normally within POST content)
        }
        else if (isControl(*ptr) || query_name.size() >= QUERY_NAME_MAX) {
          // control character detected, or max sized exceeded
          return false;
        }
        else {
          // character is part of the name
          query_name.push_back(*ptr);
        }
        break;
      case QUERY_PARSE_VALUE:
        // parsing query value
        if (*ptr == '&') {
          // end of value found (OK if empty)
          //q_param.insert( std::make_pair(query_name, query_value) );
          iota::KVP kvp(query_name, query_value);
          q_param.push_back(kvp);
          query_name.erase();
          query_value.erase();
          parse_state = QUERY_PARSE_NAME;
        }
        else if (*ptr == '\r' || *ptr == '\n') {
          // ignore linefeeds and carriage returns (normally within POST content)
        }
        else if (isControl(*ptr) || query_value.size() >= QUERY_VALUE_MAX) {
          // control character detected, or max sized exceeded
          return false;
        }
        else {
          // character is part of the value
          query_value.push_back(*ptr);
        }
        break;
    }

    ++ptr;
  }

  // handle last pair in string
  if (! query_name.empty()) {
    iota::KVP kvp(query_name, query_value);
    q_param.push_back(kvp);
    //q_param.insert( std::make_pair(query_name, query_value) );
  }

  return true;
};

bool iota::isControl(int c) {
  return ((c >= 0 && c <= 31) || c == 127);
};

bool iota::isEqualString(bool case_sensitive, std::string str1,
                         std::string str2) {
  if (str1.size() == str2.size()) {
    if (case_sensitive == true) {
      if (str1.compare(str2) == 0) {
        return true;
      }
    }
    else {
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
    trama_hex.append(boost::str(boost::format("%02X ") %
                                static_cast<unsigned short>((unsigned char)(fr.at(i)))));
  }
  return (trama_hex);
};

std::string str_to_hex(boost::asio::mutable_buffer& fr) {
  std::string trama_hex;
  char* p = boost::asio::buffer_cast<char*>(fr);
  int b_size = boost::asio::buffer_size(fr);
  for (int i = 0; i < b_size; i++) {
    trama_hex.append(boost::str(boost::format("%02X ") %
                                static_cast<unsigned short>((unsigned char)(p[i]))));
  }
  return (trama_hex);
};

std::string iota::writeDictionaryTerm(pion::http::response& resp_http) {
  pion::ihash_multimap h = resp_http.get_headers();
  std::string echo;
  for (pion::ihash_multimap::const_iterator val = h.begin();
       val != h.end(); ++val) {
    echo.append(val->first + ":" + val->second + "\n");
  }
  return echo;
}

std::string iota::make_query_string(std::multimap<std::string, std::string>&
                                    query_parameters) {
  std::string query_string;
  std::map<std::string, std::string>::iterator it = query_parameters.begin();
  while (it != query_parameters.end()) {
    query_string.append(it->first + "=" + it->second);
    if (it != query_parameters.begin()) {
      query_string += '&';
    }
    query_string += pion::algorithm::url_encode(it->first);
    query_string += '=';
    query_string += pion::algorithm::url_encode(it->second);

    ++it;
  }
  return query_string;
}

//FF
/*
std::auto_ptr< xsd::cxx::tree::date_time<char,
               xsd::cxx::tree::simple_type<
               xsd::cxx::tree::type> > > string2date_time(std::string str)
{
   std::auto_ptr< xsd::cxx::tree::date_time<char,
                  xsd::cxx::tree::simple_type<
                  xsd::cxx::tree::type> > > ptr_datetime(0);

   iota::RiotISO8601 iso_time(str);

   if ( (iso_time.getDiffUTC().hours() != 0) ||
        (iso_time.getDiffUTC().minutes() != 0) )
   {
      ptr_datetime.reset(new xsd::cxx::tree::date_time<char,
               xsd::cxx::tree::simple_type<
               xsd::cxx::tree::type> > (
                    iso_time.get_year(),
                    iso_time.get_month(),
                    iso_time.get_day(),
                    iso_time.get_hours(),
                    iso_time.get_minutes(),
                    iso_time.get_seconds(),
                    iso_time.getDiffUTC().hours(),
                    iso_time.getDiffUTC().minutes()));
   }
   else if ( iso_time.isLocal() == true )
   {
      ptr_datetime.reset(new xsd::cxx::tree::date_time<char,
               xsd::cxx::tree::simple_type<
               xsd::cxx::tree::type> > (
                    iso_time.get_year(),
                    iso_time.get_month(),
                    iso_time.get_day(),
                    iso_time.get_hours(),
                    iso_time.get_minutes(),
                    iso_time.get_seconds()));
   }
   else
   {
       ptr_datetime.reset(new xsd::cxx::tree::date_time<char,
               xsd::cxx::tree::simple_type<
               xsd::cxx::tree::type> > (
                    iso_time.get_year(),
                    iso_time.get_month(),
                    iso_time.get_day(),
                    iso_time.get_hours(),
                    iso_time.get_minutes(),
                    iso_time.get_seconds(),
                    0,
                    0));
   }

   return (ptr_datetime);
}
*/
