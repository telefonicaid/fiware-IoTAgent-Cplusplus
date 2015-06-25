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
#include "JsonTest.h"
#include "ngsi/Attribute.h"
#include "ngsi/ContextElement.h"
#include "ngsi/UpdateContext.h"
#include "ngsi/ContextResponses.h"
#include "ngsi/RegisterContext.h"
#include "ngsi/QueryContext.h"
#include "ngsi/Entity.h"
#include "ngsi/SubscribeContext.h"
#include "ngsi/SubscribeResponse.h"
#include "util/json_util.h"
#include "util/common.h"
#include "services/admin_service.h"
#include <stdexcept>
#include <boost/property_tree/json_parser.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION(JsonTest);

namespace iota {
std::string logger("main");
std::string URL_BASE("/iot");
}
iota::AdminService* AdminService_ptr;


void JsonTest::testContextElement() {
  std::cout << "@UT@START testContextElement " << std::endl;
  std::string service= "service2";
  std::string op = "UPDATE";

  boost::property_tree::ptree pt;
  pt.put("timeout", "10");
  pt.put("service", service);
  pt.put("service_path", "service_path");
  pt.put("cbroker", "cbroker");
  pt.put("token", "token");
  pt.put("entity_type", "entity_type_service");
  boost::property_tree::ptree attributes;
  boost::property_tree::ptree att;
  att.put("name", "natt");
  att.put("type", "");
  att.put("value", "vatt");
  attributes.push_back(std::make_pair("", att));
  pt.add_child("attributes", attributes);

  {
    boost::shared_ptr<iota::Device> dev;
    dev.reset(new iota::Device("dev_no_entity", service));

    iota::ContextElement ngsi_context_element;
    ngsi_context_element.set_env_info(pt, dev);
    iota::UpdateContext uc(op);
    uc.add_context_element(ngsi_context_element) ;
    std::string str = uc.get_string();
    std::cout << "@UT@1" << str << std::endl;
    CPPUNIT_ASSERT_MESSAGE("@UT@1, entity or entity_type in not correct" ,
                           str.find("\"id\":\"entity_type_service:dev_no_entity\",\"type\":\"entity_type_service\"")
                           !=std::string::npos);
  }

  {
    boost::shared_ptr<iota::Device> dev;
    dev.reset(new iota::Device("dev_entity", service));
    dev->_entity_name ="entity_name";
    dev->_entity_type = "entity_type";

    iota::ContextElement ngsi_context_element;
    ngsi_context_element.set_env_info(pt, dev);
    iota::UpdateContext uc(op);
    uc.add_context_element(ngsi_context_element) ;
    std::string str = uc.get_string();
    std::cout << "@UT@2" << str << std::endl;
    CPPUNIT_ASSERT_MESSAGE("@UT@2, entity or entity_type in not correct" ,
                           str.find("\"id\":\"entity_name\",\"type\":\"entity_type\"") !=
                           std::string::npos);
  }

  {
    boost::shared_ptr<iota::Device> dev;
    dev.reset(new iota::Device("dev_entity", service));
    dev->_entity_name = "entity_name";

    iota::ContextElement ngsi_context_element;
    ngsi_context_element.set_env_info(pt, dev);
    iota::UpdateContext uc(op);
    uc.add_context_element(ngsi_context_element) ;
    std::string str = uc.get_string();
    std::cout << "@UT@3" << str << std::endl;
    CPPUNIT_ASSERT_MESSAGE("@UT@3, entity or entity_type in not correct" ,
                           str.find("\"id\":\"entity_name\",\"type\":\"entity_type_service\"") !=
                           std::string::npos);
  }

  std::cout << "@UT@END testContextElement " << std::endl;
}


