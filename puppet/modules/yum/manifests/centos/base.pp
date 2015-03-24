# Class: yum::centos::base
#
# Description:
# This class finds the operating release and calls the yumrepo class
# to create the repo.
#
class yum::centos::base ($repo_server = 'repos.hi.inet') {

  yumrepo { 'Centos-Base':
    descr      => "Centos Base ${::operatingsystemrelease}",
    enabled    => '1',
    gpgcheck   => '0',
    baseurl    => "http://${repo_server}/redhat/centos${::operatingsystemrelease}-\$basearch/RPMS.all/",
    notify     => Exec['yum_clean_all'],
  }

}
