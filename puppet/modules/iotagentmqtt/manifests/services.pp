class iotagentmqtt::services inherits iotagentmqtt {

  include monit

  monit::watch { 'mosquitto': 
    start_cmd => "$iotagentmqtt_scriptsdir/$iotagentmqtt_initscript",
    pidfile   => "$iotagentmqtt_pidfile"
  }

}
