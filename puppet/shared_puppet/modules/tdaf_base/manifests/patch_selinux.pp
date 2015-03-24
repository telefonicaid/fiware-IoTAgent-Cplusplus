class tdaf_base::patch_selinux {	
  exec {'patch_selinux':
    command => '/usr/bin/yum -y downgrade libselinux-2.0.94-5.3.el6  libselinux-ruby-2.0.94-5.3.el6 libselinux-utils-2.0.94-5.3.el6 libselinux-python-2.0.94-5.3.el6',
    unless => '/bin/rpm -q libselinux-2.0.94-5.3.el6'
  }
}

