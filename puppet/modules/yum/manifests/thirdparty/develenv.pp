class yum::thirdparty::develenv (
  $javaurlrepo      = 'http://servilinux.hi.inet/java/yum/7/$basearch',
  $develenvrpmrepo  = 'http://develenvms.softwaresano.com/public/downloads/repos/ss-develenv-repo-1.0-0.0.noarch.rpm',
) {

  include yum::thirdparty::epel

  # These repos come with the VM template
  file {'/etc/yum.repos.d/ss-epel-x86_64.repo':                    ensure => absent, }
  file {'/etc/yum.repos.d/ss-google-chrome-x86_64.repo':           ensure => absent, }
  file {'/etc/yum.repos.d/ss-thirdparty-develenv-src.repo.rpmnew': ensure => absent, }
  file {'/etc/yum.repos.d/ss-develenv-repo-noarch.repo':           ensure => absent, }
  file {'/etc/yum.repos.d/ss-develenv-repo.repo':                  ensure => absent, }

  if $domain == 'hi.inet' {
    $enablejavarepo = "1"
  } else {
    $enablejavarepo = "0"
  }

#  yumrepo { 'google-chrome':
#    baseurl    => 'http://dl.google.com/linux/chrome/rpm/stable/$basearch',
#    descr      => 'Google Chrome Repository',
#    enabled    => '1',
#    gpgcheck   => '0',
#    notify     => Exec['clean_yum_cache'],
#  }

  yumrepo { 'java-repo':
    baseurl  => $javaurlrepo,
    descr    => 'Oracle java repository',
    enabled  => $enablejavarepo,
    gpgcheck => '0',
    notify   => Exec['clean_yum_cache'],
  }

  yumrepo { 'ss-thirdparty-develenv-noarch':
    baseurl    => 'http://thirdparty2-develenv-softwaresano.googlecode.com/svn/trunk/develenv/src/site/resources/tools/rpms/noarch',
    descr      => 'Repositorio de RPMs generados con thirdparty-develenv',
    enabled    => '1',
    gpgcheck   => '0',
    notify     => Exec['clean_yum_cache'],
  }

  yumrepo { "ss-thirdparty-develenv-${architecture}":
    baseurl  => 'http://thirdparty2-develenv-softwaresano.googlecode.com/svn/trunk/develenv/src/site/resources/tools/rpms/$basearch',
    descr    => 'Repositorio de RPMs generados con thirdparty-develenv',
    enabled  => '1',
    gpgcheck => '0',
    notify   => Exec['clean_yum_cache'],
  }

  yumrepo { "ss-thirdparty-develenv-src":
    baseurl  => 'http://thirdparty2-develenv-softwaresano.googlecode.com/svn/trunk/develenv/src/site/resources/tools/rpms/src',
    descr    => 'Repositorio de source RPMs generados con thirdparty-develenv',
    enabled  => '1',
    gpgcheck => '0',
    notify   => Exec['clean_yum_cache'],
  }

  # el rpm de arriba instala el  pero lo machacamos con este clon en artifacts. En un futuro el rpm de arriba (es dependencia
  # de otros) no existira, pero de momento hacemos esta chapucilla...
  package {'ss-develenv-repo':
    provider => 'rpm',
    ensure   => installed,
    source   => $develenvrpmrepo,
    require  => Package['ss-thirdparty-develenv-repo'],
  }

  yumrepo { "ss-develenv-noarch":
    baseurl  => 'http://artifacts.hi.inet/develenv/noarch',
    descr    => 'ss-develenv',
    enabled  => '1',
    gpgcheck => '0',
    notify   => Exec['clean_yum_cache'],
  }

  yumrepo { "ss-develenv-x86_64":
    baseurl  => 'http://artifacts.hi.inet/develenv/x86_64',
    descr    => 'ss-develenv',
    enabled  => '1',
    gpgcheck => '0',
    notify   => Exec['clean_yum_cache'],
  }

  package {'ss-thirdparty-develenv-repo':
    provider => 'rpm',
    ensure   => installed,
    source   => "http://thirdparty2-develenv-softwaresano.googlecode.com/svn/trunk/develenv/src/site/resources/tools/rpms/noarch/ss-thirdparty-develenv-repo-1.0-0.0.noarch.rpm",
  }


  exec { 'clean_yum_cache':
    command     => '/usr/bin/yum clean all',
    refreshonly => true,
  }

}
