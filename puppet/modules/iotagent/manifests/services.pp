class iotagent::services inherits iotagent {

  include monit

  monit::watch { "$iotagent_process_name_monit": 
    start_cmd => "$iotagent_bindir/init_iotagent.sh",
    stop_cmd  => "$iotagent_bindir/stop_iotagent.sh",
    pidfile   => "$iotagent_pidfile"
  }

}
