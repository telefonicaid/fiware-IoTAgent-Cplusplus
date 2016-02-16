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

#include "modbusTest.h"
#include <modbus/modbus.h>
#include <modbus/operation_processor.h>
#include "util/FuncUtil.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ModbusTest);

void ModbusTest::setUp() {}

void ModbusTest::tearDown() {}

void ModbusTest::testBuildFrame() {
  std::string expected_fc03 = "01 03 02 E7 00 06 74 47";
  iota::Modbus frame_fc03(0x01, iota::Modbus::READ_HOLDING_REGISTERS, 743, 6,
                          "AppOperation");
  std::string str_frame_fc03 = iota::str_to_hex(frame_fc03.get_modbus_frame());
  CPPUNIT_ASSERT_MESSAGE(expected_fc03,
                         expected_fc03.compare(str_frame_fc03) == 0);
  CPPUNIT_ASSERT_MESSAGE(
      "AppOperation", frame_fc03.get_operation().compare("AppOperation") == 0);

  std::string expected_fc06 = "01 06 02 DF 00 01 78 48";
  iota::Modbus frame_fc06(0x01, iota::Modbus::PRESET_SINGLE_REGISTER, 735, 1,
                          "AppOperation");
  std::string str_frame_fc06 = iota::str_to_hex(frame_fc06.get_modbus_frame());
  CPPUNIT_ASSERT_MESSAGE(expected_fc06,
                         expected_fc06.compare(str_frame_fc06) == 0);
  CPPUNIT_ASSERT_MESSAGE(
      "AppOperation", frame_fc06.get_operation().compare("AppOperation") == 0);

  // FC_0x10
  std::string expected_fc16 = "11 10 00 01 00 02 04 00 0A 01 02 C6 F0";
  std::vector<unsigned short> values;
  values.push_back(10);
  values.push_back(258);
  iota::Modbus frame_fc16(0x11, iota::Modbus::PRESET_MULTIPLE_REGISTER, 1, values,
                          "AppOperation", false);
  std::string str_frame_fc16 = iota::str_to_hex(frame_fc16.get_modbus_frame());
  CPPUNIT_ASSERT_MESSAGE(expected_fc16, expected_fc16.compare(str_frame_fc16) == 0);
  CPPUNIT_ASSERT_MESSAGE(
      "AppOperation", frame_fc16.get_operation().compare("AppOperation") == 0);
}

