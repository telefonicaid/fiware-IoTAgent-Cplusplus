class tdaf_nagios_client::plugins {

	file {'/usr/lib64/nagios':
		ensure => directory,
		owner  => 'nagios',
		group  => 'nagios',
	}

	# make sure the plugins have the right permissions
	# user and group nagios are provided by the nagios-common package
	file {'/usr/lib64/nagios/plugins':
		source  => "puppet:///modules/${module_name}/plugins-nagios/",
		ensure  => directory,
		recurse => true,
		owner   => 'nagios',
		group   => 'nagios',
		mode    =>  0644,
		require => File['/usr/lib64/nagios'],
	}

	exec {'Permisos_ejecucion':
		command => "chmod 755 /usr/lib64/nagios/plugins/*",
		path    => '/bin:/usr/bin:/usr/sbin',
		onlyif => "/usr/bin/test $(ls -lart /usr/lib64/nagios/plugins/check-https-conn-ha | grep -c 'rwxr-xr-x') -eq 0",
		require => File['/usr/lib64/nagios/plugins'],
	}
}