void JsonTest::testContext() {

  std::cout << "@UT@START testContext " << std::endl;
  boost::property_tree::ptree pt;
  pt.put("timeout", "10");
  pt.put("service", "service");
  pt.put("service_path", "service_path");
  pt.put("cbroker", "cbroker");
  pt.put("token", "token");
  pt.put("entity_type", "entity_type_service");
  boost::property_tree::ptree ptree_att;
  boost::property_tree::ptree att;
  att.put("name", "mappedatt");
  att.put("type", "");
  att.put("object_id", "originalatt");
  ptree_att.push_back(std::make_pair("", att));
  pt.add_child("attributes", ptree_att);

  boost::property_tree::ptree ptree_satt;
  boost::property_tree::ptree satt;
  satt.put("name", "snatt");
  satt.put("type", "");
  satt.put("value", "svatt");
  ptree_satt.push_back(std::make_pair("", satt));
  pt.add_child("static_attributes", ptree_satt);

  iota::Attribute attribute("name", "type", "value");
  iota::Attribute metadata("name", "typecheck", "valor");
  iota::Attribute metadata1("name1", "type1", "value1");
  attribute.add_metadata(metadata1);
  attribute.add_metadata(metadata);

  iota::ContextElement context_element("id", "type", "is_pattern");
  boost::shared_ptr<iota::Device> dev;
  context_element.set_env_info(pt, dev);

  // Attribute to map
  iota::Attribute mapped_att("originalatt", "string", "originalatt");
  context_element.add_attribute(mapped_att);
  context_element.add_attribute(attribute);
  CPPUNIT_ASSERT_MESSAGE("Attribute + static attribute",
                         context_element.get_attributes().size() == 3);

  iota::UpdateContext operation(std::string("UPDATE"));
  operation.add_context_element(context_element);

  std::istringstream is(attribute.get_string());
  iota::Attribute obj_attribute(is);
  CPPUNIT_ASSERT(obj_attribute.get_name().compare("name") == 0);
  CPPUNIT_ASSERT(obj_attribute.get_type().compare("type") == 0);
  CPPUNIT_ASSERT(obj_attribute.get_value().compare("value") == 0);
  std::vector<iota::Attribute> metadatas = obj_attribute.get_metadatas();
  CPPUNIT_ASSERT(metadatas.size() == 2);
  CPPUNIT_ASSERT(metadatas[0].get_name().compare("name1") == 0);
  CPPUNIT_ASSERT(metadatas[1].get_type().compare("typecheck") == 0);

  std::string ceSTR = context_element.get_string();
  std::cout << "@UT@" << ceSTR << std::endl;
  std::istringstream is_ce(context_element.get_string());
  iota::ContextElement obj_ce(is_ce);
  CPPUNIT_ASSERT(obj_ce.get_id().compare("id") == 0);
  CPPUNIT_ASSERT(obj_ce.get_type().compare("type") == 0);
  CPPUNIT_ASSERT(obj_ce.get_is_pattern().compare("is_pattern") == 0);
  std::vector<iota::Attribute> attributes = obj_ce.get_attributes();
  CPPUNIT_ASSERT(attributes.size() == 3);

  CPPUNIT_ASSERT_MESSAGE("Attributes ",
                         attributes[0].get_name().compare("name") == 0 ||
                         attributes[0].get_name().compare("snatt") == 0 ||
                         attributes[0].get_name().compare("mappedatt"));
  CPPUNIT_ASSERT_MESSAGE("Attributes ",
                         attributes[1].get_name().compare("snatt") == 0 ||
                         attributes[1].get_name().compare("name") == 0 ||
                         attributes[1].get_name().compare("mappedatt") == 0);
  CPPUNIT_ASSERT_MESSAGE("Attributes ",
                         attributes[2].get_name().compare("snatt") == 0 ||
                         attributes[2].get_name().compare("name") == 0 ||
                         attributes[2].get_name().compare("mappedatt") == 0);


  // Check if static attributes has TimeInstant
  bool timeinstant_exists = false;
  for (int i = 0; i < attributes.size(); i++) {
    if (attributes[i].get_name().compare("snatt") == 0) {
      std::vector<iota::Attribute> metadatas = attributes[i].get_metadatas();
      for (int j=0; j < metadatas.size(); j++) {
        iota::Attribute metadata = metadatas[j];
        if (metadata.get_name().compare("TimeInstant") == 0) {
          timeinstant_exists = true;
        }
      }
    }
  }
  CPPUNIT_ASSERT_MESSAGE("Checking TimeInstant ", timeinstant_exists);
  std::istringstream is_op(operation.get_string());
  iota::UpdateContext obj_operation(is_op);
  CPPUNIT_ASSERT(obj_operation.get_action().compare("UPDATE") == 0);
  std::vector<iota::ContextElement> contexts =
    obj_operation.get_context_elements();
  CPPUNIT_ASSERT(contexts.size() == 1);

  // Malformed
  std::istringstream is_malformed("Malformed JSON");
  CPPUNIT_ASSERT_THROW(new iota::UpdateContext(is_malformed),
                       std::runtime_error);

  // Test set
  iota::ContextElement ct("id", "type", "false");
  CPPUNIT_ASSERT(ct.get_is_pattern().compare("false") == 0);

  rapidjson::Document d;
  if (! d.Parse<0>(operation.get_string().c_str()).HasParseError()) {
    iota::UpdateContext o(d);
    CPPUNIT_ASSERT(o.get_action().compare("UPDATE") == 0);
  }
  else {
    CPPUNIT_ASSERT(false);
  }

  is_malformed.clear();
  is_malformed.str("{}");
  CPPUNIT_ASSERT_THROW(new iota::UpdateContext(is_malformed),
                       std::runtime_error);

  is_malformed.clear();
  is_malformed.str("{\"updateAction\": \"APPEND\", \"contextElements\": \"badfield\"}");
  CPPUNIT_ASSERT_THROW(new iota::UpdateContext(is_malformed),
                       std::runtime_error);

  is_malformed.clear();
  is_malformed.str("{\"updateAction\": \"APPEND\"}");
  CPPUNIT_ASSERT_THROW(new iota::UpdateContext(is_malformed),
                       std::runtime_error);

  rapidjson::Value v(10);
  CPPUNIT_ASSERT_THROW(new iota::UpdateContext(v), std::runtime_error);

  rapidjson::Document o;
  o.SetObject();
  CPPUNIT_ASSERT_THROW(new iota::UpdateContext(o), std::runtime_error);
  rapidjson::Value vo;
  vo.SetString("UPDATE");
  o.AddMember("updateAction", vo, o.GetAllocator());
  CPPUNIT_ASSERT_THROW(new iota::UpdateContext(o), std::runtime_error);

  // Add attribute no value
  iota::Attribute attribute_novalue("name", "type", "");

  int num_attributes = context_element.get_attributes().size();
  context_element.add_attribute(attribute_novalue);
  CPPUNIT_ASSERT(context_element.get_attributes().size() == num_attributes);

  // Static attributes form device
  boost::shared_ptr<iota::Device> device(new iota::Device("dev1", "serv1"));
  std::map<std::string, std::string> static_a;
  iota::Attribute st_att("stname", "sttype", "stvalue");
  static_a["stname"] = st_att.get_string();

  // With metadatas
  device->_attributes["OldName"] =
    "{\"object_id\":\"OldName\", \"name\": \"NewName\", \"type\": \"string\", \"metadatas\": [{\"name\": \"unit\", \"type\": \"string\", \"value\": \"celsius\"}]}";
  device->_attributes["MapWithMeta"] =
    "{\"object_id\":\"MapWithMeta\", \"name\": \"NewNameMeta\", \"type\": \"string\", \"metadatas\": [{\"name\": \"unit\", \"type\": \"string\", \"value\": \"celsius\"}]}";
  device->_static_attributes = static_a;
  boost::property_tree::ptree p_env;
  p_env.put("entity_type", "EntityType");

  // Add mapping for attribute
  iota::Attribute map_att("OldName", "Mtype", "value_att");
  iota::Attribute map_attr_1("MapWithMeta", "Mtype", "value_att");
  iota::ContextElement ce_with_static_att("idst", "", "false");
  ce_with_static_att.set_env_info(p_env, device);
  // and attribute for mapping
  ce_with_static_att.add_attribute(map_att);
  ce_with_static_att.add_attribute(map_attr_1);

  CPPUNIT_ASSERT_MESSAGE("Context element with static attributes",
                         ce_with_static_att.get_attributes().size() == 3);

  // Serialize and check final values
  std::string str_ce = ce_with_static_att.get_string();
  CPPUNIT_ASSERT_MESSAGE("Checking if after serialize type and id are updated",
                         ce_with_static_att.get_id().compare("EntityType:dev1") == 0);
  std::istringstream is_ce_a(str_ce);
  iota::ContextElement ce_total(is_ce_a);
  CPPUNIT_ASSERT_MESSAGE("Context element total",
                         ce_total.get_attributes().size() == 3);
  CPPUNIT_ASSERT_MESSAGE("Static attribute ",
                         ce_total.get_attributes()[0].get_name().compare("stname") == 0);
  CPPUNIT_ASSERT_MESSAGE("Mapped attribute ",
                         ce_total.get_attributes()[1].get_name().compare("NewName") == 0);
  CPPUNIT_ASSERT_MESSAGE("Mapped attribute ",
                         ce_total.get_attributes()[2].get_name().compare("NewNameMeta") == 0);
  // Metadata is included
  iota::Attribute mapped_attribute = ce_total.get_attributes()[2];

  std::cout << ce_total.get_attributes()[1].get_metadatas().size() << " **** " << mapped_attribute.get_string() << std::endl;
  CPPUNIT_ASSERT_MESSAGE("Number of metadatas in mapped attribute ", ce_total.get_attributes()[2].get_metadatas().size() == 1);
  iota::Attribute meta_mapped_attribute = ce_total.get_attributes()[2].get_metadatas()[0];
  CPPUNIT_ASSERT_MESSAGE("Checking metadata in mapped attribute ", meta_mapped_attribute.get_name().compare("unit") == 0);

  CPPUNIT_ASSERT_MESSAGE("Mapped attribute ",
                         ce_total.get_attributes()[1].get_type().compare("string") == 0);
  CPPUNIT_ASSERT_MESSAGE("Context element default entity type",
                         ce_total.get_type().compare("EntityType") == 0);
  CPPUNIT_ASSERT_MESSAGE("Context element default entity name",
                         ce_total.get_id().compare("EntityType:dev1") == 0);
}

