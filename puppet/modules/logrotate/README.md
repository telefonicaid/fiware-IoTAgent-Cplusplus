LOGROTATE MODULE
================

A simple logrotate module whose purpose is:
* To install the logrotate package
* To offer client modules a facility to automatically manage logrotate specfiles located in ```/etc/logrotate.d```. Client modules do not have to bother with dependencies.

# 1. Usage

## Module Parameters
The logrotate module has the following parameters:
1. ```$package_ensure```, to account for the different package names in EPG or DSN repositories. 

Default values are found in class monit::parmas and can be overriden by Hiera config using the automatic resolution facility by declaring in the YAML file:


```
---
logrotate::package_ensure : 'x.y.z" ```

## Adding logrotate spec files

Use the following construct:

```
logrotate::specfile{ 'myfile'
  source => puppet:///<my source Puppet URL for static file>
}
```

or if you file is a template:

```
logrotate::specfile{ 'myfile' :
  content => template("$module_name/my_template.erb"),
}
```
**Important note:** ```'myfile'``` must not contain an absolute path, only the file name.

## Removing logrotate spec files

Use the following construct:

```
logrotate::specfile{ 'myfile' :
  ensure => absent
}
```
**Important note:** ```'myfile'``` must not contain an absolute path, only the file name.

