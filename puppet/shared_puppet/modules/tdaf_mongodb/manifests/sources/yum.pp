class tdaf_mongodb::sources::yum (
	$url_repo  =  $tdaf_mongodb::params::baseurl ,
	$proxy_url = '',
)
inherits tdaf_mongodb::params {
	yumrepo { '10gen':
		baseurl   => $url_repo,
		gpgcheck  => '0',
		enabled   => '1', 
	}
	if $proxy_url != '' {
		Yumrepo <| tag == '10gen' |> {
			proxy      => $proxy_url,
		}
	}
}
