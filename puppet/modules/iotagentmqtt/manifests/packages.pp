class iotagentmqtt::packages inherits iotagentmqtt {

   package { $iotagentmqtt_package:
      ensure => $iotagentmqtt_version,
   }

}
