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
#include <iostream>

#include "mongo/client/dbclient.h"
#include "mongo/client/dbclientcursor.h"
#include "rest/types.h"
#include "util/store_const.h"
#include "util/iota_exception.h"
#include "alarm.h"
#include <boost/thread/thread.hpp>

#include "collection.h"

#define MAX_RETRY 1
#define SLEEP_TIME 250
#define ERROR_MONGO 500

iota::Collection::Collection(const std::string& collection_name)
    : m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  a_queryOptions = Options::Primary;
  setBBDD(collection_name);
  _m_conn = MongoConnection::instance()->conn();
};

iota::Collection::Collection(Collection& c)
    : m_logger(PION_GET_LOGGER(iota::Process::get_logger_name())) {
  a_bbdd = c.a_bbdd;
  _m_conn = MongoConnection::instance()->conn();
}

iota::Collection::~Collection() {
  if (a_cursor.get() != NULL) {
    a_cursor->decouple();
  }

  MongoConnection::instance()->done(_m_conn);
};

const std::string iota::Collection::_EMPTY;

mongo::BSONObj iota::Collection::findAndModify(const std::string& table,
                                               const mongo::BSONObj& query,
                                               const mongo::BSONObj& set,
                                               bool upsert /* = 1 */,
                                               int retry) {
  mongo::BSONObjBuilder obj;
  mongo::BSONObj res, r;

  obj.append("findandmodify", table);
  obj.append("query", query);
  obj.append("update", set);
  if (upsert) {
    obj.append("upsert", true);
  }

  r = obj.obj();
  std::string bbdd = getDatabaseName();

  std::string param_request("Collection:findAndModify bbdd=" + bbdd + " data=" +
                            r.toString());
  IOTA_LOG_DEBUG(m_logger, param_request);
  // int errCode = 0;
  // mongo::BSONObj errObj;
  try {
    _m_conn->runCommand(bbdd, r, res);
    // obtenemos el id y ok
    // errObj = conn->getLastErrorDetailed();
    // errCode = errObj["code"].numberInt();
  } catch (mongo::DBException& exc) {
    // try again
    reconnect();
    std::string original_exc(exc.what());
    try {
      _m_conn->runCommand(bbdd, r, res);
      // errObj = conn->getLastErrorDetailed();
      // errCode = errObj["code"].numberInt();
    } catch (std::exception& e) {
      iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(),
                         types::ERROR, e.what());
      reconnect();
      throw iota::IotaException(
          iota::types::RESPONSE_MESSAGE_DATABASE_ERROR, original_exc,
          iota::types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR);
    }
  }

  getLastError(bbdd, res);
  IOTA_LOG_DEBUG(m_logger, "Find and notify query:" << r.toString());
  IOTA_LOG_DEBUG(m_logger, "Find and notify result:" << res.toString());

  return res;
}

mongo::BSONObj iota::Collection::aggregate(
    const std::vector<mongo::BSONObj>& pipeline, int retry) {
  std::string bbdd = getDatabaseName();
  mongo::BSONObj res, r;
  mongo::BSONObjBuilder obj;

  obj.append("aggregate", a_bbdd);
  obj.append("pipeline", pipeline);

  r = obj.obj();

  std::string param_request("Collection:aggregate bbdd=" + bbdd + " data=" +
                            r.toString());
  IOTA_LOG_DEBUG(m_logger, param_request);
  // int errCode = 0;
  // mongo::BSONObj errObj;
  try {
    _m_conn->runCommand(bbdd, r, res);
    // obtenemos el id y ok
    // errObj = conn->getLastErrorDetailed();
    // errCode = errObj["code"].numberInt();
  } catch (mongo::DBException& exc) {
    // try again
    std::string original_exc(exc.what());
    reconnect();
    try {
      _m_conn->runCommand(bbdd, r, res);
      // errObj = conn->getLastErrorDetailed();
      // errCode = errObj["code"].numberInt();
    } catch (std::exception& e) {
      iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(),
                         types::ERROR, e.what());
      reconnect();
      throw iota::IotaException(
          iota::types::RESPONSE_MESSAGE_DATABASE_ERROR, original_exc,
          iota::types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR);
    }
  }

  getLastError(bbdd, res);
  IOTA_LOG_DEBUG(m_logger, "aggregate query:" << r.toString());
  IOTA_LOG_DEBUG(m_logger, "aggregate result:" << res.toString());

  return res;
}

