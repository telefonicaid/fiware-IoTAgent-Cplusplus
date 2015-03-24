mkdir -p /usr/local/dca/puppet/modules
mkdir -p /usr/local/dca/puppet/manifests
mkdir -p /usr/local/dca/puppet/hieradata
mkdir -p /etc/puppet/
                         
rsync -Pav modules/* /usr/local/dca/puppet/modules                      
rsync -Pav  manifests/* /usr/local/dca/puppet/manifests/                     
rsync -Pav hieradata/* /usr/local/dca/puppet/hieradata/
rsync -Pav  pack/hiera.yaml /etc/puppet/
ln -sf /usr/local/dca/puppet/hieradata /var/lib/hiera
mkdir -p /var/lib/puppet/state
touch /var/lib/puppet/state/last_run_report.yaml

