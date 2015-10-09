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

#ifndef SRC_UTIL_MONGO_CONNECTION_H_
#define SRC_UTIL_MONGO_CONNECTION_H_

#include <iostream>

#include "util/iota_logger.h"
#include "mongo/client/dbclient.h"
#include <boost/lockfree/queue.hpp>

namespace iota {

class MongoConnection {
 public:
  static MongoConnection* instance();
  static void release();

  virtual ~MongoConnection();

  void reconnect();
  mongo::DBClientBase* createConnection();

  mongo::DBClientBase* conn();
  void done(mongo::DBClientBase*);

  std::string get_host() { return _host; }

  std::string get_database() { return _database; }
  std::string get_usuario() { return _usuario; }
  std::string get_replica() { return _replica; }

  std::string get_endpoint();

 private:
  static MongoConnection* pinstance;

  MongoConnection();

  boost::mutex _m;

  pion::logger m_logger;

  /**  host to connect (if replica is host1:port1,host2:port2
       default 127.0.0.1 **/
  std::string _host;

  /**  name of the replica set **/
  std::string _replica;

  /**  database name for the application **/
  std::string _database;
  /**  if exists indicates that auth is it necessary,
       name of the user **/
  std::string _usuario;
  /**  password for database **/
  std::string _password;

  /**  socket Timeout in seconds, default 0 **/
  double _timeout;

  boost::lockfree::queue<mongo::DBClientBase*> _conex_pool;
};

}  // Fin namespace iota

#endif
