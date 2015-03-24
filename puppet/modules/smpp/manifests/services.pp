class smpp::services inherits smpp {

  include monit

  monit::watch { 'smpp': 
    start_cmd => "$smpp_start",
    stop_cmd  => "$smpp_stop",
    pidfile   => "$smpp_pidfile"
  }

  service{"pdi-adapter-sms-smpp":
    ensure => running,
    enable => true
  }

}