void ModbusTest::testDecodeFrame() {
  iota::Modbus frame_fc06(0x01, iota::Modbus::PRESET_SINGLE_REGISTER, 735, 1,
                          "AppOperation");
  std::string received_fc06 = "01 06 02 DF 00 01 78 48";
  boost::erase_all(received_fc06, " ");
  std::vector<unsigned char> data = iota::hex_str_to_vector(received_fc06);
  CPPUNIT_ASSERT_MESSAGE("Checking received frame ",
                         frame_fc06.receive_modbus_frame(data));
  CPPUNIT_ASSERT_MESSAGE("Checking no values ",
                         frame_fc06.get_values().size() == 0);
  CPPUNIT_ASSERT_MESSAGE("Checking if completed ", frame_fc06.completed());

  // Read holding registers
  std::string expected_fc03 = "01 03 09 C4 00 19 C6 61";
  iota::Modbus frame_fc03(0x01, iota::Modbus::READ_HOLDING_REGISTERS, 2500, 25,
                          "AppOperation");
  std::string str_frame_fc03 = iota::str_to_hex(frame_fc03.get_modbus_frame());
  CPPUNIT_ASSERT_MESSAGE(expected_fc03,
                         expected_fc03.compare(str_frame_fc03) == 0);
  std::string received_fc03 =
      "01 03 32 00 01 30 11 09 11 34 47 45 52 52 4F 52 20 20 20 20 20 31 10 08 10 10 38 33 30 38 20 20 20 20 20 20 20 00 00 00 00 00 \
00 00 00 00 00 00 00 00 00 00 00 60 AF";
  std::vector<unsigned char> data_fc03 = iota::hex_str_to_vector(received_fc03);
  CPPUNIT_ASSERT_MESSAGE("Checking received frame ",
                         frame_fc03.receive_modbus_frame(data_fc03));
  std::map<unsigned short, unsigned short> values = frame_fc03.get_values();
  CPPUNIT_ASSERT_MESSAGE("Checking 25 elements ", values.size() == 25);
  // Checking some values
  CPPUNIT_ASSERT_MESSAGE("First register ", values.begin()->first == 2501);
  CPPUNIT_ASSERT_MESSAGE("First value ", values.begin()->second == 0x0001);
  CPPUNIT_ASSERT_MESSAGE("Fith register ", values[2505] == 0x4552);
  CPPUNIT_ASSERT_MESSAGE("Last register ", values[2525] == 0x0000);
  CPPUNIT_ASSERT_MESSAGE("Checking if completed ", frame_fc03.completed());

  std::vector<unsigned short> values_to_write;
  values_to_write.push_back(10);
  values_to_write.push_back(258);
  iota::Modbus frame_fc16(0x11, iota::Modbus::PRESET_MULTIPLE_REGISTER, 1, values_to_write,
                          "AppOperation", false);
  std::string received_fc16 = "11 10 00 01 00 02 12 98";
  boost::erase_all(received_fc16, " ");
  std::vector<unsigned char> data16 = iota::hex_str_to_vector(received_fc16);
  CPPUNIT_ASSERT_MESSAGE("Checking received frame ",
                         frame_fc16.receive_modbus_frame(data16));
  CPPUNIT_ASSERT_MESSAGE("Checking no values ",
                         frame_fc16.get_values().size() == 0);

  // Frame with confirmation
  iota::Modbus frame_fc16_conf(0x11, iota::Modbus::PRESET_MULTIPLE_REGISTER, 1, values_to_write,
                          "AppOperation", true);
  std::string expected_fc16_conf = "11 10 00 01 00 02 04 00 0A 01 02 C6 F0";
  boost::erase_all(expected_fc16_conf, " ");
  std::vector<unsigned char> data16_conf = iota::hex_str_to_vector(expected_fc16_conf);
  CPPUNIT_ASSERT_MESSAGE("Checking received frame ",
                         frame_fc16_conf.receive_modbus_frame(data16_conf));
  CPPUNIT_ASSERT_MESSAGE("Checking received frame ",
                         frame_fc16_conf.receive_modbus_frame(data16));
  CPPUNIT_ASSERT_MESSAGE("Checking if completed ", frame_fc16_conf.completed());

}

void ModbusTest::testProcessor() {
  std::stringstream os;
  os << "{\"operations\": [";
  os << "{\"operation\": \"operation_name\",";
  os << "\"modbusOperation\": 3,";
  os << "\"modbusAddress\": 100,";
  os << "\"modbusNumberOfPositions\": 3,";
  os << "\"modbusNumberOfRegisters\": 10,";
  os << "\"positions\": [";
  os << "\"label_1\", \"\", \"label_3\"";
  os << "]}";
  os << ",";
  os << "{\"operation\": \"operation_name_1\",";
  os << "\"modbusOperation\": 3,";
  os << "\"modbusAddress\": 200,";
  os << "\"modbusNumberOfPositions\": 1,";
  os << "\"modbusNumberOfRegisters\": 1}";
  os << "]}";
  iota::ModbusOperationProcessor processor(os);
  boost::property_tree::ptree op_1 =
      processor.get_operation("operation_name_1");
  CPPUNIT_ASSERT_MESSAGE("Value modbusOperation 3",
                         op_1.get<unsigned short>("modbusOperation") == 3);
  CPPUNIT_ASSERT_MESSAGE("Value modbusAddress 200",
                         op_1.get<unsigned short>("modbusAddress") == 200);
  CPPUNIT_ASSERT_MESSAGE(
      "Labels ", processor.get_mapped_labels("operation_name").size() == 3);
  CPPUNIT_ASSERT_MESSAGE(
      "Label label_1 ",
      processor.get_mapped_labels("operation_name")[2].name.compare(
          "label_3") == 0);

  // Frame
  boost::property_tree::ptree op = processor.get_operation("operation_name");
  iota::Modbus frame_to_map(
      0x01,
      (iota::Modbus::FunctionCode)op.get<unsigned short>("modbusOperation"),
      op.get<unsigned short>("modbusAddress"),
      op.get<unsigned short>("modbusNumberOfPositions"), "AppOperation");
  std::string received_fc03 = "01 03 06 00 01 30 11 09 11 85 EC";
  // std::vector<unsigned char> data_fc03 =
  // iota::hex_str_to_vector(received_fc03);
  // std::cout << frame_to_map.crc(data_fc03) << std::endl;
  std::vector<unsigned char> data_fc03 = iota::hex_str_to_vector(received_fc03);
  CPPUNIT_ASSERT_MESSAGE("Checking received frame with labels ",
                         frame_to_map.receive_modbus_frame(data_fc03));
  std::map<std::string, std::string> mapped_values =
      frame_to_map.get_mapped_values(
          processor.get_mapped_labels("operation_name"));
  CPPUNIT_ASSERT_MESSAGE("Checking number of mapped values ",
                         mapped_values.size() == 2);
  std::map<std::string, std::string>::iterator i = mapped_values.begin();
  CPPUNIT_ASSERT_MESSAGE("Checking label 1", mapped_values["label_1"] == "1");
  // CPPUNIT_ASSERT_MESSAGE("Checking label 2", mapped_values["102"] == 12305);

  std::cout << "testProcessor label_3, value: " << mapped_values["label_3"]
            << std::endl;
  CPPUNIT_ASSERT_MESSAGE("Checking label 3",
                         mapped_values["label_3"] == "2321");
}

