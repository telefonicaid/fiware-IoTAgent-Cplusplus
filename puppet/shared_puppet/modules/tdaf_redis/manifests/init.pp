#
# Copyright 2013 Telefonica Digital. All rights reserved.
#
class tdaf_redis (
  $redis_mode = 'default',
  $redis_node_ips = ['localhost'],
  $redis_port = '6379',
  $sentinel_port = '26379',
  $service_enable = true,
  $service_running = running,
  
  ) {
	########################################
	# Installation
	########################################

  validate_re($redis_mode,['default','sentinel'])
  package {'redis':  
    ensure => installed;
  }

  ########################################
  # Configurations
  ########################################
  #If has other value than localhost, expect hiera form
  if $redis_node_ips[0] == 'localhost' {
    $master_ip = 'localhost'
  } else {
    # we will configure first node as master
    $master_ip = $redis_node_ips[0]['hostname']
    if $::fqdn != $redis_node_ips[0]['fqdn'] {
      # We are a slave
      $slave_mode = true
    }
  }
  ###############
  # Sentinel
  ###############
  if $redis_mode == 'sentinel' {
    $redis_config = "/etc/redis_sentinel.conf"
    $addons = ' --sentinel'
    
    firewall{"0200-ACCEPT PORT $sentinel_port":
      source      => '0/0',
      destination => '0/0',
      port        => $sentinel_port,
      action      => 'accept',
    }
  } else {
  ###############
  # Standard server
  ###############
    $redis_config = "/etc/redis.conf"
    $addons = ''
    
    firewall{"0200-ACCEPT PORT $redis_port":
      source      => '0/0',
      destination => '0/0',
      port        => $redis_port,
      action      => 'accept',
    }
  }
  file{$redis_config:
    ensure  => present,
    content  => template("${module_name}/redis_${redis_mode}.conf.erb"),
    owner   => 'root',
    group   => 'root',
    mode    => '0644',
    require => Package['redis'],
  }
  file{"/var/log/redis":
    ensure  => directory,
    owner   => 'redis',
    group   => 'redis',
    mode    => '0664',
    require => Package['redis'],
  }

  ########################################
  # Service
  ########################################
  
  file{"/etc/init.d/redis":
    ensure  => present,
    content  => template("${module_name}/redis_init.erb"),
    owner   => 'root',
    group   => 'root',
    mode    => '0755',
  }
  service{"redis":
    enable  => $service_enable,
    ensure  => $service_running,
    subscribe => [ File[$redis_config], File['/etc/init.d/redis'] ]
  }
}
