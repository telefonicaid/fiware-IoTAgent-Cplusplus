# == Class: mongodb
#
# Manage mongodb installations on RHEL, CentOS, Debian and Ubuntu - either
# installing from the 10Gen repo or from EPEL in the case of EL systems.
#
# === Parameters
#
# enable_10gen (default: false) - Whether or not to set up 10gen software repositories
# init (auto discovered) - override init (sysv or upstart) for Debian derivatives
# location - override apt location configuration for Debian derivatives
# packagename (auto discovered) - override the package name
# servicename (auto discovered) - override the service name
#
# === Examples
#
# To install with defaults from the distribution packages on any system:
#   include mongodb
#
# To install from 10gen on a EL server
#   class { 'mongodb':
#     enable_10gen => true,
#   }
#
# === Authors
#
# Craig Dunn <craig@craigdunn.org>
#
# === Copyright
#
# Copyright 2012 PuppetLabs
#
class tdaf_mongodb (
  $service_enable  = $tdaf_mongodb::params::service_enable,
  $enable_10gen    = false,
  $base_url_repo   = $tdaf_mongodb::params::baseurl,
  $proxy_url_repo  = '',
  $init            = $tdaf_mongodb::params::init,
  $location        = '',
  $packagename     = undef,
  $servicename     = $tdaf_mongodb::params::service,
  $logpath         = '/var/log/mongo/mongod.log',
  $logappend       = true,
  $mongofork       = true,
  $port            = '27017',
  $dbpath          = '/var/lib/mongo',
  $nojournal       = undef,
  $cpu             = undef,
  $noauth          = undef,
  $auth            = undef,
  $verbose         = undef,
  $objcheck        = undef,
  $quota           = undef,
  $oplog           = undef,
  $nohints         = undef,
  $nohttpinterface = undef,
  $noscripting     = undef,
  $notablescan     = undef,
  $noprealloc      = undef,
  $nssize          = undef,
  $mms_token       = undef,
  $mms_name        = undef,
  $mms_name        = undef,
  $mms_interval    = undef,
  $slave           = undef,
  $only            = undef,
  $master          = undef,
  $source          = undef,
  $replSet         = undef,
  $keyFile         = undef, 
  $bind_ip         = undef,
  $shardsvr        = undef,
  $pidfilepath     = undef,
  $configsvr       = undef,
  $oplogSize       = undef,
  $smallfiles      = undef,

  $db_template_nodes = undef,
  $path_mongo_conf = '/etc/mongod.conf', 
) inherits tdaf_mongodb::params {

  if $enable_10gen {
    class {"$tdaf_mongodb::params::source":
      url_repo  => $base_url_repo,
      proxy_url => $proxy_url_repo,
    }
    Class[$tdaf_mongodb::params::source] -> Package['mongodb-10gen']
  }

  if $packagename {
    $package = $packagename
  } elsif $enable_10gen {
    $package = $tdaf_mongodb::params::pkg_10gen
  } else {
    $package = $tdaf_mongodb::params::package
  }

  package { 'mongodb-10gen':
    name   => $package,
    ensure => installed,
  }


  if $dbpath != '/var/lib/mongo' {

    group { 'mongod':
      ensure => 'present',
    }

    notify {"Creado el grupo: mongod":
      require => Group['mongod'],
    }

    user { 'mongod':
      ensure           => 'present',
      comment          => 'mongod',
      home             => '/var/lib/mongo',
      password         => '!!',
      password_max_age => '-1',
      password_min_age => '-1',
      shell            => '/bin/false',
      require => Group['mongod'],

    }

    notify {"Creado el usuario: mongod":
      require => User['mongod'],
    }
    
    file { $dbpath:
      ensure => "directory",
      owner => "mongod",
      group => "mongod", 
      require => User['mongod'], 
    }

    notify {"Creating directory of dbpath: $dbpath":
      require => File[$dbpath],
    }

  }


  # parameter $smallfiles used in mongod.conf.erb template allows 'undef', 'true' or 'false' values
  
  file { $path_mongo_conf:
    content => template('tdaf_mongodb/mongod.conf.erb'),
    owner   => 'root',
    group   => 'root',
    mode    => '0644',
    require => Package['mongodb-10gen'],
  }

#   exec{'Start_Mongo':
#         command => "mongod -f $path_mongo_conf",
#         path => '/bin:/usr/bin', 
#         require => File[$path_mongo_conf],
#         unless => "ps ax | grep mongod | grep -v grep | awk \'{print \$5,\$6,\$7 }\'  | grep  \"mongod -f $path_mongo_conf\"",
#      }



 service { 'mongodb':
   name      => $servicename,
   ensure    => $service_enable,
   enable    => $service_enable,
   subscribe => File['/etc/mongod.conf'],
 }
  
#  firewall{'3mongo':
#    source      => '0/0',
#    destination => '0/0',
#    port        => '27017',
#    action      => 'accept',
#  }

firewall { '3mongod':
    port   => [$port],
    proto  => tcp,
    action => accept,
  }

  
#  exec{'init_replSet':
#        command => 'mongo < /root/mongo_replSet_init.js | grep -q \'"ok" : 1\' && touch /root/rsCreated || mongo < /root/mongo_replSet_init.js | grep -q "need all members up to initiate, not ok"',
#        path => '/bin:/usr/bin',
#        creates => '/root/rsCreated',
#        require => File['/root/mongo_replSet_init.js'],
#      }


#  file{'/root/mongo_replSet_init.js':
#      content => template("$module_name/replSet_init.js.erb"),
#      owner   => 'root',
#      group   => 'root',
#      mode    => '0644',
#      require => Service['mongodb'],
#    }
}
