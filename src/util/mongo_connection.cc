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
#include "mongo_connection.h"

#include <mongo/client/dbclient_rs.h>

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include "common.h"
#include "iota_exception.h"
#include "rest/riot_conf.h"
#include "store_const.h"
#include "alarm.h"

#define SIZE_POOL 10
#define MAX_SIZE_POOL 1000

iota::MongoConnection* iota::MongoConnection::pinstance =
    0;  // Inicializar el puntero

const int MONGO_TIMEOUT = 3;

iota::MongoConnection::MongoConnection()
    : _conex_pool(SIZE_POOL),
      m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  try {
    reconnect();
  } catch (std::exception exc) {
    IOTA_LOG_ERROR(m_logger, "error in MongoConnection " << exc.what());
  }
}

void iota::MongoConnection::reconnect() {
  boost::mutex::scoped_lock lock(_m);
  std::string db_info;

  IOTA_LOG_DEBUG(m_logger, "MongoConnection::reconnect, remove old connections");
  mongo::DBClientBase* conToDelete;
  while (_conex_pool.pop(conToDelete)) {
    if (conToDelete != NULL) {
      delete conToDelete;
    }
  }

  try {
    const JsonValue& storage =
        iota::Configurator::instance()->get(iota::store::types::STORAGE);

    // check if type is mongodb
    if (storage.HasMember(iota::store::types::TYPE.c_str())) {
      std::string type = storage[iota::store::types::TYPE.c_str()].GetString();
      if (type.compare(iota::types::CONF_FILE_MONGO) != 0) {
        IOTA_LOG_DEBUG(m_logger, "no config for mongodb");
        return;
      }
    } else {
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_CONFIG,
                                " [ response:type ]",
                                iota::types::RESPONSE_CODE_BAD_CONFIG);
    }

    if (storage.HasMember(iota::store::types::HOST.c_str())) {
      _host.assign(storage[iota::store::types::HOST.c_str()].GetString());
      IOTA_LOG_DEBUG(m_logger, "in storage host " << _host);
    } else {
      IOTA_LOG_DEBUG(m_logger,
                     "in storage no host defined, using localhost by default");
      _host.assign("127.0.0.1");
    }

    int pool_size = SIZE_POOL;
    if (storage.HasMember(iota::store::types::POOL_SIZE.c_str())) {
      std::string pool_sizeSTR =
          storage[iota::store::types::POOL_SIZE.c_str()].GetString();
      try {
        pool_size = boost::lexical_cast<int>(pool_sizeSTR);
        _conex_pool.reserve(pool_size);
        IOTA_LOG_DEBUG(m_logger, "pool size " << pool_size);
      } catch (std::exception& e) {
        IOTA_LOG_ERROR(m_logger, "Error in config, bad pool size, use default");
        pool_size = SIZE_POOL;
      }
    }

    if (storage.HasMember(iota::store::types::REPLICA_SET.c_str())) {
      _replica = storage[iota::store::types::REPLICA_SET.c_str()].GetString();
      IOTA_LOG_DEBUG(m_logger, "in storage replica " << _replica);
    } else {
      // PORT is only used for non replica
      if (storage.HasMember(iota::store::types::PORT.c_str())) {
        _host.append(":");
        _host.append(storage[iota::store::types::PORT.c_str()].GetString());
      } else {
        IOTA_LOG_DEBUG(m_logger,
                       "in storage no port defined, using 27017 by default");
        _host.append(":");
        _host.append("27017");
      }
    }

    if (storage.HasMember(iota::store::types::TIMEOUT.c_str())) {
      std::string timeoutSTR =
          storage[iota::store::types::TIMEOUT.c_str()].GetString();
      try {
        _timeout = boost::lexical_cast<double>(timeoutSTR);
      } catch (std::exception& e) {
        IOTA_LOG_ERROR(m_logger,
                       "Error in config, bad timeuot defined, use default");
        _timeout = MONGO_TIMEOUT;
      }
    } else {
      IOTA_LOG_DEBUG(m_logger,
                     "in storage no timeout defined, using 0 by default");
      _timeout = MONGO_TIMEOUT;
    }

    if (storage.HasMember(iota::store::types::DBNAME.c_str())) {
      _database = storage[iota::store::types::DBNAME.c_str()].GetString();
    } else {
      IOTA_LOG_DEBUG(m_logger,
                     "in storage no dbname defined, using iot by default");
      _database = "iot";
    }

    if (storage.HasMember(iota::store::types::USER.c_str())) {
      _usuario = storage[iota::store::types::USER.c_str()].GetString();
    } else {
      IOTA_LOG_DEBUG(m_logger,
                     "in storage no user defined, mongo without auth");
      _usuario = "";
    }

    if (storage.HasMember(iota::store::types::PWD.c_str())) {
      _password = storage[iota::store::types::PWD.c_str()].GetString();
    } else {
      IOTA_LOG_DEBUG(m_logger, "in storage no pwd defined, mongo without auth");
      _password = "";
    }
    db_info.assign(" mongodb=" + _host + "/" + _database);

    mongo::DBClientBase* con;
    bool no_empty_pool = true;
    int num_conns = 0;
    for (int i = 0; no_empty_pool && i < pool_size; i++) {
      con = createConnection();
      if (con != NULL) {
        no_empty_pool = _conex_pool.push(con);
        num_conns++;
      }
    }
    iota::Alarm::info(types::ALARM_CODE_NO_MONGO, _replica + " " + _host,
                      types::INFO, "MongoConnection OK");
  } catch (mongo::DBException& exc) {
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, _replica + " " + _host,
                       types::ERROR, exc.what());
    throw iota::IotaException(
        iota::types::RESPONSE_MESSAGE_DATABASE_ERROR + " [" + db_info + "]",
        exc.what(), iota::types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR);
  }
}

