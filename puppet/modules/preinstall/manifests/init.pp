class preinstall (
	$proxy = $preinstall::params::proxy,
	)inherits preinstall::params {

	Yumrepo {
		enabled         => 1,
		gpgcheck        => 0,
		priority        => 1,
		proxy           => $proxy,
		metadata_expire => 30,
	}
	
	exec { "selinux_live":
		command  => '/usr/sbin/setenforce 0',
		notify   => File['selinux'],
		onlyif   => "/usr/bin/test `/usr/sbin/getenforce` != Disabled",
	}
	->
	file { "selinux":
		path     => '/etc/selinux/config',
		ensure   => file,
		content  => template('preinstall/config.erb'),
		require  => Exec['selinux_live']
	}
	->
	# The iptables are temporally disabled
	service { "iptables":
	    enable => false,
		ensure => stopped,
		require => File['selinux'],
	}
	->
	yumrepo { "rhel-updates":
		descr    => "The rhel-updates repository",
		baseurl  => $repositorio_rhel_updates,
		require  => File ['selinux']
	}
	->
	yumrepo { "epel":
		descr    => "Extra Packages for Enterprise Linux 6",
		baseurl  => $repositorio_epel,
		require  => Yumrepo ['rhel-updates']
	}
	->
	yumrepo { "rhel-base":
		descr    => "Red Hat Enterprise Linux 6.5 - Base",
		baseurl  => $repositorio_rhel_base,
		require  => Yumrepo ['epel']
	}
	->
	yumrepo { "SBC-Repo":
		descr    => "SBC repository Artifactory",
		baseurl  => $sbc_repository,
	}
	->
	yumrepo { "SBC-Common":
		descr    => "SBC Common repository Artifactory",
		baseurl  => $sbc_common_repository,
	}
	->
	exec { 'yum-clean-all':
		user     => 'root',
		path     => '/usr/bin',
		command  => 'yum clean all',
		require  => Yumrepo ['epel']
	}
	->
	exec { 'yum-repolist':
		user     => 'root',
		path     => '/usr/bin',
		command  => 'yum repolist',
		require  => Exec ['yum-clean-all']
	}
}