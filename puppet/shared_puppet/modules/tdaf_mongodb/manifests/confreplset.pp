define tdaf_mongodb::confreplset(
    $db_template_nodes
)
{
    file{'/root/mongo_replSet_init.js':
        content => template("$module_name/replSet_init.js.erb"),
        owner   => 'root',
        group   => 'root',
        mode    => '0644',
    }
    
    exec{'init_replSet':
        command => 'mongo < /root/mongo_replSet_init.js | grep -q \'"ok" : 1\' && touch /root/rsCreated',
        path => '/bin:/usr/bin',
        creates => '/root/rsCreated',
        require => File['/root/mongo_replSet_init.js'],
    }
    
    
}