void ModbusTest::testProcessorFile() {
  iota::ModbusOperationProcessor processor;
  processor.read_operations("../../tests/iotagent/modbus_config.json");
  boost::property_tree::ptree op_1 = processor.get_operation("GetSAPCode");
  CPPUNIT_ASSERT_MESSAGE("Value modbusOperation 3",
                         op_1.get<unsigned short>("modbusOperation") == 3);
}

void ModbusTest::testCrc() {
  std::string f = "01 03 06 00 01 30 11 09 11";
  std::vector<unsigned char> o = iota::hex_str_to_vector(f);
  iota::Modbus modbus;
  std::cout << modbus.crc(o) << std::endl;
  CPPUNIT_ASSERT_MESSAGE("34284", modbus.crc(o) == 34284);
  std::string t = "01 03 08 AA AA BB BB CC CC DD DD";
  std::vector<unsigned char> o1 = iota::hex_str_to_vector(t);
  std::cout << modbus.crc(o1) << std::endl;
}

void ModbusTest::testProcessorCommandsFile() {
  iota::ModbusOperationProcessor processor;
  processor.read_operations("../../tests/iotagent/modbus_config.json");

  int base_addr = processor.get_base_address("installation_num");

  CPPUNIT_ASSERT_MESSAGE("Base address  installation_num ", base_addr == 0);

  base_addr = processor.get_base_address("test_command");
  CPPUNIT_ASSERT_MESSAGE("Base address  test_command ", base_addr == 20);

  std::vector<iota::CommandParameter> names_ordered =
      processor.get_mapped_parameters("test_command");

  std::string first_element = names_ordered[0].name;

  CPPUNIT_ASSERT_MESSAGE("not ordered parameters  test_command ",
                         first_element == "first");

  CPPUNIT_ASSERT_MESSAGE("no first parameter  test_command ",
                         names_ordered[0].name == "first");

  CPPUNIT_ASSERT_MESSAGE("no second parameter  test_command ",
                         names_ordered[1].name == "second");

  CPPUNIT_ASSERT_MESSAGE("no thrid parameter  test_command ",
                         names_ordered[2].name == "third");

  CPPUNIT_ASSERT_MESSAGE("no fourth parameter  test_command ",
                         names_ordered[3].name == "fourth");

  CPPUNIT_ASSERT_MESSAGE("not the default number of positions per parameter ",
                         names_ordered[0].num_positions == 1);

  CPPUNIT_ASSERT_MESSAGE("not the default type of parameter",
                         names_ordered[0].type == "numeric");

  boost::property_tree::ptree op_1 = processor.get_command("installation_num");
  CPPUNIT_ASSERT_MESSAGE("wrong parameter name ",
                         op_1.get<std::string>("name") == "installation_num");

  std::cout << "@UT@check get_protocol_commands";

  std::string pcommands = processor.get_protocol_commands();

  std::cout << "@UT@RES:" << pcommands << std::endl;

  CPPUNIT_ASSERT_MESSAGE(
      "no installation_num",
      pcommands.find("\"installation_num\"") != std::string::npos);

  CPPUNIT_ASSERT_MESSAGE("no tanks_num_cmd",
                         pcommands.find("\"tanks_num\"") != std::string::npos);

  CPPUNIT_ASSERT_MESSAGE(
      "no adquisition_time",
      pcommands.find("\"adquisition_time\"") != std::string::npos);

  CPPUNIT_ASSERT_MESSAGE(
      "no max_filllevel_alarm",
      pcommands.find("\"max_filllevel_alarm\"") != std::string::npos);

  CPPUNIT_ASSERT_MESSAGE(
      "no test_command",
      pcommands.find("\"test_command\"") != std::string::npos);

  names_ordered = processor.get_mapped_parameters("test_numeric_command");

  first_element = names_ordered[0].name;
  unsigned short positions = names_ordered[0].num_positions;
  std::string type = names_ordered[0].type;

  CPPUNIT_ASSERT_MESSAGE("wrong number of positions", positions == 5);

  CPPUNIT_ASSERT_MESSAGE("wrong parameter", first_element == "test_param");

  CPPUNIT_ASSERT_MESSAGE("numeric type of parameter expected",
                         type == "numeric");

  names_ordered = processor.get_mapped_parameters("test_string_command");

  first_element = names_ordered[0].name;
  type = names_ordered[0].type;

  CPPUNIT_ASSERT_MESSAGE("string type of parameter expected", type == "string");

  std::cout << "@UT@END check testProcessorCommandsFile" << std::endl;
}

