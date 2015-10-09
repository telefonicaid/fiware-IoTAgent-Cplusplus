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
#include "iot_stat_value.h"
#include <sys/time.h>

iota::IoTValue::IoTValue() : _value(0) { gettimeofday(&_to, NULL); }

iota::IoTValue::IoTValue(IoTStatistic::iot_accumulator_ptr acc, double value)
    : _value(value) {
  gettimeofday(&_to, NULL);
  _acc = acc;
}

iota::IoTValue::~IoTValue() {
  if (_acc.get() != NULL) {
    (*_acc)(_value);
  }
}

void iota::IoTValue::add_acc(IoTStatistic::iot_accumulator_ptr acc) {
  _acc = acc;
}

double iota::IoTValue::get_value() { return _value; }

void iota::IoTValue::add_value(double value) { _value = value; }
