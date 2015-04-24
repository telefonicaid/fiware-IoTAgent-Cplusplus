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

namespace iota {
extern std::string logger;
}

iota::MongoConnection::MongoConnection():m_logger(PION_GET_LOGGER(
        iota::logger)) {
  _conex_pool = NULL;
  try{
    reconnect();
  }catch(std::exception exc){
    PION_LOG_ERROR(m_logger, "error in MongoConnection " << exc.what() );
  }
}

iota::MongoConnection::MongoConnection(const std::string& ip,
                                       const std::string& puerto,
                                       const std::string& dbname,
                                       const std::string& user,
                                       const std::string& pwd) {
  try{
    reconnect(ip, puerto, dbname, user, pwd);
  }catch(std::exception exc){
    PION_LOG_ERROR(m_logger, "error in MongoConnection " << exc.what() );
  }
}


void iota::MongoConnection::reconnect(){
  std::string db_info;
  try {
    const JsonValue& storage=
      iota::Configurator::instance()->get(iota::store::types::STORAGE);


    //check if type is mongodb
    if (storage.HasMember(iota::store::types::TYPE.c_str())) {
      std::string type = storage[iota::store::types::TYPE.c_str()].GetString();
      if (type.compare(iota::types::CONF_FILE_MONGO) != 0) {
        PION_LOG_DEBUG(m_logger,
                       "no config for mongodb");
        return;
      }
    }
    else {
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_BAD_CONFIG,
                                " [ response:type ]", iota::types::RESPONSE_CODE_BAD_CONFIG);
    }

    if (storage.HasMember(iota::store::types::HOST.c_str())) {
      _host.assign(storage[iota::store::types::HOST.c_str()].GetString());
      PION_LOG_DEBUG(m_logger, "in storage host " << _host);
    }
    else {
      PION_LOG_DEBUG(m_logger,
                     "in storage no host defined, using localhost by default");
      _host.assign("127.0.0.1");
    }

    if (storage.HasMember(iota::store::types::REPLICA_SET.c_str())) {
      _replica = storage[iota::store::types::REPLICA_SET.c_str()].GetString();
      PION_LOG_DEBUG(m_logger, "in storage replica " << _replica);
    }else{
      //PORT is only used for non replica
      if (storage.HasMember(iota::store::types::PORT.c_str())) {
        _host.append(":");
        _host.append(storage[iota::store::types::PORT.c_str()].GetString());
      }
      else {
        PION_LOG_DEBUG(m_logger, "in storage no port defined, using 27017 by default");
        _host.append(":");
        _host.append("27017");
      }

    }

    if (storage.HasMember(iota::store::types::TIMEOUT.c_str())) {
      std::string timeoutSTR = storage[iota::store::types::TIMEOUT.c_str()].GetString();
      try {
        _timeout = boost::lexical_cast<double>(timeoutSTR);
      }
      catch (std::exception& e) {
        PION_LOG_ERROR(m_logger, "Error in config, bad timeuot defined, use default");
        _timeout = 0;
      }
    }
    else {
      PION_LOG_DEBUG(m_logger,
                     "in storage no timeout defined, using 0 by default");
      _timeout = 0;
    }

    if (storage.HasMember(iota::store::types::DBNAME.c_str())) {
      _database = storage[iota::store::types::DBNAME.c_str()].GetString();
    }
    else {
      PION_LOG_DEBUG(m_logger, "in storage no dbname defined, using iot by default");
      _database = "iot";
    }

    if (storage.HasMember(iota::store::types::USER.c_str())) {
      _usuario = storage[iota::store::types::USER.c_str()].GetString();
    }
    else {
      PION_LOG_DEBUG(m_logger, "in storage no user defined, mongo without auth");
      _usuario = "";
    }

    if (storage.HasMember(iota::store::types::PWD.c_str())) {
      _password = storage[iota::store::types::PWD.c_str()].GetString();
    }
    else {
      PION_LOG_DEBUG(m_logger, "in storage no pwd defined, mongo without auth");
      _password = "";
    }
    db_info.assign("|mongodb=" + _host + "/" + _database);



    if (!_replica.empty()) {
      PION_LOG_DEBUG(m_logger, "Replica is defined " <<  _replica);
      std::vector<mongo::HostAndPort> hosts;

      boost::char_separator<char> sep(",");
      boost::tokenizer< boost::char_separator<char> > tokens(_host, sep);
      BOOST_FOREACH(const std::string& t, tokens) {
        PION_LOG_DEBUG(m_logger,t);
        hosts.push_back(mongo::HostAndPort(t));
      }

      PION_LOG_DEBUG(m_logger,
                     "Conex Mongo Replica set " <<
                     _host << ":" <<
                     "/" << _database << " " <<  _usuario <<
                     "timeout in seconds:" << _timeout);
      mongo::DBClientReplicaSet *rpSet = new mongo::DBClientReplicaSet(_replica, hosts, _timeout);
      rpSet->connect();
      _conex_pool = rpSet;
    }
    else {
      PION_LOG_DEBUG(m_logger,
                     "Conex Mongo DBClientConnection " <<
                     _host <<
                     "/" << _database << " " <<  _usuario <<
                     "timeout in seconds:" << _timeout);

      mongo::DBClientConnection *conn = new mongo::DBClientConnection(true, 0,_timeout);
      conn->connect(_host);
      _conex_pool = conn;
    }

    if (!_usuario.empty()){
        std::string errmsg;
        if (_conex_pool->auth(_database, _usuario, _password,errmsg)){
            PION_LOG_ERROR(m_logger,
                " Mongodb auth ok user: " << _usuario);
        }else{
           PION_LOG_ERROR(m_logger,
             "Error in authenticate Conexion MongoDB " << errmsg);
        }

    }

    iota::Alarm::info(types::ALARM_CODE_NO_MONGO, _replica + " " + _host,
                      types::INFO, "MongoConnection OK");
  }
  catch (mongo::DBException& exc) {
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO,
                       _replica + " " + _host,
                       types::ERROR, exc.what());
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR +
                              " [" + db_info + "]", exc.what(),
                              iota::types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR);
  }
}

