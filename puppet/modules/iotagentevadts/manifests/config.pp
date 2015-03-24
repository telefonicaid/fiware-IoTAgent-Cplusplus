class iotagentevadts::config inherits iotagentevadts {
 
  if !$dev{ 
    iotagent::register{ 'iotagentevadts':
	  resource    => "$iotagentevadts_resourceName",
	  fileName    => "$iotagentevadts_fileName",
	  target      => "$iotagent_configdir/$iotagent_configfile",
	  options     => "$iotagentevadts_options",
    }
  }

}