mongo::BSONObj iota::Collection::distinct(const std::string& field,
                                          const mongo::BSONObj& query,
                                          int retry) {
  std::string bbdd = getDatabaseName();
  mongo::BSONObj res, r;
  mongo::BSONObjBuilder obj;

  obj.append("distinct", a_bbdd);
  obj.append("key", field);
  obj.append("query", query);

  r = obj.obj();

  std::string param_request("Collection:distinct bbdd=" + bbdd + " data=" +
                            r.toString());
  IOTA_LOG_DEBUG(m_logger, param_request);
  // int errCode = 0;
  // mongo::BSONObj errObj;
  try {
    _m_conn->runCommand(bbdd, r, res);
    // obtenemos el id y ok
    // errObj = conn->getLastErrorDetailed();
    // errCode = errObj["code"].numberInt();
  } catch (mongo::DBException& exc) {
    // try again
    std::string original_exc(exc.what());
    reconnect();
    try {
      _m_conn->runCommand(bbdd, r, res);
      // errObj = conn->getLastErrorDetailed();
      // errCode = errObj["code"].numberInt();
    } catch (std::exception& e) {
      iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(),
                         types::ERROR, e.what());
      reconnect();
      throw iota::IotaException(
          iota::types::RESPONSE_MESSAGE_DATABASE_ERROR, original_exc,
          iota::types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR);
    }
  }

  getLastError(bbdd, res);
  IOTA_LOG_DEBUG(m_logger, "distinct query:" << r.toString());
  IOTA_LOG_DEBUG(m_logger, "distinct result:" << res.toString());

  return res;
}

int iota::Collection::getLastError(const std::string& bbdd,
                                   const mongo::BSONObj& obj) {
  mongo::BSONObj errObj = _m_conn->getLastErrorDetailed();
  iota::Alarm::info(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                    errObj.jsonString());
  int errCode = errObj["code"].numberInt();
  int n = errObj.getIntField("n");
  if (errCode == 0) {
    int ok = errObj.getIntField("ok");
    if (ok == 0) {
      // se ha producido un error
      std::string errorSTR = errObj.getStringField("err");
      errorSTR += "   mongoiota::store:: ";
      errorSTR += bbdd;
      errorSTR += "::getLastError";
      IOTA_LOG_ERROR(m_logger, errorSTR);
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                                errorSTR, 500);
    } else {
      IOTA_LOG_DEBUG(m_logger, "Mongo operation ok");
    }
  } else if (errCode == 11000 || errCode == 11001) {
    // duplicate key
    std::ostringstream stream;
    stream << bbdd << obj << " error:" << errCode << ":"
           << errObj.getStringField("err");
    std::string errorSTR = std::string("duplicate key ");
    std::string staux = stream.str();
    IOTA_LOG_ERROR(m_logger, staux << errObj.getStringField("err"));
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER,
                              errorSTR, 409);

  } else if (errCode != 0) {
    std::ostringstream stream;
    stream << bbdd << obj << " error:" << errCode << ":"
           << errObj.getStringField("err");
    std::string errorSTR = stream.str();
    IOTA_LOG_ERROR(m_logger, errorSTR);
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, 500);
  }

  return n;
}

