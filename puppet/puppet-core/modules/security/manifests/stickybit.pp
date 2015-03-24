class security::stickybit {
  # Make sure /tmp has the sticky bit enabled
  file { '/tmp':
    ensure => directory,
    mode   => '1777',
  }
}
