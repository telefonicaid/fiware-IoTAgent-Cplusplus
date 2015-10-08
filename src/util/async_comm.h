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
#ifndef SRC_UTIL_ASYNC_COMM_H_
#define SRC_UTIL_ASYNC_COMM_H_

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace iota {

class CommonIOServicePool : private boost::noncopyable {
 public:
  // Constructor para un pool de io_service.
  explicit CommonIOServicePool(std::size_t pool_size);

  // Ejecuta los io_service.
  void run();

  // para los io_service
  void stop();

  // Recupera el io_service a utilizar
  boost::shared_ptr<boost::asio::io_service>& get_io_service();

 private:
  // El pool de io_services
  std::vector<boost::shared_ptr<boost::asio::io_service> > _pool_io_services;
  std::vector<boost::shared_ptr<boost::asio::io_service::work> > _pool_work;

  // Vector con threads
  std::vector<boost::shared_ptr<boost::thread> > threads;

  // io_service a utilizar
  std::size_t _next_io_service;

  // Bloqueo
  boost::mutex _m;
};

class CommonAsyncManager {
 public:
  explicit CommonAsyncManager(std::size_t num_threads);
  virtual ~CommonAsyncManager(){
      // std::cout << "DESTRUCTOR CommonAsyncManager" << std::endl;
  };

  // Run el bucle de io_service
  void run();

  // Para
  void stop();

  // Recupera io_service
  inline boost::shared_ptr<boost::asio::io_service>& get_io_service(void) {
    return (_pool.get_io_service());
  };

 protected:
 private:
  // / io_service que realiza la oepraciones asincronas
  CommonIOServicePool _pool;
};
};

#endif /* COMMONASYNCMANAGER_H */
