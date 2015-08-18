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
#ifndef SRC_BASE_MANAGERTEST_H_
#define SRC_BASE_MANAGERTEST_H_

#include <cppunit/extensions/HelperMacros.h>



#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../mocks/http_mock.h"

#include "util/iota_logger.h"

#include "util/service_collection.h"


class BaseTest  {

  public:
    BaseTest();

    static const std::string HOST;
    static const std::string CONTENT_JSON;

  protected:
    int http_test(const std::string& uri,
                  const std::string& method,
                  const std::string& service,
                  const std::string& service_path,
                  const std::string& content_type,
                  const std::string& body,
                  const std::map<std::string,std::string>& headers,
                  const std::string& query_string,
                  std::string& response);


    /** from a sem, create a new unique name for a service **/
    /**
     * @name    create_random_service
     * @brief   from a sem, create a new unique name for a service
     *          using miliseconds fron now
     *
     * @param [in] sem  value for starname.
     * @return  unique name for a service
     *
     * Example Usage:
     * @code
     *    create_random_service("s");
     * @endcode
     */
    std::string create_random_service(const std::string& sem);

    /**
     * @name    delete_mongo
     * @brief   remove all data for a service (in all table in mongo)
     *          if you call this at start of unit test, you ensure no duplicate data in mongo
     *
     * @param [in] service  name of service.
     * @param [in] service_path  name of service_path.
     *
     * Example Usage:
     * @code
     *    delete_mongo("service", "/srv_path");
     * @endcode
     */
    int delete_mongo(const std::string& service,
                 const std::string& service_path);

   /**
     * @name    check_mongo
     * @brief   count number of elements in mongo with the query
     *          db.<table>.count({ "service" : <service>,
     *                             "service_path":<service_path>,
     *                             <field_name> : <value>)
     *
     * @param [in] service  name of service.
     * @param [in] service_path  name of service_path.
     * @return number of elements founded
     *
     * Example Usage:
     * @code
     *    check_mongo("SERVICE", "service", "/srv_path", "apikey", "123");
     * @endcode
     */
    int check_mongo(const std::string& table,
                 const std::string& service, const std::string& service_path,
                 const std::string& field_name, const std::string& value);


    pion::http::plugin_server_ptr wserver;
    pion::one_to_one_scheduler scheduler;

};

#endif  /* _H */

