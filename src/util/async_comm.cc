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
#include "async_comm.h"
#include "rest/types.h"
#include "iota_exception.h"
#include <boost/lexical_cast.hpp>
#include <iostream>

iota::CommonAsyncManager::CommonAsyncManager(std::size_t num_threads)
    : _pool(num_threads){}

      ;
void iota::CommonAsyncManager::run(void) {
  _pool.run();
}

;
void iota::CommonAsyncManager::stop(void) {
  _pool.stop();
}

;
iota::CommonIOServicePool::CommonIOServicePool(std::size_t pool_size)
    : _next_io_service(0) {
  if (pool_size == 0) {
    throw iota::IotaException(
        iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER + " [" +
            boost::lexical_cast<std::string>(pool_size) + "]",
        "Valid value is positive number greater than 1",
        iota::types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR);
  }

  for (std::size_t i = 0; i < pool_size; ++i) {
    boost::shared_ptr<boost::asio::io_service> io_service(
        new boost::asio::io_service);
    boost::shared_ptr<boost::asio::io_service::work> work(
        new boost::asio::io_service::work(*io_service));
    _pool_io_services.push_back(io_service);
    _pool_work.push_back(work);
  }
}

void iota::CommonIOServicePool::run() {
  for (std::size_t i = 0; i < _pool_io_services.size(); ++i) {
    boost::shared_ptr<boost::thread> thread(new boost::thread(
        boost::bind(&boost::asio::io_service::run, _pool_io_services[i])));
    threads.push_back(thread);
  }

  // Espera finalizacion
  // Esto solo seria cuando es el unico bucle

  /*
   * for (std::size_t i = 0; i < threads.size(); ++i)
   * threads[i]->join();
   */
}

void iota::CommonIOServicePool::stop() {
  // Explicitly stop all io_services.
  for (std::size_t i = 0; i < _pool_io_services.size(); ++i) {
    _pool_io_services[i]->stop();
  }
  //_pool_work.clear();
  //_pool_io_services.clear();
}

boost::shared_ptr<boost::asio::io_service>&
iota::CommonIOServicePool::get_io_service() {
  boost::mutex::scoped_lock lck(_m);
  boost::shared_ptr<boost::asio::io_service>& io_service =
      _pool_io_services[_next_io_service];

  ++_next_io_service;

  if (_next_io_service == _pool_io_services.size()) {
    _next_io_service = 0;
  }
  return io_service;
}
