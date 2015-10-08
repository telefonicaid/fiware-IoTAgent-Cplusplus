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
#ifndef SRC_REST_IOT_STATISTIC_H_
#define SRC_REST_IOT_STATISTIC_H_

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include <rapidjson/document.h>

namespace iota {

class IoTStatistic {
 public:
  typedef boost::accumulators::accumulator_set<
      double,
      boost::accumulators::stats<
          boost::accumulators::tag::count, boost::accumulators::tag::sum,
          boost::accumulators::tag::max, boost::accumulators::tag::min,
          boost::accumulators::tag::mean> > iot_accumulator;

  typedef boost::shared_ptr<iot_accumulator> iot_accumulator_ptr;

  IoTStatistic(const std::string& name);
  IoTStatistic(){};
  IoTStatistic(const IoTStatistic& stat);
  ~IoTStatistic();
  void set_enable(bool enabled);
  const bool get_enable() const;
  const std::string& get_name() const;
  IoTStatistic& operator=(IoTStatistic& stat);
  iot_accumulator_ptr operator[](const std::string& acc_name);
  void add(const std::string& acc_name, double value);
  const std::map<long, std::map<std::string, iot_accumulator_ptr> >&
  get_counters() const;

  /**
   * Reset all accumulators to initial state
   */
  void reset(long timestamp);

 protected:
 private:
  std::string _name;
  bool _enabled;
  // std::map<std::string, iot_accumulator_ptr > _accumulators;
  std::map<long, std::map<std::string, iot_accumulator_ptr> > _accumulators_;
};
};

#endif  // _IOTSTATISTIC_H_
