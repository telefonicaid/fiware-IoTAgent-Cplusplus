 
$db_nodes = [['10.95.31.228'],['10.95.14.225'],['10.95.11.26']]

class { 'tdaf_mongodb':
    servicename => 'mongod',
    dbpath => '/home/mongo/shard',
    logpath => '/var/log/mongo/shard.log',
    bind_ip => '127.0.0.1',
    port => 1000,
    mongofork => true,
    shardsvr => true,
    replSet => 'shard',
    pidfilepath => '/var/run/mongo/shard.pid',
    enable_10gen => true,
    nojournal => true,
    db_template_nodes => [
    [$db_nodes[0][0],'27017',''],
    [$db_nodes[1][0],'27017',''],
    [$db_nodes[2][0],'27017',', arbiterOnly: true '],
  ],
}
