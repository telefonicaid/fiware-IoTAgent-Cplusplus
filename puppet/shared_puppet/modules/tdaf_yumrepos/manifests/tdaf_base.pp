class tdaf_yumrepos::tdaf_base (
  $baseurl = 'ftp://tdaf:yr7gjFHX8o@ftp.tid.es/artifacts',
  $project = undef,
  ) inherits tdaf_yumrepos {
  
  if ($project) {
    $baseurl = "$baseurl/$project"
  }

  yumrepo { 'tdaf_base_arch':
    descr      => 'TDAF Packages (arch)',
    enabled    => '1',
    gpgcheck   => '0',
    baseurl    => "$baseurl/\$basearch",
    notify     => Exec['clean_yum_cache'],
  }
  yumrepo { 'tdaf_base_noarch':
    descr      => 'TDAF Packages (noarch)',
    enabled    => '1',
    gpgcheck   => '0',
    baseurl    => "$baseurl/noarch",
    notify     => Exec['clean_yum_cache'],
  }
}
