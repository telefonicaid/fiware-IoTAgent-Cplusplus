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
#ifndef SRC_UTIL_COMMAND_CACHE_H_
#define SRC_UTIL_COMMAND_CACHE_H_

#include "command.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/function.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/asio.hpp>
#include <stdexcept>
#include <rest/iot_cb_comm.h>

namespace iota {

typedef boost::shared_ptr<Command> CommandPtr;
typedef std::vector<CommandPtr> CommandVect;

// Cache algorithm is based on discards.
// LRU: discards oldest
// MRU: discards most recently (fetch and discard)
// MRU: if cache is full, don't push any element. It is recommended
// to use mru with cache clock

class CommandCache {
  typedef boost::multi_index_container<
      boost::shared_ptr<Command>,
      boost::multi_index::indexed_by<
          boost::multi_index::sequenced<>,
          boost::multi_index::hashed_unique<
              boost::multi_index::identity<Command> >,
          boost::multi_index::hashed_non_unique<
              boost::multi_index::const_mem_fun<Command, std::string,
                                                &Command::unique_entity> >,
          boost::multi_index::hashed_non_unique<
              boost::multi_index::const_mem_fun<
                  Command, std::string, &Command::unique_id> > > > item_list;

 public:
  typedef item_list::iterator iterator;
  typedef boost::multi_index::nth_index<item_list, 1>::type::iterator
      item_iterator;
  typedef boost::multi_index::nth_index<item_list, 2>::type::iterator
      item_iterator_entity;
  typedef boost::multi_index::nth_index<item_list, 3>::type::iterator
      item_iterator_id;

  // Function to get an element out of cache
  typedef boost::function<boost::shared_ptr<Command>(
      boost::shared_ptr<Command>&)> GetFunction_t;

  CommandCache(std::size_t capacity, bool lru);

  ~CommandCache() {};

  void set_max_num_items(std::size_t capacity) { _max_num_items = capacity; }

  void set_function(GetFunction_t get_function) {
    _get_function = get_function;
  }

  void set_timeout_function(GetFunction_t timeout_function) {
    _timeout_function = timeout_function;
  }

  void set_entity_function(GetFunction_t get_function) {
    _get_entity_function = get_function;
  }

  void set_id_function(GetFunction_t get_function) {
    _get_id_function = get_function;
  }

  void insert(const boost::shared_ptr<Command>& item);

  boost::shared_ptr<Command> get(boost::shared_ptr<Command> key) {
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
    boost::shared_ptr<Command> item;
    item_iterator i = boost::multi_index::get<1>(_list).begin();
    i = boost::multi_index::get<1>(_list).find(*key);
    if (i != boost::multi_index::get<1>(_list).end()) {
      insert(*i);
      item = *i;
      return item;
    } else {
      if (_get_function != NULL) {
        const boost::shared_ptr<Command> new_item = _get_function(key);
        if (new_item.get() != NULL) {
          insert(new_item);
        }
        return new_item;
      }
    }
    return item;
  }

  boost::shared_ptr<Command> get_by_entity(boost::shared_ptr<Command> key) {
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
    boost::shared_ptr<Command> item;
    item_iterator_entity i = boost::multi_index::get<2>(_list).begin();
    i = boost::multi_index::get<2>(_list)
            .find(*key, entity_command_hash(), entity_command_equal());
    if (i != boost::multi_index::get<2>(_list).end()) {
      insert(*i);
      item = *i;
      return item;
    } else {
      if (_get_function != NULL) {
        const boost::shared_ptr<Command> new_item = _get_entity_function(key);
        if (new_item.get() != NULL) {
          insert(new_item);
        }
        return new_item;
      }
    }
    return item;
  }

