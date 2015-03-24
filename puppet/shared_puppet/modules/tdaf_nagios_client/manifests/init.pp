# install and configure Nagios as a xinetd service.

class tdaf_nagios_client (
	$service_enable = true,
	$service_running = running,
	$tdaf_nagios_clients_ips = ' ',
){
	$checks_dir="/etc/nagios/checks.d"
	include plugins
	include general_nrpe_checks
	class {'tdaf_nagios_client::nrpe':
		service_enable          => $service_enable,
		service_running         => $service_running,
		tdaf_nagios_clients_ips => $tdaf_nagios_clients_ips
	}
}
