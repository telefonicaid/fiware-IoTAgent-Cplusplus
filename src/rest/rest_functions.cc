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
#include <boost/regex.hpp>
#include <pion/http/types.hpp>
#include "util/FuncUtil.h"

#include "rest/rest_functions.h"

bool iota::restResourceParse(std::string& regex,
                             std::vector<std::string>& url_placeholders,
                             std::string& url_resource,
                             std::map<std::string, std::string>& resources) {
  boost::regex resource_re(regex, boost::regex::perl);
  boost::smatch m;
  bool res = false;
  std::string data, datad;

  if (regex_match(url_resource, m, resource_re)) {
    res = true;
    int i = 1;
    for (i = 1; i < m.size(); i++) {
      // Only store placeholders if provided.
      if (url_placeholders.size() > 0) {
        data = m[i];
        datad = iota::url_decode(data);
        resources.insert(std::make_pair<std::string, std::string>(
            url_placeholders.at(i - 1), datad));
      }
    }
  }
  return res;
}

void iota::format_pattern(std::string& url,
                          std::map<std::string, std::string>& filters,
                          std::string& url_regex,
                          std::vector<std::string>& url_args) {
  // Regular expression to match placeholders
  // boost::regex search_re("(POST|GET[[:space:]]+.+/)<([a-zA-Z0-9_-]+)>");
  boost::regex search_re("<([a-zA-Z0-9_-]+)>");
  boost::match_results<std::string::const_iterator> what;
  std::string::const_iterator start(url.begin());
  std::string::const_iterator end(url.end());
  boost::match_flag_type flags = boost::match_default;

  // Store temporary suffix
  std::string tmp_suffix;

  // To control if an element is repeated
  // Store element and position
  std::map<std::string, std::string::const_iterator> tmp_placeholders;

  while (boost::regex_search(start, end, what, search_re, flags)) {
    try {
      if (tmp_placeholders[what.prefix()] != what.prefix().first) {
        url_regex.append(what.prefix());
        tmp_placeholders.insert(
            std::make_pair<std::string, std::string::const_iterator>(
                what.prefix(), what.prefix().first));
      }

      if (tmp_placeholders[what.str(0)] != what[0].first) {
        url_regex.append("([^/]+)");
        tmp_placeholders.insert(
            std::make_pair<std::string, std::string::const_iterator>(
                what.str(0), what[0].first));
        url_args.push_back(what.str(1));
      }

      if (tmp_placeholders[what.suffix()] != what.suffix().first) {
        tmp_placeholders.insert(
            std::make_pair<std::string, std::string::const_iterator>(
                what.suffix(), what.suffix().first));
        tmp_suffix = what.suffix();
      }

      // Update
      start = what[0].second;
      flags |= boost::match_prev_avail;
      flags |= boost::match_not_bob;
    } catch (std::exception& e) {
      // TODO
    }
  }

  url_regex.append(tmp_suffix);

  // Fixed url
  if (url_regex.empty() == true) {
    url_regex.assign(url);
  }

  // Add filter
  // Now only method
  url_regex.insert(0, "[[:space:]]+");
  if (filters["method"].empty() == true) {
    url_regex.insert(0, ".+");
  } else {
    url_regex.insert(0, filters["method"]);
  }
}
