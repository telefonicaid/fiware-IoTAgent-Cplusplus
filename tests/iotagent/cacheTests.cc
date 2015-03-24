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
#include "cacheTests.h"
#include "util/cache.h"
#include "util/command_cache.h"
#include "util/command.h"
#include "services/admin_service.h"
#include <boost/bind.hpp>
#include <limits.h>

CPPUNIT_TEST_SUITE_REGISTRATION(CacheTest);
namespace iota {
std::string URL_BASE = "/iot";
std::string logger("main");
}
iota::AdminService* AdminService_ptr;

boost::shared_ptr<iota::Device> get_f(boost::shared_ptr<iota::Device> item) {
  boost::shared_ptr<iota::Device> m;
  if (item->_name != "Agus") {
    m.reset(new iota::Device(item->_name, "Gonzalez"));
  }
  return m;
}

std::string now_str()
{
    // Get current time from the clock, using microseconds resolution
    const boost::posix_time::ptime now =
        boost::posix_time::microsec_clock::local_time();

    // Get the time offset in current day
    const boost::posix_time::time_duration td = now.time_of_day();

    const long hours        = td.hours();
    const long minutes      = td.minutes();
    const long seconds      = td.seconds();
    const long milliseconds = td.total_milliseconds() -
                              ((hours * 3600 + minutes * 60 + seconds) * 1000);

    char buf[40];
    sprintf(buf, "%02ld:%02ld:%02ld.%03ld",
        hours, minutes, seconds, milliseconds);

    return buf;
}

boost::shared_ptr<iota::Command> timeout_f(boost::shared_ptr<iota::Command> item) {

  std::cout << "timeout_f:" << now_str() << std::endl;
  return item;
}

void CacheTest::testLRU() {

  std::cout << "testLRU" << std::endl;
  iota::Cache mru_cache_1(3, true);
  boost::shared_ptr<iota::Device> item1(new iota::Device("Agus", "Gonzalez"));
  mru_cache_1.insert(item1);
  const boost::shared_ptr<iota::Device> v1 = mru_cache_1.get(item1);
  CPPUNIT_ASSERT(v1->_name == "Agus");

  // Not found
  boost::shared_ptr<iota::Device> item2(new iota::Device("Luis", "Gonzalez"));
  CPPUNIT_ASSERT(mru_cache_1.get(item2).get() == NULL);

  {
    boost::shared_ptr<iota::Device> item3(new iota::Device("Juna", "Gonzalez"));
    mru_cache_1.insert(item3);
    CPPUNIT_ASSERT(mru_cache_1.size() == 2);
    const boost::shared_ptr<iota::Device> vx = mru_cache_1.get(item3);
    CPPUNIT_ASSERT(vx->_name == "Juna");
  }

  boost::shared_ptr<iota::Device> item_3(new iota::Device("Juna", "Gonzalez"));
  const boost::shared_ptr<iota::Device> v3 = mru_cache_1.get(item_3);
  CPPUNIT_ASSERT(mru_cache_1.size() == 2);

  // Set function to insert new element
  mru_cache_1.set_function(boost::bind(get_f, _1));
  mru_cache_1.get(item2);
  CPPUNIT_ASSERT(mru_cache_1.size() == 3);
  const boost::shared_ptr<iota::Device> v4 = mru_cache_1.get(item2);
  CPPUNIT_ASSERT(v4->_name == "Luis");

  // Duplicated
  mru_cache_1.insert(item2);
  CPPUNIT_ASSERT(mru_cache_1.size() == 3);

  // Capacity
  boost::shared_ptr<iota::Device> item_4(new iota::Device("Max", "Gonzalez"));
  mru_cache_1.insert(item_4);
  CPPUNIT_ASSERT(mru_cache_1.size() == 3);

  // item1 not found in cache
  boost::shared_ptr<iota::Device> nf = mru_cache_1.get(item1);
  CPPUNIT_ASSERT(nf.get() == NULL);
  // Remove item
  mru_cache_1.remove(*item2);
  CPPUNIT_ASSERT(mru_cache_1.size() == 2);

}

