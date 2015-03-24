# == Class: monit::systemmonitor
#
# This module configures a system to be monitored by Monit
#
# === Parameters
#
# [*host*]       - The name of the system to monitor (default: FQDN)
# [*load1*]      - Numeric threshold for the 1 minute load average
# [*load5*]      - Numeric threshold for the 5 minute load average
# [*memory*]     - Percentage threshold for used memory
# [*cpu_user*]   - Percentage threshold for cpu(user)
# [*cpu_system*] - Percentage threshold for cpu(system)
# [*cpu_wait*]   - Percentage threshold for used cpu(wait)
# [*swap*]       - Percentage threshold for used swap
# [*action*]     - The action to take if check is met (default: alert)
# [*ensure*]     - If the file should be enforced or not (default: present)
#
# === Examples
#
#  monit::systemmonitor { $::fqdn:
#    swap  => '75',
#  }
#
# === Authors
#
# Jason Corley <jason.corley@gmail.com>
#
define monit::systemmonitor (
  $host       = $::fqdn,
  $load1      = undef,
  $load5      = undef,
  $memory     = undef,
  $cpu_user   = undef,
  $cpu_system = undef,
  $cpu_wait   = undef,
  $swap       = undef,
  $action     = 'alert',
  $ensure     = present
) {
  include monit::params

  # Template uses: $host, $load1, $load5, $memory, $swap,
  #                $cpu_user, $cpu_system, $cpu_wait, $action
  file { "${monit::params::conf_dir}/${name}.conf":
    ensure  => $ensure,
    content => template('monit/system.conf.erb'),
    notify  => Service[$monit::params::monit_service],
    require => Package[$monit::params::monit_package],
  }
}
