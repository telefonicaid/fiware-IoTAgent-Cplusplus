# Class: yum::rhel
#
# Description:
# This class finds the operating system release and calls the yumrepo class
# to create the repo.
# 
class yum::rhel::base($repo_server = 'repos.hi.inet') {

  yumrepo { 'rhel-base':
    descr    => "Red Hat Enterprise Linux $::operatingsystemrelease - Base",
    enabled  => '1',
    gpgcheck => '0',
    baseurl  => "http://${repo_server}/redhat/rhel${::operatingsystemrelease}s-\$basearch/RPMS.all/",
    notify   => Exec['yum_clean_all'],
  }

}