void ModbusTest::testAllCommandsConfigFile() {
  iota::ModbusOperationProcessor processor;
  std::cout << "@UT@START check testAllCommandsConfigFile" << std::endl;
  processor.read_operations("../../tests/iotagent/modbus_config_base.json");

  std::vector<iota::CommandParameter> ordered_params =
      processor.get_mapped_parameters("gas_sensor_range");

  CPPUNIT_ASSERT_MESSAGE("no gas_sensor_range command and parameter",
                         ordered_params[0].name == "min_range");
  CPPUNIT_ASSERT_MESSAGE("wrong type on gas_sensor_range parameter",
                         ordered_params[0].type == "numeric");

  ordered_params = processor.get_mapped_parameters("c3_phone_number");

  CPPUNIT_ASSERT_MESSAGE("no c3_phone_number command and parameter",
                         ordered_params[0].name == "c3_phone_number");
  CPPUNIT_ASSERT_MESSAGE("wrong positions c3_phone_number parameter",
                         ordered_params[0].num_positions == 7);
  CPPUNIT_ASSERT_MESSAGE("wrong type on c3_phone_number parameter",
                         ordered_params[0].type == "string");
  std::cout << "@UT@END check testAllCommandsConfigFile" << std::endl;
}

void ModbusTest::testConversionDecimals() {
  std::cout << "@UT@START testConversionDecimals" << std::endl;

  std::string decimals_2 = "0.01";
  std::string decimals_1 = "0.1";
  std::string natural = "1";

  std::string wrong = "1.";
  double number = 233;
  iota::Modbus modbus;

  int test = iota::number_of_decimals(decimals_1);

  CPPUNIT_ASSERT_MESSAGE("one decimal", test == 1);

  std::cout << " value one decimal: " << std::fixed << std::setprecision(test)
            << number << std::endl;

  test = iota::number_of_decimals(decimals_2);

  CPPUNIT_ASSERT_MESSAGE("two decimals", test == 2);
  std::cout << " value two decimals: " << std::setprecision(test) << number
            << std::endl;

  test = iota::number_of_decimals(natural);
  CPPUNIT_ASSERT_MESSAGE("natural number", test == 0);
  std::cout << " value 0 decimals: " << std::setprecision(test) << number
            << std::endl;

  test = iota::number_of_decimals(wrong);
  CPPUNIT_ASSERT_MESSAGE("natural number", test == 0);
  std::cout << " value 0 decimals: " << std::setprecision(test) << number
            << std::endl;

  std::cout << "@UT@END testConversionDecimals" << std::endl;
}

