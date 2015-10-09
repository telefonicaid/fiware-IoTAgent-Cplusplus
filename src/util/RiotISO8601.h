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
/*
 * File: RiotISO8601.h
 * Author: desgw (Telefonica Investigacion y Desarrollo)
 *
 * Created on jue sep 29 09:45:55 CEST 2011
 *
 */

#ifndef SRC_UTIL_RIOTISO8601_H_
#define SRC_UTIL_RIOTISO8601_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>

namespace iota {
class RiotISO8601 {
 public:
  // Constructor por defecto
  RiotISO8601(void);
  RiotISO8601(const RiotISO8601& iso8601);
  RiotISO8601(std::string t_zone);
  RiotISO8601(std::string a, std::string m, std::string d, std::string hh,
              std::string mm, std::string ss);

  // Constructor para tiempos en formato epoch (desde 1-1-1970;
  RiotISO8601(long t_epoch);
  std::string toString(void);
  std::string toSimpleISOString(void);
  long get_year(void);
  long get_month(void);
  long get_day(void);
  long get_hours(void);
  long get_minutes(void);
  long get_seconds(void);
  long get_microseconds(void);
  std::string toIdString(void);
  iota::RiotISO8601 toUTC(void);

  const boost::posix_time::ptime& getPosixTime(void) const { return _ptime; };
  const boost::posix_time::time_duration& getDiffUTC(void) const {
    return _dUTC;
  };
  const bool isLocal(void) const { return _local; };
  boost::posix_time::ptime& getPosixTime(void) { return _ptime; };
  boost::posix_time::time_duration& getDiffUTC(void) { return _dUTC; };
  bool isLocal(void) { return _local; };

  // Comparaciones
  bool operator<(RiotISO8601& iso);
  bool operator>(RiotISO8601& iso);
  bool operator==(RiotISO8601& iso);
  bool operator!=(RiotISO8601& iso);
  RiotISO8601& operator=(const RiotISO8601& iso);

 protected:
 private:
  boost::posix_time::ptime _ptime;

  // Guarda diferencia con UTC
  boost::posix_time::time_duration _dUTC;

  // Si es local no se incluye diferencia
  bool _local;
};
}
#endif
