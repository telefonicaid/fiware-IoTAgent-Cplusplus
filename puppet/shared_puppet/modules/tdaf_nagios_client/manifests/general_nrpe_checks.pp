class tdaf_nagios_client::general_nrpe_checks {

	file {'/etc/nagios':
		ensure => directory,
		owner  => 'nagios',
		group  => 'nagios',
	}

	# make sure the plugins have the right permissions
	# user and group nagios are provided by the nagios-common package
	file {'general_checks_dir':
		path => $tdaf_nagios_client::checks_dir,
		source => "puppet:///modules/${module_name}/general_nrpe_checks/",
		ensure => directory,
		recurse => true,
		owner   => 'nagios',
		group   => 'nagios',
		require => File['/etc/nagios'],
	}

}
