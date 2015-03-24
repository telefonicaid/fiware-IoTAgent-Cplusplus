class fmr_nagios_client::nrpe (
   $nrpe_port = 5666,
   $nagios_worker_ip = "172.31.0.224",
) {
   #default values are for TDAF
   
   import 'puppetlabs-firewall'

   # make sure software is installed
   package {[nrpe,
             xinetd]:
      ensure => latest,
      require => Yumrepo['epel'],
   } 


   # make sure the firewall is open
   firewall{"0200-ACCEPT PORT $nrpe_port":
      source      => '0/0',
      destination => '0/0',
      port        => $nrpe_port,
      action      => 'accept',
   }


   # make sure the correct line is in /etc/services
   augeas {'/etc/services':
      context => 'files/etc/services',
      changes => [
         "set service-name[port = $nrpe_port]/port $nrpe_port",
         "set service-name[port = $nrpe_port] nrpe",
         "set service-name[port = $nrpe_port]/protocol tcp",
         "set service-name[port = $nrpe_port]/#comment NRPE",
      ],
   }

   # configure the xinetd service
   file {'/etc/xinetd.d/nrpe':
      content => template("${module_name}/xinetd-nrpe.erb"),
      require => Package['nrpe'],
   }

   file {'/etc/nagios/nrpe.cfg':
      content => template("${module_name}/nrpe.cfg.erb"),
      require => Package['nrpe'],
   }

   service {'xinetd':
      ensure => running,
      subscribe => [ File['/etc/xinetd.d/nrpe'],
                     File['/etc/nagios/nrpe.cfg']],
   }


}
