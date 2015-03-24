
define monit::watch(
    $start_cmd = undef,
    $stop_cmd  = "/usr/bin/pkill -f ${title}",
    $user      = 'root',
    $group     = 'root',
    $ensure    = 'file',
    $pidfile   = "/var/run/${title}.pid"
    )
{
  
  include monit

  if ($ensure != 'absent' and ! $start_cmd) {
    fail("Must specify a start_cmd parameter")
  }

	file { "${title}.conf" :
	    ensure  => $ensure,
      path    => "/etc/monit.d/${title}.conf",
      owner   => root,
      group   => root,
      mode    => 0644,
      content => template("$module_name/watched.conf.erb"),
      require => File[$monit::config_dir],
      notify  => Service[$monit::service_name]
	}
	
}