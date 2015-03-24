class yum::thirdparty::puppetlabs inherits yum::params {
  
  if ($::fqdn =~ /openstacklocal/ ) {
  # Openstack instances need to set a proxy for internal repos
    yumrepo { 'puppetlabs-products':
      descr    => "Puppet Labs Products - $majdistrelease",
      enabled  => '1',
      gpgcheck => '0',
      priority => '1',
      baseurl  => "http://artifactory.aislada.hi.inet/artifactory/yum-puppetlabs/$majdistrelease/products/\$basearch",
      proxy    => 'http://prod-epg-ost-proxy-01.hi.inet:6666',
      notify   => Exec['yum_clean_all'],
    }

    yumrepo { 'puppetlabs-deps':
      descr    => "Puppet Labs Dependencies - $majdistrelease",
      enabled  => '1',
      gpgcheck => '0',
      baseurl  => "http://artifactory.aislada.hi.inet/artifactory/yum-puppetlabs/$majdistrelease/dependencies/\$basearch",
      proxy    => 'http://prod-epg-ost-proxy-01.hi.inet:6666',
      notify   => Exec['yum_clean_all'],
    }

  } else {
    yumrepo { 'puppetlabs-products':
      descr    => "Puppet Labs Products - $majdistrelease",
      enabled  => '1',
      gpgcheck => '0',
      priority => '1',
      baseurl  => "http://artifactory.aislada.hi.inet/artifactory/yum-puppetlabs/$majdistrelease/products/\$basearch",
      notify   => Exec['yum_clean_all'],
    }

    yumrepo { 'puppetlabs-deps':
      descr    => "Puppet Labs Dependencies - $majdistrelease",
      enabled  => '1',
      gpgcheck => '0',
      baseurl  => "http://artifactory.aislada.hi.inet/artifactory/yum-puppetlabs/$majdistrelease/dependencies/\$basearch",
      notify   => Exec['yum_clean_all'],
    }

  }
}
