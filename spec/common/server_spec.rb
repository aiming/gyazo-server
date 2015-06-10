require 'spec_helper'

describe file('/usr/lib64') do
  it { should be_directory }
end

