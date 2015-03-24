# This class fixes the CVE-2014-6271 and CVE-2014-7169 bug, also known as Shellshock.
class security::bugfixes::bash {
  if $::osfamily == 'RedHat' {
    package { 'bash':
      ensure   => latest,
    }
  }
}