void iota::MongoConnection::reconnect(const std::string& ip,
                                       const std::string& puerto,
                                       const std::string& dbname,
                                       const std::string& user,
                                       const std::string& pwd) {
  _host.assign(ip);
  if (!puerto.empty()){
     _host.append(";");
     _host.append(puerto);
  }

  _database = dbname;
  _usuario = user;
  _password = pwd;

  try {
    PION_LOG_DEBUG(m_logger,
                   "Conex Mongo DBClientConnection " << _host <<
                   "/" << _database << " " <<  _usuario);

    mongo::DBClientConnection *conn = new mongo::DBClientConnection(true, 0,_timeout);
    conn->connect(_host);
    _conex_pool = conn;


    if (!_usuario.empty()){
        std::string errmsg;
        if (_conex_pool->auth(_database, _usuario, _password,errmsg)){
            PION_LOG_ERROR(m_logger,
                " Mongodb auth ok user: " << _usuario);
        }else{
           PION_LOG_ERROR(m_logger,
             "Error in authenticate Conexion MongoDB " << errmsg);
        }

    }

    iota::Alarm::info(types::ALARM_CODE_NO_MONGO, _host,
                      types::INFO, "MongoConnection OK");
  }
  catch (mongo::DBException& exc) {
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, _host,
                       types::ERROR, exc.what());
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR+
                              " [" + _host + "]", exc.what(),
                              iota::types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR);
  }

};

iota::MongoConnection::~MongoConnection() {

   if (_conex_pool != NULL){
       delete _conex_pool;
       _conex_pool = NULL;
   }
};

mongo::DBClientBase* iota::MongoConnection::conn() {
  mongo::DBClientBase* conexion = 0;

  if (_conex_pool == 0) {
     reconnect();
  }

  conexion = _conex_pool;

  return conexion;
}

std::string iota::MongoConnection::get_endpoint(){
  if (_replica.empty()){
    return _host;
  }else{
    return _replica;
  }

}

bool iota::MongoConnection::is_valid() {
  if (_conex_pool == 0) {
    return false;
  }else{
    return _conex_pool->isStillConnected ();
  }
}


