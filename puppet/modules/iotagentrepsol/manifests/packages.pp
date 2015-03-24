class iotagentrepsol::packages inherits iotagentrepsol {

   package { $iotagentrepsol_package:
      ensure => $iotagentrepsol_version,
   }

}