int iota::Collection::insert(const mongo::BSONObj& data, int retry) {
  std::string bbdd = getDatabaseName();
  bbdd.append(".");
  bbdd.append(a_bbdd);

  std::string param_request("Collection:insert bbdd=" + bbdd + " data=" +
                            data.toString());
  IOTA_LOG_DEBUG(m_logger, param_request);

  try {
    _m_conn->insert(bbdd, data);
    iota::Alarm::info(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                      "insert OK");
  } catch (mongo::SocketException& e) {
    std::string errorSTR = "SocketException ";
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  } catch (mongo::ConnectException& e) {
    std::string errorSTR = "ConnectException ";
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  } catch (mongo::UserException& e) {
    IOTA_LOG_ERROR(m_logger, "UserException " << e.what());
    if (retry < MAX_RETRY) {
      insert(data, retry + 1);
    } else {
      std::string errorSTR = "problem with dabaseUSerConnection";
      iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(),
                         types::ERROR, errorSTR);
      IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                                errorSTR, ERROR_MONGO);
    }
  } catch (mongo::AssertionException& e) {
    std::string errorSTR = "AssertionException ";
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  } catch (mongo::OperationException& e) {
    mongo::BSONObj errObj = e.obj();
    int errCode = errObj["code"].numberInt();
    IOTA_LOG_ERROR(m_logger, "OperationException " << errCode << " " << errObj);
    if (errCode == 11000 || errCode == 11001) {
      // duplicate key
      std::ostringstream stream;
      stream << "duplicate key: " << bbdd << " " << data;
      std::string errorSTR = stream.str();
      IOTA_LOG_ERROR(m_logger, errorSTR << "->" << e.what());
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DUPLICATED_DATA,
                                errorSTR, 409);

    } else {
      std::ostringstream stream;
      stream << bbdd << " " << data;
      std::string errorSTR = stream.str();
      iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(),
                         types::ERROR, errorSTR);
      IOTA_LOG_ERROR(m_logger, errorSTR << errObj.getStringField("err"));
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                                errorSTR, ERROR_MONGO);
    }
  } catch (mongo::DBException& e) {
    std::string errorSTR = "DBException ";
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  }
};

int iota::Collection::update(const std::string& id,
                             const mongo::BSONObj& setData, bool upsert) {
  mongo::BSONObj query = BSON("_id" << mongo::OID(id));
  return update(query, setData, upsert);
}

int iota::Collection::update(const mongo::BSONObj& query,
                             const mongo::BSONObj& setData, bool upsert,
                             int retry) {
  mongo::BSONObj setObj = BSON("$set" << setData);
  return update_r(query, setObj, upsert, 0);
}

int iota::Collection::update_r(const mongo::BSONObj& query,
                               const mongo::BSONObj& setData, bool upsert,
                               int retry) {
  int n = 0;
  std::string bbdd = getDatabaseName();
  bbdd.append(".");
  bbdd.append(a_bbdd);

  std::string param_request("Collection:update bbdd=" + bbdd + "query=" +
                            query.toString() + " data=" + setData.toString());
  IOTA_LOG_DEBUG(m_logger, param_request);

  try {
    _m_conn->update(bbdd, query, setData, upsert);
    n = getLastError(bbdd, setData);
  } catch (mongo::SocketException& e) {
    std::string errorSTR = "SocketException ";
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  } catch (mongo::ConnectException& e) {
    std::string errorSTR = "ConnectException ";
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  } catch (mongo::UserException& e) {
    IOTA_LOG_ERROR(m_logger, "UserException " << e.what());
    if (retry < MAX_RETRY) {
      update(query, setData, upsert, retry + 1);
    } else {
      std::string errorSTR = "problem with database UserConnection";
      iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(),
                         types::ERROR, errorSTR);
      IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                                errorSTR, ERROR_MONGO);
    }
  } catch (mongo::AssertionException& e) {
    std::string errorSTR = "AssertionException ";
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  } catch (mongo::OperationException& e) {
    mongo::BSONObj errObj = e.obj();
    int errCode = errObj["code"].numberInt();
    IOTA_LOG_ERROR(m_logger, "OperationException " << errCode << " " << errObj);
    if (errCode == 11000 || errCode == 11001) {
      // duplicate key
      std::ostringstream stream;
      stream << "duplicate key: " << bbdd << " " << query;
      std::string errorSTR = stream.str();
      IOTA_LOG_ERROR(m_logger, errorSTR << "->" << e.what());
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DUPLICATED_DATA,
                                errorSTR, 409);

    } else {
      std::ostringstream stream;
      stream << bbdd << " " << query;
      std::string errorSTR = stream.str();
      iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(),
                         types::ERROR, errorSTR);
      IOTA_LOG_ERROR(m_logger, errorSTR << errObj.getStringField("err"));
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                                errorSTR, ERROR_MONGO);
    }
  } catch (mongo::DBException& e) {
    std::string errorSTR = "DBException ";
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  }

  return n;
};

