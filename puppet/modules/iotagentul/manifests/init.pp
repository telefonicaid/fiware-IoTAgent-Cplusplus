class iotagentul(
    
	) inherits iotagentul::params {

  contain iotagent

  contain iotagentul::packages
  contain iotagentul::config
  contain iotagentul::services
  Class['iotagentul::packages'] -> Class['iotagentul::config'] -> Class['iotagentul::services']

}