void JsonTest::testResponse() {
  std::cout << "START testResponse " << std::endl;
  iota::ContextResponse response("200", "OK");
  CPPUNIT_ASSERT(response.get_code().compare("200") == 0);
  CPPUNIT_ASSERT(response.get_reason().compare("OK") == 0);
  iota::ContextElement ce("iden", "type", "false");
  response.add_context_element(ce);

  iota::ContextResponses context_responses;
  context_responses.add_context_response(response);
  CPPUNIT_ASSERT(context_responses.get_context_responses().size() == 1);

  std::string res = context_responses.get_string();
  std::cout << "@UT@RES " << res << std::endl;
  std::istringstream is(res);
  iota::ContextResponses c_responses(is);
  CPPUNIT_ASSERT(c_responses.get_context_responses().size() == 1);
  iota::ContextResponse cr = c_responses.get_context_responses()[0];
  CPPUNIT_ASSERT(cr.get_code().compare("200") == 0);
  CPPUNIT_ASSERT(cr.get_reason().compare("OK") == 0);
  iota::ContextElement cr_ce = cr.get_context_element();
  CPPUNIT_ASSERT(cr_ce.get_id().compare("iden") == 0);

  std::cout << "START testResponse " << std::endl;
}

void JsonTest::testQueryContext() {

  std::cout << "Start testQueryContext" << std::endl;
  iota::QueryContext q;
  iota::Entity entity("id", "type", "false");
  q.add_entity(entity);
  q.add_attribute("temperatura");
  //std::cout << q.get_string() << std::endl;
  std::istringstream is(q.get_string());
  iota::QueryContext q1(is);
  CPPUNIT_ASSERT(q1.get_entities().size() == 1);
  CPPUNIT_ASSERT(q1.get_entities()[0].get_id().compare("id") == 0);

  std::istringstream is_bad("malformed");
  CPPUNIT_ASSERT_THROW_MESSAGE("Checking malformed QueryContext",
                               new iota::QueryContext(is_bad), std::runtime_error);
  std::istringstream is_bad_1("{}");
  CPPUNIT_ASSERT_THROW_MESSAGE("Checking malformed QueryContext",
                               new iota::QueryContext(is_bad_1), std::runtime_error);
  std::istringstream is_bad_2("{\"entities\":\"bad\"}");
  CPPUNIT_ASSERT_THROW_MESSAGE("Checking malformed QueryContext",
                               new iota::QueryContext(is_bad_2), std::runtime_error);
  std::istringstream is_bad_3("{\"entities\":[], \"attributes\": \"bad\"}");
  CPPUNIT_ASSERT_THROW_MESSAGE("Checking malformed QueryContext",
                               new iota::QueryContext(is_bad_3), std::runtime_error);
  rapidjson::Value v(10);
  CPPUNIT_ASSERT_THROW_MESSAGE("Checking malformed QueryContext",
                               new iota::QueryContext(v), std::runtime_error);
  rapidjson::Document d;
  d.SetObject();
  std::string k(is_bad_3.str());
  d.Parse<0>(k.c_str());
  CPPUNIT_ASSERT_THROW_MESSAGE("Checking malformed QueryContext",
                               new iota::QueryContext(d), std::runtime_error);
  std::cout << "Start testQueryContext" << std::endl;
}

