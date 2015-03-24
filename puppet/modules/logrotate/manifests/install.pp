class logrotate::install inherits logrotate {
  
  if $caller_module_name != $module_name {
    fail("Use of private class ${name} by ${caller_module_name}")
  }  

  package { $package_name:
    ensure  => $package_ensure,
  }

}