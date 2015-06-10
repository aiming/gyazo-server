require 'spec_helper'

# For CentOS7
describe command('ss -lan | grep :443') do
  its(:exit_status) { should eq 0 }
end

describe command('echo | openssl s_client -connect localhost:443 -status') do
  its(:exit_status) { should eq 0 }
end
