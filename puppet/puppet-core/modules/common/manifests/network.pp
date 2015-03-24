class common::network {
  service { 'network':
    ensure => running,
    enable => true,
  }
  augeas { "network":
    notify  => Service['network'],
    context => "/files/etc/sysconfig/network",
    changes => [
      'set DHCP_HOSTNAME "$HOSTNAME"',
    ],
  }
}
