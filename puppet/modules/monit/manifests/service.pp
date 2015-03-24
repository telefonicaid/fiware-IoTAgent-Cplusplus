
class monit::service inherits monit
{
	service { $service_name:
		ensure      => $service_ensure,
        enable      => true,
    	hasstatus   => true,
    	hasrestart  => true,
        restart     => '/sbin/initctl restart monit',
        start       => '/sbin/initctl start monit',
        stop        => '/sbin/initctl stop monit',
        status      => '/sbin/initctl status monit | grep "/running" 1>/dev/null 2>&1',
	}
	
}