class tdaf_vagrant {
	
	class {'tdaf_vagrant::ec2tools': 
		install_ec2tools => false
	}
	
	class {'tdaf_vagrant::open_stack_tools': 
		install_open_stack_tools => true
	}
	
	class{'tdaf_vagrant::packages': }
	class{'tdaf_vagrant::pipeline': 
		require => Class['tdaf_vagrant::packages']
	}
	class{'tdaf_develop': }
	tdaf_develop::user{'vagrant':  }
	class{ 'tdaf_resolver': 
		domainname  => "hi.inet",
		searchpath  => ['hi.inet'],
		nameservers => ['10.95.96.4', '10.95.96.5', $ipaddress_eth0],
		}
	#Disable selinux
	exec{'disable-selinux':
		command => 'echo 0 > /selinux/enforce',
		path => '/bin:/usr/sbin',
		unless => 'getenforce | grep -v Enforcing',
  }
}
