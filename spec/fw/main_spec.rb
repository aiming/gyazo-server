require 'spec_helper'

describe service('firewalld') do
  it { should be_enabled }
  it { should be_running }
end

## CentOS 7 は /bin/sh: iptables: command not found になるので使えない
#describe iptables do
#  it { should have_rule('-p tcp -m tcp --dport 80').with_chain('IN_public_allow') }
#end

describe command('sudo firewall-cmd --list-services | grep http') do
  its(:exit_status) { should eq 0 }
end
