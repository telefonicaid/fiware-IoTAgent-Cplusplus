# To add a new sudoer user, just do like so within your node definition:
#
#  managesudoer  { 'sysadmin':
#    command => 'ALL=(ALL) NOPASSWD:ALL',
#  }
#
# Or if you also need a command alias:
#
#  managesudoer { 'icinga':
#    cmnd_alias => 'Cmnd_Alias ICOMM = /usr/local/bin/vpnc-start, /usr/bin/vpnc-disconnect , /usr/bin/vpnc',
#    command    => 'ALL= NOPASSWD:ICOMM',
#  }
#
# The default is to add the user, but you can also remove it:
#  managesudoer  { 'icinga':
#    ensure => 'absent',
#  }
#

define common::managesudoer ( $ensure = 'present', $cmnd_alias = '', $command = '' ) {
  file { "${name}":
    ensure  => $ensure,
    path    => "/etc/sudoers.d/${name}",
    owner   => 'root',
    mode    => '0440',
    content => "$cmnd_alias\n${name}  ${command}\n",
  }
}
