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
#ifndef SRC_UTIL_CACHE_H_
#define SRC_UTIL_CACHE_H_

#include "device.h"
#include "rest/process.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/function.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/asio.hpp>
#include <stdexcept>

// Cache algorithm is based on discards.
// LRU: discards oldest
// MRU: discards most recently (fetch and discard)
// MRU: if cache is full, don't push any element. It is recommended
// to use mru with cache clock

namespace iota {

class Cache {
  typedef boost::multi_index_container<
      boost::shared_ptr<Device>,
      boost::multi_index::indexed_by<
          boost::multi_index::sequenced<>,
          boost::multi_index::hashed_unique<
              boost::multi_index::identity<Device> >,
          boost::multi_index::hashed_non_unique<
              boost::multi_index::const_mem_fun<Device, std::string,
                                                &Device::unique_entity> >,
          boost::multi_index::hashed_non_unique<
              boost::multi_index::const_mem_fun<
                  Device, std::string, &Device::unique_name> > > > item_list;

 public:
  typedef item_list::iterator iterator;
  typedef boost::multi_index::nth_index<item_list, 1>::type::iterator
      item_iterator;
  typedef boost::multi_index::nth_index<item_list, 2>::type::iterator
      item_iterator_entity;
  typedef boost::multi_index::nth_index<item_list, 3>::type::iterator
      item_iterator_name;

  // Function to get an element out of cache
  typedef boost::function<boost::shared_ptr<Device>(boost::shared_ptr<Device>&)>
      GetFunction_t;

  Cache(std::size_t capacity, bool lru, unsigned short time_life = 0);

  ~Cache() {
  };

  void set_function(GetFunction_t get_function) {
    _get_function = get_function;
  }

  void set_entity_function(GetFunction_t get_function) {
    _get_entity_function = get_function;
  }

  void insert(const boost::shared_ptr<Device>& item);

  boost::shared_ptr<Device> get(boost::shared_ptr<Device> key) {
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
    boost::shared_ptr<Device> item;
    item_iterator i = boost::multi_index::get<1>(_list).begin();
    i = boost::multi_index::get<1>(_list).find(*key);
    if (i != boost::multi_index::get<1>(_list).end()) {
      insert(*i);
      item = *i;
      return item;
    } else {
      if (_get_function != NULL) {
        const boost::shared_ptr<Device> new_item = _get_function(key);
        if (new_item.get() != NULL) {
          insert(new_item);
        }
        return new_item;
      }
    }
    return item;
  }

  boost::shared_ptr<Device> get_by_entity(boost::shared_ptr<Device> key) {
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
    boost::shared_ptr<Device> item;
    item_iterator_entity i = boost::multi_index::get<2>(_list).begin();
    i = boost::multi_index::get<2>(_list)
            .find(*key, entity_hash(), entity_equal());
    if (i != boost::multi_index::get<2>(_list).end()) {
      insert(*i);
      item = *i;
      return item;
    } else {
      if (_get_function != NULL) {
        const boost::shared_ptr<Device> new_item = _get_entity_function(key);
        if (new_item.get() != NULL) {
          insert(new_item);
        }
        return new_item;
      }
    }
    return item;
  }

  boost::shared_ptr<Device> get_by_name(boost::shared_ptr<Device> key) {
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
    boost::shared_ptr<Device> item;
    item_iterator_name i = boost::multi_index::get<3>(_list).begin();
    i = boost::multi_index::get<3>(_list).find(*key, name_hash(), name_equal());
    if (i != boost::multi_index::get<3>(_list).end()) {
      insert(*i);
      item = *i;
      return item;
    } else {
      if (_get_function != NULL) {
        const boost::shared_ptr<Device> new_item = _get_entity_function(key);
        if (new_item.get() != NULL) {
          insert(new_item);
        }
        return new_item;
      }
    }
    return item;
  }

  std::size_t size() {
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
    return _list.size();
  }

  void remove(Device item) {
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
    item_iterator i = boost::multi_index::get<1>(_list).begin();
    i = boost::multi_index::get<1>(_list).find(item);
    if (i != boost::multi_index::get<1>(_list).end()) {
      boost::shared_ptr<Device> item = *i;
      item->cancel();
      boost::multi_index::get<1>(_list).erase(i);
    }
  }

  void item_timeout(boost::shared_ptr<Timer> timer,
                    const boost::system::error_code& ec) {
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);

    boost::shared_ptr<Device> item_tmp =
        boost::dynamic_pointer_cast<Device>(timer);
    if (ec == boost::asio::error::operation_aborted) {
      return;
    }
    boost::shared_ptr<Device> item = boost::dynamic_pointer_cast<Device>(timer);
    remove(*item);
  }

  void set_time_to_life(unsigned short time_life) {
    if (time_life == 0) return;

    if (_time_life == 0) {
      _time_life = time_life;
    }
  };

 private:
  std::size_t _max_num_items;
  item_list _list;
  bool _lru;
  unsigned short _time_life;
  GetFunction_t _get_function;
  GetFunction_t _get_entity_function;
  boost::recursive_mutex m_mutex;
  pion::logger m_logger;
};
}
#endif
