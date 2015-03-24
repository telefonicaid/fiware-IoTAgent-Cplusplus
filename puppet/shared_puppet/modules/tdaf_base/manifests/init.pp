class tdaf_base {
  include tdaf_base::vim
  include tdaf_base::screen
  include tdaf_base::packages
  include tdaf_base::patch_selinux
  
  class{ 'ntp': }
  
}

