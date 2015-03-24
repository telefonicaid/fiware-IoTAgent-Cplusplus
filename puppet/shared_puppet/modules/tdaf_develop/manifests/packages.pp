class tdaf_develop::packages {
  package {
    'git':            ensure => installed;
    'subversion':     ensure => installed;
    'wget':           ensure => installed;
    'gcc':            ensure => installed;
    'make':           ensure => installed;
    'cifs-utils':     ensure => installed;
    'rpmdevtools':    ensure => installed;
    'createrepo':     ensure => installed;
  }	
  ## Following is a patch to repair a problem with createrepo and shared folders
  ## See: https://github.com/mitchellh/vagrant/issues/1446
  ## We will overwrite __init__ file for createrepo-0.9.9-17
  file{'/usr/lib/python2.6/site-packages/createrepo/__init__.py':
    require => Package['createrepo'],
    owner   => 'root',
    group   => 'root',
    mode    => '0644',
    source  => 'puppet:///modules/tdaf_develop/createrepo_init_patch.py',
  }
}