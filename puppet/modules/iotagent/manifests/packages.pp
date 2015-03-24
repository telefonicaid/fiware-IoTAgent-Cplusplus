class iotagent::packages inherits iotagent {

   package { $iotagent_package:
      ensure => $iotagent_version,
   }

}