void JsonTest::testRegisterContext() {

  std::cout << "Start testRegisterContext" << std::endl;
  iota::RegisterContext rc;
  iota::AttributeRegister attribute("name", "type", "value");
  iota::Entity entity("id", "type", "false");
  iota::ContextRegistration r;
  r.add_entity(entity);
  r.add_attribute(attribute);
  r.add_provider("http://miprovider:40/ngsi");
  rc.add_duration("25");
  rc.add_context_registration(r);

  std::istringstream is(rc.get_string());
  CPPUNIT_ASSERT(rc.get_context_registrations().size() == 1);
  CPPUNIT_ASSERT(
    rc.get_context_registrations()[0].get_provider().compare("http://miprovider:40/ngsi")
    == 0);
  CPPUNIT_ASSERT(rc.get_duration().compare("25") == 0);

  iota::RegisterContext rc_from_string(is);
  CPPUNIT_ASSERT_MESSAGE("Checking contextRegistrations ",
                         rc_from_string.get_context_registrations().size() == 1);
  CPPUNIT_ASSERT_MESSAGE("Checking duration ",
                         rc_from_string.get_duration().compare("25") == 0);
  CPPUNIT_ASSERT(
    rc.get_context_registrations()[0].get_provider().compare("http://miprovider:40/ngsi")
    == 0);

  std::string c_reg = rc.get_context_registrations()[0].get_string();

  rc_from_string.add_registrationId("rid");
  CPPUNIT_ASSERT_MESSAGE("Checking registrationId ",
                         rc_from_string.get_registrationId().compare("rid") == 0);
  // Error
  std::istringstream is_m("malformed");
  CPPUNIT_ASSERT_THROW_MESSAGE("Malformed RegisterContext ",
                               new iota::RegisterContext(is_m), std::runtime_error);
  std::istringstream is_1("{}");
  CPPUNIT_ASSERT_THROW_MESSAGE("Malformed RegisterContext ",
                               new iota::RegisterContext(is_1), std::runtime_error);
  std::istringstream is_2("{\"contextRegistrations\": \"bad\"}");
  CPPUNIT_ASSERT_THROW_MESSAGE("Malformed RegisterContext ",
                               new iota::RegisterContext(is_2), std::runtime_error);
  //std::cout << rc.get_string() << std::endl;
  rapidjson::Value v(10);
  CPPUNIT_ASSERT_THROW_MESSAGE("Malformed RegisterContext ",
                               new iota::RegisterContext(v), std::runtime_error);


  std::istringstream is_cr(r.get_string());
  iota::ContextRegistration cr(is_cr);
  CPPUNIT_ASSERT_MESSAGE("Checking CR ", cr.get_entities().size() == 1);
  CPPUNIT_ASSERT_MESSAGE("Checking CR ",
                         cr.get_provider().compare("http://miprovider:40/ngsi") == 0);

  std::istringstream is_bad("malformed");

  CPPUNIT_ASSERT_THROW_MESSAGE("Cecking malformed CR",
                               new iota::ContextRegistration(is_bad), std::runtime_error);

  std::istringstream is_bad_1("{}");
  CPPUNIT_ASSERT_THROW_MESSAGE("Cecking malformed CR",
                               new iota::ContextRegistration(is_bad_1), std::runtime_error);
  std::istringstream is_bad_2("{\"attributes\": \"bad\"}");
  CPPUNIT_ASSERT_THROW_MESSAGE("Cecking malformed CR",
                               new iota::ContextRegistration(is_bad_2), std::runtime_error);
  std::istringstream is_bad_3("{\"attributes\": []}");
  CPPUNIT_ASSERT_THROW_MESSAGE("Cecking malformed CR",
                               new iota::ContextRegistration(is_bad_3), std::runtime_error);
  rapidjson::Value v1(10);
  CPPUNIT_ASSERT_THROW_MESSAGE("Cecking malformed CR",
                               new iota::ContextRegistration(v1), std::runtime_error);

  std::cout << "End testRegisterContext" << std::endl;
}

