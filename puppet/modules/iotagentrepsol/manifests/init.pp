class iotagentrepsol(
    #$iota_re = hiera_hash('iotagentrepsol'),
    
	) inherits iotagentrepsol::params {

  contain iotagent

  contain iotagentrepsol::packages
  contain iotagentrepsol::config
  contain iotagentrepsol::services
  Class['iotagentrepsol::packages'] -> Class['iotagentrepsol::config'] -> Class['iotagentrepsol::services']

}