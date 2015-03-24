class logrotate(
	$package_ensure = $logrotate::params::package_ensure,
	) 
inherits logrotate::params 
{
    contain logrotate::install
    contain logrotate::config
    contain logrotate::service 

    Class['logrotate::install'] -> Class['logrotate::config'] -> Class['logrotate::service']

}