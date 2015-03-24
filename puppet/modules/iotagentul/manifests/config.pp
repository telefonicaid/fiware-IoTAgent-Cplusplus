class iotagentul::config inherits iotagentul {
  
  if !$dev{ 
    iotagent::register{ 'iotagentul':
      resource => "$iotagentul_resourceName",
      fileName => "$iotagentul_fileName",
      target   => "$iotagent_configdir/$iotagent_configfile",
    }
  }
}
