node 'localhost' inherits default {
   class{'iotagent':
    dev => true,
  }
  class{'iotagentul':
  	dev => true,
  }
  class{'iotagentrepsol':
  	dev => true,
  }
  class{'iotagentmqtt':
  	dev => true,
  }
  class{'iotagentevadts':
  	dev => true,
  }
  class{'iotagenttt':
  	dev => true,
  }

   stage { 'preinstall':
    before => Stage['main'],
  }

  class { 'preinstall':
    stage => preinstall,
  }
}