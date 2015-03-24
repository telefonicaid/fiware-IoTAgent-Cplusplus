class yum::thirdparty::puppetlabs($repo_server = 'servilinux.hi.inet') inherits yum::params {

  yumrepo { 'puppetlabs-products':
    descr    => "Puppet Labs Products - $majdistrelease",
    enabled  => '1',
    gpgcheck => '0',
    baseurl  => "http://${repo_server}/puppet/yum/el/$majdistrelease/products/\$basearch/",
    notify   => Exec['yum_clean_all'],
  }

  yumrepo { 'puppetlabs-deps':
    descr    => "Puppet Labs Dependencies - $majdistrelease",
    enabled  => '1',
    gpgcheck => '0',
    baseurl  => "http://${repo_server}/puppet/yum/el/$majdistrelease/dependencies/\$basearch/",
    notify   => Exec['yum_clean_all'],
  }

}
