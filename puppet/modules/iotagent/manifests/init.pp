class iotagent(
  
	) inherits iotagent::params {

		contain iotagent::packages
		contain iotagent::config
		contain iotagent::services
		Class['iotagent::packages'] -> Class['iotagent::config'] -> Class['iotagent::services']

	include iotagent::limits
	
	hiera_include('iota_protocols')
}