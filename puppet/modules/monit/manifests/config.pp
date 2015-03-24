class monit::config inherits monit {
  
    if $caller_module_name != $module_name {
        fail("Use of private class ${name} by ${caller_module_name}")
    }

  include logrotate

  file { 'global' :
      ensure  => file,
      path    => $config_global_file,
      owner   => root,
      group   => root,
      mode    => 0644,
      content => template("$module_name/global.erb"),
      require => Class['monit::install'],
      notify  => Class['monit::service'] 
   }
   ->
   file {  $config_dir :
      ensure  => directory,
      owner   => root,
      group   => root,
      mode    => 755,
   }
   ->
   file { 'logging' :
      ensure  => file,
      path    => $config_logging_file ,
      owner   => root,
      group   => root,
      mode    => 0644,
      content => template("$module_name/logging.erb"),
      require => Class['monit::install'],
      notify  => Class['monit::service'] 
   }
   ->
   file { 'monit.conf' :
      ensure => file,
      path   => $config_file ,
      owner  => root,
      group  => root,
      mode   => 0700,
      source => "puppet:///modules/${module_name}/monit.conf",
      require => Class['monit::install'],
      notify  => Class['monit::service'] 
   }
   ->
   file { 'monit_init.conf' :
      ensure  => file,
      path    => $upstart_file,
      owner   => root,
      group   => root,
      mode    => 0644,
      content => template("$module_name/monit_init.conf.erb"),
      require => Class['monit::install'],
   } 
   ->
   logrotate::specfile { $log_specfile:
    content => template("$module_name/monit.log.erb"),
  }
}
