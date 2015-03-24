class common::users::clouduser(
  $ensure,
) {
  user { 'cloud-user':
    ensure => $ensure,
  }
  common::managesudoer  { 'cloud-user':
    ensure  => $ensure,
    command => 'ALL=(ALL) NOPASSWD:ALL',
  }
}
