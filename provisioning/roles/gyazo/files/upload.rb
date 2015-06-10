# encoding: utf-8
# -*- ruby -*-
#
# $Date$
# $Rev$
#
require 'cgi'
require 'digest/md5'
require 'sdbm'
require "rubygems"
require "rack"
require "pp"
require 'fileutils'

class Upload

    def call(env)
        req = Rack::Request.new(env)
        cgi = CGI.new("html3")
        params = req.params()

        res = Rack::Response.new

        id = params['id']

        imagedata = params["imagedata"][:tempfile].read

        $logger << imagedata.size
        $logger << id

        hash = Digest::MD5.hexdigest(imagedata)

        FileUtils.mkdir_p("data/#{hash[0].chr}/#{hash[1].chr}")
        File.open("data/#{hash[0].chr}/#{hash[1].chr}/#{hash}.png", 'w') do |io|
          io.write imagedata
        end

        res.write("https://example.com/data/#{hash[0].chr}/#{hash[1].chr}/#{hash}.png")
        res.finish
    end

end
