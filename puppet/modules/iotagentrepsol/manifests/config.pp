class iotagentrepsol::config inherits iotagentrepsol {

  if !$dev{  
    iotagent::register{ 'iotagentrepsol':
      resource => "$iotagentrepsol_resourceName",
      fileName => "$iotagentrepsol_fileName",
      target   => "$iotagent_configdir/$iotagent_configfile",
      options	 => "$iotagentrepsol_options"
    }
  }
  file{"$iotagentrepsol_configdir/$iotagentrepsol_configfile":
    ensure =>present,
    content => template('iotagentrepsol/Repsol.xml.erb'),
  }

}
