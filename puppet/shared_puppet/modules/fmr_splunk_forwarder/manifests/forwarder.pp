class fmr_splunk_forwarder::forwarder (
   $server_ipport = "172.31.0.224:9997",
   $max_queue_size = 50,
) {
   include repos

   package {'splunkforwarder':
      ensure => installed,
      require => Yumrepo['common-artifacts'],
   }

   splunk_input {'/var/log/httpd/api/error_log':
      sourcetype => 'apitestaccess',
      index      => 'tugoindex',
   }

   file {'/opt/splunkforwarder/etc/system/local/inputs.conf':
      content => template('splunk_forwarder/inputs.conf.erb'),
   }

}
