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
#include "iot_duration.h"
#include <sys/time.h>

iota::Duration::Duration() { gettimeofday(&_to, NULL); }

iota::Duration::Duration(IoTStatistic::iot_accumulator_ptr acc) {
  gettimeofday(&_to, NULL);
  _acc = acc;
}

iota::Duration::~Duration() {
  if (_acc.get() != NULL) {
    (*_acc)(get_duration());
  }
}

void iota::Duration::add_acc(IoTStatistic::iot_accumulator_ptr acc) {
  _acc = acc;
}

long iota::Duration::get_duration() {
  struct timeval _tf;
  gettimeofday(&_tf, NULL);
  long seconds = _tf.tv_sec - _to.tv_sec;
  long useconds = _tf.tv_usec - _to.tv_usec;
  long mtime = (seconds * 1000 + useconds / 1000.0) + 0.5;
  return mtime;
}
