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

#include "command_cache.h"

iota::CommandCache::CommandCache(std::size_t capacity, bool lru)
      : _max_num_items(capacity),
        _lru(lru),
        _get_function(NULL),
        _timeout_function(NULL),
        m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {};


void iota::CommandCache::insert(const boost::shared_ptr<Command>& item) {
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
    std::pair<iterator, bool> p;
    if (_lru) {
      p = _list.push_front(item);
      if (item->get_timeout() > 0) {
        // timeout is in seconds  for this *1000
        IOTA_LOG_INFO(m_logger, "commandtimeout" << item->get_id() <<
                      " status: " << item->get_status() << " tout:" <<  item->get_timeout());
        boost::shared_ptr<boost::asio::deadline_timer> t = item->start(
            iota::Process::get_process().get_io_service(), item->get_timeout());
        t->async_wait(boost::bind(&CommandCache::item_timeout, this, item,
                                  boost::asio::placeholders::error));
      }
      if (!p.second) {  // Duplicated
        _list.relocate(_list.begin(), p.first);
      } else if (_list.size() > _max_num_items) {
        _list.back()->cancel();
        _list.pop_back();
      }

    } else {
      if (_list.size() < _max_num_items) {
        p = _list.push_back(item);
        if (!p.second) {
          _list.relocate(_list.end(), p.first);
        }
        if (item->get_timeout() > 0) {
          IOTA_LOG_INFO(m_logger, "commandtimeout" << item->get_id() <<
                      " status: " << item->get_status() << " tout:" <<  item->get_timeout());
          boost::shared_ptr<boost::asio::deadline_timer> t = item->start(
              iota::Process::get_process().get_io_service(), item->get_timeout());
          t->async_wait(boost::bind(&CommandCache::item_timeout, this, item,
                                    boost::asio::placeholders::error));
        }
      }
    }
}

