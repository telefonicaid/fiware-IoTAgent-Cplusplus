define concatfile(
  $dir,
  $owner = "",
  $group = "",
  $mode = ""
) {

  if $owner != "" {
    File { owner => $owner }
  }
  if $group != "" {
    File { group => $group }
  }
  if $mode != "" {
    File { mode => $mode }
  }

  file {
    $dir:
      ensure => directory,
      checksum => mtime,
      recurse => true,
      purge => true,
      notify => Exec[$dir];
    $name:
      ensure => present,
      checksum => md5,
      notify => Exec[$dir];
  }

  exec {
    $dir:
      path => ['/usr/bin','/bin'],
      command => "find $dir -maxdepth 1 \\( -type f -or -type l \\) -print0 | sort -nz | xargs -0 cat > $name",
      refreshonly => true;
  }

}
