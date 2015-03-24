class common::users::epg (
  $ensure,
) {
  group { 'wheel':
    ensure => $ensure,
  } ->
  user { 'epg':
    ensure     => $ensure,
    comment    => 'EPG System user',
    groups     => 'wheel',
    home       => '/home/epg',
    managehome => true,
    shell      => '/bin/bash',
    password   => '$6$iijCH30aK6CEY7U/$iRkbS8yCcfVFByTjMHJpWPTXwPjkNh25GN3fg3RKZ//YljQl0SXkVGBjF8XhpDCrSxQ3lqB87kSJvzaMfYLHn/',
    # You can use grub-crypt to get the hash
  } ->
  ssh_authorized_key { 'root@donramon':
    ensure => $ensure,
    key    => 'AAAAB3NzaC1yc2EAAAABIwAAAQEA3yQfFIBjnhPj+Vihufjyc1lwH7mXP3MsIVHGCIlJyJUwhwK86kgt8Q3hLsyFvq+aprANspJNwM7fZLQ4emAniasWktDQq6BU+GfhTx7rys3XuHqLEh5b0SmVnY82X7vHxES+yoa/5C3tqWUFfQHMfoSuQnrt2E4YWGyFJ68wXeft47fdNR+iJ5hN+PE+3CDWP5Ed6TU9Hmt2K+PTKhpbSRx272PsMeC8kY84GIp7nOgEVUAoixI1JJEaVKfxGjuXuokYMQV0SJaMp5eFXslMjbtJetzkAI1l6Kr+awoawBj+RdYtKRzrXoNZ0AZKLOEDauVYMI7Brw76vLLegP2DYw==',
    type   => 'rsa',
    user   => 'epg',
  } ->
  common::managesudoer  { 'epg':
    ensure  => $ensure,
    command => 'ALL=(ALL) NOPASSWD:ALL',
  }
}
