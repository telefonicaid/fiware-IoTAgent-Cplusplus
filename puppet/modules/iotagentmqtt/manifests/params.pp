class iotagentmqtt::params {

  # Class parameters
 
  $dev                       = false
  
  # Package parameters
  $iotagentmqtt_package      = 'iot-agent-mqtt'
  $iotagentmqtt_version      = 'latest'
  
  # Config parameters
  $iotagent_configdir        = '/etc/iot'
  $iotagent_configfile       = 'config.json'
  $iotagentmqtt_configdir    = '/etc/iot'
  $iotagentmqtt_configfile   = 'MqttService.xml'
  $iotagentmqtt_resourceName = 'mqtt'

  $iotagentmqtt_fileName     = 'MqttService'
  #doble quoted to get escape sequences available: \n, \r,...
  $iotagentmqtt_options      = "\"ConfigFile\": \"$iotagentmqtt_configdir/$iotagentmqtt_configfile\""
  

  
  # Service parameters
  $iotagentmqtt_pidfile      = '/var/run/iot/mosquitto.pid'
  $iotagentmqtt_scriptsdir   = '/usr/local/iot/scripts'
  $iotagentmqtt_initscript   = 'init_mosquitto.sh'

}