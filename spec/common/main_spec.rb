require 'spec_helper'

describe package('git') do
  it { should be_installed }
end

describe package('gcc') do
  it { should be_installed }
end

describe package('gcc-c++') do
  it { should be_installed }
end

describe package('zlib-devel') do
  it { should be_installed }
end

describe package('pcre-devel') do
  it { should be_installed }
end

describe package('pcre') do
  it { should be_installed }
end

describe package('epel-release') do
  it { should be_installed }
end

describe file('/etc/yum.repos.d/remi.repo') do
  it { should be_file }
end
