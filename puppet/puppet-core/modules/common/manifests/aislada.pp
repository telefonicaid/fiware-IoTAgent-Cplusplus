class common::aislada {
  if ($::fqdn =~ /aislada/ ) {
  #Since the 'servidores' network doesn't share the DNS of the 'interna', we must use the old-school /etc/hosts...

    host { 'localhost':
      ip => '127.0.0.1',
    }
    host { 'servilinux.hi.inet':
      ip => '10.95.67.227',
      host_aliases => [ 'ntp-server.hi.inet', 'repos.hi.inet', 'artifacts', 'artifacts.hi.inet' ],
    }
    host { 'repositorio.aislada':
      ip => '10.95.93.230',
      host_aliases => [ 'repositorio.aislada.hi.inet' ],
    }
    host { 'puppet-code.aislada':
      ip => '10.95.93.176',
      host_aliases => [ 'puppet-code.aislada.hi.inet', ],
    }
    host { 'puppet.aislada':
      ip => '10.95.92.231',
      host_aliases => [ 'puppet.aislada.hi.inet', 'puppet-01.aislada', 'puppet' ],
    }
    host { 'backup.aislada':
      ip => '10.95.93.171',
      host_aliases => [ 'backup.aislada.hi.inet' ],
    }
    host { 'mailhost':
      ip => '10.95.64.100',
      host_aliases => [ 'mailhost.hi.inet' ],
    }

  }
}