void JsonTest::testAttributeRegister() {
  std::cout << "Start testAttributeRegister" << std::endl;
  iota::AttributeRegister attribute("name", "type", "value");
  std::string ar_str(attribute.get_string());
  std::istringstream is(ar_str);
  iota::AttributeRegister a(is);
  CPPUNIT_ASSERT(a.get_type().compare("type") == 0);
  CPPUNIT_ASSERT(a.get_name().compare("name") == 0);
  CPPUNIT_ASSERT(a.get_is_domain().compare("value") == 0);
  std::cout << "End testAttributeRegister" << std::endl;
}

void JsonTest::testEntity() {
  std::cout << "Start testEntity" << std::endl;
  iota::Entity entity("id", "type", "false");
  std::string e_str(entity.get_string());
  std::istringstream is(e_str);
  iota::Entity e(is);
  CPPUNIT_ASSERT(e.get_id().compare("id") == 0);
  CPPUNIT_ASSERT(e.get_type().compare("type") == 0);
  CPPUNIT_ASSERT(e.get_is_pattern().compare("false") == 0);
  // Error
  std::istringstream is_m("malformed");
  CPPUNIT_ASSERT_THROW_MESSAGE("Malformed entity", new iota::Entity(is_m),
                               std::runtime_error);
  std::istringstream a_m_1("{}");
  CPPUNIT_ASSERT_THROW_MESSAGE("Malformed entity", new iota::Entity(a_m_1),
                               std::runtime_error);
  rapidjson::Value v(10);
  CPPUNIT_ASSERT_THROW_MESSAGE("Malformed entity", new iota::Entity(v),
                               std::runtime_error);
  std::cout << "End testEntity" << std::endl;

}

