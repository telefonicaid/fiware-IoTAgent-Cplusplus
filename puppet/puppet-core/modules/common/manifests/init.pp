# Set $puppet_server to false on a materless setup
class common(
  $permitrootlogin          = 'no',
  $ntpserver                = '10.95.67.227',
  $puppet_server            = 'puppet.aislada', 
  $puppet_agent_enable      = true,
  $puppet_agent_ensure      = 'running',
  $puppet_agent_runinterval = '3600',
  $users_epg                = present,
  $users_sysadmin           = present,
  $users_pipeline           = present,
  $users_clouduser          = absent,
) {
  #notify {'permitrootlogin':
  #  message => "PermitRootLogin = ${permitrootlogin}",
  #}

  class { 'common::users::epg':
    ensure => $users_epg,
  }
  class { 'common::users::pipeline':
    ensure => $users_pipeline,
  }
  class { 'common::users::sysadmin':
    ensure => $users_sysadmin,
  }
  class { 'common::users::clouduser':
    ensure => $users_clouduser,
  }
  class { 'common::users::root':
  }

  require yum
  include common::aislada
  include common::sudo
  include common::syslog
  include common::network
  include common::rhel5
  include security::stickybit
  include security::passwords
  include security::bugfixes::openssl
  include security::bugfixes::bash

  import 'rewrite.pp' # For apache redirects

  
  class { 'puppet':
    puppet_server            => "${puppet_server}",
    puppet_agent_ensure      => "${puppet_agent_ensure}",
    puppet_agent_enable      => "${puppet_agent_enable}",
    puppet_agent_runinterval => "${puppet_agent_runinterval}",
  }
  class { 'ssh':
    permitrootlogin => "${permitrootlogin}",
  }
  class { 'ntp':
    ntpserver => "${ntpserver}",
  }
  class { 'snmpd':  
    initiative => 'epg', # where we store the snmpd.conf
    require    => Class['yum']
  } 
#  if $::virtual == 'vmware' {
#    include vmwaretools
#  }

}

