# == Class: monit::unmonitor
#
# This module removes a service monitor from Monit
#
# === Examples
#
#  monit::unmonitor { 'monit-watch-monit': }
#
# === Authors
#
# Jason Corley <jason.corley@gmail.com>
#
define monit::unmonitor {
  include monit::params

  file { "${monit::params::conf_dir}/${name}.conf":
    ensure  => absent,
    notify  => Service[$monit::params::monit_service],
    require => Package[$monit::params::monit_package],
  }
}