void JsonTest::testSubscribeContext() {

  iota::Entity entity("id", "type", "false");
  iota::SubscribeContext sc;
  sc.add_entity(entity);
  sc.add_attribute("temperature");
  sc.add_attribute("pressure");
  sc.add_duration("P1M");
  sc.add_throttling("PT55");
  sc.add_reference("http://localhost:1026/iot/ngsi/operation");
  std::string v("ONCHANGE");
  iota::NotifyCondition nc(v);
  nc.add_value("meloinvento");
  sc.add_condition(nc);

  std::istringstream is(sc.get_string());
  iota::SubscribeContext sc1(is);
  CPPUNIT_ASSERT(sc1.get_duration().compare("P1M") == 0);
  CPPUNIT_ASSERT(sc1.get_throttling().compare("PT55") == 0);
  CPPUNIT_ASSERT(
    sc1.get_reference().compare("http://localhost:1026/iot/ngsi/operation") == 0);
  CPPUNIT_ASSERT(sc1.get_conditions().size() == 1);
  iota::NotifyCondition c = sc1.get_conditions()[0];
  CPPUNIT_ASSERT(c.get_type().compare("ONCHANGE") == 0);
  CPPUNIT_ASSERT(c.get_values().size() == 1);
  CPPUNIT_ASSERT(c.get_values()[0].compare("meloinvento") == 0);

  std::istringstream is_nc(c.get_string());
  iota::NotifyCondition nc_1(is_nc);
  CPPUNIT_ASSERT_MESSAGE("Checking NotifyCondition",
                         nc_1.get_values().size() == 1);

  std::istringstream nc_bad("malformed");
  CPPUNIT_ASSERT_THROW_MESSAGE("Checking malformed NC ",
                               new iota::NotifyCondition(nc_bad), std::runtime_error);
  CPPUNIT_ASSERT_THROW_MESSAGE("Checking malformed SubscribeContext ",
                               new iota::SubscribeContext(nc_bad), std::runtime_error);
  std::istringstream nc_bad_1("{}");
  CPPUNIT_ASSERT_THROW_MESSAGE("Checking malformed NC ",
                               new iota::NotifyCondition(nc_bad_1), std::runtime_error);
  CPPUNIT_ASSERT_THROW_MESSAGE("Checking malformed SubscribeContext ",
                               new iota::SubscribeContext(nc_bad_1), std::runtime_error);
  rapidjson::Value ve(10);
  CPPUNIT_ASSERT_THROW_MESSAGE("Checking malformed NC ",
                               new iota::NotifyCondition(ve), std::runtime_error);
  CPPUNIT_ASSERT_THROW_MESSAGE("Checking malformed SubscribeContext ",
                               new iota::SubscribeContext(ve), std::runtime_error);

}

void JsonTest::testSubscribeResponse() {
  iota::SubscribeResponse sr;
  std::string si("id");
  sr.add_subscription_id(si);
  CPPUNIT_ASSERT(sr.get_subscription_id().compare("id") == 0);
  std::cout << sr.get_subscription_id() << std::endl;
  std::cout << sr.get_string() << std::endl;
  std::istringstream is(sr.get_string());
  iota::SubscribeResponse s(is);
  CPPUNIT_ASSERT(s.get_subscription_id().compare("id") == 0);
}

