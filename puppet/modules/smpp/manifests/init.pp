class smpp inherits smpp::params {

		contain smpp::packages
		contain smpp::config
		contain smpp::services
		Class['smpp::packages'] -> Class['smpp::config'] -> Class['smpp::services']
}