# Class: nodejs
#
# Parameters:
#
# Actions:
#
# Requires:
#
# Usage:
#
class tdaf_nodejs(
  $dev_package = false,
  $proxy       = ''
) inherits tdaf_nodejs::params {

  case $::operatingsystem {
    'Debian': {
      include 'apt'

      apt::source { 'sid':
        location    => 'http://ftp.us.debian.org/debian/',
        release     => 'sid',
        repos       => 'main',
        pin         => 100,
        include_src => false,
        before      => Anchor['nodejs::repo'],
      }

    }

    'Ubuntu': {
      include 'apt'

      # Only use PPA when necessary.
      if $::lsbdistcodename != 'Precise'{
        apt::ppa { 'ppa:chris-lea/node.js':
          before => Anchor['tdaf_nodejs::repo'],
        }
      }
    }

    'Fedora', 'RedHat', 'CentOS', 'OEL', 'OracleLinux', 'Amazon': {
      package { 'nodejs-stable-release':
        ensure => absent,
        before => Class['tdaf_yumrepos::epel'],
      }

      class{'tdaf_yumrepos::epel':  
        before => Anchor['tdaf_nodejs::repo'],
      }
    }

    default: {
      fail("Class nodejs does not support ${::operatingsystem}")
    }
  }

  # anchor resource provides a consistent dependency for prereq.
  anchor { 'tdaf_nodejs::repo': }

  package { 'nodejs':
    name    => $tdaf_nodejs::params::node_pkg,
    ensure  => present,
    require => Anchor['tdaf_nodejs::repo']
  }
  
  package { 'npm-ansi':
    ensure  => present,
    require => Anchor['tdaf_nodejs::repo']
  }
  
  package { 'npm':
    name    => $tdaf_nodejs::params::npm_pkg,
    ensure  => present,
    require => [Anchor['tdaf_nodejs::repo'], Package['npm-ansi']]
  }

  if $proxy {
    exec { 'npm_proxy':
      command => "npm config set proxy ${proxy}",
      path    => $::path,
      require => Package['npm'],
    }
  }

  if $dev_package and $tdaf_nodejs::params::dev_pkg {
    package { 'nodejs-dev':
      name    => $tdaf_nodejs::params::dev_pkg,
      ensure  => present,
      require => Anchor['tdaf_nodejs::repo']
    }
  }

}
