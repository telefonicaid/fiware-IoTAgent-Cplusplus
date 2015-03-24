class common::vim ($colorscheme = 'koehler') {
 
  package { [
    'vim-minimal',
    'vim-enhanced',
    'vim-common',] :
      ensure => latest,
  }

  file {'/etc/vimrc':
    content => template('common/vimrc.erb'),
  }

}
