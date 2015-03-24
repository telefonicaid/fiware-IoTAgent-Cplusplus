logrotate::rule { 'messages':
  path         => '/var/log/messages',
  rotate       => 5,
  rotate_every => 'week',
  postrotate   => '/usr/bin/killall -HUP syslogd',
}

logrotate::rule { 'apache':
  path          => '/var/log/httpd/*.log',
  rotate        => 5,
  mail          => 'test@example.com',
  size          => '100k',
  sharedscripts => true,
  postrotate    => '/etc/init.d/httpd restart',
}


logrotate::easy_rule {'listener.log':
  name => 'listener.log',
  rotate_every => 'daily',
}

logrotate::easy_rule {'consumer_requests_size.log':
  name => 'consumer_requests_size.log',
  size  => '500M'
}