# To add a new sudoer user, just do like so within your node definition:
#
#  class { 'common::sudoers':
#    name    => 'sysadmin',
#    command => 'ALL=(ALL) NOPASSWD:ALL',
#  }
#
# Or if you also need a command alias:
#
#  class { 'common::sudoers':
#    name       => 'icinga',
#    cmnd_alias => 'Cmnd_Alias ICOMM = /usr/local/bin/vpnc-start, /usr/bin/vpnc-disconnect , /usr/bin/vpnc',
#    command    => 'ALL=(ALL) NOPASSWD:ALL',
#  }
#
# The default is to add the user, but you can also remove it:
#  class { 'common::sudoers':
#    name    => 'sysadmin',
#    ensure  => 'absent',
#  }
#
class common::sudoers (
  $ensure     = 'present',
  $name,
  $cmnd_alias = '',
  $command    = '',
) {
  file { "${name}":
    ensure  => $ensure,
    path    => "/etc/sudoers.d/${name}",
    owner   => 'root',
    mode    => '0440',
    content => "$cmnd_alias\n${name}  ${command}\n",
  }
}
