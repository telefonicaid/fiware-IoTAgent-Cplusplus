class iotagentul::packages inherits iotagentul {

   package { $iotagentul_package:
      ensure => $iotagentul_version,
   }

}
