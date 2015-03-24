class tdaf_yumrepos::localhost {
  include rpm::packages
  $rpmbuild_dirs = [ "/vagrant/rpmbuild",                     
                     "/vagrant/rpmbuild/RPMS",
                     "/vagrant/rpmbuild/RPMS/noarch",
                     "/vagrant/rpmbuild/RPMS/x86_64",
                   ]

  file { $rpmbuild_dirs:
    owner   => 'vagrant',
    group   => 'vagrant',
    mode     => 755,
    ensure   => directory,
    require => User['vagrant'],
  }

  file { '/home/vagrant/rpmbuild':
   owner   => 'vagrant',
   group   => 'vagrant',
   ensure => 'link',
   target => '/vagrant/rpmbuild',
   require => File[$rpmbuild_dirs]
  }

  exec { 'createrepo_localhost':
    user        => 'vagrant',
    command     => "/usr/bin/createrepo -s sha -d --update /home/vagrant/rpmbuild/RPMS/noarch && /usr/bin/createrepo -s sha -d --update /home/vagrant/rpmbuild/RPMS/x86_64",
    require => [Package['createrepo'], File ['/home/vagrant/rpmbuild']],
  }

  yumrepo { 'localhost_arch':
    descr      => 'Packages generated in local(arch)',
    enabled    => '1',
    gpgcheck   => '0',
    baseurl    => 'file://localhost/home/vagrant/rpmbuild/RPMS/$basearch',
    require    => Exec['createrepo_localhost'],
    notify     => Exec['clean_yum_cache'],
  }
  
  yumrepo { 'localhost_noarch':
    descr      => 'Packages generated in local(noarch)',
    enabled    => '1',
    gpgcheck   => '0',
    baseurl    => 'file://localhost/home/vagrant/rpmbuild/RPMS/noarch',
    require => Exec['createrepo_localhost'],
    notify     => Exec['clean_yum_cache'],
  }
}
