class smpp::config inherits smpp {
  
  file{"/opt/pdi/adapter/sms-smpp/conf/config.json":
    ensure =>present,
    content=>template("dca/smpp.json.erb"),
    notify =>Service["pdi-adapter-sms-smpp"]
  }
  
}
