define logrotate::easy_rule(
   $name            = 'undef',
   $size            = 'undef',
   $rotate_every    = 'undef'
) 
{
  logrotate::rule { $name:
      path         => "/var/log/$name",
      create => true,
      notifempty => true,
      rotate_every => $rotate_every,
      rotate       => 7,
      missingok => true,
      compress => true,
      copytruncate => true,
      create_mode => '644',
      size => $size,
  }


  cron { $name:
      command => "/usr/sbin/logrotate /var/log/$name",
      user    => root,
    # hour    => 2,
      minute  => '*/30',
  }
}
