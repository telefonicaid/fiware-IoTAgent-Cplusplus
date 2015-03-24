class tdaf_base::packages {
  package {
    'mlocate':      ensure => present;
    'telnet':       ensure => present;
    'bind-utils':   ensure => present;
    'binutils':     ensure => present;
#    'corkscrew':    ensure => present;
  }	
}