int iota::Collection::remove(const mongo::BSONObj& query, int retry) {
  mongo::BSONObjBuilder obj;
  std::string bbdd = getDatabaseName();
  bbdd.append(".");
  bbdd.append(a_bbdd);
  int n = 0;

  std::string param_request("Collection:remove bbdd=" + bbdd + " query=" +
                            query.toString());
  IOTA_LOG_DEBUG(m_logger, param_request);

  try {
    _m_conn->remove(bbdd, query);
    n = getLastError(bbdd, query);
  } catch (mongo::SocketException& e) {
    std::string errorSTR = "SocketException ";
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  } catch (mongo::ConnectException& e) {
    std::string errorSTR = "ConnectException ";
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  } catch (mongo::UserException& e) {
    IOTA_LOG_ERROR(m_logger, "UserException " << e.what());
    if (retry < MAX_RETRY) {
      remove(query, retry + 1);
    } else {
      std::string errorSTR = "problem with database UserConnection";
      iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(),
                         types::ERROR, errorSTR);
      IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                                errorSTR, ERROR_MONGO);
    }
  } catch (mongo::AssertionException& e) {
    std::string errorSTR = "AssertionException ";
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  } catch (mongo::OperationException& e) {
    mongo::BSONObj errObj = e.obj();
    int errCode = errObj["code"].numberInt();
    IOTA_LOG_ERROR(m_logger, "OperationException " << errCode << " " << errObj);
    if (errCode == 11000 || errCode == 11001) {
      // duplicate key
      std::ostringstream stream;
      stream << "duplicate key: " << bbdd << " " << query;
      std::string errorSTR = stream.str();
      IOTA_LOG_ERROR(m_logger, errorSTR << "->" << e.what());
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DUPLICATED_DATA,
                                errorSTR, 409);

    } else {
      std::ostringstream stream;
      stream << bbdd << " " << query;
      std::string errorSTR = stream.str();
      iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(),
                         types::ERROR, errorSTR);
      IOTA_LOG_ERROR(m_logger, errorSTR << errObj.getStringField("err"));
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                                errorSTR, ERROR_MONGO);
    }
  } catch (mongo::DBException& e) {
    std::string errorSTR = "DBException ";
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  }

  return n;
};

void iota::Collection::instantiateID(const std::string& id) {
  find(BSON("_id" << mongo::OID(id)));
}

std::string iota::Collection::newID() {
  mongo::OID oid;
  oid.init();
  return oid.toString();
}

int iota::Collection::find(const mongo::BSONObj& query,
                           mongo::BSONObjBuilder& builderfieldsToReturn) {
  return find(a_queryOptions, query, builderfieldsToReturn);
};

int iota::Collection::find(const mongo::BSONObj& query) {
  mongo::BSONObjBuilder fieldsToReturn;
  return find(a_queryOptions, query, fieldsToReturn);
}

int iota::Collection::find(int queryOptions, const mongo::BSONObj& query,
                           mongo::BSONObjBuilder& fieldsToReturn) {
  mongo::BSONObj emptyBSON;
  return find(queryOptions, query, 0, 0, emptyBSON, fieldsToReturn);
}

