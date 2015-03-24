class smpp::params {

  # Package parameters
  $smpp_package      = 'bluevia-nsdp-bk-adapter-smpp.noarch'
  $smpp_version      = 'latest'

  # Config parameters
  $adapter_port = '4330'
  $mgmt_port    = '7777'
  $smpp_port    = '4332'
  $smpp_host    = 'localhost'
  $incoming     = 'http://localhost/handle'
  $phonenumber  = '22202'

  
  # Init parameters

  # Service parameters
  $smpp_pidfile = '/var/run/pdi-adapter-sms-smpp.pid'
  $smpp_start   = '/bin/bash -c \'/etc/init.d/pdi-adapter-sms-smpp start\''
  $smpp_stop    = '/bin/bash -c \'/etc/init.d/pdi-adapter-sms-smpp stop\''
}