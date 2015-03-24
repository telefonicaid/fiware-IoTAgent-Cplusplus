class iotagentevadts::params {

  # Class parameters
 
  $dev                         = false
  
  # Package parameters
  $iotagentevadts_package      = 'iot-agent-evadts'
  $iotagentevadts_version      = 'latest'
  
  # Config parameters
  $iotagent_configdir          = '/etc/iot'
  $iotagent_configfile         = 'config.json'
  $iotagentevadts_configdir    = '/etc/iot'
  $iotagentevadts_configfile   = 'evadts.json'
  $iotagentevadts_resourceName = 'evadts'
  $iotagentevadts_fileName     = 'EvadtsService'

  #doble quoted to get escape sequences available: \n, \r,...
  $iotagentevadts_options      = "\"ConfigFile\": \"$iotagentevadts_configdir/$iotagentevadts_configfile\""
  
  
  # Init parameters
  
  # Service parameters

}