  CommandVect get_by_entityV(boost::shared_ptr<Command> key, int new_status) {
    CommandVect result;
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
    boost::shared_ptr<Command> item;
    item_iterator_entity i = boost::multi_index::get<2>(_list).begin();
    i = boost::multi_index::get<2>(_list)
            .find(*key, entity_command_hash(), entity_command_equal());
    if (i != boost::multi_index::get<2>(_list).end()) {
      while (i != boost::multi_index::get<2>(_list).end()) {
        boost::shared_ptr<Command> item;
        (*i)->set_status(new_status);
        insert(*i);
        item = *i;
        i++;
        result.push_back(item);
      }
    } else {
      if (_get_function != NULL) {
        // TODO la funcion _get_entity_function solo devuelve uno
        const boost::shared_ptr<Command> new_item = _get_entity_function(key);
        if (new_item.get() != NULL) {
          new_item->set_status(new_status);
          insert(new_item);
          result.push_back(new_item);
        }
      }
    }

    return result;
  }

  boost::shared_ptr<Command> get_by_id(boost::shared_ptr<Command> key) {
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
    boost::shared_ptr<Command> item;
    item_iterator_id i = boost::multi_index::get<3>(_list).begin();
    i = boost::multi_index::get<3>(_list)
            .find(*key, id_command_hash(), id_command_equal());
    if (i != boost::multi_index::get<3>(_list).end()) {
      insert(*i);
      item = *i;
      return item;
    } else {
      if (_get_function != NULL) {
        const boost::shared_ptr<Command> new_item = _get_id_function(key);
        if (new_item.get() != NULL) {
          insert(new_item);
        }
        return new_item;
      }
    }
    return item;
  }

  boost::shared_ptr<Command> get_by_id_from_mongo(boost::shared_ptr<Command> key) {
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
    boost::shared_ptr<Command> item;
    if (_get_function != NULL) {
        const boost::shared_ptr<Command> new_item = _get_id_function(key);
        return new_item;
    }
    return item;
  }

  CommandVect get_by_idV(boost::shared_ptr<Command> key) {
    CommandVect result;
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
    boost::shared_ptr<Command> item;
    item_iterator_id i = boost::multi_index::get<3>(_list).begin();
    i = boost::multi_index::get<3>(_list)
            .find(*key, id_command_hash(), id_command_equal());
    while (i != boost::multi_index::get<3>(_list).end()) {
      boost::shared_ptr<Command> item;
      insert(*i);
      item = *i;
      i++;
      result.push_back(item);
    }

    return result;
  }

  std::size_t size() {
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
    return _list.size();
  }

  void remove(boost::shared_ptr<Command> key) {
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
    item_iterator_id i = boost::multi_index::get<3>(_list).begin();
    i = boost::multi_index::get<3>(_list)
            .find(*key, id_command_hash(), id_command_equal());
    if (i != boost::multi_index::get<3>(_list).end()) {
      boost::shared_ptr<Command> item;
      item = *i;
      item->cancel();
      boost::multi_index::get<3>(_list).erase(i);
    }
  }

  void remove_all() {
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
    item_iterator_id i = boost::multi_index::get<3>(_list).begin();
    if (i != boost::multi_index::get<3>(_list).end()) {
      boost::multi_index::get<3>(_list).erase(i);
    }
  }

  void item_timeout(boost::shared_ptr<Timer> timer,
                    const boost::system::error_code& ec) {
    boost::unique_lock<boost::recursive_mutex> scoped_lock(m_mutex);
    boost::shared_ptr<Command> item_tmp =
        boost::dynamic_pointer_cast<Command>(timer);

    if (ec == boost::asio::error::operation_aborted) {
      return;
    }
    boost::shared_ptr<Command> item =
        boost::dynamic_pointer_cast<Command>(timer);

    if (_timeout_function != NULL) {
      _timeout_function(item);
    }
    remove(item);
  }

 private:
  std::size_t _max_num_items;
  item_list _list;
  bool _lru;
  GetFunction_t _get_function;
  GetFunction_t _get_entity_function;
  GetFunction_t _get_id_function;
  GetFunction_t _timeout_function;
  pion::logger m_logger;
  boost::recursive_mutex m_mutex;
};
};
#endif
