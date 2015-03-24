Simple mysql puppet module

This module is used to install MySQL databases. Usually, mysql modules are complex and with lots of depencies. This module aims exactly the opposite.

Don't forget to add this to /etc/puppet/manifests/site.pp:

$mysql_password = "secret"


