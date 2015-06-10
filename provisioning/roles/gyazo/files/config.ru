# encoding: utf-8

require "#{File.dirname(__FILE__)}/upload.rb"
require "logger"

$logger = File.open("#{File.dirname(__FILE__)}/log/debug.log", "a")

use Rack::ShowExceptions
use Rack::CommonLogger, $logger

use Rack::Static, :urls => ["/data"]

map '/upload' do
    run Upload.new
end


