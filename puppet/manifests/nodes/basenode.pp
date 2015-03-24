node 'default' {



class {'mongodb::globals':
    server_package_name => 'mongo-10gen-server',
    user                => 'mongod',
    group               => 'mongod',
    version             => '2.4.9-mongodb_1',
    manage_package_repo => false,
  }->
  class {'mongodb::server':
    logpath    => '/var/log/mongo/mongod.log',
    config     => '/etc/mongod.conf',
    bind_ip    => ['0.0.0.0'],
    #default port is 27017, 27018 is used for sharding, typically
    port    => hiera('iota::base::mongodb_port'),
    verbose => true,
  }


  
}
node basenode {
#
#  $puppet_environment    = 'dca-test1'
#  $puppet_server         = 'puppet.aislada'
#  $ssh_permit_root_login = 'no'

  ## Setting order of execution:

}
