class tdaf_base::screen {
    package { 'screen':
        ensure => present,
    }

    file { '/root/.screenrc':
        ensure => present,
        owner  => 'root',
        group  => 'root',
        mode   => '0440',
        source => 'puppet:///modules/tdaf_base/screenrc',
    }

}
