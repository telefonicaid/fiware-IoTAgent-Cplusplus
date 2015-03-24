define concatfile::part(
  $source = "",
  $content = "",
  $file = "",
  $owner = "",
  $group = "",
  $mode = ""

) {

  $dir = dirname($name)

  file {
    $name:
      require => File[$dir],
      notify => Exec[$dir],
  }

  if $owner != "" {
    File[$name] { owner => $owner }
  }
  if $group != "" {
    File[$name] { group => $group }
  }
  if $mode != "" {
    File[$name] { mode => $mode }
  }

  # This is why we should have an elsif ;-)
  if ($source != "" and $content == "" and $file == "") {
    File[$name] { source => $source }
  } else {
    if ($source == "" and $content != "" and $file == "") {
      File[$name] { content => $content }
    } else {
      if ($source == "" and $content == "" and $file != "") {
        File[$name] { ensure => $file }
      } else {
        if ($source == "" and $content == "" and $file == "") {
          File[$name] { ensure => present }
        } else {
          err "Only one of the parameters source, content, file can be specified."
        }
      }
    }
  }

}