void JsonTest::testErrors() {

  std::cout << "testErrors" << std::endl;
  // Attribute
  std::istringstream a_m("Malformed");
  CPPUNIT_ASSERT_THROW(new iota::Attribute(a_m), std::runtime_error);
  std::istringstream a_m_1("{}");
  CPPUNIT_ASSERT_THROW(new iota::Attribute(a_m_1), std::runtime_error);
  std::istringstream a_m_2("{\"name\": \"name\", \"metadatas\": \"badfield\"}");
  std::string a("{\"name\": \"name\"}");
  rapidjson::Document d;
  if (! d.Parse<0>(a.c_str()).HasParseError()) {
    iota::Attribute o(d);
    CPPUNIT_ASSERT(o.get_name().compare("name") == 0);
  }
  else {
    CPPUNIT_ASSERT(false);
  }
  rapidjson::Value v(10);
  CPPUNIT_ASSERT_THROW(new iota::Attribute(v), std::runtime_error);

  rapidjson::Document o;
  o.SetObject();
  CPPUNIT_ASSERT_THROW(new iota::Attribute(o), std::runtime_error);
  rapidjson::Value vo;
  vo.SetString("name");
  o.AddMember("name", vo, o.GetAllocator());
  rapidjson::Value vo1;
  vo1.SetObject();
  o.AddMember("metadatas", vo1, o.GetAllocator());
  CPPUNIT_ASSERT_THROW(new iota::Attribute(o), std::runtime_error);

  // AttributeRegister
  CPPUNIT_ASSERT_THROW(new iota::AttributeRegister(a_m), std::runtime_error);
  CPPUNIT_ASSERT_THROW(new iota::AttributeRegister(a_m_1), std::runtime_error);
  if (! d.Parse<0>(a.c_str()).HasParseError()) {
    iota::AttributeRegister o(d);
    CPPUNIT_ASSERT(o.get_name().compare("name") == 0);
  }
  else {
    CPPUNIT_ASSERT(false);
  }
  CPPUNIT_ASSERT_THROW(new iota::AttributeRegister(v), std::runtime_error);

  // ContextElement
  CPPUNIT_ASSERT_THROW(new iota::ContextElement(a_m), std::runtime_error);
  CPPUNIT_ASSERT_THROW(new iota::ContextElement(a_m_1), std::runtime_error);
  std::string ce("{\"id\": \"id\"}");
  if (! d.Parse<0>(ce.c_str()).HasParseError()) {
    iota::ContextElement o(d);
    CPPUNIT_ASSERT(o.get_id().compare("id") == 0);
  }
  else {
    CPPUNIT_ASSERT(false);
  }

  rapidjson::Document o1;
  o1.SetObject();
  vo.SetString("id");
  o1.AddMember("id", vo, o1.GetAllocator());
  vo1.SetArray();
  o1.AddMember("attributes", vo1, o1.GetAllocator());
  iota::ContextElement cc(o1);
  CPPUNIT_ASSERT(cc.get_attributes().size() == 0);

  std::string
  test_error("{  \"contextElements\" : [\"contextElement\" : {\"type\" : \"thing\",\"isPattern\" : \"false\",\"id\" : \"room2\",\"attributes\" : [{\"name\" : \"ping\",\"type\" : \"command\",\"value\" : \"567\"}]}},\"updateAction\" : \"UPDATE\"}");
  std::istringstream is_bad(test_error);
  CPPUNIT_ASSERT_THROW_MESSAGE("Bad update context",
                               new iota::UpdateContext(is_bad), std::runtime_error);
}


