require 'rake'
require 'rspec/core/rake_task'
require 'yaml'
require 'highline/import'

ENV['TARGET_HOST_YML'] = 'spec/env/servers.yml'

desc "Run serverspec to all hosts"
task :spec => 'serverspec:all'

namespace :serverspec do
  #ENV['TARGET_HOST_YML'] = ask("Enter Servers.yml(path): ") { |q| q.echo = true }
    properties = YAML.load_file(ENV['TARGET_HOST_YML'])
      task :all => properties.keys.map {|key| 'serverspec:' + key.split('.')[0] }
      properties.keys.each do |key|
        desc "Run serverspec to {key}"
        RSpec::Core::RakeTask.new(key.split('.')[0].to_sym) do |t|
        ENV['TARGET_HOST'] = key
        t.pattern = 'spec/{' + properties[key]['roles'].join(',') + '}/*_spec.rb'
      end
    end
end
