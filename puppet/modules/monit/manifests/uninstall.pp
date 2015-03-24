class monit::uninstall inherits monit::params {
  
  require logrotate

  service { $service_name:
  	ensure  => stopped,
    enable  => false,
  }
  ->
  file { [ $config_global_file , $config_logging_file, $config_file, $upstart_file] :
      ensure  => absent,
  }
  ->
  package { $package_name:
    ensure  => absent,
  }
  ->
  logrotate::specfile{ $log_specfile : 
    ensure => absent,
  }
}