class iotagent::params {
  
  # Class parameters
  $num_services                = 0
  $dev                         = false
 
  #Searching for config values in hiera data. Some of them will fail if not found.

  $iota_server_port            = hiera('iota::base::iota_server_port')
  $iota_server_address         = hiera('iota::base::iota_server_address')
  $iota_server_name            = hiera('iota::base::iota_server_name')
  $iota_log_level              = hiera('iota::base::iota_log_level','DEBUG')
  $mongodb_host                = hiera('iota::base::mongodb_host')
  $mongodb_port                = hiera('iota::base::mongodb_port')

  #Authorization parameters
  $trust_token_url             = hiera('iota::authorization::trust_token_url')
  $user_token_url              = hiera('iota::authorization::user_token_url')
  $user_roles_url              = hiera('iota::authorization::user_roles_url')
  $trust_token_user            = hiera('iota::authorization::trust_token_user')
  $trust_token_password        = hiera('iota::authorization::trust_token_password')
  $user_subservices_url        = hiera('iota::authorization::user_subservices_url')
  $user_access_control_url     = hiera('iota::authorization::user_access_control_url')
  $pep_user                    = hiera('iota::authorization::pep_user')
  $pep_password                = hiera('iota::authorization::pep_password')
  $pep_domain                  = hiera('iota::authorization::pep_domain')
  $auth_timeout                = hiera('iota::authorization::timeout')
      
  #NGSI section
  $context_broker_endpoint     = hiera('iota::ngsi::context_broker_endpoint')
  $update_context_path         = hiera('iota::ngsi::update_context_path', '/NGSI10/updateContext')
  $register_context_path       = hiera('iota::ngsi::register_context_path', '/NGSI9/registerContext')
  $query_context_path          = hiera('iota::ngsi::query_context_path','/NGSI10/queryContext')

  

  # Package parameters
  $iotagent_package            = 'iot-agent-base'
  $iotagent_version            = 'latest'
  
  # Config parameters
  $iotagent_configdir          = '/etc/iot'
  $iotagent_configfile         = 'config.json'
  $iotagent_bindir             = '/usr/local/iot/bin'
 
  $iotagent_librarydir         = '/usr/local/iot/lib'
  $iotagent_start              = "/usr/local/iot/bin/iotagent -n $iota_server_name"
  $iotagent_pidonlyfile        = 'iotagent.pid'
  $iotagent_piddir             = '/var/run/iot'
  $iotagent_pidfile            = "$iotagent_piddir/$iotagent_pidonlyfile"
  $iotagent_logdir             = '/var/log/iot/'
  $iotagent_process_name_monit = "iotagent-$iota_server_name"
  
  
  # Init parameters
  
  # Service parameters

}