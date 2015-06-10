require 'spec_helper'

## Doesn't use at source install ver.
#describe package('nginx') do
#  it { should be_installed }
#end

describe file('/opt/nginx/') do
  it { should be_directory }
  it { should be_mode 755 }
  it { should be_owned_by 'root' }
  it { should be_grouped_into 'root' }
end

describe service('nginx') do
  it { should be_enabled }
  it { should be_running }
end

# Doesn't use the CentOS7
#describe port('80') do
#  it { should be_listening }
#end

# For CentOS7
describe command('/sbin/ss -lan | grep :80') do
  its(:exit_status) { should eq 0 }
end

