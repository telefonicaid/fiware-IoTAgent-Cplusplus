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
#include "csv_reader.h"
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/bind.hpp>
#include <sstream>

void csv_reader(std::string csv_data,
                std::vector<std::map<std::string, std::string> >& data) {
  std::istringstream in(csv_data.c_str());

  typedef boost::tokenizer<boost::escaped_list_separator<char> > Tokenizer;
  std::vector<std::string> items;
  std::vector<std::pair<std::string, std::string> > values;
  std::string line;

  // First line contains items to read
  int line_count = 0;
  int num_items = 0;
  while (getline(in, line)) {
    values.clear();
    Tokenizer tok(line);
    if (line_count == 0) {
      line_count++;
      items.assign(tok.begin(), tok.end());
      num_items = items.size();
    } else {
      Tokenizer::iterator it_tokens = tok.begin();
      int i = 0;
      int n_tokens = 0;
      while (it_tokens != tok.end()) {
        try {
          boost::trim(items.at(i));
          std::string token(*it_tokens);
          boost::trim(token);
          values.push_back(std::make_pair(items.at(i), token));
          ++it_tokens;
          ++i;
          ++n_tokens;
        } catch (std::exception& e) {
          data.clear();
          return;
        }
      }
      if (n_tokens != num_items) {
        data.clear();
        return;
      }
      std::map<std::string, std::string> items_and_values(values.begin(),
                                                          values.end());
      data.push_back(items_and_values);
    }
  }
}
