class tdaf_yumrepos {
  exec{
    'clean_yum_cache':
      command     => '/usr/bin/yum clean expire-cache',
      refreshonly => true,
  }   
}