class fmr_nagios_client {
  
  require 'tdaf_yumrepos::epel'

  include plugins
  include nrpe

}
