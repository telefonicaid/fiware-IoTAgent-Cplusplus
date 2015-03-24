# Class: yum::rhel
#
# Description:
# This class finds the operating system release and calls the yumrepo class
# to create the repo.
# 
class yum::rhel::base($repo_server = 'repos.hi.inet') {

  if ($::fqdn =~ /openstacklocal/ ) {
  # Openstack instances need to set a proxy for internal repos
    yumrepo { 'rhel-base':
      descr    => "Red Hat Enterprise Linux $::operatingsystemrelease - Base",
      enabled  => '1',
      gpgcheck => '0',
      baseurl  => "http://${repo_server}/redhat/rhel${::operatingsystemrelease}s-\$basearch/RPMS.os/",
      proxy      => 'http://prod-epg-ost-proxy-01.hi.inet:6666',
      notify   => Exec['yum_clean_all'],
    }
  } else {
    yumrepo { 'rhel-base':
      descr    => "Red Hat Enterprise Linux $::operatingsystemrelease - Base",
      enabled  => '1',
      gpgcheck => '0',
      baseurl  => "http://${repo_server}/redhat/rhel${::operatingsystemrelease}s-\$basearch/RPMS.os/",
      notify   => Exec['yum_clean_all'],
    }
  }
}
