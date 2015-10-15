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

#include "cache.h"

iota::Cache::Cache(std::size_t capacity, bool lru, unsigned short time_life)
    : _max_num_items(capacity),
      _lru(lru),
      _time_life(time_life),
      _get_function(NULL),
      m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
};

void iota::Cache::insert(const boost::shared_ptr<Device>& item) {
  boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
  std::pair<iterator, bool> p;
  if (_lru) {
    p = _list.push_front(item);
    if (_time_life > 0) {
      boost::shared_ptr<boost::asio::deadline_timer> t = item->start(iota::Process::get_process().get_io_service(), _time_life);
      t->async_wait(boost::bind(&Cache::item_timeout, this, item,
                                boost::asio::placeholders::error));
    }
    if (!p.second) {  // Duplicated
      _list.relocate(_list.begin(), p.first);
    } else if (_list.size() > _max_num_items) {
      if (_time_life > 0) {
        _list.back()->cancel();
      }
      _list.pop_back();
    }

  } else {
    if (_list.size() < _max_num_items) {
      p = _list.push_back(item);
      if (!p.second) {
        _list.relocate(_list.end(), p.first);
      }
      if (_time_life > 0) {
        boost::shared_ptr<boost::asio::deadline_timer> t = item->start(iota::Process::get_process().get_io_service(), _time_life);
        t->async_wait(boost::bind(&Cache::item_timeout, this, item,
                                  boost::asio::placeholders::error));
      }
    }
  }
}
