# == Class: puppetdb
#
# This class installs and configures puppetdb. Currently it is only supported
# to run on the same host as puppet master.
#
# This module needs to be extended if you want to be able to provision PuppetDB
# on a separate host than puppet master.
#
# === Parameters
#
# [*vardir*]
#   Where to store MQ/DB data
#   Default: <code>/var/lib/puppetdb</code>
#
# [*logging_config*]
#   Use an external log4j config file.
#   Default: <code>/etc/puppetdb/logback.xml</code>
#
# [*resource_query_limit*]
#   Maximum number of results that a resource query may return
#   Default: 20000
#
# [*threads*]
#   How many command-processing threads to use, defaults to (CPUs / 2), so
#   It is set to <code>undef</code> here, because puppetdb process does
#   calculation itself. It can be specified to whatever number you want if
#   needed.
#
# [*db_subprotocol*]
#   What database backend protocol should be used.
#   Valid values: <code>hsqldb</code> or <code>postgresql</code>
#   Default: <code>hsqldb</code> - embedded db
#
# [*psql_host*]
#   Postgresql database host.
#   Default: <code>undef</code>
#
# [*psql_username*]
#   Connect to psql database as a specific user.
#   Default <code>undef</code>
#
# [*psql_password*]
#   Connect to psql database with a specific password.
#   Default: <code>undef</code>
#
# [*gc_interval*]
#   How often in minutes to compact the database.
#   Default: <code>60</code>
#
# [*node_ttl*]
#   Auto-deactivate nodes that haven't seen any activity (no new catalogs,
#   facts, etc) in the specified amount of time.
#     Valid values:
#       `d`  - days
#       `h`  - hours
#       `m`  - minutes
#       `s`  - seconds
#       `ms` - milliseconds
#
#   Default: <code>undef</code>.
#
# [*log_slow_statements*]
#   Number of seconds before any SQL query is considered 'slow'.
#   Default: <code>10</code>
#
# [*jvm_heap_size*]
#   JVM heap size for PuppetDB. It accepts memory size with a letter
#   m for megabytes, g for gigabytes, etc.
#   Default: <code>512m</code>
#
# === Requires
#
# None
#
# === Examples
#
#     ---
#     classes:
#       - puppetdb
#
#     puppetdb::threads: '4'
#
# === Authors
#
# - Vaidas Jablonskis <jablonskis@gmail.com>
# - Xavi Carrillo <xavi.carrillo@gmail.com>
#


# Set ssl_host to false to have a standalone non-ssl puppetdb server (for masterless puppet setups)

class puppetdb(
    $ensure               = running,
    $enable               = true,
    $vardir               = '/var/lib/puppetdb',
    $logging_config       = '/etc/puppetdb/logback.xml',
    $resource_query_limit = '20000',
    $threads              = undef,
    $db_subprotocol       = 'hsqldb',
    $psql_host            = undef,
    $psql_username        = undef,
    $psql_password        = undef,
    $gc_interval          = '60',
    $node_ttl             = undef,
    $node_purge_ttl       = undef,
    $log_slow_statements  = '10',
    $jvm_heap_size        = '512m',
    $log_level            = 'WARN',
    $ssl_host             = $hostname, 
    $store_usage          = '1000',
    $temp_usage           = '1000',
    $version,
  ) {
  case $::operatingsystem {
    CentOS, RedHat: {
      $package_name = 'puppetdb'
      $service_name = 'puppetdb'
    }
    default: {
      fail("Module ${module_name} is not supported on ${::operatingsystem}")
    }
  }

  $config_file         = '/etc/puppetdb/conf.d/config.ini'
  $db_config_file      = '/etc/puppetdb/conf.d/database.ini'
  $conf_template       = 'config.ini.erb'
  $sysconfig_file      = '/etc/sysconfig/puppetdb'
  $sysconf_template    = 'sysconfig_puppetdb.erb'
  $db_conf_template    = 'database.ini.erb'
  $store_password_file = '/etc/puppetdb/ssl/puppetdb_keystore_pw.txt'

  user { 'puppetdb':
    ensure => present,
  } ->
  group { 'puppetdb':
    ensure => present,
  } ->
  package {
    'puppetdb':          ensure => $version;
    'puppetdb-terminus': ensure => $version;
  }

  service { $service_name:
    ensure     => $ensure,
    enable     => $enable,
    hasrestart => true,
    hasstatus  => true,
    require    => Package[$package_name],
  }


  file { '/etc/puppet/puppetdb.conf':
    ensure  => file,
    owner   => 'puppetdb',
    group   => 'puppetdb',
    mode    => '0644',
    content => "[main]\n  port = 8081\n  server = ${ssl_host}\n",
    notify  => Service[$service_name],
  }

  file { $config_file:
    ensure  => file,
    require => Package[$package_name],
    owner   => 'puppetdb',
    group   => 'puppetdb',
    mode    => '0640',
    content => template("${module_name}/${conf_template}"),
    notify  => Service[$service_name],
  }

  file { $sysconfig_file:
    ensure  => file,
    require => Package[$package_name],
    owner   => 'root',
    group   => 'root',
    mode    => '0640',
    content => template("${module_name}/${sysconf_template}"),
    notify  => Service[$service_name],
  }

  file { $db_config_file:
    ensure  => file,
    require => Package[$package_name],
    owner   => 'puppetdb',
    group   => 'puppetdb',
    mode    => '0640',
    content => template("${module_name}/${db_conf_template}"),
    notify  => Service[$service_name],
  }

  file { '/etc/puppetdb/conf.d/jetty.ini':
    ensure  => file,
    require => Package[$package_name],
    owner   => 'puppetdb',
    group   => 'puppetdb',
    mode    => '0640',
    content => template("${module_name}/jetty.ini.erb"),
    notify  => Service[$service_name],
  }

  file { '/etc/puppet/routes.yaml':
    ensure  => file,
    owner   => 'root',
    group   => 'root',
    mode    => '0644',
    source  => 'puppet:///modules/puppetdb/routes.yaml',
    notify  => Service[$service_name],
  }

  # jdk 1.6 is deprecated
  file { '/etc/alternatives/java':
    ensure => link,
    target => '/usr/lib/jvm/jre-1.7.0-openjdk.x86_64/bin/java',
  }

  exec { 'create_ssl_certs':
    command => '/usr/sbin/puppetdb ssl-setup',
    creates => '/etc/puppetdb/ssl/ca.pem',
  }

  if ($db_subprotocol == 'postgresql') {
    # Check what database backend is configured to be used, if psql, fail if
    # psql credentials are not set.
    if ($psql_host == undef) or
      ($psql_username == undef) or
      ($psql_password == undef) {
        fail('Incomplete postgresql configuration.')
    }
    #exec { 'postgre_initdb':
    #  command => '/sbin/service postgresql initdb',
    #  creates => '/var/lib/pgsql/data/pg_log/',
    #  before  => File['pg_hba.conf'],
    #}
    class { 'postgresql::client': } ->
    class { 'postgresql::server':
      manage_pg_hba_conf => false,
    }
    postgresql::server::db { 'puppetdb':
      user     => 'puppetdb',
      password => postgresql_password('puppetdb', 'puppetdb'),
    }
    # In order to make the puppetdb user to authenticate with no password on localhost
    file { 'pg_hba.conf':
      path    => '/var/lib/pgsql/data/pg_hba.conf',
      owner   => 'postgres',
      group   => 'postgres',
      source  => 'puppet:///modules/puppetdb/pg_hba.conf',
      #require => Exec['postgre_initdb'],
    }
  }
}
