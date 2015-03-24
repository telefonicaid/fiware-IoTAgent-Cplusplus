class tdaf_nagios_client::nrpe (
	$nrpe_port = 5666,
	# IP clients alowed to connect to Nagios NRPE daemon. List with a blank spaced IPs
	$tdaf_nagios_clients_ips = "127.0.0.2 127.0.0.3",
	$service_enable = true,
	$service_running = running,
	) {

	# make sure software is installed

	package { "nagios-common":
		ensure => '3.4.1-2.el6',
	}

	package { "nagios-plugins":
		ensure => '1.4.15-2.el6.rf',
	}

	package {
		['xinetd',
		'nagios-nrpe',
		'python',
		'pdi-python-pymongo',
		'perl-Time-HiRes',
		'sysstat',
		'sudo',
		]:
		ensure => present,
	} 

	# make sure the firewall is open
	firewall{"0200-ACCEPT PORT $nrpe_port":
		source      => '0/0',
		destination => '0/0',
		port        => $nrpe_port,
		action      => 'accept',
	}

	# make sure the correct line is in /etc/services
	augeas {'/etc/services':
		context => 'files/etc/services',
		changes => [
			"set service-name[port = $nrpe_port]/port $nrpe_port",
			"set service-name[port = $nrpe_port] nrpe",
			"set service-name[port = $nrpe_port]/protocol tcp",
			"set service-name[port = $nrpe_port]/#comment NRPE",
		],
	}

	# configure the xinetd service
	file {'/etc/xinetd.d/nrpe':
		content => template("${module_name}/xinetd-nrpe.erb"),
		require => Package['nagios-nrpe'],
	}

	# configure root execution to nagios plugins
	file {'/etc/sudoers.d/nagios.sodoers':
		owner   => 'root',
		group   => 'root',
		mode    =>  0440,
		source  => "puppet:///modules/${module_name}/sudoers.d/nagios.sudoers",
		require => Package['sudo'],
	}

	file {'/etc/nagios/nrpe.cfg':
		content => template("${module_name}/nrpe.cfg.erb"),
		require => Package['nagios-nrpe'],
	}

	exec{ 'Disable_Selinux' :
		command => "echo 0 > /selinux/enforce",
		path    => '/bin:/usr/bin:/usr/sbin',
	}

	exec { "add_nrpe_services":
		command => "/bin/echo 'nrpe             5666/tcp               #NRPE' >> /etc/services",
		onlyif => "/usr/bin/test $(/bin/grep -c nrpe /etc/services) -eq 0",
	}

	service {'xinetd':
		enable  => $service_enable,
		ensure  => $service_running,
		subscribe => [ 
			File['/etc/xinetd.d/nrpe'],
			File['/etc/nagios/nrpe.cfg']
		],
		require => [ 
			File['/etc/xinetd.d/nrpe'],
			Class['tdaf_nagios_client::plugins'],
		],
	}


	# Only test if service is running
	if $service_running == 'running' {
		exec { "Test_Check_Alive":
			command => "/usr/bin/test $(/usr/lib64/nagios/plugins/check_nrpe -H localhost -c check_alive | grep -c OK) -eq 1",
			require => [
				Service['xinetd'],
				Class [tdaf_nagios_client::general_nrpe_checks],
			],
		}
	}
}