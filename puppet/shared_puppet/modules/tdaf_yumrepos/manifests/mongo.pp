class yumrepos::mongo {
    file {
    '/etc/yum.repos.d/mongo-10gen.repo':
        ensure => present,
        owner  => 'root',
        group  => 'root',
        mode   => '0644',
        source => 'puppet:///modules/yumrepos/mongo-10gen.repo',
        notify => Exec['clean_yum_cache'];
    }
}
