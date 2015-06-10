require 'spec_helper'

describe file('/var/www/gyazo/') do
  it { should be_directory }
  it { should be_mode 755 }
  it { should be_owned_by 'root' }
  it { should be_grouped_into 'root' }
end

['data', 'tmp', 'log', 'public'].each do |d|
  describe file("/var/www/gyazo/#{d}") do
    it { should be_directory }
    it { should be_mode 775 } if d == "data"
    it { should be_owned_by 'root' }
    it { should be_grouped_into 'nobody' }
  end
end

