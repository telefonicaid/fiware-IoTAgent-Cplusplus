class puppet::master (
  $puppet_version          = undef,
  $unicorn                 = undef, # boolean. Set Unicorn as the Rails server, and Nginx as the WEb Frontend
  $unicorn_workers         = undef,
  $unicorn_workers_timeout = undef,
  $puppetmaster_log        = 'syslog', # can either be syslog or a path (ie /var/log/puppet/puppetmaster.log)
) {

  include puppet::tid

  if $unicorn {
    service { 'puppetmaster':
      ensure => stopped,
      enable => false,
    }
    service { 'puppetunicorn':
      enable    => true,
      ensure    => running,
      require   => File['/etc/puppet/config.ru'],
      subscribe => File['/etc/puppet/puppet.conf'],
    }
    package {['nginx', 'make', 'gcc', 'ruby-devel',
              'rubygems',
              'rubygem-activerecord',    # To avoid the  "no such file to load -- active_record" error
              'rubygem-sqlite3-ruby' ]:    # To avoid the "no such file to load -- sqlite3" error
      ensure => present,
    }
    # Already declared on mcollective::server
    #realize (Package['rubygems'])
    package { 'unicorn':
      ensure   => present,
      provider => gem,
    }
    package { 'rack':
      ensure   => present,
      provider => gem,
    }
    service { 'nginx':
      enable => true,
      ensure => running,
    }
    service {'httpd':
      ensure => running,
      enable => true,
    }
    file { '/etc/puppet/unicorn.conf':
      content => template('puppet/unicorn.conf.erb'),
      notify  => Service['puppetunicorn'],
    }
    file { '/etc/puppet/config.ru':
      mode   => '0644',
      source => 'puppet:///modules/puppet/puppet/config.ru',
      notify => Service['puppetunicorn'],
    }
    file { '/etc/nginx/conf.d/puppetmaster.conf':
      content => template('puppet/puppetmaster_nging.conf.erb'),
      notify => Service['nginx'],
    }
    file { '/etc/init.d/puppetunicorn':
      source => 'puppet:///modules/puppet/init.d/puppetunicorn',
      mode   => '0755',
    }
  } else {
    service { 'puppetmaster':
      enable    => true,
      ensure    => running,
      subscribe => File['/etc/puppet/puppet.conf'],
    }
    service { 'puppetunicorn':
      ensure => stopped,
      enable => false,
    }
    file { '/etc/puppet/unicorn.conf':
      ensure => absent,
    }
    file { '/etc/puppet/config.ru':
      ensure => absent,
    }
    file { '/etc/nginx/conf.d/puppetmaster.conf':
      ensure => absent,
    }
    file { '/etc/init.d/puppetunicorn':
      ensure => absent,
    }
  }

  # Common stuff (with or without Unicorn)
  package { 'puppet-server':
    ensure => $puppet_version;
  }
  file { '/etc/sysconfig/puppetmaster':
    content => template('puppet/sysconfig_puppetmaster.erb'),
    notify  => Service['puppetunicorn'],
  }

  if $puppetmaster_log == 'syslog' {
    include common::syslog
    # Basicaly to add this:  *.info;puppet-master.none;mail.none;authpriv.none;cron.none                /var/log/messages
    file { '/etc/rsyslog.conf':
      content => template('puppet/rsyslog.conf.erb'),
      notify  => Service['rsyslog'],
    }
    file { '/etc/rsyslog.d/puppet-master.conf':
      content => template('puppet/rsyslog_puppet-master.conf.erb'),
      notify  => Service['rsyslog'],
    }
    file { '/var/log/puppet/puppetmaster.log':
      ensure => present,
      owner  => 'puppet',
      group  => 'puppet',
    }
    file { '/etc/logrotate.d/puppet-master':
      ensure => present,
      source => 'puppet:///modules/puppet/logrotate.d/puppet-master',
    }
  }
}
