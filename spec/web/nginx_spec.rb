require 'spec_helper'

describe command('/opt/nginx/sbin/nginx -V') do
  let(:disable_sudo) { true }
  its(:stdout) { should match /--add-module=.*1\.9\.3-p551.*passenger-3\.0\.21\/ext\/nginx/ }
end

describe command('sudo /opt/nginx/sbin/nginx -t') do
  let(:disable_sudo) { true }
  its(:exit_status) { should eq 0 }
end
