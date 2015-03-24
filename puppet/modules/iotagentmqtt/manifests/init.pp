class iotagentmqtt(
   	) inherits iotagentmqtt::params {

  contain iotagent

  contain iotagentmqtt::packages
  contain iotagentmqtt::config
  contain iotagentmqtt::services
  Class['iotagentmqtt::packages'] -> Class['iotagentmqtt::config'] -> Class['iotagentmqtt::services']

}