class tdaf_vagrant::packages {
 # Needed packages to guest additions installation  
  package {
    'kernel':           ensure => latest;
    'kernel-headers':   ensure => latest;
    'kernel-devel':     ensure => latest;
    'sudo':             ensure => installed;
    'perl':             ensure => installed;
    'dkms':             ensure => installed;
    'openssh-clients':  ensure => installed;
  }	
}