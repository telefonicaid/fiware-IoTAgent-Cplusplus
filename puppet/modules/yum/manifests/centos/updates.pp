# Class: yum::centos:updates
#
# Description:
# This class finds the operating release and calls the yumrepo class
# to create the repo.
#
class yum::centos::updates ($repo_server = 'repos.hi.inet') inherits yum::params {

  yumrepo { 'Centos-Updates':
    descr      => 'Centos Updates',
    enabled    => '1',
    gpgcheck   => '0',
    baseurl    => "http://${repo_server}/redhat/centos${majdistrelease}-\$basearch/RPMS.all",
    notify     => Exec['yum_clean_all'],
  }

}
