rsync -Pav --delete  hieradata/* /usr/local/iot/puppet/hieradata/
rsync -Pav --delete  modules/* /usr/local/iot/puppet/modules
rsync -Pav --delete  manifests/* /usr/local/iot/puppet/manifests

puppet apply  --hiera_config /etc/puppet/hiera.yaml --modulepath=/usr/local/iot/puppet/modules /usr/local/iot/puppet/manifests/site.pp
