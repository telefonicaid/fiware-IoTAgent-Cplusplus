class yum::thirdparty::rpmforge {
  yumrepo { 'rpmforge':
    descr    => "RHEL \$releasever - RPMforge.net - dag",
    enabled  => '1',
    gpgcheck => '0',
    baseurl  => "http://apt.sw.be/redhat/el${::operatingsystemmajrelease}/en/\$basearch/rpmforge",
    notify   => Exec['yum_clean_all'],
  } 
}
