class iotagent::config inherits iotagent {
  
  file { "$iotagent_bindir/init_iotagent.sh":
    ensure  => present,
    owner   => 'root',
    group   => 'root',
    mode    => '0755',
    content => template('iotagent/init_iotagent.sh.erb'),
  }

  file { "$iotagent_bindir/stop_iotagent.sh":
    ensure  => present,
    owner   => 'root',
    group   => 'root',
    mode    => '0755',
    content => template('iotagent/stop_iotagent.sh.erb'),
  }
  
  
  if $dev{
    
    file{"$iotagent_configdir/$iotagent_configfile":
      owner  => 'root',
      group  => 'root',
      mode   => '0644',
      ensure => present,
      source => 'puppet:///modules/iotagent/configDev.json',
      notify => Exec["restart_monit_$iotagent_process_name_monit"],
    }
  }else{

   
      concat{"$iotagent_configdir/$iotagent_configfile":
      owner => 'root',
      group => 'root',
      mode  => '0644',
      notify => Exec["restart_monit_$iotagent_process_name_monit"],
    }
    concat::fragment{'header_config':
      target  => "$iotagent_configdir/$iotagent_configfile",
      order   => '01',
      content => "{ \n  \"resources\": [\n",
    }
    concat::fragment{'finish_config':
      target  => "$iotagent_configdir/$iotagent_configfile",
      order   => '99',
      content => template('iotagent/iotagent-base_conf.json.erb'),
    }
  }
  
  exec{"restart_monit_$iotagent_process_name_monit":
    command => "/usr/bin/monit restart $iotagent_process_name_monit",
    onlyif  => "/usr/bin/test -f /etc/monit.d/$iotagent_process_name_monit.conf",
  }
  
  file{"$iotagent_logdir":
    ensure  => directory,
    owner   => 'root',
    group   => 'root',
    mode    => '0755',
  }
}


# used by other modules to register the resource in the iot-agent-base's config file
define iotagent::register(
  $resource    = $title, 
  #$cbroker     = 'http://127.0.0.1:1026',
  $order       = '10',
  $fileName    = $title,
  $options     = '',
  $entity_type = 'thing',
  $apikey      = '',
  $token       = 'token2',
  $target,
  ) {
  
  #append a comma into the options not null vble
  if $options != ''{
    $configResource="
     {
     \"resource\": \"/iot/$resource\",
     \"options\": {
        $options,
        \"FileName\": \"$fileName\"
       }
     },"
  }else{
    $configResource="
     {
     \"resource\": \"/iot/$resource\",
     \"options\": {
        \"FileName\": \"$fileName\"
       }
     },"
  }
 

  concat::fragment{ "config_fragment_$name":
    target  => "$target",
    order   => "$order",
    content => "$configResource \n"
  }

}