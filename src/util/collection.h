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
#ifndef SRC_UTIL_COLLECTION_H_
#define SRC_UTIL_COLLECTION_H_

#include <string>

#include <mongo/client/dbclient.h>
#include <mongo/client/dbclientcursor.h>

#include <climits>
#include <cfloat>
#include "mongo_connection.h"
#include "util/iota_logger.h"

#define ERROR_NO_DATA -1;

namespace iota {

class Collection {
 public:
  struct Options {
    enum _v {
      Primary = mongo::QueryOption_NoCursorTimeout,
      Secondary =
          mongo::QueryOption_NoCursorTimeout | mongo::QueryOption_SlaveOk
    };
  };
  Collection(const std::string& collection_name);

  Collection(Collection&);

  virtual ~Collection();

  void setQueryOptions(int queryOptions) { a_queryOptions = queryOptions; };

  mongo::BSONObj findAndModify(const std::string& table,
                               const mongo::BSONObj& query,
                               const mongo::BSONObj& set, bool upsert = true,
                               int retry = 0);

  mongo::BSONObj aggregate(const std::vector<mongo::BSONObj>& pipeline,
                           int retry);

  mongo::BSONObj distinct(const std::string& field, const mongo::BSONObj& query,
                          int retry);

  int dropIndexes();

  int createIndex(const mongo::BSONObj& index, bool uniqueIndex);

  int insert(const mongo::BSONObj& data, int retry = 0);

  int update(bool upsert = false);
  int update(const std::string& id, const mongo::BSONObj& update_data,
             bool upsert = false);
  int update(const mongo::BSONObj& query, const mongo::BSONObj& update_data,
             bool upsert = false, int retry = 0);
  int update_r(const mongo::BSONObj& query, const mongo::BSONObj& setData,
               bool upsert = false, int retry = 0);

  void instantiateID(const std::string& id);
  std::string newID();

  int find(const mongo::BSONObj& query,
           mongo::BSONObjBuilder& builderfieldsToReturn);

  int find(const mongo::BSONObj& query);

  int find(int queryOptions, const mongo::BSONObj& query,
           mongo::BSONObjBuilder& fieldsToReturn);

  int find(int queryOptions, const mongo::BSONObj& query, int limit, int skip,
           const mongo::BSONObj& order, mongo::BSONObjBuilder& fieldsToReturn,
           int retry = 0);

  int count(const mongo::BSONObj& query, int retry = 0);

  int remove(const mongo::BSONObj& query, int retry = 0);

  bool more();
  mongo::BSONObj next();

  std::string get_endpoint();

  const std::string& getBBDD() const throw() { return a_bbdd; }

  void setBBDD(const std::string& value) throw() { a_bbdd = value; }

  void reconnect();

  int ensureIndex(const std::string& name_index, const mongo::BSONObj& index,
                  bool is_unique);

 protected:
  int getLastError(const std::string& bbdd, const mongo::BSONObj& obj);

  std::string getDatabaseName();

  std::auto_ptr<mongo::DBClientCursor> a_cursor;
  std::string a_bbdd;
  int a_queryOptions;

 protected:
  pion::logger m_logger;

 private:
  mongo::DBClientBase* _m_conn;

  static const std::string _EMPTY;

};  // end class Collection

}  // end namespace  riot

#endif
