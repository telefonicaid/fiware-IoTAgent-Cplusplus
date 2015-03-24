class fmr_nagios_client::plugins {

   # make sure the plugins have the right permissions
   file {'/usr/lib64/nagios/plugins':
      source => "puppet:///modules/${module_name}/plugins-nagios/",
      ensure => directory,
      recurse => true,
      owner   => 'nagios',
      group   => 'nagios',
      # user and group nagios are provided by the nagios-common package
   }

   file {'/usr/lib64/nagios':
      ensure => directory,
      owner  => 'nagios',
      group  => 'nagios',
   }

}
