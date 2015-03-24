class tdaf_yumrepos::epel  inherits tdaf_yumrepos {
  if ! defined(Yumrepo['epel']) {
    yumrepo { 'epel':
      descr      => 'Extra Packages for Enterprise Linux $releasever - $basearch',
      enabled    => '1',
      gpgcheck   => '0',
      baseurl    => 'http://dl.fedoraproject.org/pub/epel/$releasever/$basearch',
      notify     => Exec['clean_yum_cache'],
    }
  }
}
