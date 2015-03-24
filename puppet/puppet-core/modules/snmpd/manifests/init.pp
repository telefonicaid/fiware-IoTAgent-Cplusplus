
class snmpd ($initiative) {
  case $::osfamily{
    'Debian':  {
      $packages = 'snmpd'
    }
    'RedHat':  {
      $packages = 'net-snmp'
    }
    default: {
      fail("Unsupported platform: ${::operatingsystem}")
    }
  } 

  package { $packages: ensure => 'latest' }
  service { 'snmpd': ensure => 'running' }

  file { '/etc/snmp/snmpd.conf':
    ensure => present,
    owner  => 'root',
    group  => 'root',
    mode   => '0644',
    notify => Service['snmpd'],
    source => [ 
      "puppet:///modules/snmpd/$initiative/$::fqdn.snmpd.conf",
      "puppet:///modules/snmpd/$initiative/snmpd.conf",
      "puppet:///modules/snmpd/$::fqdn.snmpd.conf",
      "puppet:///modules/snmpd/default/snmpd.conf",
    ],
  }
}