int iota::Collection::ensureIndex(const std::string& name_index,
                                  const mongo::BSONObj& index, bool is_unique) {
  int result = -1;
  std::string bbdd = getDatabaseName();
  bbdd.append(".");
  bbdd.append(a_bbdd);

  std::string param_request("Collection:ensureIndex bbdd=" + bbdd + " key:");
  IOTA_LOG_DEBUG(m_logger, param_request << index);

  try {
    mongo::StringData bbddStringdata(bbdd);
    mongo::IndexSpec indexSpec;
    indexSpec.addKeys(index);
    indexSpec.unique(is_unique);
    indexSpec.dropDuplicatesDeprecated(is_unique);

    IOTA_LOG_DEBUG(m_logger, "before query");
    _m_conn->createIndex(bbddStringdata, indexSpec);
    result = 0;
  } catch (mongo::DBException& e) {
    std::string errorSTR = "DBException ";
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  }

  return result;
}

int iota::Collection::dropIndexes() {
  int result = -1;
  std::string bbdd = getDatabaseName();
  bbdd.append(".");
  bbdd.append(a_bbdd);

  std::string param_request("Collection:dropIndex bbdd=" + bbdd);
  IOTA_LOG_DEBUG(m_logger, param_request);

  try {
    IOTA_LOG_DEBUG(m_logger, "before query");
    _m_conn->dropIndexes(bbdd);
    result = 0;
  } catch (mongo::DBException& e) {
    std::string errorSTR = "DBException in  dropIndexes";
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    reconnect();
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());

  } catch (std::exception& e) {
    std::string errorSTR = "Exception in  dropIndexes";
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       e.what());
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
  }

  return result;
}

int iota::Collection::createIndex(const mongo::BSONObj& index,
                                  bool uniqueIndex) {
  int res = 200;

  std::string bbdd = getDatabaseName();
  bbdd.append(".");
  bbdd.append(a_bbdd);
  Collection indexCol("system.indexes");
  int num_index = indexCol.count(BSON("ns" << bbdd));
  bool dropIndex = false;

  IOTA_LOG_DEBUG(m_logger, bbdd << " checkIndex");
  if (num_index == 2) {
    // check if the index exists
    if (uniqueIndex) {
      dropIndex == (indexCol.count(BSON("ns" << bbdd << "key" << index
                                             << "unique" << true)) != 1);
    } else {
      dropIndex == (indexCol.count(BSON("ns" << bbdd << "key" << index)) != 1);
    }
  } else {
    // no correct numbner of index
    dropIndex = true;
  }

  if (dropIndex) {
    IOTA_LOG_DEBUG(m_logger, bbdd << " dropIndex an create " << index);
    dropIndexes();

    ensureIndex("shardKey", index, uniqueIndex);
  }

  return res;
}

