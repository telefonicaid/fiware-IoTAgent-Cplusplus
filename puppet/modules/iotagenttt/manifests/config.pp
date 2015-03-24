class iotagenttt::config inherits iotagenttt {
  
  if !$dev{ 
    iotagent::register{ 'iotagenttt':
      resource => "$iotagenttt_resourceName",
      fileName => "$iotagenttt_fileName",
      target   => "$iotagent_configdir/$iotagent_configfile",
      options	 => "$iotagenttt_options",
    
    }
  }
}
