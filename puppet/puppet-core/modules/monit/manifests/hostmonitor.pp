# == Class: monit::hostmonitor
#
# This module configures a host to be monitored by Monit
#
# [*address*]         - IP address of the host 
# [*checks*]          - Array of monit check statements
#
# === Parameters
#
# === Authors
#
# Mauro Morales <contact@mauromorales.com>
#
define monit::hostmonitor (
  $address = undef,
  $checks  = [],
) {
  include monit::params
  if ($address == undef) {
    fail("Missing ip address for the host ${name}.")
  }
  if ($checks == []) {
    fail("Missing checks for the host ${name}.")
  }
  file { "${monit::params::conf_dir}/${name}.conf":
    ensure  => $ensure,
    content => template('monit/host.conf.erb'),
    notify  => Service[$monit::params::monit_service],
    require => Package[$monit::params::monit_package],
  }
}
