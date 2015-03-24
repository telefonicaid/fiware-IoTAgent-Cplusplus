class tdaf_vagrant::ec2tools (
	$install_ec2tools = true,
) {
	if $install_ec2tools {
		$ec2toolsURI = 'http://s3.amazonaws.com/ec2-downloads/ec2-api-tools.zip'
		if $EC2_USER == undef {
			# XXX: Default parameters going to old account, this content will have to be crushed when all migration is done
			$EC2_USER="AKIAI52ERUQP3UL5K2QA"
			$EC2_PASS="nISSY87k44Ea+7Apv43dh6up3WKftt9+nxQzylx/"
		}
		exec{'get-ec2tools':
			command => "/usr/bin/wget $ec2toolsURI",
			cwd => "/tmp",
			creates => "/tmp/ec2-api-tools.zip"
		}
		exec{'unzip-ec2tools':
			command => "/usr/bin/unzip /tmp/ec2-api-tools.zip; mv /opt/ec2-api-tools-* /opt/ec2-api-tools",
			cwd => '/opt',
			creates => '/opt/ec2-api-tools',
			require => [Exec['get-ec2tools'], Package['unzip']],
		}
		file{'/etc/profile.d/ec2tools.sh':
			owner   => 'root',
			group   => 'root',
			content  => template('tdaf_vagrant/ec2tools.sh.erb'),
			require => Exec['unzip-ec2tools'],
		}
		package{'unzip': ensure => present;}
		if ! defined(Package['jdk']) {
			package{'jre': ensure => '1.7.0_25-fcs'}
		}
	}
	else {
		notify {"EC2 tools will not be installed":}
	}
}