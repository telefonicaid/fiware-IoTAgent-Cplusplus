/*
This class sets up TID-only stuff
*/
class puppet::tid {

  # These permissions are needed given our specific custom setup
  # where all manifests are in different git repos under /var/lib/puppet/initiatives
  # (One repo per initiative+environment)
  file { '/var/lib/puppet':
    ensure => directory,
    owner  => 'puppet',
    group  => 'puppet',
    mode   => '0755',
  }

  file { '/var/lib/puppet/initiatives':
    ensure => directory,
    owner  => 'puppet',
    group  => 'git',
    mode   => '0770',
  }

  # With the new setup: manifestdir = $vardir/initiatives/$environment/manifests
  # The default puppet environment is 'production', so we need the directory to exist, otherwise the puppet master won't start.
  file {'/var/lib/puppet/initiatives/production':
    ensure => link,
    target => '/etc/puppet',
  }
  file {'/var/lib/puppet/initiatives/default':
    ensure => link,
    target => '/etc/puppet',
  }

  group { 'git': ensure => present } ->
  user { 'git':
    ensure     => present,
    groups     => 'git',
    home       => '/home/git',
    shell      => '/bin/bash',
    managehome => true,
  }
}

