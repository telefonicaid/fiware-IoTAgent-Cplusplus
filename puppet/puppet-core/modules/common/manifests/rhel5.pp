# This class is for RHEL5-only stuff
class common::rhel5 {
  if $::osfamily == 'RedHat' {
    if $::operatingsystemmajrelease == '5' {
      #notify { 'ola q ase, RHEL5 o q ase?': }
      # python-simplejson is needed by Ansible, and it's found on rpmforge
      class { 'yum::thirdparty::rpmforge': } ->
      package { 'python-simplejson':
        ensure => present,
      }
    }
  }
}
