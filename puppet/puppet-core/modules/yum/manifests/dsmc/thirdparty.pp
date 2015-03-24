class yum::dsmc::thirdparty {
  yumrepo { 'dsmc-thirdparty':
    descr    => "DSMC Thirdparty repo",
    enabled  => '1',
    gpgcheck => '0',
    priority => '98',
    baseurl  => 'http://artifacts.hi.inet/dsmc/thirdparty',
    notify   => Exec['yum_clean_all'],
  }
}