mongo::DBClientBase* iota::MongoConnection::createConnection() {
  mongo::DBClientBase* res = NULL;

  try {
    if (!_replica.empty()) {
      IOTA_LOG_DEBUG(m_logger, "Replica is defined " << _replica);
      std::vector<mongo::HostAndPort> hosts;

      boost::char_separator<char> sep(",");
      boost::tokenizer<boost::char_separator<char> > tokens(_host, sep);
      BOOST_FOREACH (const std::string& t, tokens) {
        IOTA_LOG_DEBUG(m_logger, t);
        hosts.push_back(mongo::HostAndPort(t));
      }

      IOTA_LOG_DEBUG(m_logger, "Conex Mongo Replica set "
                                   << _host << ":"
                                   << "/" << _database << " " << _usuario
                                   << "timeout in seconds:" << _timeout);
      mongo::DBClientReplicaSet* rpSet =
          new mongo::DBClientReplicaSet(_replica, hosts, _timeout);
      rpSet->connect();
      res = rpSet;
    } else {
      IOTA_LOG_DEBUG(m_logger, "Conex Mongo DBClientConnection "
                                   << _host << "/" << _database << " "
                                   << _usuario
                                   << "timeout in seconds:" << _timeout);

      mongo::DBClientConnection* conn =
          new mongo::DBClientConnection(true, 0, _timeout);
      conn->connect(_host);
      res = conn;
    }

    if (!_usuario.empty()) {
      std::string errmsg;
      if (res->auth(_database, _usuario, _password, errmsg)) {
        IOTA_LOG_ERROR(m_logger, " Mongodb auth ok user: " << _usuario);
      } else {
        IOTA_LOG_ERROR(m_logger, "Error in authenticate Conexion MongoDB "
                                     << errmsg);
      }
    }
  } catch (mongo::DBException& e) {
    std::string errorSTR = "DBException ";
    errorSTR.append(e.what());
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, 500);
  }

  return res;
};

iota::MongoConnection::~MongoConnection() {
  boost::mutex::scoped_lock lock(_m);
  mongo::DBClientBase* con;
  while (_conex_pool.pop(con)) {
    if (con != NULL) {
      delete con;
    }
  }
};

mongo::DBClientBase* iota::MongoConnection::conn() {
  boost::mutex::scoped_lock lock(_m);
  mongo::DBClientBase* conexion = 0;

  if (!_conex_pool.pop(conexion)) {
    IOTA_LOG_ERROR(m_logger, "It has reached the maximum mongo pool");
  }

  if (conexion == 0) {
    IOTA_LOG_ERROR(m_logger, "create a new con");
    conexion = createConnection();
  }
  IOTA_LOG_DEBUG(m_logger, "MongoConnection::conn returns " << conexion);
  return conexion;
}

void iota::MongoConnection::done(mongo::DBClientBase* conexion) {
  boost::mutex::scoped_lock lock(_m);
  if (!_conex_pool.push(conexion)) {
    IOTA_LOG_ERROR(m_logger,
                   "It has reached the maximum mongo pool, delete con");
  }
}

std::string iota::MongoConnection::get_endpoint() {
  if (_replica.empty()) {
    return _host;
  } else {
    return _replica;
  }
}

iota::MongoConnection* iota::MongoConnection::instance() {
  if (pinstance == 0) {
    pinstance = new MongoConnection();
  }
  return pinstance;
}

void iota::MongoConnection::release() {
  if (pinstance != 0) {
    delete pinstance;
    pinstance = 0;
  }
}
