require 'spec_helper'

describe 'monit::monitor', :type => :define do
  context "osfamily = RedHat" do
    let :facts do
      {
        :osfamily        => 'RedHat',
        :lsbdistid       => 'CentOS',
        :kernel          => 'Linux',
        :operatingsystem => 'CentOS',
      }
    end

    context "both pidfile and matching specified" do
      let(:title) { 'monit-monitor-both-pidfile-matching' }

      let(:params) {
        {
          'pidfile' => '/var/run/monit.pid',
          'matching' => 'some_process_name',
        }
      }
      it {
        expect { should raise_error(Puppet::Error) }
      }
    end

    context "both program_file and pidfile specified" do
      let(:title) { 'monit-monitor-both-program_file-pidfile' }

      let(:params) {
        {
          'program_file' => '/usr/local/bin/some_program',
          'pidfile'      => '/var/run/monit.pid',
        }
      }
      it {
        expect { should raise_error(Puppet::Error) }
      }
    end

    context "both program_file and matching specified" do
      let(:title) { 'monit-monitor-both-program_file-matching' }

      let(:params) {
        {
          'program_file' => '/usr/local/bin/some_program',
          'matching'     => 'some_process_name',
        }
      }
      it {
        expect { should raise_error(Puppet::Error) }
      }
    end

    context "no pidfile nor matching nor program_file specified" do
      let(:title) { 'monit-monitor-no-pidfile-matching-program_file' }

      let(:params) { }
      it {
        expect { should raise_error(Puppet::Error) }
      }
    end

    context "default usage (osfamily = RedHat)" do
      let(:title) { 'monit-monitor-basic' }

      let(:params) {
        {
          'pidfile' => '/var/run/monit.pid',
        }
      }

      it 'should compile' do
        should contain_file('/etc/monit.d/monit-monitor-basic.conf')
      end
    end

    context "default usage (osfamily = RedHat)" do
      let(:title) { 'monit-monitor-basic2' }

      let(:params) {
        {
          'matching' => 'some_process_name',
        }
      }

      it 'should compile' do
        should contain_file('/etc/monit.d/monit-monitor-basic2.conf')
      end
    end

    context "Custom group  (osfamily = RedHat)" do
      let(:title) { 'monit-monitor-group' }

      let(:params) {
        {
          'pidfile' => '/var/run/monit.pid',
          'group'   => 'somegroup',
        }
      }

      it 'should compile' do
        should contain_file('/etc/monit.d/monit-monitor-group.conf').with_content(
          "check process monit-monitor-group with pidfile /var/run/monit.pid\n" +
          "  start program = \"/etc/init.d/monit-monitor-group start\"\n" +
          "  stop program  = \"/etc/init.d/monit-monitor-group stop\"\n" +
          "  group somegroup\n"
        )
      end
    end

    context "Custom start/stop scripts  (osfamily = RedHat)" do
      let(:title) { 'monit-monitor-startstop' }

      let(:params) {
        {
          'pidfile'      => '/var/run/monit.pid',
          'start_script' => '/bin/start_my_app',
          'stop_script'  => '/bin/stop_my_app',
        }
      }

      it 'should compile' do
        should contain_file('/etc/monit.d/monit-monitor-startstop.conf').with_content(
          "check process monit-monitor-startstop with pidfile /var/run/monit.pid\n" +
          "  start program = \"/bin/start_my_app\"\n" +
          "  stop program  = \"/bin/stop_my_app\"\n" +
          "  group monit-monitor-startstop\n"
        )
      end
    end

    context "Custom start/stop timeouts  (osfamily = RedHat)" do
      let(:title) { 'monit-monitor-startstop' }

      let(:params) {
        {
          'pidfile'       => '/var/run/monit.pid',
          'start_script'  => '/bin/start_my_app',
          'start_timeout' => '32',
          'stop_script'   => '/bin/stop_my_app',
          'stop_timeout'  => '16',
        }
      }

      it 'should compile' do
        should contain_file('/etc/monit.d/monit-monitor-startstop.conf').with_content(
          "check process monit-monitor-startstop with pidfile /var/run/monit.pid\n" +
          "  start program = \"/bin/start_my_app\"\n" +
          "    with timeout 32 seconds\n" +
          "  stop program  = \"/bin/stop_my_app\"\n" +
          "    with timeout 16 seconds\n" +
          "  group monit-monitor-startstop\n"
        )
      end
    end
  end

  context "osfamily = Debian" do
    let :facts do
      {
        :osfamily        => 'Debian',
        :lsbdistid       => 'Ubuntu',
        :lsbdistrelease  => '12.10',
        :kernel          => 'Linux',
        :operatingsystem => 'Ubuntu',
      }
    end

    context "default usage (osfamily = Debian)" do
      let(:title) { 'monit-monitor-basic' }

      let(:params) {
        {
          'pidfile' => '/var/run/monit.pid',
        }
      }

      it 'should compile' do
        should contain_file('/etc/monit/conf.d/monit-monitor-basic.conf')
      end
    end

    context "Custom group  (osfamily = Debian)" do
      let(:title) { 'monit-monitor-group' }

      let(:params) {
        {
          'pidfile' => '/var/run/monit.pid',
          'group'   => 'somegroup',
        }
      }

      it 'should compile' do
        should contain_file('/etc/monit/conf.d/monit-monitor-group.conf').with_content(
          "check process monit-monitor-group with pidfile /var/run/monit.pid\n" +
          "  start program = \"/etc/init.d/monit-monitor-group start\"\n" +
          "  stop program  = \"/etc/init.d/monit-monitor-group stop\"\n" +
          "  group somegroup\n"
        )
      end
    end

    context "Custom start/stop scripts  (osfamily = Debian)" do
      let(:title) { 'monit-monitor-startstop' }

      let(:params) {
        {
          'pidfile'      => '/var/run/monit.pid',
          'start_script' => '/bin/start_my_app',
          'stop_script'  => '/bin/stop_my_app',
        }
      }

      it 'should compile' do
        should contain_file('/etc/monit/conf.d/monit-monitor-startstop.conf').with_content(
          "check process monit-monitor-startstop with pidfile /var/run/monit.pid\n" +
          "  start program = \"/bin/start_my_app\"\n" +
          "  stop program  = \"/bin/stop_my_app\"\n" +
          "  group monit-monitor-startstop\n"
        )
      end
    end

    context "Custom start/stop timeouts  (osfamily = Debian)" do
      let(:title) { 'monit-monitor-startstop' }

      let(:params) {
        {
          'pidfile'       => '/var/run/monit.pid',
          'start_script'  => '/bin/start_my_app',
          'start_timeout' => '64',
          'stop_script'   => '/bin/stop_my_app',
          'stop_timeout'  => '8',
        }
      }

      it 'should compile' do
        should contain_file('/etc/monit/conf.d/monit-monitor-startstop.conf').with_content(
          "check process monit-monitor-startstop with pidfile /var/run/monit.pid\n" +
          "  start program = \"/bin/start_my_app\"\n" +
          "    with timeout 64 seconds\n" +
          "  stop program  = \"/bin/stop_my_app\"\n" +
          "    with timeout 8 seconds\n" +
          "  group monit-monitor-startstop\n"
        )
      end
    end

    context "Custom user & group ID  (osfamily = Debian)" do
      let(:title) { 'monit-monitor-uidgid' }

      let(:params) {
        {
          'pidfile'       => '/var/run/monit.pid',
          'start_script'  => '/bin/start_my_app',
          'stop_script'   => '/bin/stop_my_app',
          'uid'           => 'www-data',
          'gid'           => 'users',
        }
      }

      it 'should compile' do
        should contain_file('/etc/monit/conf.d/monit-monitor-uidgid.conf').with_content(
          "check process monit-monitor-uidgid with pidfile /var/run/monit.pid\n" +
          "  start program = \"/bin/start_my_app\" as uid \"www-data\" and gid \"users\"\n" +
          "  stop program  = \"/bin/stop_my_app\"\n" +
          "  group monit-monitor-uidgid\n"
        )
      end
    end

  end

end
