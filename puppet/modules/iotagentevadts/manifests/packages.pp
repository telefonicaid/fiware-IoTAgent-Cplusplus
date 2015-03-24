class iotagentevadts::packages inherits iotagentevadts {

   package { $iotagentevadts_package:
      ensure => $iotagentevadts_version,
   }

}
