class monit::params {
  
  # Default vaules for class parameters

  $port           = 2812
  $period         = 60	# 60 seconds monitoring
  $logfile        = '/var/log/monit.log'
  $statefile      = '/tmp/monit.state'
  $login          = 'dummyuser'
  $passwd         = 'dummypasswd'
  $package_ensure = 'present'

 # Other suff
  $package_name        = 'monit'
  $service_name        = 'monit'
  $service_ensure      = 'running'
  $respawn_limit       = 5  # times
  $respawn_time        = 10 # seconds  
  $config_file         = '/etc/monit.conf'
  $config_dir          = '/etc/monit.d'
  $exec_file           = '/usr/bin/monit'
  $upstart_file        = '/etc/init/monit.conf'
  $config_global_file  = '/etc/monit.d/global'
  $config_logging_file = '/etc/monit.d/logging'
  $log_specfile        = 'monit'

}