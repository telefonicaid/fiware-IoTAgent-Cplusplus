class tdaf_mongodb::sources::apt inherits tdaf_mongodb::params {
  include apt

  if $tdaf_mongodb::location {
    $location = $tdaf_mongodb::location
  } else {
    $location = $tdaf_mongodb::params::locations[$tdaf_mongodb::init]
  }

  apt::source { '10gen':
    location    => $location,
    release     => 'dist',
    repos       => '10gen',
    key         => '7F0CEB10',
    key_server  => 'keyserver.ubuntu.com',
    include_src => false,
  }
}