void JsonTest::testAttrCompound() {

  std::cout << "START testAttrCompound" << std::cout;

  iota::Attribute attribute("name", "compound");
  iota::Attribute subattr1("name1", "type1", "value1");
  iota::Attribute subattr2("name2", "type2", "value2");
  attribute.add_value_compound(subattr1);
  attribute.add_value_compound(subattr2);

  std::istringstream is(attribute.get_string());
  iota::Attribute obj_attribute(is);
  CPPUNIT_ASSERT(obj_attribute.get_name().compare("name") == 0);
  CPPUNIT_ASSERT(obj_attribute.get_type().compare("compound") == 0);
  std::vector<iota::Attribute> datas = obj_attribute.get_value_compound();
  CPPUNIT_ASSERT(datas.size() == 2);
  CPPUNIT_ASSERT(datas[0].get_name().compare("name1") == 0);
  CPPUNIT_ASSERT(datas[0].get_type().compare("type1") == 0);
  CPPUNIT_ASSERT(datas[0].get_value().compare("value1") == 0);
  CPPUNIT_ASSERT(datas[1].get_name().compare("name2") == 0);
  CPPUNIT_ASSERT(datas[1].get_type().compare("type2") == 0);
  CPPUNIT_ASSERT(datas[1].get_value().compare("value2") == 0);

  // Context Element
  iota::ContextElement ce("ce1", "", "false");
  iota::Attribute tstamp("tstamp", "tstamp", "tstamp");
  attribute.add_metadata(tstamp);
  ce.add_attribute(attribute);
  ce.add_attribute(tstamp);
  CPPUNIT_ASSERT_MESSAGE("Checking for all attributes",
                         ce.get_attributes().size() == 2);
  std::cout << "END testAttrCompound" << std::cout;
}

void JsonTest::testConversion() {
  rapidjson::Value vi(10);
  CPPUNIT_ASSERT_MESSAGE("Conversion to string from int",
                         iota::get_value_from_rapidjson<std::string>(vi) == "10");
  rapidjson::Value vd(10.123);
  CPPUNIT_ASSERT_MESSAGE("Conversion to string from int",
                         iota::get_value_from_rapidjson<std::string>(vd) == "10.123");
  rapidjson::Value vs("10.123");
  CPPUNIT_ASSERT_MESSAGE("Conversion to double from string",
                         iota::get_value_from_rapidjson<double>(vs) == 10.123);
  rapidjson::Value vo;
  vo.SetObject();
  CPPUNIT_ASSERT_THROW_MESSAGE("Conversion to double from object null",
                               iota::get_value_from_rapidjson<double>(vo), std::runtime_error);
  rapidjson::Value va;
  va.SetArray();
  CPPUNIT_ASSERT_THROW_MESSAGE("Conversion to double from array",
                               iota::get_value_from_rapidjson<double>(va), std::runtime_error);
  rapidjson::Value vb(false);
  CPPUNIT_ASSERT_MESSAGE("Conversion to bool from object bool",
                         iota::get_value_from_rapidjson<bool>(vb) == false);
  CPPUNIT_ASSERT_MESSAGE("Conversion to string from bool",
                         iota::get_value_from_rapidjson<std::string>(vb) == "0");
  rapidjson::Value vsi("21");
  CPPUNIT_ASSERT_MESSAGE("Conversion to int from string",
                         iota::get_value_from_rapidjson<int>(vsi) == 21);

  iota::JsonValue jvi(10);
  CPPUNIT_ASSERT_MESSAGE("Conversion to string from int",
                         iota::get_value_from_rapidjson<std::string>(jvi) == "10");
  iota::JsonValue jvd(10.123);
  CPPUNIT_ASSERT_MESSAGE("Conversion to string from int",
                         iota::get_value_from_rapidjson<std::string>(jvd) == "10.123");
  iota::JsonValue jvs("10.123");
  CPPUNIT_ASSERT_MESSAGE("Conversion to double from string",
                         iota::get_value_from_rapidjson<double>(jvs) == 10.123);
  iota::JsonValue jvo;
  jvo.SetObject();
  CPPUNIT_ASSERT_THROW_MESSAGE("Conversion to double from object null",
                               iota::get_value_from_rapidjson<double>(jvo), std::runtime_error);
  iota::JsonValue jva;
  jva.SetArray();
  CPPUNIT_ASSERT_THROW_MESSAGE("Conversion to double from array",
                               iota::get_value_from_rapidjson<double>(jva), std::runtime_error);
  iota::JsonValue jvb(false);
  CPPUNIT_ASSERT_MESSAGE("Conversion to bool from object bool",
                         iota::get_value_from_rapidjson<bool>(jvb) == false);
  CPPUNIT_ASSERT_MESSAGE("Conversion to string from bool",
                         iota::get_value_from_rapidjson<std::string>(jvb) == "0");
  iota::JsonValue jvsi("21");
  CPPUNIT_ASSERT_MESSAGE("Conversion to int from string",
                         iota::get_value_from_rapidjson<int>(jvsi) == 21);



}

