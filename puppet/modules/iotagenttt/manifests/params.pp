class iotagenttt::params {

  # Class parameters
  
  $dev                     = false
  
  # Package parameters
  $iotagenttt_package      = 'iot-agent-tt'
  $iotagenttt_version      = 'latest'
  
  # Config parameters
  $iotagent_configdir        = '/etc/iot'
  $iotagent_configfile       = 'config.json'
  $iotagenttt_configdir    = '/etc/iot'
  $iotagenttt_configfile   = 'TTService.xml'
  $iotagenttt_resourceName = 'tt'
   
  $iotagenttt_fileName     = 'TTService'
  #doble quoted to get escape sequences available: \n, \r,...
  $iotagenttt_options      = "\"ConfigFile\": \"$iotagenttt_configdir/$iotagenttt_configfile\""
  
  
  
  # Init parameters
  
  # Service parameters
  


}