require 'spec_helper'

describe 'monit', :type => :class do
  context "osfamily = RedHat" do
    let :facts do
      {
        :osfamily        => 'RedHat',
        :lsbdistid       => 'RedHat',
        :kernel          => 'Linux',
        :operatingsystem => 'CentOS',
      }
    end

    context "default usage (osfamily = RedHat)" do
      let(:title) { 'monit-syslog' }

      let :params do
        {
          :logfile => 'blah syslog blah',
        }
      end

      it 'should compile' do
        should_not contain_file('/etc/logrotate.d/monit')
        should contain_file('/etc/monit.conf').with({
          'content' => /^set logfile blah syslog blah$/,
        })
      end
    end

    context "default usage (osfamily = RedHat)" do
      let(:title) { 'monit-basic' }

      it 'should compile' do
        should contain_file('/etc/monit.conf')
        should contain_file('/etc/monit.d')
        should contain_file('/etc/logrotate.d/monit')
        should contain_service('monit')
      end

      it 'should have service status' do
        should contain_service('monit').with({
          'hasstatus' => true,
        })
      end
    end
  end

  context "lsbdistcodename = squeeze" do
    let :facts do
      {
        :osfamily        => 'Debian',
        :operatingsystem => 'Debian',
        :lsbdistcodename => 'squeeze'
      }
    end

    it 'should not have service status' do
      should contain_service('monit').with({
        'hasstatus' => false,
      })
    end
  end

  context "lsbdistcodename = wheezy" do
    let :facts do
      {
        :osfamily        => 'Debian',
        :operatingsystem => 'Debian',
        :lsbdistcodename => 'wheezy'
      }
    end

    it 'should have service status' do
      should contain_service('monit').with({
        'hasstatus' => true,
      })
    end
  end

  context "osfamily = Debian" do
    let :facts do
      {
        :osfamily        => 'Debian',
        :lsbdistid       => 'Debian',
        :kernel          => 'Linux',
        :operatingsystem => 'Debian',
      }
    end

    context "default usage (osfamily = Debian)" do
      let(:title) { 'monit-basic' }

      it 'should compile' do
        should contain_file('/etc/monit/monitrc')
        should contain_file('/etc/monit/conf.d')
        should contain_file('/etc/logrotate.d/monit')
        should contain_service('monit')
      end
    end
  end
end
