class tdaf_vagrant::python {
 # Needed packages to guest additions installation  
  package {
    ## For Python building
    'pdi-python-distribute':  ensure => installed;
    'pip':                    ensure => installed;
    'virtualenv':             ensure => installed;
  }	
}