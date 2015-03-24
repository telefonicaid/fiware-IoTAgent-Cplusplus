
class common::sudo {

  package { 'sudo':
    ensure => 'latest'
  }

  file { '/etc/sudoers':
    owner   => 'root',
    group   => 'root',
    mode    => '0440',
    content => template('common/sudoers.erb'),
  }

  file { '/etc/sudoers.d':
    ensure => directory,
  }

}