int iota::Collection::find(int queryOptions, const mongo::BSONObj& queryObj,
                           int limit, int skip, const mongo::BSONObj& order,
                           mongo::BSONObjBuilder& fieldsToReturn, int retry) {
  int result = -1;
  std::string bbdd = getDatabaseName();
  bbdd.append(".");
  bbdd.append(a_bbdd);

  std::string param_request("Collection:find bbdd=" + bbdd + " query=" +
                            queryObj.toString());
  IOTA_LOG_DEBUG(m_logger, param_request << " limit=" << limit
                                         << " offset=" << skip
                                         << " options=" << queryOptions);

  mongo::BSONObj fields_to_return;
  mongo::Query query(queryObj);

  try {
    if (order.nFields() > 0) {
      query.sort(order);
    }

    // Remove _id
    fieldsToReturn.append("_id", 0);
    fields_to_return = fieldsToReturn.obj();
    IOTA_LOG_DEBUG(m_logger, "before getConnection");

    if (queryOptions == INT_MIN) {
      queryOptions = a_queryOptions;
    }
    IOTA_LOG_DEBUG(m_logger, "before query");
    a_cursor = _m_conn->query(bbdd, query, limit, skip, &fields_to_return,
                              queryOptions);
    // a_cursor = conn->query(bbdd, query);

    if (a_cursor.get() == NULL) {
      std::string err = "query returns null";
      IOTA_LOG_ERROR(m_logger, err);
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                                err, 500);
    } else if (a_cursor->more()) {
      result = 0;
    } else {
      //  NO_DATA
      std::ostringstream stream;
      stream << "error: NO DATA  " << bbdd << ": " << query;
      std::string obj = stream.str();

      IOTA_LOG_INFO(m_logger, obj);
      result = ERROR_NO_DATA;
    }

  } catch (mongo::SocketException& e) {
    std::string errorSTR = "SocketException ";
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  } catch (mongo::ConnectException& e) {
    std::string errorSTR = "ConnectException ";
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  } catch (mongo::UserException& e) {
    IOTA_LOG_ERROR(m_logger, "UserException " << e.what());
    if (retry < MAX_RETRY) {
      return find(queryOptions, queryObj, limit, skip, order, fieldsToReturn,
                  retry + 1);
    } else {
      std::string errorSTR = "problem with database UserConnection";
      iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(),
                         types::ERROR, errorSTR);
      IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                                errorSTR, ERROR_MONGO);
    }
  } catch (mongo::AssertionException& e) {
    std::string errorSTR = "AssertionException ";
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  } catch (mongo::OperationException& e) {
    mongo::BSONObj errObj = e.obj();
    int errCode = errObj["code"].numberInt();
    IOTA_LOG_ERROR(m_logger, "OperationException " << errCode << " " << errObj);
    if (errCode == 11000 || errCode == 11001) {
      // duplicate key
      std::ostringstream stream;
      stream << "duplicate key: " << bbdd << " " << query;
      std::string errorSTR = stream.str();
      IOTA_LOG_ERROR(m_logger, errorSTR << "->" << e.what());
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DUPLICATED_DATA,
                                errorSTR, 409);

    } else {
      std::ostringstream stream;
      stream << bbdd << " " << query;
      std::string errorSTR = stream.str();
      iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(),
                         types::ERROR, errorSTR);
      IOTA_LOG_ERROR(m_logger, errorSTR << errObj.getStringField("err"));
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                                errorSTR, ERROR_MONGO);
    }
  } catch (mongo::DBException& e) {
    std::string errorSTR = "DBException ";
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    IOTA_LOG_ERROR(m_logger, errorSTR << e.what());
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, ERROR_MONGO);
  }

  return result;
};

int iota::Collection::count(const mongo::BSONObj& query, int retry) {
  int result = -1;
  try {
    std::string bbdd = getDatabaseName();
    bbdd.append(".");
    bbdd.append(a_bbdd);

    std::string param_request("Collection:count bbdd=" + bbdd + " options=" +
                              " query=" + query.toString());
    IOTA_LOG_DEBUG(m_logger, param_request);

    result = _m_conn->count(bbdd, query);

  } catch (mongo::DBException& e) {
    IOTA_LOG_ERROR(m_logger, "Capturada mongo::DBException en find");
    std::string staux = query.toString();
    IOTA_LOG_ERROR(m_logger, staux);
    IOTA_LOG_ERROR(m_logger, e.what());
    std::ostringstream stream;
    stream << "error: " << a_bbdd << ": " << query << " what=" << e.what();
    std::string obj = stream.str();
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       obj);
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR, obj,
                              500);
  }

  return result;
};

