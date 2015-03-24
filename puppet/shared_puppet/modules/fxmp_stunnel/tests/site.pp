node default {
  include fxmp_stunnel
  fxmp_stunnel::Tun {
    require => Package[$fxmp_stunnel::data::package],
    notify => Service[$fxmp_stunnel::data::service],
  }
  fxmp_stunnel::tun { 'rsyncd':
    certificate => "/etc/puppet/ssl/certs/${::clientcert}.pem",
    private_key => "/etc/puppet/ssl/private_keys/${::clientcert}.pem",
    client      => false,
    accept      => '1873',
    connect     => '873',
  }
  fxmp_stunnel::tun { 'rsync':
    certificate => "/etc/puppet/ssl/certs/${::clientcert}.pem",
    private_key => "/etc/puppet/ssl/private_keys/${::clientcert}.pem",
    client      => true,
    accept      => '1874',
    connect     => 'server.example.com:1873',
  }
}
