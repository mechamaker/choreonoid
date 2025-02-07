#!/usr/bin/ruby

require 'pathname'

def make_pathname_from_include(orgPath)
  path = Pathname(orgPath)
  while true
    path = path.parent
    if path.basename.to_s == "include"
      return Pathname(orgPath).relative_path_from(path)
    end
    break if path.root?
  end
  return nil
end

n = ARGV.size - 1

if n <= 0
  puts "make_header_public.rb header-files ... directory-to-link-headers"
  exit
end

output_directory = File.expand_path(ARGV[n], Dir.pwd)
output_pathname = Pathname(output_directory)
source_top_directory = Pathname("/home/hayat/choreonoid")

for i in n.times
  orgFileName = ARGV[i]
  linkFilePath = (Pathname(output_directory) + Pathname(orgFileName).basename(".h")).to_s
  orgPathName = Pathname(File.expand_path(orgFileName, Dir.pwd))
  relpath = orgPathName.relative_path_from(source_top_directory)
  print "make public header <" + make_pathname_from_include(linkFilePath).to_s + ">\n"
  file = open(linkFilePath, "w")
  file.print "\#include \"#{relpath}\"\n"
end


