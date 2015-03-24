class iotagentevadts(
   
	) inherits iotagentevadts::params {

  contain iotagent

  contain iotagentevadts::packages
  contain iotagentevadts::config
  contain iotagentevadts::services
  Class['iotagentevadts::packages'] -> Class['iotagentevadts::config'] -> Class['iotagentevadts::services']

}