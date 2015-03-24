class tdaf_vagrant::open_stack_tools (
	$install_open_stack_tools = true,
) {
	if $install_open_stack_tools {
		package {
			# Open Stack tools
			'python-novaclient':  ensure => installed;
			'python-heatclient':  ensure => installed;
		}
	}
	else {
		notify {"Open Stack tools will not be installed":}
	}
}