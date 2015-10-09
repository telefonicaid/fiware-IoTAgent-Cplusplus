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
#ifndef SRC_UTIL_TIMER_H_
#define SRC_UTIL_TIMER_H_

#include <boost/asio.hpp>

namespace iota {

class Timer {
 public:
  Timer(){};

  virtual ~Timer(){};

  boost::shared_ptr<boost::asio::deadline_timer> start(
      boost::asio::io_service& io_service, unsigned short timeout) {
    if (_timer.get() != NULL) {
      _timer->cancel();
    }
    _timer.reset(new boost::asio::deadline_timer(
        io_service, boost::posix_time::seconds(timeout)));
    return _timer;
  }
  void cancel() {
    if (_timer.get() != NULL) {
      _timer->cancel();
    }
  }

  long time_to_expire() {
    long ms = 0;
    if (_timer.get() != NULL) {
      ms = _timer->expires_from_now().total_milliseconds();
    }
    return ms;
  }

 protected:
 private:
  boost::shared_ptr<boost::asio::deadline_timer> _timer;
};
};

#endif
