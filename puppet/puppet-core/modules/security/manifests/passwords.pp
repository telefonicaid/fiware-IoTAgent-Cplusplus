class security::passwords {
  # Control the password policy
  file { "/etc/pam.d/system-auth":
    owner  => 'root',
    group  => 'root',
    source => 'puppet:///modules/common/system-auth',
  }
}
