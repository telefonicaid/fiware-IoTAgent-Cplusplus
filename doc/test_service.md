# Sample plugin
## Test Service ###

Test Service is an example of iotagent plugin. This plugin receive information from a device, parse it
to a NGSI updateContext and send it to contextBroker, it can receive also NGSI messages.

Iot plugins are based in PION libraries. In order to generate a plugin you need to create a class with some special functions. You can find an example in dca-core/src/iotagent/sample_test.

First you should create a class which inherits from RestHandle, this class provides some iotagent special functions. Our sample plugin is named TestService.

You need to declare two functions outside your class. One to create new TestService objects and other one to destroy this objects. You should replace "TestService" by the name of your plugin. 

```
extern "C" PION_PLUGIN riot::TestService *pion_create_TestService(void)
{
    return new riot::TestService();
}

extern "C" PION_PLUGIN void pion_destroy_TestService(riot::TestService *service_ptr)
{
    delete service_ptr;
}
```

You needs also create some function inside your class. Such as :

- "start" function, which will be called when a plugin object is started. Here you need to invoke "add_url" to assign callback function for incoming HTTP messages from devices. If your plugin needs receive NGSI messages, you should call "enable_ngsi_service"  and specify the callback function to threat received NGSI messages, "op_ngsi" in our example.

```
 void example::TestService::start() {
  std::map<std::string, std::string> filters;
  add_url("", filters, REST_HANDLE(&example::TestService::service), this);
  // This function should be called only if plugin needs receive NGSI messages
  enable_ngsi_service(filters, REST_HANDLE(&example::TestService::op_ngsi), this);
}
```

- Callback function, named "service" in our example which will be invoked when plugin receives an HTTP message from devices. Callback function  receives a pointer to HTTP request an offers an HTTP response parameter.

```
void example::TestService::service(
                pion::http::request_ptr& http_request_ptr,
                std::map<std::string, std::string>& url_args,
                std::multimap<std::string, std::string>& query_parameters,
                pion::http::response& http_response, 
                std::string& response) {
```

- Optionally if your plugin should receive NGSI messages, you must declare a callback function for incoming NGSI requests, "op_ngsi"  in our example: 

```
void example::TestService::op_ngsi(
                pion::http::request_ptr& http_request_ptr,
                std::map<std::string, std::string>& url_args,
                std::multimap<std::string, std::string>& query_parameters,
                pion::http::response& http_response, 
                std::string& response) {
```


Once plugin code is writed,  you need to create an CMakeLists.txt. You can find an example in "dca-core/src/iotagent/sample_test", here you should replace "TestService" by the name of your plugin and "testsrv" by the name of the library in which you want generate your plugin.

Finally to compile the plugin you should move to "dca-core/build/Debug" directory, set cmake environment 

```
cmake -DBOOST_ROOT=/home/develop/iot/boost_1_55_0 -DCMAKE_BUILD_TYPE=Debug ../../
```

And execute "make install"

Iotagent binary will be generated in "dca-core/bin/Debug" and libraries will be placed in "dca-core/lib/Debug"


IoTAgent reqires a configuration file to start, an example is provided in dca-core/config.json

This config.json includes the definition of the sample plugin resource TestService, which is started in
 url /iot/test. Information of test service is also provided. Some field have format limitations : 

- "service" value must not be longer than 50 characters and may only contain underscores and alphanumeric characters.

- "service_path" value must begin with / and contain only alphanumeric characters


IoTAgent start requires some parameters, is mandatory specify configuration file (-c), IP address (-i) and libraries location (-d). By default listen in port 8080.

An example of start command should be:

```
/usr/local/iot/bin/iotagent  -n qa -i xxx.xxx.xxx.xxx -p 8023 -d /usr/local/iot/lib -c ./config.json -v DEBUG
```

Once started IotAgent generates a log file "/usr/local/iot/lib" in "dir_log" specified in config.json 

A message to SouthBound can be send whit the curl:

```
curl -v --data "Hello Word"  --request POST  "http://xxx.xxx.xxx.xxx:8023/iot/test?k=apikey-test&i=dev_1"
```
where:
- i: parameter to define the device.
- k: parameter to define the service apikey.


A message to NGSI interface can be generated with the curl:

```
curl --request POST  http://xxx.xxx.xxx.xxx:8023/iot/ngsi/test/updateContext  --header 'Content-Type: application/xml' $CURL_VERBOSE --data-binary @ngsi_update.xml
```

Where ngsi_update.xml  should contain a valid NGSI updateContext



## Test Command Service ##

Test Command Service is a more complex and complete example of iotagent plugin, which includes command functionality.

You can find the code of this example plugin in  dca-core/src/iotagent/sample_test/test_command_service.cc

In test_command_service.h you can find definition of class TestCommandService, which inherits from iota::CommandHandle

As in all PION plugins you need to create two functions "pion_create_TestCommandService" and "pion_destroy_TestCommandService" outside your class

Now we will explain  functions of Test Command Service class


### start

A start function is also always necessary and it is called by pion to init the plugin .  In this function you need to invoke "add_url" to assign callback function for incoming HTTP messages from devices. 
This example plugin will receive NGSI messages, then you must call "enable_ngsi_service"  to specify the callback function to threat received NGSI messages, which in this example is default_op_ngsi of CommandHandle class.
Command handle class includes a cache of commands, in default_op_ngsi  method polling commands are stored in this cache waiting for a response from device. 


###  service

Callback function, named "service" in our example which will be invoked when plugin receives an HTTP message from devices.

Here http method is checked :

- If verb is GET, get_commands_polling  is invoked to get polling commands.

- If method is PUT, we call  receive_command_results to receive the result of command

- Otherwise, it is considered a measure reception and receive_observations is invoked


###  get_commands_polling

This function receives an HTTP message with verb GET in param http_request_ptr. This message is sent by  a device when it  wake up and check with iotagent if there are polling commands to execute. This function search outstanding commands in cache, remove them from cache and send them to device.


### receive_command_results 

Receives commands results from device into param http_request_ptr.  Search the command in cache and if find it change command status, transform it into an updateContext message and send it to contextBroker. 


### receive_observations

This function receives a HTTP message in param http_request_ptr including observations from devices, translates observations into an NGSI updateContext message and send it to contextBroker.


###  execute_command

Virtual function that must be implemented in every Iot plugin which inherits from iota::CommandHandle


