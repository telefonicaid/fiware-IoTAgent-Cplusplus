class yum::thirdparty::java($repo_server = 'repos.hi.inet', $java_majrelease = '7') inherits yum::params {

  if ($::fqdn =~ /openstacklocal/ ) {
  # Openstack instances need to set a proxy for internal repos
    yumrepo { 'java':
      descr    => "Oracle Java",
      enabled  => '1',
      gpgcheck => '0',
      baseurl  => "http://${repo_server}/redhat/java-\$basearch/RPMS.${java_majrelease}/",
      proxy    => 'http://prod-epg-ost-proxy-01.hi.inet:6666',
      notify   => Exec['yum_clean_all'],
    }
  } else {
    yumrepo { 'java':
      descr    => "Oracle Java",
      enabled  => '1',
      gpgcheck => '0',
      baseurl  => "http://${repo_server}/redhat/java-\$basearch/RPMS.${java_majrelease}/",
      notify   => Exec['yum_clean_all'],
    }
  }
}
