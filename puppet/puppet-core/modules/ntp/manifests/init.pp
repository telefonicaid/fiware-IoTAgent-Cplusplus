
# 10.95.67.227 = ntp-server.hi.inet but from the servers network the DNS resolution doesn't work so...
class ntp ($ntpserver = '10.95.67.227',) {
  case $::osfamily{
    'Debian':  {
      $service_name = 'ntp'
    }
    'RedHat':  {
      $service_name = 'ntpd'
    }
    default: {
      fail("Unsupported platform: ${::operatingsystem}")
    }
  }

  $packages = 'ntp'
  package { $packages: ensure => 'latest' }

  service { $service_name:
    ensure    => 'running',
    enable    => true,
    hasstatus => true,
  }

  file { '/etc/ntp.conf':
    mode    => '0600',
    content => template('ntp/ntp.conf.erb'),
    notify  => Service[$service_name]
  }
}

