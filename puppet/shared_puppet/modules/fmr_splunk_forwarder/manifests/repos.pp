class fmr_splunk_forwarder::repos {
   yumrepo { 'common-artifacts': 
      descr => "Common artifacts - extras",
      baseurl => "http://artifacts.hi.inet/common_re",
      gpgcheck => 0,
      enabled => 1,
   }
}