void ModbusTest::testOperationsWithFloat() {
  iota::ModbusOperationProcessor processor;
  std::cout << "@UT@START check testOperationsWithFloat" << std::endl;
  processor.read_operations("../../tests/iotagent/modbus_config.json");

  std::vector<iota::FloatPosition> positions =
      processor.get_mapped_labels("GetDataHistory");

  CPPUNIT_ASSERT_MESSAGE("servicePressure",
                         positions[1].name == "servicePressure");
  CPPUNIT_ASSERT_MESSAGE("factor 0.01 ", positions[1].factor == 0.01F);

  std::cout << "@UT@END check testOperationsWithFloat" << std::endl;
}

void ModbusTest::testCheckFactor() {
  iota::ModbusOperationProcessor processor;
  std::cout << "@UT@START check testCheckFactor" << std::endl;
  processor.read_operations("../../tests/iotagent/modbus_configF.json");

  boost::property_tree::ptree op = processor.get_operation("GetDataHistory");
  std::cout << "@UT@1" << std::endl;
  iota::Modbus frame_to_map(
      0x01,
      (iota::Modbus::FunctionCode)op.get<unsigned short>("modbusOperation"),
      op.get<unsigned short>("modbusAddress"),
      op.get<unsigned short>("modbusNumberOfPositions"), "AppOperation");
  std::string received_fc03 = "01 03 06 00 01 30 11 09 11 85 EC";

  std::vector<unsigned char> data_fc03 = iota::hex_str_to_vector(received_fc03);
  std::cout << "@UT@receive_modbus_frame" << std::endl;
  CPPUNIT_ASSERT_MESSAGE("Checking received frame with labels ",
                         frame_to_map.receive_modbus_frame(data_fc03));
  std::cout << "@UT@get_mapped_values" << std::endl;
  std::map<std::string, std::string> mapped_values =
      frame_to_map.get_mapped_values(
          processor.get_mapped_labels("GetDataHistory"));

  std::cout << "@UT@servicePressure:" << mapped_values["servicePressure"]
            << std::endl;
  CPPUNIT_ASSERT_MESSAGE("servicePressure 123.05 ",
                         mapped_values["servicePressure"] == "123.05");

  std::cout << "@UT@END check testCheckFactor" << std::endl;
}