bool iota::Collection::more() {
  try {
    return (a_cursor.get() != NULL) && (a_cursor->more());
  } catch (mongo::SocketException& e) {
    IOTA_LOG_ERROR(m_logger, "SocketException " << e.what());
  } catch (mongo::ConnectException& e) {
    IOTA_LOG_ERROR(m_logger, "ConnectException " << e.what());
  } catch (mongo::UserException& e) {
    IOTA_LOG_ERROR(m_logger, "UserException " << e.what());
    boost::this_thread::sleep(boost::posix_time::milliseconds(500));
    return (a_cursor.get() != NULL) && (a_cursor->more());
    IOTA_LOG_ERROR(m_logger, "after UserException " << e.what());
  } catch (mongo::AssertionException& e) {
    IOTA_LOG_ERROR(m_logger, "AssertionException " << e.what());
  } catch (mongo::OperationException& e) {
    IOTA_LOG_ERROR(m_logger, "OperationException " << e.what());
    mongo::BSONObj errObj = e.obj();
    iota::Alarm::info(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                      errObj.jsonString());
    int errCode = errObj["code"].numberInt();
    std::ostringstream stream;
    stream << " more error:" << errCode << ":" << errObj.getStringField("err");
    std::string errorSTR = stream.str();
    IOTA_LOG_ERROR(m_logger, errorSTR);
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              errorSTR, 500);

  } catch (mongo::DBException& e) {
    reconnect();
    IOTA_LOG_ERROR(m_logger, "mongo::DBException " << e.what());
  }
};

mongo::BSONObj iota::Collection::next() {
  try {
    if ((a_cursor.get() != NULL) && (a_cursor->more())) {
      // there are more data
      return a_cursor->next();
    } else {
      std::string err("no more data");
      ;
      throw iota::IotaException(iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER,
                                err, iota::types::RESPONSE_CODE_BAD_REQUEST);
    }
  } catch (mongo::UserException& e) {
    std::string errorSTR =
        "Capturada mongo::UserException salimos de next, excepcion: ";
    errorSTR += e.what();
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       errorSTR);
    throw iota::IotaException(
        iota::types::RESPONSE_MESSAGE_DATABASE_ERROR, e.what(),
        iota::types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR);
  } catch (mongo::DBException& e) {
    std::string msg = "fetch";
    msg += "  Error: ";
    msg += e.what();
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(), types::ERROR,
                       msg);
    reconnect();
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR,
                              e.what(), 500);
  }
};

std::string iota::Collection::getDatabaseName() {
  return MongoConnection::instance()->get_database();
}

/*
void iota::Collection::ensureIndex(
  const std::string& name_index,
  const mongo::BSONObj& index,
  bool is_unique) {

  mongo::BSONObjBuilder obj;

  mongo::DBClientBase* conn = getConnection();

  std::string bbdd = getDatabaseName();
  bbdd.append(".");
  bbdd.append(a_bbdd);

  std::string bbddINDEX = getDatabaseName();
  bbddINDEX.append(".");
  bbddINDEX.append("system.indexes");

  ///{ "v" : 1, "unique" : true, "key" : { "device_id" : 1, "service" : 1,
"service_path" : 1 }, "name" : "device_id_1_service_1_service_path_1", "ns" :
"iot.DEVICE" }

  obj.append("ns", bbdd);
  obj.append("name", name_index);
  obj.append("v", 1);
  if (is_unique) {
    obj.append("unique" , true);
  }
  obj.append("key", index);


  try {
    mongo::BSONObj data = obj.obj();

    std::string param_request("Collection:ensureIndex|bbdd=" + bbdd + "|index="
+
                              data.toString());
    IOTA_LOG_DEBUG(m_logger, param_request);

    IOTA_LOG_DEBUG(m_logger,"insert in  " << bbddINDEX << " with " << data);
    conn->insert(bbddINDEX, data);
    getLastError(bbddINDEX, data);
  }
  catch (mongo::DBException& e) {
    iota::Alarm::error(types::ALARM_CODE_NO_MONGO, get_endpoint(),
                       types::ERROR, e.what());
    throw iota::IotaException(iota::types::RESPONSE_MESSAGE_DATABASE_ERROR + "["
+
                              bbdd + "]", e.what(),
iota::types::RESPONSE_CODE_RECEIVER_INTERNAL_ERROR);
  }

}*/

std::string iota::Collection::get_endpoint() {
  return MongoConnection::instance()->get_endpoint();
}

void iota::Collection::reconnect() { MongoConnection::instance()->reconnect(); }
