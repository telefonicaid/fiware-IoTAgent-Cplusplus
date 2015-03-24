#
# To configure a puppet agent:
#   class { 'puppet':
#     puppet_server => 'puppet.aislada',
#   }
# To configure a puppet agent on a masterless setup:
#   class { 'puppet':
#     puppet_server => false,
#   }
#
# To configure a Puppet Master with:
#   - Unicorn as the Rails Server
#   - Nginx as the Web Frontend
#   - PuppetDB + PostgreSQL as the Storeconfigs backend
#   - Foreman + SQLite for the reports GUI
#
#   class { 'puppet':
#     puppet_server   => 'puppet.aislada',
#     master          => true,
#     foreman         => true,
#     unicorn         => true,
#     unicorn_workers => '8',
#   }
#
# This module heavily depends on these:
# puppetlabs-concat
# puppetlabs-postgresql
# Vaidas Jablonskis' puppetdb + Xavi Carrillo's modifications


class puppet (
  $version                  = '3.7.3-1',
  $puppetdb_version         = undef,
  $puppetdb_jvm_heap_size   = '512M',
  $puppetlog                = '/var/log/puppet/puppet.log',
  $puppet_agent_ensure      = 'running',
  $puppet_agent_enable      = true,
  $puppet_agent_runinterval = '3600',
  $master                   = false,
  $unicorn                  = false,
  $unicorn_workers          = undef,
  $unicorn_workers_timeout  = '120',
  $foreman                  = false,
  $puppet_server,
) {

  include yum
  include yum::thirdparty::puppetlabs

  case $::osfamily {
    RedHat: {
      case $::operatingsystemrelease {
        /^5/: { $puppet_version = "${version}.el5" }
        /^6/: { $puppet_version = "${version}.el6" }
      }
    }
  }

  package { 'puppet': ensure => "${puppet_version}" }
  package { 'facter': ensure => latest }
  package { 'hiera':  ensure => latest }

  file { '/etc/puppet/hiera.yaml':
    ensure => present,
    owner  => 'puppet',
    group  => 'puppet',
    source => "puppet:///modules/puppet/puppet/hiera.yaml",
  }

  file { '/etc/puppet/puppet.conf':
    ensure  => present,
    owner   => 'root',
    group   => 'puppet',
    mode    => '0640',
    content => template('puppet/puppet.conf.erb'),
    notify  => Service['puppet'],
  }

  file { '/etc/sysconfig/puppet':
    ensure  => present,
    content => template('puppet/sysconfig_puppet.erb'),
    notify  => Service['puppet'],
  }

  file { '/etc/logrotate.d/puppet-agent':
    ensure => present,
    source => 'puppet:///modules/puppet/logrotate.d/puppet-agent',
  }

  service { 'puppet':
    ensure => "${puppet_agent_ensure}",
    enable => "${puppet_agent_enable}",
  }
  
  if $master {
    # The node will be a Puppet Master as well
    class { 'puppet::master':
      puppet_version          => $puppet_version, 
      unicorn                 => $unicorn,
      unicorn_workers         => $unicorn_workers,
      unicorn_workers_timeout => $unicorn_workers_timeout,
    }
    class { 'puppetdb':
      version        => $puppetdb_version,
      ssl_host       => $puppet_server,
      jvm_heap_size  => $puppetdb_jvm_heap_size,
      psql_host      => 'localhost',
      psql_username  => 'puppetdb',
      psql_password  => 'puppetdb',
      db_subprotocol => 'postgresql',
      node_ttl       => '5d',
      node_purge_ttl => '10d',
    }
  }
  if $foreman { 
    # We have Foreman in a different host:
    class { 'foreman::puppetmaster':
      foreman_url => 'http://foreman.aislada.hi.inet:3000',
      enc         => true, # Even if we don't use it as an ENC, it is needed in order to push facts to foreman via node.rb
    } # To create the report script:  /usr/lib/ruby/site_ruby/1.8/puppet/reports/foreman.rb

    # If you want to install Foreman, add this on the node definition:
    #class { 'foreman':
    #  version   => '1.3.2-1.el6',
    #  repo      => 'releases/1.3',
    #  db_type   => 'sqlite',
    #  passenger => false,
    #  ssl       => false,
    #}
  }
}

