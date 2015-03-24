class common::users::sysadmin(
  $ensure,
) {
  user { 'sysadmin':
    ensure => $ensure,
    home       => '/home/sysadmin',
    managehome => true,
    shell      => '/bin/bash',
  }
  common::managesudoer  { 'sysadmin':
    ensure  => $ensure,
    command => 'ALL=(ALL) NOPASSWD:ALL',
  }
}
