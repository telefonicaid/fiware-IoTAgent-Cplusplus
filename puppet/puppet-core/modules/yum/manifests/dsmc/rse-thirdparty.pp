class yum::dsmc::rse-thirdparty {
  yumrepo { 'rse-thirdparty':
    descr    => "RSE Thirdparty repo for enablers",
    enabled  => '1',
    gpgcheck => '0',
    priority => '9',
    baseurl  => 'http://artifacts.hi.inet/enablers/thirdparty',
    notify   => Exec['yum_clean_all'],
  }
}
