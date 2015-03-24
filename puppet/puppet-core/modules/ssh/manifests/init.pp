class ssh(
  $permitrootlogin = 'no',
) {
  case $::osfamily{
    'Debian':  {
      $ssh_packages = [ 'openssh-client', 'openssh-server' ]
      $service_name = 'ssh'
    }
    'RedHat':  { 
      $ssh_packages = [ 'openssh', 'openssh-server' ]
      $service_name = 'sshd'
    }
    default: {
      fail("Unsupported platform: ${::operatingsystem}")
    }
  }
  package { $ssh_packages: ensure => 'latest' }
  service { $service_name: ensure => 'running' }

  $banner = ""  

  file {'/etc/banner':
    content => $banner,
  }
  augeas { "sshd_config":
    changes => [
      "set /files/etc/ssh/sshd_config/PermitRootLogin ${permitrootlogin}",
      "set /files/etc/ssh/sshd_config/Banner /etc/banner",
    ],
    notify => Service[$service_name],
  }
}
