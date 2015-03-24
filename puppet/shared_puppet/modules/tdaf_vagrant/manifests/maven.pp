class tdaf_vagrant::maven {
  if ! defined(Package['jdk']) {
    package{'jdk': ensure => '1.7.0_25-fcs'}
  }
  if ! defined(Package['wget']) {
    package{'wget': ensure => present}
  }
  $mavenURI = 'http://ftp.cixug.es/apache/maven/maven-3/3.0.5/binaries/apache-maven-3.0.5-bin.tar.gz'
  exec{'remote_file_maven':
    command => "/usr/bin/wget $mavenURI",
    cwd => '/tmp',
    user => 'root',
    creates => '/tmp/apache-maven-3.0.5-bin.tar.gz',
    require => [Package['jdk'],Package['wget']]
  }
  exec{'install_maven':
    command => '/bin/tar -xzf /tmp/apache-maven-3.0.5-bin.tar.gz',
    user => 'root',
    cwd => '/opt',
    creates => '/opt/apache-maven-3.0.5/bin/mvn',
    require => Exec['remote_file_maven'],
  }
  
  file{'/etc/profile.d/mvn.sh':
    owner   => 'root',
    group   => 'root',
    source  => 'puppet:///modules/tdaf_vagrant/mvn.sh',
    require => Exec['unzip-ec2tools'],
  }
}