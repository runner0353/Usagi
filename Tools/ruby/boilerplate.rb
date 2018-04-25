# Usagi Engine, Copyright © Vitei, Inc. 2013
#!ruby
#boilerplate.rb
#Generates ComponentManager.cpp boilerplate

require 'erb'
require 'zlib'
require 'yaml'
require 'tempfile'
require 'pathname'
require 'fileutils'

require 'optparse'

optparser = OptionParser.new do |opts|
  opts.banner = "Usage: #{$0} [options]"
  opts.on( '-y file', 'Specifies the boilerplate yml file to use' ) do |f|
    $yml = f
  end

  opts.on( '-o file', 'Specifies the filename to output' ) do |f|
    $out = f
  end

  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end
end

optparser.parse!

def canonical_path(p)
  Pathname.new(p).cleanpath.sub("_includes/","").sub("_build/proto/","")
end

input_file = ARGV[0]

data = YAML.load(File.read($yml))

SYSTEMS    = (data["Systems"]    || []).sort_by{ |s| s["priority"].to_i }

def component_id(c)
  Zlib.crc32(c.split("::").last)
end

TEMPLATE = input_file ? File.read(input_file) : STDIN.read
OUTPUT = ERB.new(TEMPLATE).result

out_stream = $stdout

if $out
  FileUtils.mkdir_p(File.dirname $out)
  out_stream = File.open($out, "wb")
end

out_stream.puts "//Don't edit this file!"
out_stream.puts "//This file is automatically generated based on #{input_file}"
out_stream.puts "//If you wish to edit it, that's the file you should edit"
out_stream.puts OUTPUT
out_stream.close if $out
