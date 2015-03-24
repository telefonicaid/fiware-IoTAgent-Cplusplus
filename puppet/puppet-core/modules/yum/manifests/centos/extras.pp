# Class: yum::centos:extras
#
# Description:
# This class finds the operating release and calls the yumrepo class
# to create the repo.
#
class yum::centos::extras ($repo_server = 'artifactory.aislada.hi.inet') inherits yum::params {

  if ( $::operatingsystemrelease =~ /6.6|5.11/ )
  {
    $repo = 'yum-centos'
  } else {
    $repo = 'yum-centosvault'
  }
  
  if ($::fqdn =~ /openstacklocal/ ) {
  # Openstack instances need to set a proxy for internal repos
    yumrepo { 'CentOS-Extras':
      descr      => 'Centos Extras',
      enabled    => '1',
      gpgcheck   => '0',
      baseurl    => "http://${repo_server}/artifactory/${repo}/${::operatingsystemrelease}/extras/\$basearch/",
      proxy      => 'http://prod-epg-ost-proxy-01.hi.inet:6666',
      notify     => Exec['yum_clean_all'],
    }
  } else {
      yumrepo { 'CentOS-Extras':
      descr      => 'Centos Extras',
      enabled    => '1',
      gpgcheck   => '0',
      baseurl    => "http://${repo_server}/artifactory/${repo}/${::operatingsystemrelease}/extras/\$basearch/",
      notify     => Exec['yum_clean_all'],
    }

  }
}
