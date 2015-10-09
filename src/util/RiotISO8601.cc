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
 * File: RiotISO8601.cc
 * Author: desgw (Telefonica Investigacion y Desarrollo)
 *
 * Created on jue sep 29 09:45:56 CEST 2011
 *
 */

#include "RiotISO8601.h"
#include "iota_exception.h"
#include "rest/types.h"
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/format.hpp>

iota::RiotISO8601::RiotISO8601(void)
    : _ptime(boost::posix_time::microsec_clock::local_time()) {
  _local = true;
  tm TM;
  time_t r = time(NULL);
  localtime_r(&r, &TM);
  _dUTC = boost::posix_time::hours(TM.tm_gmtoff / 3600) +
          boost::posix_time::minutes(TM.tm_gmtoff % 3600 / 60);
}

iota::RiotISO8601::RiotISO8601(const iota::RiotISO8601& iso8601) {
  _ptime = iso8601.getPosixTime();
  _dUTC = iso8601.getDiffUTC();
  _local = iso8601.isLocal();
};

iota::RiotISO8601::RiotISO8601(std::string iso) {
  // Puede tratarse de UTC, local o relativa
  _local = true;
  int iso_length_v = 0;
  if (iso[iso.length() - 1] == 'Z') {
    // Se trata de hora UTC
    _dUTC = boost::posix_time::hours(0) + boost::posix_time::minutes(0);
    iso_length_v = iso.length() - 1;
    _local = false;
  } else {
    // Si no es UTC, puede ser relativa a UTC
    if (iso[iso.length() - 6] == '+') {
      // Relativa UTC +
      std::string dif = iso.substr(iso.length() - 6);
      _dUTC = boost::posix_time::duration_from_string(dif);
      iso_length_v = iso.length() - 6;
    } else if (iso[iso.length() - 6] == '-') {
      std::string dif = iso.substr(iso.length() - 6);
      _dUTC = boost::posix_time::duration_from_string(dif);
      iso_length_v = iso.length() - 6;
    } else {
      // Se trata de hora local
      iso_length_v = iso.length();
    }
  }

  std::string iso_nx;
  int i = 0;
  for (i = 0; i < iso_length_v; i++) {
    if ((iso[i] != ':') && (iso[i] != '-')) {
      iso_nx.append(1, iso[i]);
    }
  }
  try {
    _ptime = boost::posix_time::from_iso_string(iso_nx);
  } catch (std::exception& e) {
    throw iota::IotaException(
        iota::types::RESPONSE_MESSAGE_INVALID_PARAMETER + " [" + iso + "]",
        e.what(), iota::types::RESPONSE_CODE_BAD_REQUEST);
  }
}

iota::RiotISO8601::RiotISO8601(std::string a, std::string m, std::string d,
                               std::string hh, std::string mm, std::string ss) {
  std::string iso(a);
  iso.append(m);
  iso.append(d);
  iso.append("T");
  iso.append(hh);
  iso.append(mm);
  iso.append(ss);
  _ptime = boost::posix_time::from_iso_string(iso);
  _local = true;
};

iota::RiotISO8601::RiotISO8601(long t_epoch) {
  // boost::posix_time::ptime ptime_epoch(boost::gregorian::date(1970,1,1));
  std::time_t epoch_t(t_epoch);
  _ptime = boost::posix_time::from_time_t(epoch_t);
  _local = false;
}

std::string iota::RiotISO8601::toString(void) {
  std::string mi_iso_str;
  std::ostringstream oid;
  oid << boost::posix_time::to_iso_extended_string(_ptime);
  if ((_dUTC.hours() == 0) && (_local == false)) {
    // UTC
    oid << "Z";
  } else if (_local == true) {
    if (_dUTC.hours() != 0) {
      // Se trata de hora local relativa a UTC
      if (_dUTC.hours() > 0) {
        oid << "+";
        oid << boost::format("%02d") % _dUTC.hours();
      } else {
        oid << boost::format("%03d") % _dUTC.hours();
      }
      oid << ":";
      oid << boost::format("%02d") % _dUTC.minutes();
    }
  }
  mi_iso_str = oid.str();
  return mi_iso_str;
}

