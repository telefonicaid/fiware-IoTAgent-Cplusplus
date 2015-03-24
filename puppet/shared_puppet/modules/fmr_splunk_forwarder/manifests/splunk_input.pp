define fmr_splunk_forwarder::splunk_input (
   $monitor = $title,
   $sourcetype,
   $index
) {

   notice("sourcetype $sourcetype")

   augeas {"$title":
      lens    => "Puppet.lns",
      incl    => "/opt/splunkforwarder/etc/system/local/inputs.conf",
      changes => [
         "set /monitor/sourcetype $sourcetype",
      ],
   }

} 
