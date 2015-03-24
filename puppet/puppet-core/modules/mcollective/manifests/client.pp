# = Class: mcollective::client
#
# This class installs/configures/manages an Mcollective Client.
#
# == Parameters:
#
# $stomphost:         hostname of the stomp server (ActiveMQ). Defaults to localhost for the client (mcollective.aislada)
# $stomppassword::    ActiveMQ password
# $psk:               Shared PSK secret key. Defaults to a secure enough one.
#
# == Requires:
#
# Nothing.
#
# == Sample Usage:
#
#  class { 'mcollective::client':}
#
#  class { 'mcollective::client':
#    psk => 'secret',
#  }
#
#
# == Author:
# Xavi Carrillo <epgbnc3@tid.es>
#

class mcollective::client( 
  $ensure        = 'running',
  $enable        = true,
  $stomphost     = 'localhost',
  $stomppassword = $::mcollective::params::stomppassword,
  $psk           = $::mcollective::params::psk, 
) inherits mcollective::params {

  #notify {"ola q ase $ensure":}

  service { 'activemq':
    ensure => "${ensure}",
    enable => "${enable}",
  }

  package { [
    'ruby',
    'rubygem-stomp',
    'activemq',
    'mcollective-client',]:
     ensure => present,
  }
  # This is to avoid duplicate resources, since it is needed by the server as well
  realize (Package['rubygems'])

  file { '/etc/mcollective/client.cfg':
    ensure  => present,
    owner   => 'root',
    group   => 'root',
    content => template('mcollective/mcollective_config.erb'),
  }
  file { '/etc/activemq/activemq.xml':
    ensure  => present,
    owner   => 'root',
    group   => 'root',
    content => template('mcollective/activemq.xml.erb'),
    require => Package['activemq'],
    notify  => Service['activemq'],
  }
}