void CacheTest::testMRU() {

  std::cout << "testMRU" << std::endl;
  iota::Cache mru_cache_1(3, false);
  boost::shared_ptr<iota::Device> item1(new iota::Device("Anna", "Gonzalez"));
  mru_cache_1.insert(item1);
  const boost::shared_ptr<iota::Device> v1 = mru_cache_1.get(item1);
  CPPUNIT_ASSERT(v1->_name == "Anna");

  // Not found
  boost::shared_ptr<iota::Device> item2(new iota::Device("Luis", "Gonzalez"));
  CPPUNIT_ASSERT(mru_cache_1.get(item2).get() == NULL);

  {
    boost::shared_ptr<iota::Device> item3(new iota::Device("Juna", "Gonzalez"));
    mru_cache_1.insert(item3);
    CPPUNIT_ASSERT(mru_cache_1.size() == 2);
    const boost::shared_ptr<iota::Device> vx = mru_cache_1.get(item3);
    CPPUNIT_ASSERT(vx->_name == "Juna");
  }

  boost::shared_ptr<iota::Device> item_3(new iota::Device("Juna", "Gonzalez"));
  const boost::shared_ptr<iota::Device> v3 = mru_cache_1.get(item_3);
  CPPUNIT_ASSERT(mru_cache_1.size() == 2);

  // Set function to insert new element
  mru_cache_1.set_function(boost::bind(get_f, _1));
  mru_cache_1.get(item2);
  CPPUNIT_ASSERT(mru_cache_1.size() == 3);
  const boost::shared_ptr<iota::Device> v4 = mru_cache_1.get(item2);
  CPPUNIT_ASSERT(v4->_name == "Luis");

  // Duplicated
  mru_cache_1.insert(item2);
  CPPUNIT_ASSERT(mru_cache_1.size() == 3);

  // Capacity
  boost::shared_ptr<iota::Device> item_4(new iota::Device("Agus", "Gonzalez"));
  mru_cache_1.insert(item_4);
  CPPUNIT_ASSERT(mru_cache_1.size() == 3);

  // item4 not found in cache
  boost::shared_ptr<iota::Device> nf = mru_cache_1.get(item_4);
  CPPUNIT_ASSERT(nf.get() == NULL);
  // Remove item
  mru_cache_1.remove(*item2);
  CPPUNIT_ASSERT(mru_cache_1.size() == 2);

}

// This test is based on timers.
void CacheTest::testCacheClock() {

  std::cout << "testCacheClock" << std::endl;
  iota::Cache mru_cache_1(3, true, 2);
  boost::asio::io_service io;
  boost::asio::deadline_timer t(io);
  t.expires_from_now(boost::posix_time::seconds(3));

  boost::shared_ptr<iota::Device> item1(new iota::Device("Test", "Gonzalez"));
  boost::shared_ptr<iota::Device> item2(new iota::Device("Test1", "Gonzalez"));
  mru_cache_1.insert(item1);
  mru_cache_1.insert(item2);
  boost::shared_ptr<iota::Device> v = mru_cache_1.get(item1);
  CPPUNIT_ASSERT(v->_name == "Test");
  CPPUNIT_ASSERT(mru_cache_1.size() == 2);
  t.wait();
  CPPUNIT_ASSERT(mru_cache_1.size() == 0);

  //Duplicated restart timer
  mru_cache_1.insert(v);
  sleep(1);
  CPPUNIT_ASSERT(mru_cache_1.size() == 1);
  boost::shared_ptr<iota::Device> v1 = mru_cache_1.get(v);
  t.expires_from_now(boost::posix_time::seconds(3));
  t.wait();
  CPPUNIT_ASSERT(mru_cache_1.size() == 0);
  iota::Cache mru_cache_2(1, true, 2);
  boost::shared_ptr<boost::asio::io_service> io2 = mru_cache_2.get_io_service();
  boost::shared_ptr<iota::Device> item3(new iota::Device("Cancel", "Gonzalez"));
  boost::shared_ptr<iota::Device> item4(new iota::Device("Test3", "Gonzalez"));
  mru_cache_2.insert(item3);
  mru_cache_2.insert(item4);
  t.expires_from_now(boost::posix_time::seconds(3));
  t.wait();
  CPPUNIT_ASSERT(mru_cache_2.size() == 0);

  iota::Cache mru_cache_3(1, false, 2);
  t.expires_from_now(boost::posix_time::seconds(3));
  mru_cache_3.insert(item3);
  mru_cache_3.insert(item4);
  t.wait();
}

void CacheTest::testByEntity() {
  std::cout << "testByEntity" << std::endl;
  iota::Cache mru_cache_1(3, false);
  boost::shared_ptr<iota::Device> item1(new iota::Device("Anna", "Gonzalez"));
  mru_cache_1.insert(item1);
  const boost::shared_ptr<iota::Device> v1 = mru_cache_1.get(item1);
  CPPUNIT_ASSERT(v1->_name == "Anna");

  // By entity
  // empty entity fields
  const boost::shared_ptr<iota::Device> ve1 = mru_cache_1.get_by_entity(item1);
  CPPUNIT_ASSERT(ve1.get() != NULL);

  // Complete device
  boost::shared_ptr<iota::Device> item2(new iota::Device("d1", "s1"));
  item2->_entity_name ="e";
  item2->_entity_type = "t";
  mru_cache_1.insert(item2);
  CPPUNIT_ASSERT(mru_cache_1.size() == 2);

  // Find
  boost::shared_ptr<iota::Device> vitem2 = mru_cache_1.get_by_entity(item2);
  CPPUNIT_ASSERT(vitem2->_entity_name == "e");
  CPPUNIT_ASSERT(vitem2->_name == "d1");

  // Find entity
  boost::shared_ptr<iota::Device> item3(new iota::Device("e1", "t1", "s1"));
  vitem2 = mru_cache_1.get_by_entity(item3);
  CPPUNIT_ASSERT(vitem2.get() == NULL);

  boost::shared_ptr<iota::Device> item4(new iota::Device("e", "t", "s1"));
  vitem2 = mru_cache_1.get_by_entity(item4);
  CPPUNIT_ASSERT(vitem2.get() != NULL);

}

