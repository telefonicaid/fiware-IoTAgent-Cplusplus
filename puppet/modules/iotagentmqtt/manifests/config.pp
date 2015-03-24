class iotagentmqtt::config inherits iotagentmqtt {
  
  if !$dev{ 
    iotagent::register{ 'iotagentmqtt':
      resource => "$iotagentmqtt_resourceName",
      fileName => "$iotagentmqtt_fileName",
      target   => "$iotagent_configdir/$iotagent_configfile",
      options	 => "$iotagentmqtt_options",
    }
  }
}
