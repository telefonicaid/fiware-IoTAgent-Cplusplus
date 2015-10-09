#ifndef IOTAMQTTCOMMANDS_H
#define IOTAMQTTCOMMANDS_H

#include <rest/command_handle.h>

namespace iota {
namespace esp {
namespace ngsi {

class IotaMqttCommands : public iota::CommandHandle {
 public:
  IotaMqttCommands(){};
  virtual ~IotaMqttCommands(){};

  /**
    @name serializeMqttCommand
    @brief This function returns the string of the Mqtt command including the
    topic and payload, both are separated by double back-slash '//'.
    @param apikey: [IN] std::string, must be the existing apikey for the service
    this device belongs to
    @param device: [IN] std::string, device ID.
    @param command: [IN] std::string, name of the command. Must be known by the
    device obviously
    @param payload: [IN] std::string, payload of the command.
    @return
      std::string, example: 1234/device/cmd/PING//device@PING#param1|value1
      the Topic "cmd" is by convention what MQTT devices will use to accept
    commands.

  */
  virtual std::string serializeMqttCommand(std::string apikey,
                                           std::string device,
                                           std::string command,
                                           std::string payload) = 0;

  /**
  @name execute_mqtt_command
  @brief It will publish a mqtt message on the broker that represents the
  command intended for a particular device.
  @param apikey: [IN] std::string, must be the existing apikey for the service
  this device belongs to
  @param device: [IN] std::string, device ID.
  @param command: [IN] std::string, name of the command. Must be known by the
  device obviously
  @param command_ayload: [IN] std::string, payload of the command.
  @param command_id: [IN] it will be included in the mqtt payload.

  @return this is the code in HTTP status code format.
    202: message was published with no errors (device may not get it yet)
    400: couldn't publish for any reasons.

  */
  virtual int execute_mqtt_command(std::string apikey, std::string device,
                                   std::string name,
                                   std::string command_payload,
                                   std::string command_id) = 0;

  /**
  @name respond_mqtt_command
  @brief This function will dispose the excecuted command identified by its
  command id. It will also trigger the updateContext to CB internally.
  So basically it has to retrieve the command from the storage and remove it,
  then send its status to CB including the response that goes in
  command_payload.

  @param apikey: [IN] std::string, must be the existing apikey for the service
  this device belongs to
  @param device: [IN] std::string, device ID.
  @param command_payload: [IN] std::string, response of the command.
  @param command_id: [IN] it was included in the mqtt payload but at this point,
  it's separated from it.
  */
  virtual void respond_mqtt_command(std::string apikey, std::string device,
                                    std::string command_payload,
                                    std::string command_id) = 0;

 protected:
 private:
};
}
}
}
#endif  // IOTAMQTTCOMMANDS_H
