define apache::redirect::rewrite ($target) {

  $virtualhost=$name

  include apache
  include apache::mod::proxy

  #package { 'mod_proxy_html': ensure => latest }

  #NameVirtualHost *:80 must be enabled on httpd.conf

  $content = "
    <VirtualHost *:80>
      ServerName ${virtualhost}
      <IfModule !proxy_http_module>
        LoadModule proxy_http_module modules/mod_proxy_http.so
      </IfModule>
      ErrorLog /var/log/httpd/${virtualhost}_error.log
      #CustomLog /var/log/httpd/${virtualhost}_access.log common
      RewriteEngine on
      RewriteRule /(.*) ${target}/\$1 [P,L]
      <Location />
        Order allow,deny
        Allow from all
      </Location>
    </VirtualHost>"

  file { "${virtualhost}.conf":
    ensure  => present,
    path    => "/etc/httpd/conf.d/${virtualhost}.conf",
    owner   => 'root',
    mode    => 0644,
    content => "${content}",
    notify  => Service['httpd'],
  }
}

