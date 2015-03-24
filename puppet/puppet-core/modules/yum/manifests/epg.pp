# Class: yum::epg
#
# Description:
# This class finds the operating system release and calls the yumrepo class
# to create the repo.
# 
class yum::epg($repo_server = 'artifactory.aislada.hi.inet') {

  if ($::fqdn =~ /openstacklocal/ ) {
  # Openstack instances need to set a proxy for internal repos
    yumrepo { 'epg-arch':
      descr    => "Official EPG approved packages for RedHat/CentOS $::operatingsystemmajrelease",
      enabled  => '1',
      gpgcheck => '0',
      baseurl  => "http://${repo_server}/artifactory/yum-epg/${::operatingsystemmajrelease}/\$basearch/",
      proxy    => 'http://prod-epg-ost-proxy-01.hi.inet:6666',
      notify   => Exec['yum_clean_all'],
    }
	yumrepo { 'epg-noarch':
      descr    => "Official EPG approved packages for RedHat/CentOS $::operatingsystemmajrelease",
      enabled  => '1',
      gpgcheck => '0',
      baseurl  => "http://${repo_server}/artifactory/yum-epg/${::operatingsystemmajrelease}/noarch/",
      proxy    => 'http://prod-epg-ost-proxy-01.hi.inet:6666',
      notify   => Exec['yum_clean_all'],
    }
  } else {
    yumrepo { 'epg-arch':
      descr    => "Official EPG approved packages for RedHat/CentOS $::operatingsystemmajrelease",
      enabled  => '1',
      gpgcheck => '0',
      baseurl  => "http://${repo_server}/artifactory/yum-epg/${::operatingsystemmajrelease}/\$basearch/",
      notify   => Exec['yum_clean_all'],
    }
	yumrepo { 'epg-noarch':
      descr    => "Official EPG approved packages for RedHat/CentOS $::operatingsystemmajrelease",
      enabled  => '1',
      gpgcheck => '0',
      baseurl  => "http://${repo_server}/artifactory/yum-epg/${::operatingsystemmajrelease}/noarch/",
      notify   => Exec['yum_clean_all'],
    }
  }
}
