class logrotate::params {
  
  $config_path        = '/etc/logrotate.conf'
  $config_basedir     = '/etc/logrotate.d'
  $package_name       = 'logrotate'
  $package_ensure     = 'present' # or one of the versions needed ...
}