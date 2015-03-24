class iotagenttt::packages inherits iotagenttt {

   package { $iotagenttt_package:
      ensure => $iotagenttt_version,
   }

}