std::string iota::RiotISO8601::toSimpleISOString(void) {
  return boost::posix_time::to_iso_string(_ptime);
};

long iota::RiotISO8601::get_year(void) { return _ptime.date().year(); }
long iota::RiotISO8601::get_month(void) { return _ptime.date().month(); }
long iota::RiotISO8601::get_day(void) { return _ptime.date().day(); }

long iota::RiotISO8601::get_hours(void) { return _ptime.time_of_day().hours(); }
long iota::RiotISO8601::get_minutes(void) {
  return _ptime.time_of_day().minutes();
}
long iota::RiotISO8601::get_seconds(void) {
  return _ptime.time_of_day().seconds();
}
long iota::RiotISO8601::get_microseconds(void) {
  long s = _ptime.time_of_day().hours() * 3600 +
           _ptime.time_of_day().minutes() * 60 + _ptime.time_of_day().seconds();
  return _ptime.time_of_day().total_microseconds() - s * 1000000;
}

std::string iota::RiotISO8601::toIdString() {
  /*
   std::string id = boost::lexical_cast<std::string>(get_year());
   id.append(boost::lexical_cast<std::string>(get_month()));
   id.append(boost::lexical_cast<std::string>(get_day()));
   id.append(boost::lexical_cast<std::string>(get_hours()));
   id.append(boost::lexical_cast<std::string>(get_minutes()));
   id.append(boost::lexical_cast<std::string>(get_seconds()));
   id.append(boost::lexical_cast<std::string>(get_microseconds()));
  */
  std::string id;
  std::ostringstream oid;
  oid << boost::format("%04d") % get_year();
  oid << boost::format("%02d") % get_month();
  oid << boost::format("%02d") % get_day();
  oid << boost::format("%02d") % get_hours();
  oid << boost::format("%02d") % get_minutes();
  oid << boost::format("%02d") % get_seconds();
  oid << boost::format("%06d") % get_microseconds();
  id = oid.str();
  return (id);
}

iota::RiotISO8601 iota::RiotISO8601::toUTC(void) {
  if ((_local == false) || ((_local == true) && (_dUTC.hours() != 0))) {
    std::ostringstream oid;
    oid << boost::posix_time::to_iso_extended_string(_ptime - _dUTC);
    oid << "Z";
    iota::RiotISO8601 mi_utc(oid.str());
    return mi_utc;
  } else {
    return *this;
  }
};

bool iota::RiotISO8601::operator<(iota::RiotISO8601& iso) {
  // Comprobamos en UTC

  iota::RiotISO8601 obj = toUTC();
  iota::RiotISO8601 obj_iso = iso.toUTC();
  if (obj_iso.getPosixTime() < obj.getPosixTime()) {
    return true;
  }
  return false;
};
bool iota::RiotISO8601::operator>(iota::RiotISO8601& iso) {
  // Comprobamos en UTC

  iota::RiotISO8601 obj(toUTC());
  iota::RiotISO8601 obj_iso(iso.toUTC());
  if (obj_iso.getPosixTime() > obj.getPosixTime()) {
    return true;
  }
  return false;
};
bool iota::RiotISO8601::operator==(iota::RiotISO8601& iso) {
  // Comprobamos en UTC

  iota::RiotISO8601 obj(toUTC());
  iota::RiotISO8601 obj_iso(iso.toUTC());
  if (obj_iso.getPosixTime() == obj.getPosixTime()) {
    return true;
  }
  return false;
};
bool iota::RiotISO8601::operator!=(iota::RiotISO8601& iso) {
  return !(*this == iso);
};

iota::RiotISO8601& iota::RiotISO8601::operator=(const iota::RiotISO8601& iso) {
  if (this != &iso) {
    _ptime = iso.getPosixTime();
    _dUTC = iso.getDiffUTC();
    _local = iso.isLocal();
  }
  return *this;
};
