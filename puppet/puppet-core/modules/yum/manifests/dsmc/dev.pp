class yum::dsmc::dev {
  yumrepo { 'dsmc-dev':
    descr    => "DSMC DEV repo",
    enabled  => '1',
    gpgcheck => '0',
    priority => '99',
    baseurl  => 'http://artifacts.hi.inet/dsmc/dev',
    notify   => Exec['yum_clean_all'],
  }
}
