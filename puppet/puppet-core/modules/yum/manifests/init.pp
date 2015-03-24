# Class: yum
#
# Description:
# This class finds the operating and calls the specific subclass's
# to create the repo.
#
# Repos available but not installed:
# yum::rhel::optional
# yum::thirdparty::epel

class yum {

  package {'epel-release': ensure => absent, }

  file {'/etc/yum.repos.d/epel.repo.rpmsave':     ensure => absent, }
  file {'/etc/yum.repos.d/puppetlabs.repo':       ensure => absent, }
  file {'/etc/yum.repos.d/epg.repo':              ensure => absent, }
  file {'/etc/yum.repos.d/epg-puppet.conf':       ensure => absent, }
  file {'/etc/yum.repos.d/epg-puppet.repo':       ensure => absent, }
  file {'/etc/yum.repos.d/CentOS-Base.repo':      ensure => absent, }
  file {'/etc/yum.repos.d/CentOS-Debuginfo.repo': ensure => absent, }
  file {'/etc/yum.repos.d/CentOS-Media.repo':     ensure => absent, }
  file {'/etc/yum.repos.d/CentOS-Vault.repo':     ensure => absent, }
  file {'/etc/yum.repos.d/CentOS-Plus.repo':      ensure => absent, }
  file {'/etc/yum.repos.d/CentOS-Contrib.repo':   ensure => absent, }
  file {'/etc/yum.repos.d/CentOS-Extras.repo':    ensure => absent, }
  file {'/etc/yum.repos.d/Centos-Debuginfo.repo': ensure => absent, }
  file {'/etc/yum.repos.d/Centos-Media.repo':     ensure => absent, }
  file {'/etc/yum.repos.d/Centos-Vault.repo':     ensure => absent, }
  file {'/etc/yum.repos.d/Centos-Plus.repo':      ensure => absent, }
  file {'/etc/yum.repos.d/Centos-Contrib.repo':   ensure => absent, }
  file {'/etc/yum.repos.d/Centos-Extras.repo':    ensure => absent, }
  file {'/etc/yum.repos.d/rhel.repo':          	  ensure => absent, }
  file {'/etc/yum.repos.d/redhat.repo':        	  ensure => absent, }


# Quan canvii el nom a common::yum fer un grep de yum::thirdparty::epel i de yum::rhel::optional a initiatives, per si algu els esta fent servir (i a modules, perque develenv ho fa servir)
  case $::operatingsystem {
    'RedHat': {
      include yum::rhel::base
    }
    'CentOS': {
      include yum::centos::base
      include yum::centos::updates
    }
  }

  include yum::thirdparty::puppetlabs

  exec { 'yum_clean_all':
    command     => '/usr/bin/yum clean all',
    refreshonly => true,
  }

}
