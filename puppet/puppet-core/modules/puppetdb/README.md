# puppetdb
This module installs and configures puppetdb. Currently, puppetdb is only
supported to be run on the same host as puppet master.

This module needs to be extended if you want to be able to provision PuppetDB
on a separate host than puppet master. I may get this fixed one day.

## Parameters
### puppetdb class

* `vardir` - where to store MQ/DB data. Default: `/var/lib/puppetdb`.

* `logging_config` - use an external log4j config file. Default:
`/etc/puppetdb/log4j.properties`.

* `resource_query_limit` - maximum number of results that a resource query may
return. Default: `20000`.

* `threads` - how many command-processing threads to use, defaults to (CPUs / 2).
It is set to `undef` by default, because puppetdb process does calculation
itself. It can be specified to whatever number you want if needed.

* `db_subprotocol` - what database backend protocol should be used. Valid
values: `hsqldb` or `postgresql`. Default: `hsqldb` - embedded db.

* `psql_host` - postgresql database host. Default: `undef`.

* `psql_username` - connect to psql database as a specific user.
Default: `undef`.

* `psql_password` - connect to psql database with a specific password.
Default: `undef`.

* `gc_interval` - how often in minutes to compact the database.
Default: `60`.

* `node_ttl` - auto-deactivate nodes that haven't seen any activity
(no new catalogs, facts, etc) in the specified amount of time. Valid values:
`d`  - days, `h`  - hours, `m`  - minutes, `s`  - seconds, `ms` - milliseconds.
Default: `undef`.

* `log_slow_statements` - number of seconds before any SQL query is considered
'slow'. Default: `10`.

* `jvm_heap_size` - JVM heap size for PuppetDB. It accepts memory size with a
letter suffix - `m` for megabytes, `g` for gigabytes, etc. Default: `512m`.

## Requires
- Repository added with puppetdb packages
- CentOS/RHEL/Fedora distribution

## Examples
Below example shows how to use this module with hiera + yaml backend.

    ---
    classes:
      - puppetdb
    
    puppetdb::threads:       '4'
    puppetdb::jvm_heap_size: '1024m'

## Authors
- Vaidas Jablonskis <jablonskis@gmail.com>

Many modifications made by Xavi Carrillo
