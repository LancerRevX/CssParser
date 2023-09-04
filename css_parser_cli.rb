require 'optparse'

require_relative 'css_parser'

$source_text = nil

option_parser = OptionParser.new do |options|
  options.banner = "Usage: #$PROGRAM_NAME [options]"
  options.on('-t TEXT', '--source-text') do |text|
    $source_text = text
  end
end
option_parser.parse!

if $source_text
    puts "Parsing the given text..."
    css_file = Css::File.parse($source_text)
    puts css_file
end