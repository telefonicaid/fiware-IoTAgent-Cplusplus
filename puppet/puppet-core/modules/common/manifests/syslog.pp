class common::syslog {

  package { 'rsyslog':
    ensure => installed,
  } ->
  service { 'syslog':
    enable => false,
    ensure => stopped,
  } ->
  file { '/etc/rsyslog.d':
    ensure => directory,
  } ->
  service { 'rsyslog':
    enable => true,
    ensure => running,
  }
  file { '/etc/logrotate.d/syslog':
    ensure => present,
    owner => 'root',
    group => 'root',
    source => 'puppet:///modules/common/logrotate.d/syslog',
  }

}
