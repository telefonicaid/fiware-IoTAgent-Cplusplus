# This class fixes the CVE-2014-0160 bug, also known as OpenSSL Heartbleed and the latest CVE-2014-0224 bug.
class security::bugfixes::openssl {
  if $::osfamily == 'RedHat' {
    if $::operatingsystemrelease == '6.5' {
      # notify { 'ola q ase, parcheando heartbleed o q ase?': }
      # Cyclic dependency. You can re-install it later on
      package { 'openssl-devel':
        ensure => absent,
      } ->
      package { 'openssl':
        ensure   => latest,
      }
    }
  }
}
