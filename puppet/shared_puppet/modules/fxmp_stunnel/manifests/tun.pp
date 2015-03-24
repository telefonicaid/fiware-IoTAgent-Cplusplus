# == Define: stunnel::tun
#
# Creates a tunnel config to be started by the stunnel application on startup.
#
# === Parameters
#
# [*namevar*]
#   The namevar in this type is the title you give it when you define a resource
#   instance.  It is used for a handful of purposes; defining the name of the
#   config file and the tunnel section in the config, as well as things like
#   the PID file.
#
# [*certificate*]
#   Signed SSL certificate to be used during authentication and encryption.
#   This module is meant to work in conjuction with an already established
#   Puppet infrastructure so we are defaulting to the default location of the
#   agent certificate on Puppet Enterprise.
#
# [*private_key*]
#   In order to encrypt and decrypt things there needs to be a private_key
#   someplace among the system.  Just like certificate we use data from Puppet
#   Enterprise.
#
# [*pid_file*]
#   Where the process ID of the running tunnel is saved.  This values needs to
#   be relative to your chroot directory.
#
# [*debug_level*]
#   The debug leve of your defined tunnels that is sent to the log.
#
# [*log_dest*]
#   The file that log messages are delivered to.
#
# [*client*]
#   If we running our tunnel in client mode.  There is a difference in stunnel
#   between initiating connections or listening for them.
#
# [*accept*]
#   For which host and on which port to accept connection from.
#
# [*connect*]
#  What port or host and port to connect to.
#
# [*conf_dir*]
#   The default base configuration directory for your version on stunnel.
#   By default we look this value up in a stunnel::data class, which has a
#   list of common answers.
#
# === Examples
#
#   stunnel::tun { 'rsyncd':
#     certificate => "/etc/puppet/ssl/certs/${::clientcert}.pem",
#     private_key => "/etc/puppet/ssl/private_keys/${::clientcert}.pem",
#     client      => false,
#     accept      => '1873',
#     connect     => '873',
#   }
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
define fxmp_stunnel::tun(
    $certificate,
    $private_key,
    $pid_file    = "/var/run/stunnel.${name}.pid",
    $debug_level = '0',
    $log_dest    = "/var/log/stunnel.${name}.log",
    $client      = 'false',
    $accept,
    $connect,
    $conf_dir    = $fxmp_stunnel::conf_dir
) {

  $client_on = $client ? {
    true  => 'yes',
    false => 'no',
  }

  file { "${conf_dir}/stunnel.${name}.conf":
    ensure  => file,
    content => template("${module_name}/stunnel.conf.erb"),
    mode    => '0644',
    owner   => '0',
    group   => '0',
    require => File[$conf_dir],
    notify  => Service["$fxmp_stunnel::service"]
  }
}
