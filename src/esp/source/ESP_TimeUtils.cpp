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
#include "ESP_TimeUtils.h"

ESP_TimeUtils::ESP_TimeUtils() {
  // ctor
}

ESP_TimeUtils::~ESP_TimeUtils() {
  // dtor
}

void ESP_TimeUtils::getCurrentUTCTimestamp(int* day, int* month, int* year,
                                           int* h, int* m, int* s, int* ms) {
  // Date::getLocalDate(*year,*month,*day,*h,*m,*s,*ms,dayofW);
  time_t seconds_since_the_epoch;
  time(&seconds_since_the_epoch);
  struct tm* tm_UTC = gmtime(&seconds_since_the_epoch);
  *year = tm_UTC->tm_year + 1900;
  *month = tm_UTC->tm_mon + 1;  // month 0-11.
  *day = tm_UTC->tm_mday;
  *h = tm_UTC->tm_hour;
  *m = tm_UTC->tm_min;
  *s = tm_UTC->tm_sec;
  *ms = 0;
}

std::string ESP_TimeUtils::formatUTCTimestamp(int day, int month, int year,
                                              int h, int m, int s) {
  char bufferDateTime[64];
  sprintf(bufferDateTime, "%04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, h,
          m, s);
  return std::string(bufferDateTime, strlen(bufferDateTime));
}
