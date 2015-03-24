define logrotate::specfile(
  $ensure   = 'file', 
  $content  = undef,
  $source   = undef,
) {

  include logrotate

  if $ensure != 'absent' {
    if $content and $source {
      fail('You may not supply both content and source parameters to logrotate::specfile')
    } elsif $content == undef and $source == undef {
      fail('You must supply either the content or source parameter to logrotate::specfile')
    }
  }
 
  file { "${logrotate::config_basedir}/${title}":
    ensure  => $ensure,
    owner   => root,
    group   => root,
    mode    => 0640,
    content => $content,
    source  => $source,
    require => File[$logrotate::config_basedir],
    notify  => Class['logrotate::service'],
  }
}