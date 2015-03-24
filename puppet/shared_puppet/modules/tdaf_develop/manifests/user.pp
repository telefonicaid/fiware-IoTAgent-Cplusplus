define tdaf_develop::user (
  $userid   = $title,
  $homepath = "/home/$title",
) {  
  group { $userid:
  	ensure  => present
  }
  user {$userid:
    ensure     => 'present',
    home       => "/home/$userid",
    shell      => '/bin/bash',
    managehome => true,
    gid     => $userid,
  }
   
  file {"$homepath/.gitconfig":
    owner    => $userid,
    group    => $userid,
    backup   => false,
    source   => 'puppet:///modules/tdaf_develop/.gitconfig',
    require  => User[$userid],
  } 
  file {"$homepath/.git-completion.bash":
    owner    => $userid,
    group    => $userid,
    backup   => false,
    source   => 'puppet:///modules/tdaf_develop/.git-completion.bash',
    require  => User[$userid],

  }
  file {"$homepath/.git-prompt.sh":
    owner    => $userid,
    group    => $userid,
    backup   => false,
    source   => 'puppet:///modules/tdaf_develop/.git-prompt.sh',
    require  => User[$userid],

  }
  file {"$homepath/.bashrc":
    owner    => $userid,
    group    => $userid,
    backup   => false,
    source   => 'puppet:///modules/tdaf_develop/.bashrc',
    require  => User[$userid],
  }
  
  file {"$homepath/.bash_profile":
    owner    => $userid,
    group    =>   $userid,
    backup   => false,
    source   => 'puppet:///modules/tdaf_develop/.bash_profile',
    require  => User[$userid],
  }
  
  file {"$homepath/.ssh/config":
    owner    => $userid,
    group    => $userid,
    backup   => false,
    source   => 'puppet:///modules/tdaf_develop/.ssh_config',
    require  => User[$userid],
  }
  
  file {"$homepath/.ssh/id_dsa_contint":
    owner    => $userid,
    group    => $userid,
    mode     => '0600',
    backup   => false,
    source   => 'puppet:///modules/tdaf_develop/.ssh_id_dsa_contint',
    require  => User[$userid],
  }

  file {"$homepath/.ssh/tdafDEV-key.pem":
    owner    => $userid,
    group    => $userid,
    mode     => '0600',
    backup   => false,
    source   => 'puppet:///modules/tdaf_develop/tdafDEV-key.pem',
    require  => User[$userid],
  }

  file {"$homepath/.ssh/known_hosts":
    owner    => $userid,
    group    => $userid,
    mode     => '0600',
    backup   => false,
    source   => 'puppet:///modules/tdaf_develop/.ssh_known_hosts',
    require  => User[$userid],
  }
  
  file { "$homepath/bin":
    ensure => "directory",
    owner    => $userid,
    group    => $userid,
    backup   => false,
    require  => User[$userid],
  }

    file { "$homepath/.ssh":
    ensure => "directory",
    owner    => $userid,
    group    => $userid,
    mode     => '0600',
    backup   => false,
    require  => User[$userid],
  }

}