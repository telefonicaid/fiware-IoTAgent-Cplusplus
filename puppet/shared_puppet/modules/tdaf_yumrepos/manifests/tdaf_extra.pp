class tdaf_yumrepos::tdaf_extra (
  $baseurl = 'ftp://tdaf:yr7gjFHX8o@ftp.tid.es/artifacts'
  ) inherits tdaf_yumrepos {
  yumrepo { 'tdaf_extra_arch':
    descr      => 'Extra Packages for tdaf(arch)',
    enabled    => '1',
    gpgcheck   => '0',
    baseurl    => "$baseurl/extra/\$basearch",
    notify     => Exec['clean_yum_cache'],
  }
  yumrepo { 'tdaf_extra_noarch':
    descr      => 'Extra Packages for tdaf(noarch)',
    enabled    => '1',
    gpgcheck   => '0',
    baseurl    => "$baseurl/extra/noarch",
    notify     => Exec['clean_yum_cache'],
   }
}
