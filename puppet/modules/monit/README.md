MONIT MODULE
==========

A simplemonitor module whose purpose is:
* To install the monit package
* To offer client modules a facility to automatically add monitored processes to ```/etc/monit.d/```. Client modules do not have to bother with dependencies. Monit is restarted for each new addition.

# 1. Usage

## Module Parameters
The monit module has the following parameters:
1. ```$period``` . Monitoring period
2. ```$port``` HTTP Port to listen to when receiving management commands **on localhost only**.
3. ```$login``` HTTP username used for management purposes
4. ```$passwd``` HTTP password used for management purposes
5. ```$statefile``` where monit stores states between runs. In HA set up, should be under /tmp (default)
5. ```$package_ensure```  to account for the different package names in EPG or DSN repositories.

Default values are found in class monit::parmas and can be overriden by Hiera config using the automatic resolution facility by declaring in the YAML file:

```
---
monit::port           : 2812
monit::period         : 60
monit::package_ensure : 'x.y.z' 
```

## 1.1 Adding monitored processes specfiles

Use the following construct:

```
monit::watch{ 'keystone-proxy.conf':
  start_cmd => '/usr/bin/node /opt/kesytoneproxy/server.js'
  stop_cmd => '/usr/bin/pkill node'
}
```

## 1.2 Removing monitored process specifles

Use the following construct:

```
monit::watch{ 'keystone-proxy.conf':
  ensure => absent
}
```


