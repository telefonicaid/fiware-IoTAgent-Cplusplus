class tdaf_yumrepos::pdi-epel  inherits tdaf_yumrepos {
  if ! defined(Yumrepo['epel']) {
    yumrepo { 'epel':
      descr => 'Extra Packages for Enterprise Linux $releasever - $basearch',
      baseurl => 'http://repos.hi.inet/redhat/epel6-x86_64/RPMS.all',
      gpgcheck => 0,
      enabled => 1,
      notify     => Exec['clean_yum_cache'],
    }
  }
}