void CacheTest::testCommandTimeout() {
  std::cout << "START testCommandTimeout" << std::endl;

  //OJO el timeout que se usa es el del device no el definido en la cola
  iota::CommandCache command_cache_1(SHRT_MAX, true);
  std::cout << "insert:" << now_str() << std::endl;
  boost::asio::io_service io;
  boost::asio::deadline_timer t(io);
  t.expires_from_now(boost::posix_time::seconds(2));

  command_cache_1.set_timeout_function(timeout_f);
  boost::property_tree::ptree pt;
  pt.put("body", "command");
  boost::shared_ptr<iota::Command> item1(new iota::Command("id1", "id1",
                                   "servico1", "/", "seq1",
                                   "room1", "thing", "http://uri:888//ee", 1, "",
                                   pt));

  std::cout << "insert:" << now_str() << std::endl;
  command_cache_1.insert(item1);

  std::cout << "command_cache_1 size 1" << std::endl;
  CPPUNIT_ASSERT(command_cache_1.size() == 1);
  t.wait();
  std::cout << "after wait 0" << std::endl;
  CPPUNIT_ASSERT(command_cache_1.size() == 0);

  std::cout << "END testCommandTimeout" << std::endl;

}


void CacheTest::testCommand() {
  std::cout << "START testCommand" << std::endl;
  boost::property_tree::ptree command;
  command.put("body", "texto del comando para el device");

  iota::CommandCache command_cache_1(3, false);

  boost::shared_ptr<iota::Command> item1(new iota::Command("id1",
                                   "servico1", "/",  "seq1", "room1", "http://uri:888//ee",
                                   command));
  item1->set_status(1);
  command_cache_1.insert(item1);
  boost::shared_ptr<iota::Command> item2(new iota::Command("id2",
                                   "servico1", "/", "seq2", "room1", "http://uri:888//ee",
                                   command));
  item2->set_status(1);
  command_cache_1.insert(item2);
  boost::shared_ptr<iota::Command> item3(new iota::Command("id3",
                                   "servico1","/",  "seq3", "roomm3", "http://uri:888//ee",
                                   command));
  item3->set_status(1);
  command_cache_1.insert(item3);
  boost::shared_ptr<iota::Command> item4(new iota::Command("id4",
                                   "servico1","/",  "seq4", "", "http://uri:888//ee",
                                   command));
  item4->set_status(1);
  command_cache_1.insert(item4);

  std::cout << "get" << std::endl;
  const boost::shared_ptr<iota::Command> v1 = command_cache_1.get(item1);

  //By Id
  std::cout << "getById" << std::endl;
  iota::Command* cp = new iota::Command("", "servico1", "/");
  cp->set_id("id1");
  boost::shared_ptr<iota::Command> itemQid(cp);
  boost::shared_ptr<iota::Command> vitem = command_cache_1.get_by_id(itemQid);
  CPPUNIT_ASSERT(vitem.get() != NULL);
  std::cout << "getById da " << vitem->get_id() << vitem->get_device() <<
            std::endl;

  // By entity
  // empty entity fields
  std::cout << "getByEntity" << std::endl;
  boost::shared_ptr<iota::Command> itemQ(new iota::Command("room1", "servico1", "/"));
  itemQ->set_status(1);
  boost::shared_ptr<iota::Command> vitem2 = command_cache_1.get_by_entity(itemQ);
  CPPUNIT_ASSERT(vitem2.get() != NULL);
  std::cout << "getByEntity da " << vitem2->get_id() << std::endl;

  iota::CommandVect commands = command_cache_1.get_by_entityV(itemQ, 2);
  std::cout << "getByEntity da " << commands.size() << std::endl;
  iota::CommandVect::const_iterator i;
  iota::CommandPtr ptr;
  for (i=commands.begin(); i!=commands.end(); ++i) {
    ptr = *i;
    std::cout<<(ptr->get_id())<<",";
  }
  std::cout << std::endl;

  //una vez que los hemos leido han cambiado de estado
  iota::CommandVect commands2 = command_cache_1.get_by_entityV(itemQ, 2);
  std::cout << "getByEntity da " << commands2.size() << std::endl;
  iota::CommandVect::const_iterator i2;
  iota::CommandPtr ptr2;
  for (i2=commands2.begin(); i2!=commands2.end(); ++i) {
    ptr2 = *i2;
    std::cout<<(ptr2->get_id())<<",";
  }
  std::cout << std::endl;

  CPPUNIT_ASSERT(commands2.size() == 0);

  std::cout << "END testCommand" << std::endl;
}

