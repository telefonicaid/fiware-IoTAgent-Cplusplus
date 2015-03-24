class preinstall::params {

	$proxy                     = "http://prod-epg-ost-proxy-01.hi.inet:6666"
	$selinux_status           = "disabled"
	$repositorio_rhel_updates  = "http://repos.hi.inet/redhat/rhel6s-x86_64/RPMS.updates/"
	$repositorio_rhel_base     = "http://repos.hi.inet/redhat/rhel6.5s-\$basearch/RPMS.all/"
	$repositorio_epel          = "http://repos.hi.inet/redhat/epel6-\$basearch/RPMS.all/"
	$sbc_repository          = "http://artifactory.hi.inet/artifactory/simple/yum-sbc/iot-agent/x86_64/"
	$sbc_common_repository           = "http://artifactory.hi.inet/artifactory/simple/yum-sbc/common6.5/x86_64/"
}