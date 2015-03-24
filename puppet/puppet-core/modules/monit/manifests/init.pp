# == Class: monit
#
# This module controls Monit
#
# === Parameters
#
# [*ensure*]    - If you want the service running or not
# [*admin*]     - Admin email address
# [*interval*]  - How frequently the check runs
# [*delay*]     - How long to wait before actually performing any action
# [*logfile*]   - What file for monit use for logging
# [*mailserver] - Which mailserver to use
# [*collector]  - mmonit server url
# [*httpd_port*]      - Configures the httpd port to be used, by default 2812,
# [*enable_ssl*]      - Enable ssl, false by default
# [*pemfile*]         - Path to the pemfile
# [*allow_self_cert*] - Allow self certification, by default set to false
# [*only_localhost*]  - Allow connections from localhost only, by default true
# [*allows*]          - Allowed connections, by default 'localhost'. Use an
#                     - array for multiple connections.
# === Examples
#
#  class { 'monit':
#    admin    => 'me@mydomain.local',
#    interval => 30,
#  }
#
# === Authors
#
# Eivind Uggedal <eivind@uggedal.com>
# Jonathan Thurman <jthurman@newrelic.com>
#
# === Copyright
#
# Copyright 2011 Eivind Uggedal <eivind@uggedal.com>
#
class monit (
  $ensure     = present,
  $admin      = undef,
  $interval   = 60,
  $delay      = undef,
  $idfile     = 'UNSET',
  $logfile    = 'UNSET',
  $mailserver = 'localhost',
  $mailformat = undef,
  $collector  = undef,
  $httpd_port = '2812',
  $enable_ssl = false,
  $pemfile = undef,
  $allow_self_cert = false,
  $only_localhost = true,
  $allows = 'localhost',
) {
  include monit::params

  $idfile_real = $idfile ? {
    'UNSET' => $monit::params::idfile,
    default => $idfile
  }

  $logfile_real = $logfile ? {
    'UNSET' => $monit::params::logfile,
    default => $logfile
  }

  if ($delay == undef) {
    $use_delay = $interval * 2
  }
  else {
    $use_delay = $delay
  }

  $conf_include = "${monit::params::conf_dir}/*.conf"

  if ($ensure == 'present') {
    $run_service = true
    $service_state = 'running'
  } else {
    $run_service = false
    $service_state = 'stopped'
  }

  package { $monit::params::monit_package:
    ensure => $ensure,
  }

  # Template uses: $admin, $conf_include, $interval, $logfile_real, $idfile
  file { $monit::params::conf_file:
    ensure  => $ensure,
    content => template('monit/monitrc.erb'),
    mode    => '0600',
    require => Package[$monit::params::monit_package],
    notify  => Service[$monit::params::monit_service],
  }

  file { $monit::params::id_dir:
    ensure => directory,
    owner  => 'root',
    group  => 'root',
    mode   => '0755',
    before => Service[$monit::params::monit_service]
  }

  file { $monit::params::conf_dir:
    ensure  => directory,
    owner   => 'root',
    group   => 'root',
    mode    => '0755',
    require => Package[$monit::params::monit_package],
  }

  # Not all platforms need this
  if ($monit::params::default_conf) {
    if ($monit::params::default_conf_tpl) {
      file { $monit::params::default_conf:
        ensure  => $ensure,
        content => template("monit/${monit::params::default_conf_tpl}"),
        require => Package[$monit::params::monit_package],
      }
    } else { fail('You need to provide config template')}
  }

  if ($logfile_real =~ /syslog/) {
    service { $monit::params::monit_service:
      ensure     => $service_state,
      enable     => $run_service,
      hasrestart => true,
      hasstatus  => $monit::params::service_has_status,
      subscribe  => File[$monit::params::conf_file],
      require    => File[$monit::params::conf_file],
    }
  } else {
    # Template uses: $logfile_real
    file { $monit::params::logrotate_script:
      ensure  => $ensure,
      content => template("monit/${monit::params::logrotate_source}"),
      require => Package[$monit::params::monit_package],
    }
    service { $monit::params::monit_service:
      ensure     => $service_state,
      enable     => $run_service,
      hasrestart => true,
      hasstatus  => $monit::params::service_has_status,
      subscribe  => File[$monit::params::conf_file],
      require    => [
        File[$monit::params::conf_file],
        File[$monit::params::logrotate_script]
      ],
    }
  }
}
