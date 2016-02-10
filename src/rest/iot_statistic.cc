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
#include "iot_statistic.h"

iota::IoTStatistic::IoTStatistic(const std::string& name)
    : _name(name), _enabled(true) {
  // ctor
}

iota::IoTStatistic::~IoTStatistic() {
  // dtor
}

iota::IoTStatistic::IoTStatistic(const iota::IoTStatistic& stat) {
  _name = stat.get_name();
  _accumulators_ = stat.get_counters();
  _enabled = stat.get_enable();
}

const std::string& iota::IoTStatistic::get_name() const { return _name; }

void iota::IoTStatistic::set_enable(bool enabled) { _enabled = enabled; }

const bool iota::IoTStatistic::get_enable() const { return _enabled; }

// iota::IoTStatistic::iota::IoTStatistic&
// iota::IoTStatistic::operator=(iota::IoTStatistic::iota::IoTStatistic& stat) {
iota::IoTStatistic& iota::IoTStatistic::operator=(iota::IoTStatistic& stat) {
  _name = stat.get_name();
  _enabled = stat.get_enable();
  _accumulators_ = stat.get_counters();
  return *this;
}

iota::IoTStatistic::iot_accumulator_ptr iota::IoTStatistic::operator[](
    const std::string& acc_name) {
  if (_enabled == false) {
    return iota::IoTStatistic::iot_accumulator_ptr();
  }
  time_t to = time(0);
  long acc_time = (to / 3600) * 3600;
  std::map<long, std::map<std::string,
                          iota::IoTStatistic::iot_accumulator_ptr> >::iterator
      it = _accumulators_.begin();
  it = _accumulators_.find(acc_time);
  if (it != _accumulators_.end()) {
    std::map<std::string, iota::IoTStatistic::iot_accumulator_ptr>::iterator
        it_acc = it->second.begin();
    it_acc = it->second.find(acc_name);
    if (it_acc == it->second.end()) {
      iota::IoTStatistic::iot_accumulator_ptr acc(
          new iota::IoTStatistic::iot_accumulator);
      return (it->second.insert(
                  std::make_pair<std::string,
                                 iota::IoTStatistic::iot_accumulator_ptr>(
                      acc_name, acc)))
          .first->second;
    } else {
      return it_acc->second;
    }
  } else {
    reset(acc_time);
    iota::IoTStatistic::iot_accumulator_ptr acc(
        new iota::IoTStatistic::iot_accumulator);
    std::map<std::string, iota::IoTStatistic::iot_accumulator_ptr> accs;

    accs.insert(
        std::make_pair<std::string, iota::IoTStatistic::iot_accumulator_ptr>(
            acc_name, acc));
    return (_accumulators_.insert(
                std::make_pair<
                    long, std::map<std::string,
                                   iota::IoTStatistic::iot_accumulator_ptr> >(
                    acc_time, accs)))
        .first->second[acc_name];
  }
}

void iota::IoTStatistic::add(const std::string& acc_name, double value) {
  if (_enabled == false) {
    return;
  }
  time_t to = time(0);
  long acc_time = (to / 3600) * 3600;
  std::map<long, std::map<std::string,
                          iota::IoTStatistic::iot_accumulator_ptr> >::iterator
      it = _accumulators_.begin();
  it = _accumulators_.find(acc_time);
  if (it != _accumulators_.end()) {
    std::map<std::string, iota::IoTStatistic::iot_accumulator_ptr>::iterator
        it_acc = it->second.begin();
    it_acc = it->second.find(acc_name);
    if (it_acc != it->second.end()) {
      (*(it_acc->second))(value);
    } else {
      iota::IoTStatistic::iot_accumulator_ptr acc(
          new iota::IoTStatistic::iot_accumulator);
      (*acc)(value);
      it->second.insert(
          std::make_pair<std::string, iota::IoTStatistic::iot_accumulator_ptr>(
              acc_name, acc));
    }
  } else {
    reset(acc_time);
    iota::IoTStatistic::iot_accumulator_ptr acc(
        new iota::IoTStatistic::iot_accumulator);
    (*acc)(value);
    std::map<std::string, iota::IoTStatistic::iot_accumulator_ptr> accs;
    accs.insert(
        std::make_pair<std::string, iota::IoTStatistic::iot_accumulator_ptr>(
            acc_name, acc));
    _accumulators_.insert(
        std::make_pair<
            long,
            std::map<std::string, iota::IoTStatistic::iot_accumulator_ptr> >(
            acc_time, accs));
  }
}

const std::map<long,
               std::map<std::string, iota::IoTStatistic::iot_accumulator_ptr> >&
iota::IoTStatistic::get_counters() const {
  return _accumulators_;
}

void iota::IoTStatistic::reset(long timestamp) {
  if (_accumulators_.size() >= 24) {
    _accumulators_.erase(_accumulators_.begin());
  }
  //_accumulators_.clear();
}
