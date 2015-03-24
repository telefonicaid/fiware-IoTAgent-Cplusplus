class tdaf_base::vim {
    package {'vim-enhanced': ensure => installed }
    file { '/root/.vimrc':
        ensure => present,
        owner  => 'root',
        group  => 'root',
        mode   => '0440',
        source => 'puppet:///modules/tdaf_base/vimrc',
    }

}
