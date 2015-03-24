#!/bin/bash

if [[ $1 == 'test' ]]
then
  PUPPET_MASTER='puppetmaster-test.hi.inet'
else
  PUPPET_MASTER='puppet.aislada'
fi

if [[ `grep 5. /etc/redhat-release 2>/dev/null` ]]
then
  OS='RHEL'
  VERSION=5
elif [[ `grep 6. /etc/redhat-release 2>/dev/null` ]]
then
  OS='RHEL'
  VERSION=6
elif [[ `grep Ubuntu /etc/issue 2>/dev/null` ]]
then
  OS='Ubuntu'
  UBUNTU_RELEASE=$( lsb_release -c -s )
fi

if [[ $OS == 'RHEL' ]]
then
  curl http://servilinux.hi.inet/repos/el6/epg-epel.repo -o /etc/yum.repos.d/epg-epel.repo
  curl http://servilinux.hi.inet/repos/el6/epg-rhel.repo -o /etc/yum.repos.d/epg-rhel.repo
  curl http://servilinux.hi.inet/repos/el6/epg-puppet.repo -o /etc/yum.repos.d/epg-puppet.repo
  curl http://servilinux.hi.inet/repos/el6/epg-java.repo -o /etc/yum.repos.d/epg-java.repo
  curl http://servilinux.hi.inet/repos/el6/rhel-optional.repo -o /etc/yum.repos.d/rhel-optional.repo

  yum clean all
  yum install puppet -y --nogpgcheck
  service puppet stop
  puppet agent --test --debug --server $PUPPET_MASTER --environment default --enable
  puppet agent --test --debug --server $PUPPET_MASTER --environment default
  puppet --version
  service puppet status
elif [[ $OS == 'Ubuntu' ]]
then
  curl http://apt.puppetlabs.com/pubkey.gpg -q -O- | apt-key add -
  curl http://apt.puppetlabs.com/keyring.gpg  | apt-key add -
  curl http://servilinux.hi.inet/repos/ubuntu/epg-puppet.list | sed s/UBUNTU_RELEASE/$UBUNTU_RELEASE/g > /etc/apt/sources.list.d/epg-puppet.list
  apt-get update
  apt-get install -y puppet
  puppet agent --test --debug --server $PUPPET_MASTER --environment default
fi

