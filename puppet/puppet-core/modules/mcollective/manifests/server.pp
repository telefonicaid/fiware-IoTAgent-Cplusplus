# = Class: mcollective::server
# 
# This class installs/configures/manages an Mcollective Server.
# It can optionally disable mcollective
#
# The server is installed on all the machines. And controlled by the client (on mcollective-01)
# 
# == Parameters: 
#
# $enable::                  Whether to start the Mcollective service on boot.
#                            Valid values: true and false. Defaults to true
# $ensure::                  Whether to run the Mcollective service.
#                            Valid values: running and stopped. Defaults to running
# $stomphost:                hostname of the stomp server (ActiveMQ). Defaults to localhost for the client (mcollective.aislada)
# $stomppassword::           ActiveMQ password 
# $psk:		                   Shared PSK secret key. Defaults to a secure enough one.
#
# == Requires: 
# 
# Nothing.
# 
# == Sample Usage:
#
#  class { 'mcollective::server':}
#
#  class { 'mcollective::server':
#    psk => 'secret',
#  }
#
#  class { 'mcollective::server':
#    stomphost => 'activemq.hi.inet',
#  }
#
# == Author:
# Xavi Carrillo <epgbnc3@tid.es>
#

class mcollective::server(
  $ensure        = running,
  $enable        = true,
  $package       = 'installed',
  $stomphost     = 'prod-epg-mco-01.hi.inet',
  $stomppassword = $::mcollective::params::stomppassword,
  $psk           = $::mcollective::params::psk,
) inherits mcollective::params {

    # This is to avoid duplicate resources. Anyway the client will be a server too so we don't need a common class
    #@package { 'rubygems': ensure => latest }
    #realize (Package['rubygems'])
    package { [
      #'rubygems',
      'mcollective',
      'mcollective-common',] :
       ensure => $package,
    }

    # stomp gem
    package { 'stomp':
      provider => 'gem',
      ensure   => $package,
    }

    # Plugins:
    package { [
      'mcollective-facter-facts','mcollective-puppet-agent','mcollective-puppet-client',
      'mcollective-package-agent','mcollective-package-client',
      'mcollective-service-agent','mcollective-service-client',] :
       ensure => $package,
    }


    file { '/etc/mcollective/server.cfg':
      ensure  => present,
      owner   => 'root',
      group   => 'root',
      content => template('mcollective/mcollective_config.erb'),
      notify  => Service['mcollective'],
    }

    service { 'mcollective':
      ensure     => $ensure,
      enable     => $enable,
      hasstatus  => true,
      hasrestart => true,
      require    => File['/etc/mcollective/server.cfg'],
    }

    Package[mcollective-facter-facts]-> Service[mcollective]

}
