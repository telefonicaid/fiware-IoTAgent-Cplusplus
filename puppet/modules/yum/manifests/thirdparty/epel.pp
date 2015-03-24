class yum::thirdparty::epel($repo_server = 'repos.hi.inet') inherits yum::params {

  yumrepo { 'epel':
    descr    => "Extra Packages for Enterprise Linux $majdistrelease",
    enabled  => '1',
    gpgcheck => '0',
    baseurl  => "http://${repo_server}/redhat/epel${::operatingsystemmajrelease}-\$basearch/RPMS.all/",
    notify   => Exec['yum_clean_all'],
  }

}