void ModbusTest::testCommandsToOperations() {
  std::cout << "START testCommandsToOperations" << std::endl;
  iota::ModbusOperationProcessor processor;
  /**
   {
          "name" : "test_command",
          "parameters" : [
           {
             "name":"second",
             "address": 21
           },
           {
             "name":"third",
             "address": 22
           },
           {
             "name":"fourth",
             "address": 23
           },
           {
             "name":"first",
             "address": 20
           }
          ]
      },
      {
          "name" : "test_numeric_command",
          "parameters" : [
           {
             "name":"test_param",
             "type" : "numeric",
             "positions" : 5,
             "address": 21
           }]
      },
      {
          "name" : "test_string_command",
          "parameters" : [
           {
             "name":"test_param",
             "type": "string",
             "positions" : 25,
             "address": 21
           }]
      }
  */
  processor.read_operations("../../tests/iotagent/modbus_configF.json");

  // Extract some commands as operations.

  // First scenario
  {
    boost::property_tree::ptree& pt_operation =
        processor.get_operation("test_command");

    CPPUNIT_ASSERT_MESSAGE(
        "Value modbusOperation 3",
        pt_operation.get<unsigned short>("modbusOperation") == 3);
    CPPUNIT_ASSERT_MESSAGE(
        "Value modbusAddress 20",
        pt_operation.get<unsigned short>("modbusAddress") == 20);
    CPPUNIT_ASSERT_MESSAGE(
        "Labels ", processor.get_mapped_labels("test_command").size() == 4);
    CPPUNIT_ASSERT_MESSAGE(
        "Label third ",
        processor.get_mapped_labels("test_command")[2].name.compare("third") ==
            0);
  }
  // Second scenario: numeric command
  {
    boost::property_tree::ptree& pt_operation =
        processor.get_operation("test_numeric_command");

    CPPUNIT_ASSERT_MESSAGE(
        "Value modbusOperation 3",
        pt_operation.get<unsigned short>("modbusOperation") == 3);
    CPPUNIT_ASSERT_MESSAGE(
        "Value modbusAddress 21",
        pt_operation.get<unsigned short>("modbusAddress") == 21);
    CPPUNIT_ASSERT_MESSAGE(
        "Value modbusNumberOfPositions 5",
        pt_operation.get<unsigned short>("modbusNumberOfPositions") == 5);

    CPPUNIT_ASSERT_MESSAGE(
        "Value modbusNumberOfRegisters 1",
        pt_operation.get<unsigned short>("modbusNumberOfRegisters") == 1);

    CPPUNIT_ASSERT_MESSAGE("Type of command numeric",
                           pt_operation.get<std::string>("modbusRegisterType")
                                   .compare("numeric") == 0);

    CPPUNIT_ASSERT_MESSAGE(
        "Label, only one ",
        processor.get_mapped_labels("test_numeric_command").size() == 1);
    CPPUNIT_ASSERT_MESSAGE(
        "Label test_param ",
        processor.get_mapped_labels("test_numeric_command")[0].name.compare(
            "test_param") == 0);
  }
  // Third scenario: string command
  {
    boost::property_tree::ptree& pt_operation =
        processor.get_operation("test_string_command");

    CPPUNIT_ASSERT_MESSAGE(
        "Value modbusOperation 3",
        pt_operation.get<unsigned short>("modbusOperation") == 3);
    CPPUNIT_ASSERT_MESSAGE(
        "Value modbusAddress 21",
        pt_operation.get<unsigned short>("modbusAddress") == 21);
    CPPUNIT_ASSERT_MESSAGE(
        "Value modbusNumberOfPositions 25",
        pt_operation.get<unsigned short>("modbusNumberOfPositions") == 25);

    CPPUNIT_ASSERT_MESSAGE(
        "Value modbusNumberOfRegisters 1",
        pt_operation.get<unsigned short>("modbusNumberOfRegisters") == 1);

    CPPUNIT_ASSERT_MESSAGE("Type of command numeric",
                           pt_operation.get<std::string>("modbusRegisterType")
                                   .compare("string") == 0);

    CPPUNIT_ASSERT_MESSAGE(
        "Label, only one ",
        processor.get_mapped_labels("test_string_command").size() == 1);
    CPPUNIT_ASSERT_MESSAGE(
        "Label test_param ",
        processor.get_mapped_labels("test_string_command")[0].name.compare(
            "test_param") == 0);
  }

  {
    boost::property_tree::ptree& pt_operation =
        processor.get_operation("test_command");
    iota::Modbus frame_to_map(
        0x01, (iota::Modbus::FunctionCode)pt_operation.get<unsigned short>(
                  "modbusOperation"),
        pt_operation.get<unsigned short>("modbusAddress"),
        pt_operation.get<unsigned short>("modbusNumberOfPositions"),
        "test_command");
    std::string received_frame = "01 03 08 00 01 00 02 00 03 00 04";

    std::vector<unsigned char> v_bytes =
        iota::hex_str_to_vector(received_frame);
    unsigned short crc = frame_to_map.crc(v_bytes);
    v_bytes.push_back((crc >> 8) & 0xFF);
    v_bytes.push_back(crc & 0xFF);
    std::cout << "FRAME: " << iota::str_to_hex(v_bytes) << std::endl;

    CPPUNIT_ASSERT_MESSAGE("Checking received frame with labels ",
                           frame_to_map.receive_modbus_frame(v_bytes));

    std::map<std::string, std::string> mapped_values =
        frame_to_map.get_mapped_values(
            processor.get_mapped_labels("test_command"));

    CPPUNIT_ASSERT_MESSAGE("Checking number of mapped values ",
                           mapped_values.size() == 4);
    std::map<std::string, std::string>::iterator i = mapped_values.begin();
    CPPUNIT_ASSERT_MESSAGE("Checking first parameter == 1",
                           mapped_values["first"] == "1");

    // CPPUNIT_ASSERT_MESSAGE("Checking label 2", mapped_values["102"] ==
    // 12305);

    std::cout << " second parameter, value: " << mapped_values["second"]
              << std::endl;

    // CPPUNIT_ASSERT_MESSAGE("Checking label 3",
    //                       mapped_values["label_3"] == "2321");
  }

  std::cout << "END testCommandsToOperations" << std::endl;
}
