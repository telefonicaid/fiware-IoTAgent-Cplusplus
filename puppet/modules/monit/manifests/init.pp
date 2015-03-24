
class monit(
	$period         = $monit::params::period,
	$port           = $monit::params::port,
	$login          = $monit::params::login,
	$passwd         = $monit::params::passwd,
    $statefile      = $monit::params::statefile,
    $package_ensure = $monit::params::package_ensure
	) inherits monit::params {

        contain monit::install
        contain monit::config
        contain monit::service 
        Class['monit::install'] -> Class['monit::config'] -> Class['monit::service']
}