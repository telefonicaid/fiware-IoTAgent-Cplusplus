class cosmetic {
  File {
    owner => 'root',
    group => 'root',
  }
  file { '/root/.bash_profile':
    source => 'puppet:///modules/cosmetic/bash_profile',
  }
  file { '/root/.git-prompt.sh':
    source => 'puppet:///modules/cosmetic/git-prompt.sh',
  }
  file { '/etc/profile.d/history.sh':
    source => 'puppet:///modules/cosmetic/profile.d/history.sh',
  }
}
 
