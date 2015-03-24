monit::monitor { 'ssh':
  pidfile => '/var/run/sshd.pid',
}
