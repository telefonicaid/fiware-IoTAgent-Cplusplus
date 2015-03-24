class tdaf_vagrant::pipeline {
  $mountString = '//oriente.hi.inet/artifacts /mnt/develenv/repos/rpms cifs rw,username=contint,password=admintid33,domain=hi,gid=502,uid=500,nobrl     0           0'
  $user=split($::hostname,'-')
  $username=$user[2]
  $userRepoPath="/mnt/develenv/repos/rpms/enabling/tdaf/users/$username"
  
  package{'pdi-tdaf-dp': ensure => latest; } 
  file{['/mnt/develenv','/mnt/develenv/repos','/mnt/develenv/repos/rpms']:
    ensure => directory,
    owner  => 'vagrant',
    group  => 'vagrant',
  }
  exec{'config-mount-repo':
    command => "echo '$mountString' >> /etc/fstab",
    path => '/bin:/usr/bin',
    unless =>'grep -q artifacts /etc/fstab',
    require => File['/mnt/develenv/repos/rpms'],
  }
  exec{'mount-repo':
    command => 'mount -a',
    path => '/bin:/usr/bin',
    unless =>'mount | grep -q artifacts',
    require => Exec['config-mount-repo'],
  }
  exec{'init-user-repo-arch':
    command => "mkdir -p $userRepoPath/x86_64; createrepo $userRepoPath/x86_64",
    path => '/bin:/usr/bin',
    creates => "$userRepoPath/x86_64",
    require => [Exec['mount-repo'],Package['createrepo']],
  }
  exec{'init-user-repo-noarch':
    command => "mkdir -p $userRepoPath/noarch; createrepo $userRepoPath/noarch",
    path => '/bin:/usr/bin',
    creates => "$userRepoPath/noarch",
    require => [Exec['mount-repo'],Package['createrepo']],
  }
  file{'/etc/profile.d/pipeline.sh':
    ensure  => absent,
    require => Package['pdi-tdaf-dp'],
  }
  file{'/var/tmp/rpm':
    ensure  => directory,
    mode    => '0777',
  }
}
