class logrotate::config inherits logrotate {
  
    if $caller_module_name != $module_name {
        fail("Use of private class ${name} by ${caller_module_name}")
    }

   file {  $config_basedir :
      ensure => directory,
      owner  => root,
      group  => root,
      mode   => 755,
   }
   ->
   file { 'logrotate.conf' :
      ensure  => file,
      path    => $config_path,
      owner   => root,
      group   => root,
      mode    => 0644,
      source  => "puppet:///modules/${module_name}/logrotate.conf",
   }

}
