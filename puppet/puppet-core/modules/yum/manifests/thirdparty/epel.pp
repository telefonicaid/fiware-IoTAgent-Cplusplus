class yum::thirdparty::epel($repo_server = 'repos.hi.inet') inherits yum::params {

  if ($::fqdn =~ /openstacklocal/ ) {
  # Openstack instances need to set a proxy for internal repos
    yumrepo { 'epel':
      descr    => "Extra Packages for Enterprise Linux $majdistrelease",
      enabled  => '1',
      gpgcheck => '0',
      baseurl  => "http://${repo_server}/redhat/epel${::operatingsystemmajrelease}-\$basearch/RPMS.all/",
      proxy      => 'http://prod-epg-ost-proxy-01.hi.inet:6666',
      notify   => Exec['yum_clean_all'],
    }
  } else {
    yumrepo { 'epel':
      descr    => "Extra Packages for Enterprise Linux $majdistrelease",
      enabled  => '1',
      gpgcheck => '0',
      baseurl  => "http://${repo_server}/redhat/epel${::operatingsystemmajrelease}-\$basearch/RPMS.all/",
      notify   => Exec['yum_clean_all'],
    }
  }
}
