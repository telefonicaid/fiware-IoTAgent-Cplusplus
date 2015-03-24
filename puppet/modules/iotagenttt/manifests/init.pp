class iotagenttt(
    ) inherits iotagenttt::params {

  contain iotagent

  contain iotagenttt::packages
  contain iotagenttt::config
  contain iotagenttt::services
  Class['iotagenttt::packages'] -> Class['iotagenttt::config'] -> Class['iotagenttt::services']

}