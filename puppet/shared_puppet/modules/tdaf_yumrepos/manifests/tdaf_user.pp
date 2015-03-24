class tdaf_yumrepos::tdaf_user (
  $baseurl = 'ftp://tdaf:yr7gjFHX8o@ftp.tid.es/artifacts'
  ) inherits tdaf_yumrepos{
  $user=split($::hostname,'-')
  $USERNAME=$user[2]
  
  yumrepo { 'tdaf_user_arch':
    descr      => 'Extra Packages for tdaf(arch)',
    enabled    => '1',
    gpgcheck   => '0',
    baseurl    => "$baseurl/users/$USERNAME/\$basearch",
    notify     => Exec['clean_yum_cache'],
  }
  yumrepo { 'tdaf_user_noarch':
    descr      => 'Extra Packages for tdaf(noarch)',
    enabled    => '1',
    gpgcheck   => '0',
    baseurl    => "$baseurl/users/$USERNAME/noarch",
    notify     => Exec['clean_yum_cache'],
  }
}
