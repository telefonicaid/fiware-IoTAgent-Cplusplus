class smpp::packages inherits smpp {

   package { $smpp_package:
      ensure => $smpp_version,
   }

}
