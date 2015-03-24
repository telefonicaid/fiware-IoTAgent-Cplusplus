# == Class: stunnel
#
# This module sets up SSL encrypted and authenticated tunnels using the
# common application stunnel.
#
# === Parameters
#
# [*package*]
#   The package name that represents the stunnel application on your
#   distribution.  By default we look this value up in a stunnel::data class,
#   which has a list of common answers.
#
# [*service*]
#   The service name that represents the stunnel application on your
#   distribution.  By default we look this value up in a stunnel::data class,
#   which has a list of common answers.
#
# [*conf_dir*]
#   The default base configuration directory for your version on stunnel.
#   By default we look this value up in a stunnel::data class, which has a
#   list of common answers.
#
# === Examples
#
# include stunnel
#
# === Authors
#
# Cody Herriges <cody@puppetlabs.com>
# Xavier Morales <xmorales.bcn@gmail.com>
#
# === Copyright
#
# Copyright 2012 Puppet Labs, LLC
#
class fxmp_stunnel(
  $package  = $fxmp_stunnel::data::package,
  $service  = $fxmp_stunnel::data::service,
  $conf_dir = $fxmp_stunnel::data::conf_dir
) inherits fxmp_stunnel::data {

  package { $package:
    ensure => present,
  }

  file { $conf_dir:
    ensure  => directory,
    require => Package[$package],
    purge   => true,
    recurse => true,
  }

  file { "/etc/init.d/$service":
    ensure  => file,
    content => template("${module_name}/stunnel.erb"),
    mode    => '0755',
    owner   => '0',
    group   => '0',
    require => Package[$package],
  }

  service { $service:
    ensure     => running,
    enable     => true,
    hasrestart => true,
    hasstatus  => true,
    require    => File["/etc/init.d/$service"],
  }
}
