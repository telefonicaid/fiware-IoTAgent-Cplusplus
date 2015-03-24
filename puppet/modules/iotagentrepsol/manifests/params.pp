class iotagentrepsol::params {

  # Class parameters
  
  $dev                         = false
  
  $smpp_adapter_port           = hiera('iotagentrepsol::smpp_port')
  $smpp_adapter_host           = hiera('iotagentrepsol::smpp_host')
  $smpp_adapter_url            = hiera('iotagentrepsol::smpp_url')
  
  # Package parameters
  $iotagentrepsol_package      = 'iot-agent-sms-repsol'
  $iotagentrepsol_version      = 'latest'
  
  # Config parameters
  $iotagent_configdir          = '/etc/iot'
  $iotagent_configfile         = 'config.json'
  $iotagentrepsol_configdir    = '/etc/iot'
  $iotagentrepsol_configfile   = 'Repsol.xml'
  $iotagentrepsol_resourceName = 'repsol'
  $iotagentrepsol_fileName     = 'RepsolService'
  #doble quoted to get escape sequences available: \n, \r,...
  $iotagentrepsol_options      = "\"ConfigFile\": \"$iotagentrepsol_configdir/$iotagentrepsol_configfile\""
  
  $from_number                 = '682996050'

  
  # Init parameters

  # Service parameters

}